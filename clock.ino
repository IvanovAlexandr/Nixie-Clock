/*
    @file clock.ino
    @brief Firmware for Arduino in Nixie Clock
    @author Alexandr Ivanov (alexandr.ivanov.1995@gmail.com)
*/

#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <EEPROM.h>

#define LED_PIN_GREEN 9
#define LED_PIN_RED 11
#define LED_PIN_BLUE 10
#define DECODE_PIN_1 7
#define DECODE_PIN_2 8
#define DECODE_PIN_3 2
#define DECODE_PIN_4 3
#define LAMP_LOW_MINUTE_PIN 12
#define LAMP_HIGH_MINUTE_PIN 4
#define LAMP_LOW_HOUR_PIN 6
#define LAMP_HIGH_HOUR_PIN 5
#define PHOTOCELL_PIN A1


typedef struct color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;


const int t = 4;
const int LED_brightness_step = 40;
const int n_color = 135;

Color arr_color[n_color] = { 
            {255, 250, 250},
            {248, 248, 255},
            {245, 245, 245},
            {220, 220, 220},
            {255, 250, 240},
            {253, 245, 230},
            {250, 240, 230},
            {250, 235, 215},
            {255, 239, 213},
            {255, 235, 205},

            {255, 228, 196},
            {255, 218, 185},
            {255, 222, 173},
            {255, 228, 181},
            {255, 248, 220},
            {255, 255, 240},
            {255, 250, 205},
            {255, 245, 238},
            {240, 255, 240},
            {245, 255, 250},

            {240, 255, 255},
            {240, 248, 255},
            {230, 230, 250},
            {255, 240, 245},
            {255, 228, 225},
            {255, 255, 255},
            {0, 0, 0},
            {47, 79, 79},
            {105, 105, 105},
            {112, 128, 144},

            {119, 136, 153},
            {190, 190, 190},
            {211, 211, 211},
            {25, 25, 112},
            {0, 0, 128},
            {100, 149, 237},
            {72, 61, 139},
            {106, 90, 205},
            {123, 104, 238},
            {132, 112, 255},

            {0, 0, 205},
            {65, 105, 225},
            {0, 0, 255},
            {30, 144, 255},
            {0, 191, 255},
            {135, 206, 235},
            {135, 206, 250},
            {70, 130, 180},
            {176, 196, 222},
            {173, 216, 230},

            {176, 224, 230},
            {175, 238, 238},
            {0, 206, 209},
            {72, 209, 204},
            {64, 224, 208},
            {0, 255, 255},
            {224, 255, 255},
            {95, 158, 160},
            {102, 205, 170},
            {127, 255, 212},

            {0, 100, 0},
            {85, 107, 47},
            {143, 188, 143},
            {46, 139, 87},
            {60, 179, 113},
            {32, 178, 170},
            {152, 251, 152},
            {0, 255, 127},
            {124, 252, 0},
            {0, 255, 0},

            {127, 255, 0},
            {0, 250, 154},
            {173, 255, 47},
            {50, 205, 50},
            {154, 205, 50},
            {34, 139, 34},
            {107, 142, 35},
            {189, 183, 107},
            {238, 232, 170},
            {250, 250, 210},

            {255, 255, 224},
            {255, 255, 0},
            {255, 215, 0},
            {238, 221, 130},
            {218, 165, 32},
            {184, 134, 11},
            {188, 143, 143},
            {205, 92, 92},
            {139, 69, 19},
            {160, 82, 45},

            {205, 133, 63},
            {222, 184, 135},
            {245, 245, 220},
            {245, 222, 179},
            {244, 164, 96},
            {210, 180, 140},
            {210, 105, 30},
            {178, 34, 34},
            {165, 42, 42},
            {233, 150, 122},

            {250, 128, 114},
            {255, 160, 122},
            {255, 165, 0},
            {255, 140, 0},
            {255, 127, 80},
            {240, 128, 128},
            {255, 99, 71},
            {255, 69, 0},
            {255, 0, 0},
            {255, 105, 180},

            {255, 20, 147},
            {255, 192, 203},
            {255, 182, 193},
            {219, 112, 147},
            {176, 48, 96},
            {199, 21, 133},
            {208, 32, 144},
            {255, 0, 255},
            {238, 130, 238},
            {221, 160, 221},

            {218, 112, 214},
            {186, 85, 211},
            {153, 50, 204},
            {148, 0, 211},
            {138, 43, 226},
            {160, 32, 240},
            {147, 112, 219},
            {216, 191, 216},
            {0, 0, 139},
            {0, 139, 139},

            {139, 0, 139},
            {139, 0, 0},
            {0, 139, 0},
            {139, 139, 0},
            {144, 238, 144}

};

