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
		printf("�޷�����VISA��Դ�������ĻỰ��\n");
		exit(EXIT_FAILURE);
	}

	status = viFindRsrc(defaultRM, "USB?*INSTR", &findList, &numInstrs, instrResourceString);

	if (status < VI_SUCCESS)
	{
		printf("������Դʱ��������\n��Enter������");
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
			printf("�޷������豸�ĻỰ %d.\n", i + 1);
			continue;
		}
		status = viWrite(instr, (ViBuf)enter_local, (ViUInt32)strlen(enter_local), &writeCount);//���ڳ�ʼ�������������Զ��ģʽ��������Ϊ����ģʽ
		if (status < VI_SUCCESS)
		{
			printf("д���豸ʱ���� %d.\n", i + 1);
			status = viClose(instr);
			break;
		}
		printf("����EXIT���˳�\n\n");

		while (true)
		{
			//char plus_remote_command[600] = ":SYST:REM";
			//printf("%s\n", plus_remote_command);
			bool in_flag = true;

			printf("���������");
			gets_s(in_command);
			if (strcmp(in_command,"EXIT") == 0)				//����ѭ���ֶ��˳�ѡ������дEXIT���˳���������ģʽ
				break;
			for (k = 5; k <= 8; k++) {						//��ȡ�����ַ����ؼ���
				tail[k - 5] = in_command[k];
				if (k == 8)
					tail[k - 5] = '\0';
			}
			if (strcmp(tail, "OFF") == 0) {					//�ж��Ƿ������Դ����ر�����Ե�Դʵ���ֶ���������ر�����Ĺ���
				//printf("%s\n", header);
				in_flag = false;
			}
			status = viWrite(instr, (ViBuf)enter_remote, (ViUInt32)strlen(enter_remote), &writeCount);	//����Զ�̿���ģʽ����ʱǰ��尴ť������
			if (status < VI_SUCCESS)
			{
				printf("д���豸ʱ���� %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}
			if (in_flag) {
				status = viWrite(instr, (ViBuf)output_off, (ViUInt32)strlen(output_off), &writeCount);		//�Զ��رյ�Դ�����
				if (status < VI_SUCCESS)
				{
					printf("д���豸ʱ���� %d.\n", i + 1);
					status = viClose(instr);
					flag = false;
					break;
				}
			}
			printf("��Դ����ر�\n");
			strcat(in_command, "\n");
			//printf("�������һλΪ:%c\n", in_command[j - 1]);
			
			//strcat(plus_remote_command, in_command);
			//printf("%s\n", plus_remote_command);
			strcpy(stringinput, in_command);
			//strcpy(stringinput, plus_remote_command);
			//printf("%s\n", stringinput);
			status = viWrite(instr, (ViBuf)stringinput, (ViUInt32)strlen(stringinput), &writeCount);	//д����ִ�е�����
			if (status < VI_SUCCESS)
			{
				printf("д���豸ʱ���� %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}
			if (in_flag) {
				printf("����д����������Դ�������...\n");
				Sleep(100);																					//���õ�Դ�������ʱ���ò�����֪������ر����ٴ�
				status = viWrite(instr, (ViBuf)output_on, (ViUInt32)strlen(output_on), &writeCount);		//�Զ��򿪵�Դ�����
				if (status < VI_SUCCESS)
				{
					printf("д���豸ʱ���� %d.\n", i + 1);
					status = viClose(instr);
					flag = false;
					break;
				}
			}

			status = viWrite(instr, (ViBuf)enter_local, (ViUInt32)strlen(enter_local), &writeCount);	//������������ģʽ���ɿ���ǰ��尴ť
			if (status < VI_SUCCESS)
			{
				printf("д���豸ʱ���� %d.\n", i + 1);
				status = viClose(instr);
				flag = false;
				break;
			}

			/*if (in_command[j-1] == '?')
			{
				status = viRead(instr, buffer, 100, &retCount);
				if (status < VI_SUCCESS)
				{
					printf("���豸��ȡ��Ӧʱ���� %d.\n", i + 1);
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
	printf("��Enter������");
	fflush(stdin);
	getchar();

	return 0;
}
