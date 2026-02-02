#include <stdio.h>
#include "pico/stdlib.h"

const int STEP_FREQ = 500;

// Channel pin definitions
typedef struct {
    int up_control_pin;
    int down_control_pin;
    int dir_fast_pin;
    int dir_slow_pin;
    int en_fast_pin;
    int en_slow_pin;
} ChannelPins;

// Channel 1 pins
const ChannelPins channel1_pins = {
    .up_control_pin = 6,
    .down_control_pin = 7,
    .dir_fast_pin = 22,
    .dir_slow_pin = 14,
    .en_fast_pin = 28,
    .en_slow_pin = 17
};

// Channel 2 pins
const ChannelPins channel2_pins = {
    .up_control_pin = 8,
    .down_control_pin = 9,
    .dir_fast_pin = 21,
    .dir_slow_pin = 16,
    .en_fast_pin = 26,
    .en_slow_pin = 18
};

// Channel 3 pins
const ChannelPins channel3_pins = {
    .up_control_pin = 12,
    .down_control_pin = 13,
    .dir_fast_pin = 20,
    .dir_slow_pin = 15,
    .en_fast_pin = 27,
    .en_slow_pin = 19
};

const ChannelPins* all_channels[] = {&channel1_pins, &channel2_pins, &channel3_pins};
const int num_channels = sizeof(all_channels) / sizeof(all_channels[0]);


typedef enum {
    IDLE,
    SLOW_UP,
    SLOW_DOWN,
    FAST_UP,
    FAST_DOWN
} ChannelState;

ChannelState state[] = {IDLE, IDLE, IDLE};



void initialize_gpio() {
    for (int i = 0; i < num_channels; i++) {
        const ChannelPins* channel = all_channels[i];

        // Configure control pins as inputs with pull-ups
        gpio_init(channel->up_control_pin);
        gpio_set_dir(channel->up_control_pin, GPIO_IN);
        gpio_pull_up(channel->up_control_pin);

        gpio_init(channel->down_control_pin);
        gpio_set_dir(channel->down_control_pin, GPIO_IN);
        gpio_pull_up(channel->down_control_pin);

        // Configure dir and en pins as outputs, initially low
        const int output_pins[] = {
            channel->dir_fast_pin, channel->dir_slow_pin,
            channel->en_fast_pin, channel->en_slow_pin
        };
        for (int j = 0; j < sizeof(output_pins) / sizeof(output_pins[0]); j++) {
            uint pin = output_pins[j];
            gpio_set_dir(pin, GPIO_OUT);
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, 0);
            state[i] = IDLE;
        }
    }
}

const int SLOW_STATE_RATIO = 4 ; // relative duration of the slow rise/fall phase
const int DELAY_US = 1000000L / (STEP_FREQ * num_channels * (4+SLOW_STATE_RATIO) );


int main()
{
    stdio_init_all();

    initialize_gpio();

    int cycles_in_slow_state = 0; 

    while (true) {
        for(int ch = 0; ch < num_channels; ch++) {

            bool going_up = (gpio_get(all_channels[ch]->up_control_pin) == 0);
            bool going_down = !going_up && (gpio_get(all_channels[ch]->down_control_pin) == 0);
            if(going_up) {
                switch(state[ch]) {
                    case IDLE:
                        gpio_put(all_channels[ch]->en_slow_pin, false);
                        state[ch] = FAST_UP;
                        gpio_put(all_channels[ch]->dir_fast_pin, true);
                        gpio_put(all_channels[ch]->en_fast_pin, true);
                        break;
                    case FAST_UP:
                    case SLOW_UP: // this case is not part of the standard scenario, but might happen upon quick reversal
                        gpio_put(all_channels[ch]->en_fast_pin, false);
                        gpio_put(all_channels[ch]->dir_fast_pin, false);
                        state[ch] = SLOW_DOWN;
                        cycles_in_slow_state = 0;
                        gpio_put(all_channels[ch]->dir_slow_pin, false);
                        gpio_put(all_channels[ch]->en_slow_pin, true);
                        break;
                    case SLOW_DOWN:
                        cycles_in_slow_state++;
                        if(cycles_in_slow_state >= SLOW_STATE_RATIO)
                        {
                            gpio_put(all_channels[ch]->en_slow_pin, false);
                            state[ch] = FAST_DOWN;
                            gpio_put(all_channels[ch]->dir_fast_pin, false);
                            gpio_put(all_channels[ch]->en_fast_pin, true);
                        }
                        break;
                    case FAST_DOWN:
                        gpio_put(all_channels[ch]->en_fast_pin, false);
                        state[ch] = IDLE;
                        gpio_put(all_channels[ch]->dir_slow_pin, false);
                        gpio_put(all_channels[ch]->en_slow_pin, true);
                        break;                    
                }
            }

            //-------
            if(going_down) {
                switch(state[ch]) {
                    case IDLE:
                    case SLOW_DOWN: // this case is not expected in typical scenario, but might occur upon quick direction reversal
                        state[ch] = SLOW_UP;
                        cycles_in_slow_state = 0;
                        gpio_put(all_channels[ch]->dir_slow_pin, true);
                        gpio_put(all_channels[ch]->en_slow_pin, true);
                        break;
                    case SLOW_UP:
                        cycles_in_slow_state++;
                        if(cycles_in_slow_state >= SLOW_STATE_RATIO) {
                            gpio_put(all_channels[ch]->en_slow_pin, false);
                            state[ch] = FAST_UP;
                            gpio_put(all_channels[ch]->dir_fast_pin, true);
                            gpio_put(all_channels[ch]->en_fast_pin, true);
                        }
                        break;
                    case FAST_UP:
                        state[ch] = FAST_DOWN;
                        gpio_put(all_channels[ch]->dir_fast_pin, false);
                        gpio_put(all_channels[ch]->en_fast_pin, true);
                        break;
                    case FAST_DOWN:
                        gpio_put(all_channels[ch]->en_fast_pin, false);
                        state[ch] = IDLE;
                        gpio_put(all_channels[ch]->dir_slow_pin, false);
                        gpio_put(all_channels[ch]->en_slow_pin, true);
                        break;    
                }
            }

            sleep_us(DELAY_US);
        }
    }
}

