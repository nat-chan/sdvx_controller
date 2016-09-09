#include<Keyboard.h>

//{{{
const int VOL_L_pin[] = {A0, A1};
const int VOL_R_pin[] = {A2, A3};
const int BT_A_pin = 2;
const int BT_A_led = 3;
const int BT_B_pin = 4;
const int BT_B_led = 5;
const int BT_C_pin = 7;
const int BT_C_led = 6;
const int BT_D_pin = 8;
const int BT_D_led = 9;
const int FX_L_pin = 12;
const int FX_L_led = 10;
const int FX_R_pin = 13;
const int FX_R_led = 11;
//}}}

//つまみを止めた時に逆回転が入力されたり、
//本体の振動でキーが入力される時はこの値を増やす
const int chattering = 5;
//この値を増やすと低速回転でも入力が途切れなくなる
const int sensitive = 400;

class VOL{
	private:
		char key[2];//{正回転で送る文字, 逆回転で送る文字}
		int pin[2];//{A相に繋ぐピン, B相に繋ぐピン}
		int phase[2];//{現在の位相, 前回の位相}
		int count[4];
		int state;

		int get_phase(){
			//正回転時に0,1,2,3,0,1,2...
			//逆回転時に3,2,1,0,3,2,1...と出力
			int A = digitalRead(pin[0]);
			int B = digitalRead(pin[1]);
			return 2*B+A^B;
		}

		int get_delta(){
			phase[0] = get_phase();
			int delta = (phase[0] - phase[1])%4;
			phase[1] = phase[0];
			return delta;
		}

	public:
		VOL(char keyPos, char keyNeg, int pinA, int pinB){
			pinMode(pinA, INPUT);
			pinMode(pinB, INPUT);
			key[0] = keyPos;
			key[1] = keyNeg;
			pin[0] = pinA;
			pin[1] = pinB;
		}
		void judge();
};

void VOL::judge(){
	int delta = get_delta();
	switch(delta){
		case 0:
			count[0]++;
			break;
		case 1:
			count[0] = 0;
			count[1]++;
			break;
		case 3:
			count[0] = 0;
			count[3]++;
			break;
	}
	if(count[0] == sensitive){
		if(state == 1){
			Keyboard.release(key[0]);
		}else if(state == 3){
			Keyboard.release(key[1]);
		}
		state = 0;
		count[1] = 0;
		count[3] = 0;
	}else if(count[1] == chattering){
		if(state == 3){
			Keyboard.release(key[1]);
		}
		Keyboard.press(key[0]);
		state = 1;
		count[3] = 0;
	}else if(count[3] == chattering){
		if(state == 1){
			Keyboard.release(key[0]);
		}
		Keyboard.press(key[1]);
		state = 3;
		count[1] = 0;
	}
	//Serial.println(state);
}

class BT{
	private:
		char key;//ボタンが押さた時送る文字
		int pin[2];//{スイッチに繋ぐピン, ledに繋ぐピン}
		int count[2];
		bool state;
		bool is_pushed(){
			return !digitalRead(pin[0]);
		}

	public:
		BT(char key_, int pinSW, int pinLED){
			pinMode(pinSW, INPUT_PULLUP);
			pinMode(pinLED, OUTPUT);
			key = key_;
			pin[0] = pinSW;
			pin[1] = pinLED;
		}
		void judge();
		void set_led(int level){
			analogWrite(pin[1], level);
		}
};

void BT::judge(){
	switch(is_pushed()){
		case 0:
			count[0]++;
			count[1] = 0;
			break;
		case 1:
			count[1]++;
			count[0] = 0;
			break;
	}
	if(count[0] == chattering){
		Keyboard.release(key);
		analogWrite(pin[1], 0);
		state = 0;
	}else if(count[1] == chattering){
		Keyboard.press(key);
		analogWrite(pin[1], 255);
		state = 1;
	}
	//Serial.println(state);
};

VOL VOL_L('w', 'q', VOL_L_pin[0], VOL_L_pin[1]);
VOL VOL_R('p', 'o', VOL_R_pin[0], VOL_R_pin[1]);

BT BT_A('h', BT_A_pin, BT_A_led);
BT BT_B('j', BT_B_pin, BT_B_led);
BT BT_C('k', BT_C_pin, BT_C_led);
BT BT_D('l', BT_D_pin, BT_D_led);
BT FX_L('n', FX_L_pin, FX_L_led);
BT FX_R('m', FX_R_pin, FX_R_led);

void setup() {
	Serial.begin(9600);
	Keyboard.begin();
	//起動時イルミネーション
	float pi = 3.1415926;
	for(float i=0;i<2*pi*5;i+=0.003){
		BT_A.set_led(int(64*(sin(i)+1)));
		BT_B.set_led(int(64*(sin(i+pi/5)+1)));
		BT_C.set_led(int(64*(sin(i+2*pi/5)+1)));
		BT_D.set_led(int(64*(sin(i+3*pi/5)+1)));
		FX_L.set_led(int(64*(sin(i+pi/10)+1)));
		FX_R.set_led(int(64*(sin(i+pi/2)+1)));
	}
	BT_A.set_led(0);
	BT_B.set_led(0);
	BT_C.set_led(0);
	BT_D.set_led(0);
	FX_L.set_led(0);
	FX_R.set_led(0);
}

void loop(){
	int s = micros();
	VOL_L.judge();
	VOL_R.judge();
	BT_A.judge();
	BT_B.judge();
	BT_C.judge();
	BT_D.judge();
	FX_L.judge();
	FX_R.judge();
	int e = micros();
	//Serial.println((e-s)/4);
}


