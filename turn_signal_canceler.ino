#define LEFT_INPUT 2
#define RIGHT_INPUT 4

#define LEFT_OUTPUT 1
#define RIGHT_OUTPUT 0

#define CURRENT_INPUT 3 // adc3

#define TIMEOUT_SECONDS 30


/*#include <BasicSerial.h>

void serOut(const char* str)
{
   while (*str) TxByte (*str++);
}*/

void setup() {

    pinMode(LEFT_OUTPUT, OUTPUT);
    pinMode(RIGHT_OUTPUT, OUTPUT);

    pinMode(LEFT_INPUT, INPUT);
    pinMode(RIGHT_INPUT, INPUT);
    pinMode(CURRENT_INPUT, INPUT);

    analogReference(INTERNAL);
}

double REF_VOLTAGE = 1.053251064;

#define ADC_ONE_SIDE 356
#define ADC_TWO_SIDES 713

#define STATE_OFF 0
#define STATE_LEFT_TURN 1
#define STATE_RIGHT_TURN 2
#define STATE_HAZARD 3

bool last_left_input_state = 0;
bool last_right_input_state = 0;

uint8_t state = 0;

unsigned long last_State_Change = 0;

bool flashing_state = 0;

unsigned long last_Output_Change = 0;

unsigned long last_Bulb_Check = 0;

#define BULB_BURNED_OUT 100

uint16_t flash_delay = 400;

void loop() {   

    unsigned long now = millis();

    uint16_t adc_reading = analogRead(CURRENT_INPUT);

    //float vin = (REF_VOLTAGE * (float)adc_reading) / 1024.0;

    // to find adc_reading from voltage, 
    //
    //                         voltage_input * 1024
    //          ADC_Int =  ----------------------------
    //                           reference_voltage
    //


    // input detection
    bool left_input_state = !digitalRead(LEFT_INPUT);
    bool right_input_state = !digitalRead(RIGHT_INPUT);

    if (last_left_input_state != left_input_state || last_right_input_state != right_input_state)
    {
        if (state == STATE_OFF)
        {
            if (left_input_state && right_input_state)
                setState(STATE_HAZARD, now);
            else if (left_input_state)
                setState(STATE_LEFT_TURN, now);
            else if (right_input_state)
                setState(STATE_RIGHT_TURN, now);
        }
        else if (state == STATE_LEFT_TURN)
        {
            if (left_input_state && right_input_state)
                setState(STATE_HAZARD, now);
            else if (left_input_state)
                setState(STATE_OFF, now);
            else if (right_input_state)
                setState(STATE_RIGHT_TURN, now);
        }
        else if (state == STATE_RIGHT_TURN)
        {
            if (left_input_state && right_input_state)
                setState(STATE_HAZARD, now);
            else if (left_input_state)
                setState(STATE_LEFT_TURN, now);
            else if (right_input_state)
                setState(STATE_OFF, now);
        }
        else if (state == STATE_HAZARD)
        {
            if (left_input_state && right_input_state)
                setState(STATE_OFF, now);
        }
    }




    last_left_input_state = left_input_state;
    last_right_input_state = right_input_state;




    // low current detection (burned out bulb)
    if (state != STATE_OFF && flashing_state && flash_delay != BULB_BURNED_OUT && now - last_Bulb_Check > 2*1000)
    {
        if (now - last_Output_Change >= flash_delay/2 && now - last_Output_Change <= (flash_delay/2)+50)
        {
            uint16_t expected_value = 0;

            if (state == STATE_LEFT_TURN || state == STATE_RIGHT_TURN)
                expected_value = ADC_ONE_SIDE;
            else if (state == STATE_HAZARD)
                expected_value = ADC_TWO_SIDES;

            if (adc_reading < expected_value)
                flash_delay = BULB_BURNED_OUT;
        }
    }

    // output modifications

    if (now - last_Output_Change > flash_delay)
    {
        // timeout detection
        if ((state == STATE_LEFT_TURN || state == STATE_RIGHT_TURN) && now - last_State_Change > TIMEOUT_SECONDS*1000)
            setState(STATE_OFF, now);

        if (state == STATE_OFF)
        {
            digitalWrite(LEFT_OUTPUT, LOW);
            digitalWrite(RIGHT_OUTPUT, LOW);
        }
        else if (state == STATE_LEFT_TURN)
        {
            flashing_state = !flashing_state;

            digitalWrite(LEFT_OUTPUT, flashing_state);
            digitalWrite(RIGHT_OUTPUT, LOW);
        }
        else if (state == STATE_RIGHT_TURN)
        {
            flashing_state = !flashing_state;

            digitalWrite(LEFT_OUTPUT, LOW);
            digitalWrite(RIGHT_OUTPUT, flashing_state);
        }
        else if (state == STATE_HAZARD)
        {
            flashing_state = !flashing_state;
            digitalWrite(LEFT_OUTPUT, flashing_state);
            digitalWrite(RIGHT_OUTPUT, flashing_state);
        }

        last_Output_Change = now;
    }

    delay(10);
}

void setState(uint8_t _new_state, unsigned long now) {
    if (now - last_State_Change > 200 || _new_state == STATE_HAZARD)
    {
        state = _new_state;
        flashing_state = 0;
        last_Output_Change = 0;

        last_State_Change = now;
    }
}

/*void D(uint16_t out)
{
    char textToWrite[ 8 ];
    itoa(out, textToWrite, 10);

    D(textToWrite);
}

void D(char* out)
{
    serOut(out);
}*/
