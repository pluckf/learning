#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "pwm.h"
#include "math.h"
#include "count.h"
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))//现在目标值的大小
#define PI 3.1415926

float ua,ub,uc;
float pwm_ua,pwm_ub,pwm_uc;
float ualpha,ubeta;
float voltage_supply=10.8;
float zero_elec_angle=0;
float shaft_angle=0;
float ts;
float ts_mid[5]={0};
u8 ts_time=0;
void set_voltage(float ua ,float ub, float uc);
float strain_angel(float angle);
float get_elec_angle(float shaft_angle, int pole_pairs);
void set_phase_voltage(float uq,float ud, float elec_angle);
float open_velocity(float target_velocity);
int main(void)
{
	OLED_Init();
	pwm_init();
	//OLED_ShowString(4,1,"pwm_ok");
	count_init();
	while (1)
	{	
		restart_count();
		open_velocity(10);
		if (ts_time<5)
		{
			ts_mid[ts_time]=get_count();
			ts_time++;
		}
		else
		{
			ts_time=0;
			ts_mid[ts_time]=get_count();
			ts_time++;
		}
		ts=(ts_mid[0]+ts_mid[1]+ts_mid[2]+ts_mid[3]+ts_mid[4])/5*1e-6f;//计算一次while循环的时间
		//OLED_ShowNum(2,1,ts*1e6f,8);
		//OLED_ShowNum(1,1,shaft_angle,4);
		
	}
}


void set_phase_voltage(float uq,float ud, float elec_angle)
{
	elec_angle= strain_angel(elec_angle + zero_elec_angle);
	//帕克逆变换，将电角度转化到ualpha ，ubeta坐标系
	ualpha =  -uq*sin(elec_angle); 
	ubeta =   uq*cos(elec_angle); 
	//克拉克逆变换//求出三相电压
	ua = ualpha + voltage_supply/3;
	ub = (sqrt(3)*ubeta-ualpha)/2 +  voltage_supply/3;
	uc = (-ualpha-sqrt(3)*ubeta)/2 + voltage_supply/3;
	//将电压等比缩小用pwm输出信号
	pwm_ua=constrain(ua/voltage_supply,0.0f,1.0f);
	pwm_ub=constrain(ub/voltage_supply,0.0f,1.0f);
	pwm_uc=constrain(uc/voltage_supply,0.0f,1.0f);
	set_voltage(pwm_ua,pwm_ub,pwm_uc);
	
		
	
	
}
float get_elec_angle(float shaft_angle, int pole_pairs)//得到电角度
{

	return (shaft_angle*pole_pairs);//机械角度*极对数

}

float strain_angel(float angle)//角度限幅（角度归一化）
{
	float a = fmod(angle, 2*PI);
	return a >= 0 ? a : (a + 2*PI);  
}


void set_voltage(float ua ,float ub, float uc)//将电压值转化为pwm的ccr值
{	
//	u16 compare1,compare2,compare3;
//	compare1=ua/0.033;
//	compare2=ub/0.033;
//	compare3=uc/0.033;
	
	set_pwm(ua*255,ub*255,uc*255);
//	OLED_ShowNum(1,1,ua*255,4);
//	OLED_ShowNum(2,1,ub*255,4);
//	OLED_ShowNum(3,1,uc*255,4);
	
}
float open_velocity(float target_velocity)
{
	//求出机械角度
	shaft_angle = strain_angel(shaft_angle + target_velocity*ts);
	//shaft_angle=shaft_angle + target_velocity*ts;
	float uq = voltage_supply/3;
	//根据uq值设置ua，ub，uc
	set_phase_voltage(uq,0,get_elec_angle(shaft_angle,6));//这里我的电机极对数是6
	return uq;
}
