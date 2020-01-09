#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
/* Functions like strcpy are technically not secure because they do */
/* not contain a 'length'. But we disable this warning for the VISA */
/* examples since we never copy more than the actual buffer size.   */
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "visa.h"

static ViSession defaultRM;
static ViSession instr;
static ViUInt32 numInstrs;
static ViFindList findList;
static ViUInt32 retCount;
static ViUInt32 writeCount;
static ViStatus status;
static char instrResourceString[VI_FIND_BUFLEN];

static unsigned char buffer[100];
static char stringinput[512];

static char enter_local[10] = "SYST:LOC";
static char enter_remote[10] = "SYST:REM";
static char output_on[10] = "OUTP ON";
static char output_off[10] = "OUTP OFF";

int main(void)
{
	int i,j,k;
	char in_command[512],tail[5];
	bool flag = true;

	status = viOpenDefaultRM(&defaultRM);
	if (status < VI_SUCCESS)
	{
		printf("无法打开与VISA资源管理器的会话！\n");
		exit(EXIT_FAILURE);
	}

	status = viFindRsrc(defaultRM, "USB?*INSTR", &findList, &numInstrs, instrResourceString);

	if (status < VI_SUCCESS)
	{
		printf("查找资源时发生错误。\n按Enter继续。");
		fflush(stdin);
		getchar();
		viClose(defaultRM);
		return status;
	}

	for (i = 0; i<numInstrs; i++)
	{
		if (i > 0)
			viFindNext(findList, instrResourceString);

		status = viOpen(defaultRM, instrResourceString, VI_NULL, VI_NULL, &instr);

		if (status < VI_SUCCESS)
		{
			printf("无法打开与设备的会话 %d.\n", i + 1);
			continue;
		}
		status = viWrite(instr, (ViBuf)enter_local, (ViUInt32)strlen(enter_local), &writeCount);//由于初始链接仪器会进入远程模式，需设其为本地模式
		if (status < VI_SUCCESS)
		{
			printf("写入设备时出错 %d.\n", i + 1);
			status = viClose(instr);
			break;
		}
		printf("输入EXIT可退出\n\n");

		while (true)
		{
			//char plus_remote_command[600] = ":SYST:REM";
			//printf("%s\n", plus_remote_command);
			bool in_flag = true;

			printf("请输入命令：");
			gets_s(in_command);
			if (strcmp(in_command,"EXIT") == 0)				//制作循环手动退出选项，输入大写EXIT可退出输入命令模式
				break;
			for (k = 5; k <= 8; k++) {						//获取输入字符串关键字
				tail[k - 5] = in_command[k];
				if (k == 8)
					tail[k - 5] = '\0';
			}
			if (strcmp(tail, "OFF") == 0) {					//判断是否输入电源输出关闭命令，对电源实现手动输入命令关闭输出的功能
				//printf("%s\n", header);
				in_flag = false;
			}
			status = viWrite(instr, (ViBuf)enter_remote, (ViUInt32)strlen(enter_remote), &writeCount);	//进入远程控制模式，此时前面板按钮不可用
			if (status < VI_SUCCESS)
			{
				printf("写入设备时出错 %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}
			if (in_flag) {
				status = viWrite(instr, (ViBuf)output_off, (ViUInt32)strlen(output_off), &writeCount);		//自动关闭电源的输出
				if (status < VI_SUCCESS)
				{
					printf("写入设备时出错 %d.\n", i + 1);
					status = viClose(instr);
					flag = false;
					break;
				}
			}
			printf("电源输出关闭\n");
			strcat(in_command, "\n");
			//printf("命令最后一位为:%c\n", in_command[j - 1]);
			
			//strcat(plus_remote_command, in_command);
			//printf("%s\n", plus_remote_command);
			strcpy(stringinput, in_command);
			//strcpy(stringinput, plus_remote_command);
			//printf("%s\n", stringinput);
			status = viWrite(instr, (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);	//写入需执行的命令
			if (status < VI_SUCCESS)
			{
				printf("写入设备时出错 %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}
			if (in_flag) {
				printf("命令写入仪器，电源输出打开中...\n");
				Sleep(100);																					//设置电源输出打开延时，让操作者知道输出关闭又再打开
				status = viWrite(instr, (ViBuf)output_on, (ViUInt32)strlen(output_on), &writeCount);		//自动打开电源的输出
				if (status < VI_SUCCESS)
				{
					printf("写入设备时出错 %d.\n", i + 1);
					status = viClose(instr);
					flag = false;
					break;
				}
			}

			status = viWrite(instr, (ViBuf)enter_local, (ViUInt32)strlen(enter_local), &writeCount);	//进入仪器本地模式，可控制前面板按钮
			if (status < VI_SUCCESS)
			{
				printf("写入设备时出错 %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}

			/*if (in_command[j-1] == '?')
			{
				status = viRead(instr, buffer, 100, &retCount);
				if (status < VI_SUCCESS)
				{
					printf("从设备读取响应时出错 %d.\n", i + 1);
				}
				else
				{
					printf("\ndevice %d: %*s\n", i + 1, retCount, buffer);
				}
			}*/

		}

		if (!flag)
		{
			continue;
		}
		
		status = viClose(instr);
	}


	status = viClose(defaultRM);
	printf("按Enter继续。");
	fflush(stdin);
	getchar();

	return 0;
}
