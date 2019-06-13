/* Touch Pad Read Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#if CONFIG_IDF_TARGET_ESP32
#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_FILTER_MODE_EN  (1)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
/*
  Read values sensed at all available touch pads.
 Print out values in a loop on a serial monitor.
 */
static void tp_example_read_task(void *pvParameter)
{
    uint16_t touch_value;
    uint16_t touch_filter_value;
#if TOUCH_FILTER_MODE_EN
    printf("Touch Sensor filter mode read, the output format is: \nTouchpad num:[raw data, filtered data]\n\n");
#else
    printf("Touch Sensor normal mode read, the output format is: \nTouchpad num:[raw data]\n\n");
#endif
    while (1) {
        for (int i = 0; i < TOUCH_PAD_MAX; i++) {
#if TOUCH_FILTER_MODE_EN
            // If open the filter mode, please use this API to get the touch pad count.
            touch_pad_read_raw_data(i, &touch_value);
            touch_pad_read_filtered(i, &touch_filter_value);
            printf("T%d:[%4d,%4d] ", i, touch_value, touch_filter_value);
#else
            touch_pad_read(i, &touch_value);
            printf("T%d:[%4d] ", i, touch_value);
#endif
        }
        printf("\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static void tp_example_touch_pad_init()
{
    for (int i = 0;i< TOUCH_PAD_MAX;i++) {
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

void app_main()
{
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    touch_pad_init();
    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    tp_example_touch_pad_init();
#if TOUCH_FILTER_MODE_EN
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
#endif
    // Start task to read values sensed by pads
    xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
#elif CONFIG_IDF_TARGET_ESP32S2BETA

#define TOUCH_BUTTON_NUM    14

static const char * TAG = "touch read";
static const touch_pad_t button[TOUCH_BUTTON_NUM] = {
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM2,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM4, 
    TOUCH_PAD_NUM5,
    TOUCH_PAD_NUM6,
    TOUCH_PAD_NUM7,
    TOUCH_PAD_NUM8,
    TOUCH_PAD_NUM9,
    TOUCH_PAD_NUM10,
    TOUCH_PAD_NUM11,
    TOUCH_PAD_NUM12,
    TOUCH_PAD_NUM13,
    TOUCH_PAD_NUM14
};

/*
  Read values sensed at all available touch pads.
 Print out values in a loop on a serial monitor.
 */
static void tp_example_read_task(void *pvParameter)
{
    uint16_t touch_value;
    uint16_t touch_filter_value;
    
    /* Wait touch sensor init done */
    vTaskDelay(100 / portTICK_RATE_MS);
    printf("Touch Sensor read, the output format is: \nTouchpad num:[raw data]\n\n");

    while (1) {
        for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
            touch_pad_read_raw(button[i], &touch_value);    // read raw data.
            printf("T%d: [%4d] ", button[i], touch_value);
        }
        printf("\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    /* Initialize touch pad peripheral. */
    touch_pad_init();
    for (int i = 0; i < TOUCH_BUTTON_NUM; i++) {
        touch_pad_config(button[i]);
        touch_pad_set_thresh(button[i], TOUCH_PAD_THRESHOLD_MAX);
    }

    /* Denoise setting at TouchSensor 0. */
    touch_pad_denoise_t denoise = {
            /* The bits to be cancelled are determined according to the noise level. */
            .grade = TOUCH_PAD_DENOISE_BIT4,    
            .cap_level = TOUCH_PAD_DENOISE_CAP_L7,
    };
    touch_pad_denoise_set_config(denoise);
    touch_pad_denoise_enable();
    ESP_LOGI(TAG, "Denoise function init");

    /* Enable touch sensor clock. Work mode is "timer trigger". */
    touch_pad_fsm_start(TOUCH_FSM_MODE_TIMER);

    /* Start task to read values by pads. */
    xTaskCreate(&tp_example_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);
}
#endif