uint8_t m_hour[2], m_minute[2], m_day[2], m_month[2], m_temp[2];
uint8_t arr_time_delay[2], arr_date_delay[2], arr_temp_delay[2];
uint16_t photocellReading, prev_photocellReading;
uint64_t last_time, current_time;
int buf;
uint8_t which;
int i = 0;
int prev_temp = 0;

//from EEPROM----------------
uint8_t LEDbrightness; // addr = 7
Color user_color; //addr = 8, 9, 10
uint32_t time_delay; //addr = 4
uint32_t date_delay; // addr = 5
uint32_t temp_delay; // addr = 6

uint8_t mode;   // addr = 0
uint8_t index_color; //addr =  3
bool led_on; // addr = 1
bool brightness_auto; // addr = 2
//-------------------------------

uint8_t settings_state = 0;
uint8_t color_flag = 0;
uint8_t flag_clock = 0;
uint8_t prev_mode;
bool off;
bool set_color_mode = false;
bool state = 1;
bool flag_prev = false;
bool save_index_color_flag = false;
bool save_LED_brightness_flag = false;

bool fix_temp = true;
int add_temp = 0;
uint8_t cmp_temp = 1;

uint64_t time_save_index_color = 0;
uint64_t time_save_LEDbrightness = 0;

RtcDS3231 Rtc;
RtcDateTime now;


void read_eeprom() {
    mode = EEPROM.read(0);
    if (EEPROM.read(1) == 1)
        led_on = true;
    else led_on = false;
    if (EEPROM.read(2) == 1)
        brightness_auto = true;
    else brightness_auto = false;
    index_color = EEPROM.read(3);
    time_delay = EEPROM.read(4) * 1000;
    date_delay = EEPROM.read(5) * 1000;
    temp_delay = EEPROM.read(6) * 1000;
    LEDbrightness = EEPROM.read(7);
    user_color.red = EEPROM.read(8);
    user_color.green = EEPROM.read(9);
    user_color.blue = EEPROM.read(10);
}


void write_eeprom() {
    EEPROM.write(0, 3);
    EEPROM.write(1, 1);
    EEPROM.write(2, 1);
    EEPROM.write(3, index_color);
    EEPROM.write(4, 20);
    EEPROM.write(5, 5);
    EEPROM.write(6, 5);
    EEPROM.write(7, LEDbrightness);
    EEPROM.write(8, user_color.red);
    EEPROM.write(9, user_color.green);
    EEPROM.write(10, user_color.blue);
}


void setup() { 
    Serial.begin(9600);
    read_eeprom();
    //write_eeprom();
    Rtc.Begin();
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

    pinMode(DECODE_PIN_4, OUTPUT);
    pinMode(DECODE_PIN_3, OUTPUT);
    pinMode(DECODE_PIN_2, OUTPUT);
    pinMode(DECODE_PIN_1, OUTPUT);
    pinMode(LAMP_LOW_MINUTE_PIN, OUTPUT);
    pinMode(LAMP_HIGH_MINUTE_PIN, OUTPUT);
    pinMode(LAMP_LOW_HOUR_PIN, OUTPUT);
    pinMode(LAMP_HIGH_HOUR_PIN, OUTPUT);
    pinMode(LED_PIN_BLUE, OUTPUT);
    pinMode(LED_PIN_GREEN, OUTPUT);
    pinMode(LED_PIN_RED, OUTPUT);

    prev_photocellReading = 0;
    brightness();
    setMyTime();
    last_time = millis();
}


