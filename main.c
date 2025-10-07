#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/gpio.h"
#include "boards.h"
#include "ice_cram.h"
#include "ice_fpga.h"
#include "ice_led.h"
#include "tusb.h"
#include "ice_HAL.h"
#include <stdio.h>
#include <string.h>

#define DONE "DONEDONEDONEDONE"
#define GPIO_PIN 20 //fpga pin 27

uint8_t command[200000] = {0};
int command_index = 0;

int pulse_count = 0;

void count(uint gpio, uint32_t e) {
    pulse_count++;
}

void process_command() {
    ice_led_init();
    ice_fpga_init(FPGA_DATA, 48);
    ice_fpga_start(FPGA_DATA);

    ice_cram_open(FPGA_DATA);
    ice_cram_write(command, command_index);
    ice_cram_close();
    printf("uploaded\r\n");
    command_index = 0;


    pulse_count = 0;
    int time  = to_ms_since_boot(get_absolute_time());
    time += 10000;

    while (to_ms_since_boot(get_absolute_time()) < time) {
        tud_task();
    }

    printf("pulses: %d\r\n", pulse_count);
}


void next_char() {
    int c;

    while ((c = getchar_timeout_us(10000)) == PICO_ERROR_TIMEOUT) {
        tud_task();
    }
    command[command_index] = c;
    command_index++;

    if (command_index < strlen(DONE)) {
        return;
    }

    if (memcmp(command + command_index - strlen(DONE), DONE, strlen(DONE))) {
            return;
    }

    command_index = command_index - strlen(DONE);
    process_command();
}

int main(void) {
    tusb_init();
    stdio_init_all();

    gpio_init(GPIO_PIN);
    gpio_disable_pulls(GPIO_PIN);
    gpio_put(GPIO_PIN, false);
    gpio_set_dir(GPIO_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(GPIO_PIN, GPIO_IRQ_EDGE_RISE, true, &count);


    while (1) {
        next_char();
    }
    return 0;
}
