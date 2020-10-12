#include <Arduino.h>

static int speaker_pin = D1;
static int straightKey_pin = D5;

static int ditPaddle_pin = D5;
static int dahPaddle_pin = D6;

static bool straightKey_enabled = false;

static int cw_freq = 500;

static int wpm = 17;
static int wpm_farnsworth = wpm;
static int dah_to_dit_ratio = 3;

static int volume = 5;

typedef enum {
  CWS_Idle = 0,
  CWS_manual_active,
  CWS_dit_active,
  CWS_dit_rest,
  CWS_dah_active,
  CWS_dah_rest
} CWState;
static CWState state = CWS_Idle;
static unsigned long lastStateTime = 0;

typedef struct {
  unsigned long dit_length;
  unsigned long dit_rest;
  unsigned long dah_length;
  unsigned long dah_rest;
} WPMConfig;
static WPMConfig wpmconfig = {0};

// ---------
// Boring Function Declarations

void calcWPMConfig();

CWState handle_straight_key(CWState curState);
CWState handle_paddles(CWState curState, CWState lastState);

void playTone(int channel, int frequency);
void endTone(int channel);

// ---------
// Setup

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);

  pinMode(speaker_pin, OUTPUT);
  analogWrite(speaker_pin, 0);

  pinMode(straightKey_pin, INPUT_PULLUP);
  pinMode(ditPaddle_pin, INPUT_PULLUP);
  pinMode(dahPaddle_pin, INPUT_PULLUP);

  calcWPMConfig();

  Serial.begin(115200);
  Serial.print("Starting loop");
}

void calcWPMConfig() {
  // https://morsecode.world/international/timing.html
  unsigned long ms_per_dit = 60000/(50 * wpm);
  wpmconfig.dit_length = ms_per_dit;
  wpmconfig.dit_rest = ms_per_dit;
  wpmconfig.dah_length = dah_to_dit_ratio * ms_per_dit;
  wpmconfig.dah_rest = ms_per_dit;

  Serial.print("Dit length is ");
  Serial.println(ms_per_dit);
}

// ---------
// Main Loop
void loop() {
  CWState lastState = state;
  CWState new_state = state;

  // Check if our current state has been happening for too long
  unsigned long ms = millis();
  switch(state) {
    case CWS_Idle:
    case CWS_manual_active:
      break;
    
    case CWS_dit_active:
      if ((ms - lastStateTime) >= wpmconfig.dit_length) {
        Serial.println("Moving to dit rest state");
        new_state = CWS_dit_rest;
      }
      break;
    
    case CWS_dit_rest:
      if ((ms - lastStateTime) >= wpmconfig.dit_rest) {
        Serial.println("Moving to idle state from dit state");
        new_state = CWS_Idle;
      }
      break;

    case CWS_dah_active:
      if ((ms - lastStateTime) >= wpmconfig.dah_length) {
        Serial.println("Moving to dah rest state");
        new_state = CWS_dah_rest;
      }
      break;
    
    case CWS_dah_rest:
      if ((ms - lastStateTime) >= wpmconfig.dah_rest) {
        Serial.println("Moving to idle state from dah rest");
        new_state = CWS_Idle;
      }
      break;

  }

  if (straightKey_enabled == true) {
    new_state = handle_straight_key(new_state);
  } else {
    new_state = handle_paddles(new_state, lastState);
  }

  // Act on the new state
  if (new_state != state) {
    switch (new_state) {
      case CWS_manual_active:
      case CWS_dit_active:
      case CWS_dah_active:
        Serial.println("Playing tone");
        playTone(speaker_pin, cw_freq);
        digitalWrite(LED_BUILTIN, 0);
        break;

      case CWS_dit_rest:
      case CWS_dah_rest:
      case CWS_Idle:
        Serial.println("Ending tone");
        endTone(speaker_pin);
        digitalWrite(LED_BUILTIN, 1);
        break;
    }

    lastStateTime = millis();
    state = new_state;
  }
}

// ---------
// Straight Key

CWState handle_straight_key(CWState curState) {
  if (straightKey_enabled == false) return curState;
  
  if (digitalRead(straightKey_pin) == 0) {
    return CWS_manual_active;
  } 
  return CWS_Idle;
}

// ---------
// Paddles


CWState handle_paddles(CWState curState, CWState lastState) {
  if (straightKey_enabled == true) return curState;
  
  CWState nextState = curState;
  if (digitalRead(ditPaddle_pin) == 0) {
    if (curState == CWS_Idle) {
      Serial.println("dit paddle is active");
      nextState = CWS_dit_active;
    } else {
      // TODO: Enqueue the paddle press
    }
  }   
  if (digitalRead(dahPaddle_pin) == 0) {
    if (curState == CWS_Idle) {
      nextState = CWS_dah_active;
    } else {
      // TODO: Enqueue the paddle press
    }
  } 
  return nextState;
}

// ---------
// Tone Playing 
// (the ESP8266 Arduino library doesn't implement tone())

void playTone(int pin, int frequency) {
  analogWriteFreq(frequency);
  analogWrite(pin, volume);
}

void endTone(int pin) {
  analogWrite(pin, 0);
}