void loop() {

    check_uart();

    if (state == 1) {
        brightness();
        if (settings_state == 0) {
            if (!set_color_mode) {
                setMyTime();
                switch (mode) {
                    case 1: {
                                m_clock();
                                break;
                            }
                    case 2: {
                                time_and_temp();
                                break;
                            }
                    case 3: {
                                time_and_temp_and_date();
                                break;
                            }
                } 
            } else {
                switch(color_flag) {
                    case 0: {
                                off_lamp();
                                break;
                            }
                    case 1: {
                                show_color_setting(user_color.red);
                                break;
                            }
                    case 2: {
                                show_color_setting(user_color.green);
                                break;
                            }
                    case 3: {
                                show_color_setting(user_color.blue);
                                break;
                            }
                }
            }
        } else if (settings_state == 1) {
            set_time();
        } else if (settings_state == 2) {
            set_date();
        } else if (settings_state == 3) {
            set_time_delay();
        } else if (settings_state == 4) {
            set_date_delay();
        } else if (settings_state == 5) {
            set_temp_delay();
        }
        } else {
            if (!off) {
                off_lamp();
                off_led();
                off = true;
            }
        }

        if (save_index_color_flag) {
            if (millis() - time_save_index_color >= 10000) {
                save_index_color_flag = false;
                EEPROM.write(3, index_color);
                EEPROM.write(8, user_color.red);
                EEPROM.write(9, user_color.green);
                EEPROM.write(10, user_color.blue);
            }
        }
        if (save_LED_brightness_flag) {
            if (millis() - time_save_LEDbrightness >= 10000) {
                save_LED_brightness_flag = false;
                EEPROM.write(7, LEDbrightness);
            }
        }
}


void time_and_temp() {
    if (flag_clock == 0) {
        current_time =  millis();
            if (current_time > last_time) {
                if (current_time - last_time >= time_delay) {
                    flag_clock = 1;
                    last_time = millis();
                }
            } else
                last_time = millis();
    } else {
        current_time =  millis();
        if (current_time > last_time) {
            if (current_time - last_time >= temp_delay) {
                if (flag_prev) {
                    mode = prev_mode;
                    flag_prev = false;
                }
                flag_clock = 0;
                last_time = millis();
            }
        } else
            last_time = millis();
    }

    if (flag_clock == 0) {
        m_clock();
    } else {
        m_temperature();
    }
}


void time_and_temp_and_date() {
    if (flag_clock == 0) {
        current_time =  millis();
        if (current_time > last_time) {
            if (current_time - last_time >= time_delay) {
                flag_clock = 1;
                last_time = millis();
            }
        } else
            last_time = millis();
    } else if (flag_clock == 1) {
        current_time =  millis();
        if (current_time > last_time) {
            if (current_time - last_time >= temp_delay) {
                flag_clock = 2;
                last_time = millis();
            }
        } else
            last_time = millis();
    } else if (flag_clock == 2) {
        current_time =  millis();
        if (current_time > last_time) {
            if (current_time - last_time >= time_delay) {
                flag_clock = 3;
                last_time = millis();
            }
        } else
            last_time = millis();
    } else {
        current_time =  millis();
        if (current_time > last_time) {
            if (current_time - last_time >= date_delay) {
                if (flag_prev) {
                    mode = prev_mode;
                    flag_prev = false;
                }
                flag_clock = 0;
                last_time = millis();
            }
        } else
            last_time = millis();
    }

    if (flag_clock == 0 || flag_clock == 2) {
        m_clock();
    } else if (flag_clock == 1) {
        m_temperature();
    } else {
        m_date();
    }
}


void m_clock() {

    setDecoder(m_minute[0]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_minute[1]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_hour[0]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 1);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_hour[1]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
    delay(t);
}


void m_date() {

    setDecoder(m_month[0]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);
     
    setDecoder(m_month[1]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_day[0]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 1);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_day[1]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
    delay(t);
}


void m_temperature() {

    setDecoder(m_temp[0]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(m_temp[1]);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 1);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);
}


void show_color_setting(uint8_t x) {

    setDecoder(x % 10);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(x /10 % 10);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);

    setDecoder(x / 100);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 1);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
    delay(t);
}


void setDecoder(int x) { 

    if (x == 0) { 
        digitalWrite(DECODE_PIN_1, 1);
        digitalWrite(DECODE_PIN_2, 1);
        digitalWrite(DECODE_PIN_3, 1);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 1) {

        digitalWrite(DECODE_PIN_1, 0);
        digitalWrite(DECODE_PIN_2, 1);
        digitalWrite(DECODE_PIN_3, 1);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 2) {

        digitalWrite(DECODE_PIN_1, 1);
        digitalWrite(DECODE_PIN_2, 1);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 3) {

        digitalWrite(DECODE_PIN_1, 0);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 1);

        return;
    }

    if (x == 4) {

        digitalWrite(DECODE_PIN_1, 0);
        digitalWrite(DECODE_PIN_2, 1);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 5) {

        digitalWrite(DECODE_PIN_1, 1);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 1);

        return;
    }

    if (x == 6) {

        digitalWrite(DECODE_PIN_1, 0);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 7) {

        digitalWrite(DECODE_PIN_1, 1);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 0);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 8) {

        digitalWrite(DECODE_PIN_1, 1);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 1);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }

    if (x == 9) {

        digitalWrite(DECODE_PIN_1, 0);
        digitalWrite(DECODE_PIN_2, 0);
        digitalWrite(DECODE_PIN_3, 1);
        digitalWrite(DECODE_PIN_4, 0);

        return;
    }
}


