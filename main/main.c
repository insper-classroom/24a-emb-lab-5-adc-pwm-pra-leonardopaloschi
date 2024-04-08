/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

#define ADC_CHANNELY 0
#define ADC_CHANNELX 1
#define deadzone 150

const int ADC_PINY = 26;
const int ADC_PINX = 27;


typedef struct adc {
    int axis;
    int val;
} adc_t;


void uart_task(void *p) {
    adc_t data;

    while (1) {
        if(xQueueReceive(xQueueAdc, &data, portMAX_DELAY)){
            int value = data.val/100;
            int MSB = value >> 8;
            int LSB = value & 0xFF;

            uart_putc_raw(uart0, data.axis);
            uart_putc_raw(uart0, MSB);
            uart_putc_raw(uart0, LSB);
            uart_putc_raw(uart0, -1);
        }

    }
}

void x_task(void *p){
    adc_t data;
    adc_init();
    adc_gpio_init(ADC_PINX);
    while(true){
        adc_select_input(ADC_CHANNELX);
        int result = adc_read();
        result = result -2048;
        result = result/8;
        if(abs(result) < deadzone){
            result = 0;
        }
        else{
            data.val = result;
            data.axis = 1;
            xQueueSend(xQueueAdc, &data, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void y_task(void *p){
    adc_t data;
    adc_init();
    adc_gpio_init(ADC_PINY);
    while(true){
        adc_select_input(ADC_CHANNELY);
        int result = adc_read();
        result = result -2048;
        result = result/8;
        if(abs(result) < deadzone){
            result = 0;
        }
        else{
            data.val = result;
            data.axis = 0;
            xQueueSend(xQueueAdc, &data, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }   
}
    int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "x_task", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
