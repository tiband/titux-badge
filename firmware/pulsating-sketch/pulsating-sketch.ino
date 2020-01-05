const uint16_t PWM_MAX_VALUE = 128;
const uint16_t PWM_CYCLE = 30; // us

// digitalWrite is too slow…
inline void set_pin(uint8_t pin) {
  if(pin < 8) {
    PORTA |= 1 << pin;
  } else {
    PORTB |= 1 << (10 - pin);
  }
}
inline void reset_pin(uint8_t pin) {
  if(pin < 8) {
    PORTA &= ~(1 << pin);
  } else {
    PORTB &= ~(1 << (10 - pin));
  }
}

struct PwmPin {
  uint8_t location;
  uint16_t value;
  uint16_t tick;
};

enum Leds {
  LED_LEFT_R,
  LED_LEFT_B,
  LED_RIGHT_R,
  LED_RIGHT_B,
  LED_TOP_R,
  LED_TOP_B,
};

PwmPin pins[] = {
  { 0, 20, 0},
  { 1, 20, 0},
  { 2, 10, 0},
  { 3, 10, 0},
  {10, 5, 0},
  { 9, 5, 0},
};

unsigned long last_micros = 0;
void pwm_tick() {
  if(micros() - last_micros > PWM_CYCLE) {
    for(auto& pin : pins) {
      if(pin.tick < pin.value) {
        set_pin(pin.location);
      } else {
        reset_pin(pin.location);
      }

      pin.tick = (pin.tick + 1) % PWM_MAX_VALUE;
    }

    last_micros = micros();
  }
}

void setup() {
  for(auto& pin : pins) {
    pinMode(pin.location, OUTPUT);
  }
}


struct FadeStep {
  uint16_t target;
  uint16_t step_width;
};

FadeStep pulsating_pattern_top[] = {
   {21, 1},
   {2, 1},
   {0, 0},
};
FadeStep pulsating_pattern_left[] = {
   {35, 1},
   {6, 1},
   {0, 0},
};
FadeStep pulsating_pattern_right[] = {
   {40, 1},
   {9, 1},
   {0, 0},
};

struct PatternState {
  uint8_t pin;
  FadeStep* pattern_start;
  FadeStep* pattern_current;
};

PatternState patterns[] = {
  {LED_TOP_R, pulsating_pattern_top, pulsating_pattern_top},
  {LED_TOP_B, pulsating_pattern_top, pulsating_pattern_top},
  {LED_LEFT_R, pulsating_pattern_left, pulsating_pattern_left},
  {LED_LEFT_B, pulsating_pattern_left, pulsating_pattern_left},
  {LED_RIGHT_R, pulsating_pattern_right, pulsating_pattern_right},
  {LED_RIGHT_B, pulsating_pattern_right, pulsating_pattern_right},
};

unsigned long last_millis = 0;
void loop() {
  pwm_tick();

  if(millis() - last_millis > 100) {
    for(auto& pattern : patterns) {
      if(pins[pattern.pin].value > pattern.pattern_current->target) {
        pins[pattern.pin].value = max(pins[pattern.pin].value - pattern.pattern_current->step_width, pattern.pattern_current->target);
      } else if(pins[pattern.pin].value < pattern.pattern_current->target) {
        pins[pattern.pin].value = min(pins[pattern.pin].value + pattern.pattern_current->step_width, pattern.pattern_current->target);
      }

      if(pins[pattern.pin].value == pattern.pattern_current->target) {
        pattern.pattern_current = pattern.pattern_current + 1;
        if(pattern.pattern_current->step_width == 0) {
          pattern.pattern_current = pattern.pattern_start;
        }
      }
    }

    last_millis = millis();
  }
}