void off_lamp() {
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
}


void off_led() {
    digitalWrite(LED_PIN_RED, 0);
    digitalWrite(LED_PIN_GREEN, 0);
    digitalWrite(LED_PIN_BLUE, 0);
}


void brightness() {
    if (brightness_auto) {
        photocellReading = analogRead(PHOTOCELL_PIN);

        if (abs(prev_photocellReading - photocellReading) > LED_brightness_step) {
            if (photocellReading < 160)
                photocellReading = 160;
            if (photocellReading > 650)
                photocellReading = 650;

            LEDbrightness = map(photocellReading, 150, 650, 0, 255);
        }
    }

    if (led_on) {
        int red = map(LEDbrightness, 0, 255, 0, user_color.red);
        analogWrite(LED_PIN_RED, red);
        int green = map(LEDbrightness, 0, 255, 0, user_color.green);
        analogWrite(LED_PIN_GREEN, green);
        int blue = map(LEDbrightness, 0, 255, 0, user_color.blue);
        analogWrite(LED_PIN_BLUE, blue);
        prev_photocellReading = photocellReading;
    }
}


void setMyTime() {
    now = Rtc.GetDateTime();
    m_hour[0] = now.Hour() % 10;
    m_hour[1] = now.Hour() / 10;
    m_minute[0] = now.Minute() % 10;
    m_minute[1] = now.Minute() / 10;
    m_day[0] = now.Day() % 10;
    m_day[1] = now.Day() / 10;
    m_month[0] = now.Month() % 10;
    m_month[1] = now.Month() / 10;
    int temp = (int) Rtc.GetTemperature().AsFloat();
    if (fix_temp) {
        switch (cmp_temp) {
            case 1 :{
                        if (millis() > 345000) {
                            add_temp = -1;
                            cmp_temp = 2;
                        }
                        break;
                    }
            case 2 :{
                        if (millis() > 601000) {
                            add_temp = -2;
                            cmp_temp = 3;
                        }
                        break;
                    }
            case 3 :{
                        if (millis() > 793000) {
                            add_temp = -3;
                            cmp_temp = 4;
                        }
                        break;
                    }
            case 4 :{
                        if (millis() > 1049000) {
                            add_temp = -4;
                            cmp_temp = 5;
                        }
                         break;
                    }
            case 5 : {
                        if (millis() > 1369000) {
                            add_temp = -5;
                            cmp_temp = 6;
                        }
                        break;
                    }
            case 6 :{
                        if (millis() > 1817000) {
                            add_temp = -6;
                            cmp_temp = 7;
                        }
                        break;
                    }
            case 7 :{
                        if (millis() > 2713000) {
                            add_temp = -7;
                            cmp_temp = 0;
                            fix_temp = false;
                        }
                        break;
                    }
                
        } 
    }

    temp += add_temp;
    m_temp[0] = temp % 10;
    m_temp[1] = temp / 10;
}


void check_uart() {

    if (Serial.available()) {
        int inByte = Serial.read();
        //Serial.println(inByte);
        check(inByte);
    }
}


