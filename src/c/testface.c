#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_weekday_layer;
static TextLayer *s_date_layer;

static Layer *s_battery_layer;

static int s_battery_level;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void battery_callback(BatteryChargeState state) {
    // Record the new battery level
    s_battery_level = state.charge_percent;

    // Update meter
    layer_mark_dirty(s_battery_layer);
}

static void update_time() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                            "%H:%M" : "%I:%M", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void update_weekday(){
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[4];
    strftime(s_buffer, sizeof(s_buffer), "%a", tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_weekday_layer, s_buffer);
}

// static void update_battery(){
//     // Buffer for battery percentage + null terminator
//     char s_bat[4];

//     // Convert battery percentage to string
//     snprintf(s_bat, sizeof(s_bat), "%u", battery_state_service_peek().charge_percent);
//     strcat(s_bat, "%");

//     // Display this time on the TextLayer
//     text_layer_set_text(s_bat_layer, s_bat);
// }

static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // Find the width of the bar (total width = 144px)
    // int width = (s_battery_level * 144) / 100;
    int width = (s_battery_level * 108) / 100;

    // Draw the background
    // graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_context_set_fill_color(ctx, GColorFromRGB(86, 86, 86));
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // Draw the bar
    // graphics_context_set_fill_color(ctx, GColorFromRGB(86, 86, 181));
    // graphics_context_set_fill_color(ctx, GColorFromRGB(86, 86, 86));
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0, 0, width, 5), 0, GCornerNone);
}

static void update_date() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current date into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), "%d/%m", tick_time);

    text_layer_set_text(s_date_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    update_date();
    update_weekday();
    // update_battery();
}

bool is_night_time() {
    // Get the current time
    time_t now;
    struct tm *local;
    
    time(&now);           // Get the current time
    local = localtime(&now); // Convert to local time format

    int hour = local->tm_hour;

    // Return true if the time is between 18:00 (6 PM) and 07:59 (7:59 AM)
    if (hour >= 18 || hour < 8) {
        return true;
    } else {
        return false;
    }
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create GBitmap
    s_background_bitmap = is_night_time() ? 
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_NIGHT) :
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_DAY);

    // Create BitmapLayer to display the GBitmap
    s_background_layer = bitmap_layer_create(bounds);

    // Set the bitmap onto the layer and add to the window
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

    // Improve the layout to be more like a watchface

    // Create the TextLayer with specific bounds
    s_time_layer = text_layer_create(
        // GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
        GRect(0, 0, bounds.size.w, 50));
        
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    
    s_weekday_layer = text_layer_create(
    // GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
    GRect(17, 55, bounds.size.w / 2, 25));
    text_layer_set_background_color(s_weekday_layer, GColorClear);
    text_layer_set_text_color(s_weekday_layer, GColorWhite);
    text_layer_set_font(s_weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_weekday_layer));

    s_date_layer = text_layer_create(
    // GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
    GRect(bounds.size.w / 2, 55, (bounds.size.w / 2) - 17, 25));
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorWhite);
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Create battery meter Layer
    // s_battery_layer = layer_create(GRect(0, 166, 144, 5));
    s_battery_layer = layer_create(GRect(17, 50, 111, 5));
    layer_set_update_proc(s_battery_layer, battery_update_proc);

    // Add to Window
    layer_add_child(window_get_root_layer(window), s_battery_layer);
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_weekday_layer);
    // text_layer_destroy(s_bat_layer);

    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);

    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);
}

static void init() {
    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    window_stack_push(s_main_window, true);

    window_set_background_color(s_main_window, GColorBlack);

    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);

    // Ensure battery level is displayed from the start
    battery_callback(battery_state_service_peek());

    // Load the font and assign it to the font variable
    //s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
    //s_time_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);

    // Make sure the time is displayed from the start
    update_time();
    update_date();
    update_weekday();
}

static void deinit() {
    window_destroy(s_main_window);
    layer_destroy(s_battery_layer);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