void check(int x) {
    if (x == 0x1b) {
        buf = 0;
        return;
    }
    if (x == 0xD) {
        buf = 1;
        return;
    }
    if (x == 0xF) {
        buf = 2;
        return;
    }
    if (x == 0x11) {
        buf = 3;
        return;
    }
    if (x == 0x15) {
        buf = 4;
        return;
    }
    if (x == 0x16) {
        buf = 5;
        return;
    }
    if (x == 0x17) {
        buf = 6;
        return;
    }
    if (x == 0x18) {
        buf = 7;
        return;
    }
    if (x == 0x19) {
        buf = 8;
        return;
    }
    if (x == 0x1A) {
        buf = 9;
        return;
    }

    if (x == 0x0) {
        state = !state;
        off = false;
        return;
    }
    if (x == 0x1) {
        led_on = !led_on;
        if (!led_on) {
            off_led();
        }
        EEPROM.write(1, led_on);
        return;
    }
    if (x == 0x6) {
        prev_mode = mode;
        mode = 3;
        flag_clock = 3;
        flag_prev = true;
        return;
    }
    if (x == 0x8) {
        prev_mode = mode;
        mode = 2;
        flag_clock = 1;
        flag_prev = true;
        return;
    }
    if (x == 0x1C) {
        brightness_auto = true;
        EEPROM.write(2, brightness_auto);
        return;
    }
    if (x == 0x2) {
        if (brightness_auto) {
            brightness_auto = false;
            EEPROM.write(2, brightness_auto);
        }
        
        if (LEDbrightness > 0) {
            if (LEDbrightness > 0 + 10)
                LEDbrightness -= 10;
            else
                LEDbrightness = 0;
        }
        save_LED_brightness_flag = true;
        time_save_LEDbrightness = millis();

        return;
    }
    if (x == 0x3) {
        if (brightness_auto) {
            brightness_auto = false;
            EEPROM.write(2, brightness_auto);
        }
        if (LEDbrightness < 255 )
        {
            if (LEDbrightness < 255 - 10)
                LEDbrightness += 10;
            else 
                LEDbrightness = 255;
        }
        save_LED_brightness_flag = true;
        time_save_LEDbrightness = millis();
        return;
    }
    if (x == 0x1D) {
        if (index_color > 0)
            index_color--;
        else
            index_color = n_color-1;
        user_color = arr_color[index_color];
        save_index_color_flag = true;
        time_save_index_color = millis();
        return;
    }
    if (x == 0x1E) {
        if (index_color < n_color)
            index_color++;
        else
            index_color = 0;
        user_color = arr_color[index_color];
        save_index_color_flag = true;
        time_save_index_color = millis();
        return;
    }
    if (x == 0x4) {
        set_color_mode = !set_color_mode;
        color_flag = 0;
        if (!set_color_mode) {
            EEPROM.write(8, user_color.red);
            EEPROM.write(9, user_color.green);
            EEPROM.write(10, user_color.blue);
        }
        return;
    }
    if (x == 0xB) {
        color_flag = 3;
        if (user_color.blue > 0)
            user_color.blue--;
        return;
    }
    if (x == 0xC) {
        color_flag = 3;
        if (user_color.blue < 255)
            user_color.blue++;
        return;
    }
    if (x == 0x5) {
        color_flag = 1;
        if (user_color.red > 0)
            user_color.red--;
        return;
    }
    if (x == 0x7) {
        color_flag = 1;
        if (user_color.red < 255)
            user_color.red++;
        return;
    }
    if (x == 0x9) {
        color_flag = 2;
        if (user_color.green > 0)
            user_color.green--;
        return;
    }
    if (x == 0xA) {
        color_flag = 2;
        if (user_color.green < 255)
            user_color.green++;
        return;
    }
    if (x == 0xE) {
        mode = 1;
        EEPROM.write(0, mode);
        return;
    }
    if (x == 0x10) {
        flag_clock = 0;
        mode = 2;
        EEPROM.write(0, mode);
        return;
    }
    if (x == 0x12) {
        flag_clock = 0;
        mode = 3;
        EEPROM.write(0, mode);
        return;
    }
    if (x == 0x28) {
        which--;
        if (which == 0) {
            which = 4;
        }
        return;
    }
    if (x == 0x27) {
        which++;
        if (which == 5) {
            which = 1;
        }
        return;
    }
    if (x == 0x13) {
        settings_state = 1;
        which = 4;
        buf = -1;
        i = 0;
        return;
    }
    if (x == 0x22) {
        save_time();
        settings_state = 2;
        which = 4;
        buf = -1;
        i = 0;
        return;
    }
    if (x == 0x23) {
        save_date();
        settings_state = 3;
        which = 2;
        buf = -1;
        i = 0;
        arr_time_delay[0] = time_delay / 1000 % 10;
        arr_time_delay[1] = time_delay / 10000;
        return;
    }
    if (x == 0x24) {
        save_time_delay();
        settings_state = 4;
        which = 2;
        buf = -1;
        i = 0;
        arr_date_delay[0] = date_delay / 1000 % 10;
        arr_date_delay[1] = date_delay / 10000;
        return;
    }
    if (x == 0x25) {
        save_date_delay();
        settings_state = 5;
        which = 2;
        buf = -1;
        i = 0;
        arr_temp_delay[0] = temp_delay / 1000 % 10;
        arr_temp_delay[1] = temp_delay / 10000;
        return;
    }
    if (x == 0x26) {
        save_temp_delay();
        settings_state = 0;
        return;
    }
}


void set_time() {

    if (which == 1) {
        if (buf != -1) {
            m_minute[0] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_minute[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_minute[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 2) {
        if (buf != -1) {
            m_minute[1] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_minute[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_minute[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 3) {
        if (buf != -1) {
            m_hour[0] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_hour[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 1);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_hour[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 1);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 4) {
        if (buf != -1) {
            m_hour[1] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_hour[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    }  else {
        setDecoder(m_hour[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
        delay(t);
    }
}


void set_date() {

    if (which == 1) {
        if (buf != -1) {
            m_day[0] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_month[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_month[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 2) {
        if (buf != -1) {
            m_day[1] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_month[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_month[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 3) {
        if (buf != -1) {
            m_month[0] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_day[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 1);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(m_day[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 1);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    if (which == 4) {
        if (buf != -1) {
            m_month[1] = buf;
            buf = -1;
        }
        if (i < 30) {
            setDecoder(m_day[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
            delay(t);
            i++;
        } else if (i < 60) {
            i++;
        } else
            i = 0;
    }  else {
        setDecoder(m_day[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
        delay(t);
    }
}


void set_time_delay() {
    if (which == 1 || which == 3) {
        if (buf != -1) {
            arr_time_delay[0] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_time_delay[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(arr_time_delay[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);   
        delay(t);
    }

    if (which == 2 || which == 4) {
        if (buf != -1) {
            arr_time_delay[1] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_time_delay[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
     } else {
        setDecoder(arr_time_delay[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    setDecoder(1);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
    delay(t);
}


void set_date_delay() {
    if (which == 1 || which == 3) {
        if (buf != -1) {
            arr_date_delay[0] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_date_delay[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(arr_date_delay[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);   
        delay(t);
    }

    if (which == 2 || which == 4) {
        if (buf != -1) {
            arr_date_delay[1] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_date_delay[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(arr_date_delay[1]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
        delay(t);
    }

    setDecoder(2);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
    delay(t);
}


void set_temp_delay() {
    if (which == 1 || which == 3) {
        if (buf != -1) {
            arr_temp_delay[0] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_temp_delay[0]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
    } else {
        setDecoder(arr_temp_delay[0]);
        digitalWrite(LAMP_LOW_MINUTE_PIN, 1);
        digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
        digitalWrite(LAMP_LOW_HOUR_PIN, 0);
        digitalWrite(LAMP_HIGH_HOUR_PIN, 0);   
        delay(t);
    }

    if (which == 2 || which == 4) {
        if (buf != -1) {
            arr_temp_delay[1] = buf;
            buf = -1;
        }
        if (i < 60) {
            setDecoder(arr_temp_delay[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
            i++;
        } else if (i < 120) {
            i++;
        } else
            i = 0;
        } else {
            setDecoder(arr_temp_delay[1]);
            digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
            digitalWrite(LAMP_HIGH_MINUTE_PIN, 1);
            digitalWrite(LAMP_LOW_HOUR_PIN, 0);
            digitalWrite(LAMP_HIGH_HOUR_PIN, 0);
            delay(t);
        }

    setDecoder(3);
    digitalWrite(LAMP_LOW_MINUTE_PIN, 0);
    digitalWrite(LAMP_HIGH_MINUTE_PIN, 0);
    digitalWrite(LAMP_LOW_HOUR_PIN, 0);
    digitalWrite(LAMP_HIGH_HOUR_PIN, 1);
    delay(t);
}


void save_time() {
    now = Rtc.GetDateTime();
    Rtc.SetDateTime(RtcDateTime(now.Year(), now.Month(), now.Day(), (uint8_t) (m_hour[1] * 10 + m_hour[0]),
                    (uint8_t) (m_minute[1] * 10 + m_minute[0]), now.Second()));
}


void save_date() {
     now = Rtc.GetDateTime();
     Rtc.SetDateTime(RtcDateTime(now.Year(), (m_month[1] * 10 + m_month[0]), (m_day[1] * 10 + m_day[0]),
                     now.Hour(), now.Minute(), now.Second()));
}


void save_time_delay() {
    time_delay = arr_time_delay[1] *10000 + arr_time_delay[0] * 1000;
    EEPROM.write(4, time_delay/1000);
}


void save_date_delay() {
    date_delay = arr_date_delay[1] *10000 + arr_date_delay[0] * 1000;
    EEPROM.write(5, date_delay/1000);
}

void save_temp_delay() {
    temp_delay = arr_temp_delay[1] * 10000 + arr_temp_delay[0] * 1000;
    EEPROM.write(6, temp_delay/1000);
}
