// ImageProcess1.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <opencv2\opencv.hpp>
#include <opencv2\core\core.hpp>  
#include <opencv2\highgui\highgui.hpp>  
#include <opencv2\imgproc.hpp>
#include <string>
#include <stdlib.h>
#include <Kinect.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include "math.h"
#include <algorithm>


#include<cstring>
#pragma comment(lib, "ws2_32.lib")


#define pi 3.1415926
#define finalHeight 150
#define finalWidth 225
#define initialXValue 130
#define initialYValue 150
#define circleNumOfAll 50

#define tolorenceValue 36  //允许总的误差值


using namespace cv;
using namespace std;

template<class Interface>

inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}


int server(string InSendData)
{
	

	//初始化WSA  
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//创建套接字  
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//绑定IP和端口  
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(5050);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (::bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//开始监听  
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}
	//循环接收数据  
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[255];
	int loop = 1;

	


	while (true)
	{
		printf("等待连接...\n");
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			continue;
		}
		//printf("接受到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));
		//InetPton(AF_INET, _T("172.20.10.2"), &remoteAddr.sin_addr.s_addr);
		//接收数据  
		printf("接受到一个连接");
		int ret = recv(sClient, revData, 255, 0);
		if (ret > 0)
		{
			revData[ret] = 0x00;
			printf(revData);
		}

		//发送数据  
		/*if (loop < 4)
		{
			if (loop == 1) { const char * sendData = "fuckyou"; send(sClient, sendData, strlen(sendData), 0); }
			if (loop == 2) { const char * sendData = "fuckyou2"; send(sClient, sendData, strlen(sendData), 0); }

			if (loop == 3) { const char * sendData = "fuckyou3"; send(sClient, sendData, strlen(sendData), 0); }
		}*/
		const char * sendData = InSendData.c_str();
		send(sClient, sendData, strlen(sendData), 0);
	}
	closesocket(sClient);
	closesocket(slisten);
	WSACleanup();
	return 0;
}



Mat realDepthMapWAF(finalHeight, finalWidth, CV_16UC1,Scalar(0)); //加权平均后的矩阵,实际拍摄
//Mat backgroundDepthMapWAF(finalHeight, finalWidth, CV_16UC1, Scalar(0)); //背景矩阵


//捕获特定区域图像,找出棱所在的位置并分割平面
void get_split_depthImage
(double &x1,
 double &y1, 
double &z1,
double &x2,
double &y2,
double &z2,
double &x3,
double &y3,
double &z3,
double &x4,
double &y4,
double &z4,
double &x5,
double &y5,
double &z5,
double &x6,
double &y6,
double &z6,
double &centerX,
double &centerY,
double &centerZ,
int numOfCircle)
{
	Mat realDepthMap(finalHeight, finalWidth, CV_16UC1); //实际拍得的矩阵
	//Mat realDepthMap32(finalHeight, finalWidth, CV_32SC1);

	//realDepthMap.convertTo(realDepthMap32, CV_32SC1, 1.0 / 255);
	//Weighted average filtering


	cout << "识别程序里的循环次数:" << numOfCircle << endl;

	if (numOfCircle < circleNumOfAll - 1)
	{
		//读取实际拍得的矩阵		
		FileStorage fs1("splitRealDepthMat.xml", FileStorage::READ);
		read(fs1["data"], realDepthMap);

		////背景矩阵
		//Mat baseDepthMap;
		//FileStorage fs("splitDepthMat.xml", FileStorage::READ);
		//read(fs["data"], baseDepthMap);

		//backgroundDepthMapWAF = backgroundDepthMapWAF + baseDepthMap; //累加背景矩阵

		//对实时矩阵中值滤波
		Mat realDepthMapWAFandMedian;
		medianBlur(realDepthMap, realDepthMapWAFandMedian, 3);


		realDepthMapWAF = realDepthMapWAF + realDepthMapWAFandMedian; //累加实时拍摄

		//存为txt
		fstream splitRealDepthMatSave;
		splitRealDepthMatSave.open("splitRealDepthMat.txt", ios::out);
		splitRealDepthMatSave << realDepthMap;
		splitRealDepthMatSave.close();



		//高斯滤波
		//Mat realDepthMapAfterFilter;
		//GaussianBlur(realDepthMap, realDepthMapAfterFilter, Size(3, 3), 1);

		//存为txt
		//fstream realDepthMapAfterFilterSave;
		//realDepthMapAfterFilterSave.open("realDepthMatAfterFilter.txt", ios::out);
		//realDepthMapAfterFilterSave << realDepthMapAfterFilter;
		//realDepthMapAfterFilterSave.close();

		//存为Xml文件
		//FileStorage fs2("realDepthMatAfterFilter.xml", FileStorage::WRITE);
		//write(fs2, "data", realDepthMapAfterFilter);

	}


	else
	{
		//加权滤波存为txt
		realDepthMapWAF = realDepthMapWAF / (circleNumOfAll - 1);

		for (int i = 0; i < realDepthMapWAF.rows; i++)
		{
			for (int j = 0; j < realDepthMapWAF.cols; j++)
			{
				if (realDepthMapWAF.at<UINT16>(i, j) < 600)
				{
					realDepthMapWAF.at<UINT16>(i, j) = 705;
				}
			}
		}

		fstream realDepthMapWAFSave;
		realDepthMapWAFSave.open("realDepthMatAfterWAF.txt", ios::out);
		realDepthMapWAFSave << realDepthMapWAF;
		realDepthMapWAFSave.close();

		//backgroundDepthMapWAF = backgroundDepthMapWAF / 9;
		//for (int i = 0; i < backgroundDepthMapWAF.rows; i++)
		//{
		//	for (int j = 0; j < backgroundDepthMapWAF.cols; j++)
		//	{
		//		if (backgroundDepthMapWAF.at<UINT16>(i, j) < 700)
		//		{
		//			backgroundDepthMapWAF.at<UINT16>(i, j) = 710;
		//		}
		//	}
		//}
		//fstream backgroundDepthMapWAFSave;
		//backgroundDepthMapWAFSave.open("backgroundDepthMapWAF.txt", ios::out);
		//backgroundDepthMapWAFSave << backgroundDepthMapWAF;
		//backgroundDepthMapWAFSave.close();

		//FileStorage fs5("finalBackGroundDepthMat.xml", FileStorage::WRITE);
		//write(fs5, "data", backgroundDepthMapWAF);

		cout << "加权滤波结束" << endl;


		//背景矩阵
		Mat baseDepthMap;
		FileStorage fs("finalBackGroundDepthMat.xml", FileStorage::READ);
		read(fs["data"], baseDepthMap);

		Mat baseDepthMapAfterGaussian; //滤波后的背景
		GaussianBlur(baseDepthMap, baseDepthMapAfterGaussian, Size(3, 3), 1);

		fstream baseDepthMapSave;
		baseDepthMapSave.open("baseDepthMap.txt", ios::out);
		baseDepthMapSave << baseDepthMapAfterGaussian;
		baseDepthMapSave.close();



		Mat errorMap; //偏差矩阵

		//计算误差矩阵
		absdiff(realDepthMapWAF, baseDepthMapAfterGaussian, errorMap);


		//除掉误差矩阵中的粗大误差
		for (int i = 0; i < errorMap.rows; i++)
		{
			for (int j = 0; j < errorMap.cols; j++)
			{
				if (errorMap.at<UINT16>(i, j) > 300)
				{
					errorMap.at<UINT16>(i, j) = 0;
				}
			}
		}

		Mat errorMapAfterF;
		medianBlur(errorMap, errorMapAfterF, 3);

		fstream errorMapFile;
		errorMapFile.open("errorMapAfterF.txt", ios::out);
		errorMapFile << errorMapAfterF;
		errorMapFile.close();

		////识别到的图像的左上角的坐标
		//int leftBorderValue = 0;
		//int upBorderValue = 0;

		////识别到的图像的右下角的坐标
		//int rightDownPointXValue = 0;
		//int rightDownPointYValue = 0;

		int upBorderValue = 0;
		int leftBorderValue = 0;
		int downBorderValue = 0;
		int rightBorderValue = 0;


		//找出左上角的坐标
		std::cout << "开始找左上角坐标" << endl;
		//int total = 0;
		int dimensionOfConvolutionMatrix = 3;
		
		int baseErrorValue = 0;    //计算的误差值
		int forBreakValue = 0;    //用于跳出多层循环
		for (int x = 0; x < errorMapAfterF.rows; x = x + dimensionOfConvolutionMatrix) //行
		{
			for (int y = 0; y < errorMapAfterF.cols; y = y + dimensionOfConvolutionMatrix) //列
			{
				for (int i = x; i < x + dimensionOfConvolutionMatrix; i++)    //行
				{
					for (int j = y; j < y + dimensionOfConvolutionMatrix; j++)     //列
					{
						//total++;
						//std::cout << total << ":" << "矩阵第" << i << "行" << "  第" << j << "列" << endl;
						baseErrorValue = errorMapAfterF.at<UINT16>(i, j) + baseErrorValue;
					}
				}
				forBreakValue = baseErrorValue;
				if (baseErrorValue > tolorenceValue)
				{
					//leftBorderValue = x + 1;
					//upBorderValue = y + 1;
					//std::cout << "左上角的坐标是：" << leftBorderValue << "," << upBorderValue << endl;
					upBorderValue = x + 1;
					std::cout << "上边界所在的行大概是：" << upBorderValue << endl;

					break;
				}
				baseErrorValue = 0;
			}
			if (forBreakValue > tolorenceValue) break;
		}


		int baseErrorValue2 = 0;    //计算的误差值
		int forBreakValue2 = 0;    //用于跳出多层循环
		for (int y = 0; y < errorMapAfterF.cols; y = y + dimensionOfConvolutionMatrix) //列
		{
			for (int x = 0; x < errorMapAfterF.rows - 3; x = x + dimensionOfConvolutionMatrix) //行
			{
				for (int i = x; i < x + dimensionOfConvolutionMatrix; i++)    //行
				{
					for (int j = y; j < y + dimensionOfConvolutionMatrix; j++)     //列
					{
						//total++;
						//std::cout << total << ":" << "矩阵第" << i << "行" << "  第" << j << "列" << endl;
						baseErrorValue2 = errorMapAfterF.at<UINT16>(i, j) + baseErrorValue2;
					}
				}

				forBreakValue2 = baseErrorValue2;
				if (baseErrorValue2 > tolorenceValue)
				{
					//leftBorderValue = x + 1;
					//upBorderValue = y + 1;
					//std::cout << "左上角的坐标是：" << leftBorderValue << "," << upBorderValue << endl;

					leftBorderValue = y + 1;
					std::cout << "左边界所在的列大概是：" << leftBorderValue << endl;
					break;
				}
				baseErrorValue2 = 0;
			}
			if (forBreakValue2 > tolorenceValue) break;
		}


		////找出右下角的坐标
		////int total = 0;
		//std::cout << "开始找右下角坐标" << endl;
		//int baseErrorValue1 = 0;
		//int forBreakValue1 = 0;
		//for (int x = errorMapAfterF.rows - 1; x >= 0; x = x - dimensionOfConvolutionMatrix)
		//{
		//	for (int y = errorMapAfterF.cols - 1; y >= 0; y = y - dimensionOfConvolutionMatrix)
		//	{
		//		for (int i = x; i > x - dimensionOfConvolutionMatrix; i--)
		//		{
		//			for (int j = y; j > y - dimensionOfConvolutionMatrix; j--)
		//			{
		//				//total++;
		//				//std::cout << total << ":" << "矩阵第" << i << "行" << "  第" << j << "列" << endl;
		//				baseErrorValue1 = errorMapAfterF.at<UINT16>(i, j) + baseErrorValue1;
		//			}
		//		}
		//		forBreakValue1 = baseErrorValue1;
		//		if (baseErrorValue1 > tolorenceValue)
		//		{
		//			rightDownPointXValue = x - 1;
		//			rightDownPointYValue = y - 1;
		//			std::cout << "右下角的坐标是：" << rightDownPointXValue << "," << rightDownPointYValue << endl;
		//			break;
		//		}
		//		baseErrorValue1 = 0;
		//	}
		//	if (forBreakValue1 > tolorenceValue) break;
		//}


		//找出右下角的坐标
		//int total = 0;
		std::cout << "开始找右下角坐标" << endl;
		int baseErrorValue1 = 0;
		int forBreakValue1 = 0;
		for (int x = errorMapAfterF.rows - 1; x >= 0; x = x - dimensionOfConvolutionMatrix)
		{
			for (int y = errorMapAfterF.cols - 1; y >= 0; y = y - dimensionOfConvolutionMatrix)
			{
				for (int i = x; i > x - dimensionOfConvolutionMatrix; i--)
				{
					for (int j = y; j > y - dimensionOfConvolutionMatrix; j--)
					{
						//total++;
						//std::cout << total << ":" << "矩阵第" << i << "行" << "  第" << j << "列" << endl;
						baseErrorValue1 = errorMapAfterF.at<UINT16>(i, j) + baseErrorValue1;
					}
				}
				forBreakValue1 = baseErrorValue1;
				if (baseErrorValue1 > tolorenceValue)
				{
					//rightDownPointXValue = x - 1;
					//rightDownPointYValue = y - 1;
					//std::cout << "右下角的坐标是：" << rightDownPointXValue << "," << rightDownPointYValue << endl;
					downBorderValue = x - 1;
					std::cout << "下边界所在的行大概是：" << downBorderValue << endl;


					break;
				}
				baseErrorValue1 = 0;
			}
			if (forBreakValue1 > tolorenceValue) break;
		}


		int baseErrorValue3 = 0;
		int forBreakValue3 = 0;
		for (int y = errorMapAfterF.cols - 1; y >= 0; y = y - dimensionOfConvolutionMatrix)
		{
			for (int x = errorMapAfterF.rows - 1; x >= 0; x = x - dimensionOfConvolutionMatrix)
			{
				for (int i = x; i > x - dimensionOfConvolutionMatrix; i--)
				{
					for (int j = y; j > y - dimensionOfConvolutionMatrix; j--)
					{
						//total++;
						//std::cout << total << ":" << "矩阵第" << i << "行" << "  第" << j << "列" << endl;
						baseErrorValue3 = errorMapAfterF.at<UINT16>(i, j) + baseErrorValue3;
					}
				}
				forBreakValue3 = baseErrorValue3;
				if (baseErrorValue3 > tolorenceValue)
				{
					//rightDownPointXValue = x - 1;
					//rightDownPointYValue = y - 1;
					//std::cout << "右下角的坐标是：" << rightDownPointXValue << "," << rightDownPointYValue << endl;
					rightBorderValue = y - 1;
					std::cout << "右边界所在的列大概是：" << downBorderValue << endl;


					break;
				}
				baseErrorValue3 = 0;
			}
			if (forBreakValue3 > tolorenceValue) break;
		}




		//分割出识别到的图像的矩阵
		int b = upBorderValue; //行
		int a = leftBorderValue;  //列
		int d = downBorderValue - upBorderValue;
		int c = rightBorderValue - leftBorderValue;

		/*Rect函数，Rect（a,b,c,d),a表示左上角x坐标，b表示左上角y坐标，c表示矩阵的宽，d表手矩阵的高*/
		Mat profileMatBefore;
		if (d <= 0 || c <= 0 || a + c >= finalWidth || b + d > finalHeight)
		{
			std::cout << "识别算法失败了！" << endl;
		}
		else
		{
			profileMatBefore = realDepthMapWAF(Rect(a, b, c, d));

			fstream profileMatBeforeToTxt;
			profileMatBeforeToTxt.open("profileMatBeforeToTxt.txt", ios::out);
			profileMatBeforeToTxt << profileMatBefore;
			profileMatBeforeToTxt.close();

			Mat profileMat;
			//对最终得到的矩阵进行高斯滤波
			GaussianBlur(profileMatBefore, profileMat,Size(3,3),1);

			fstream profileMatToTxt;
			profileMatToTxt.open("profileMatToTxt.txt", ios::out);
			profileMatToTxt << profileMat;
			profileMatToTxt.close();

			std::cout << "识别到的图像的像素大小是：" << profileMat.cols << "(宽)" << "x" << profileMat.rows << "(高)" << endl;
			std::cout << endl;
			//找出棱所在的位置并分割平面
			//取中间10列数据，求平均值，消除误差
			//centerX = (double)(512 - ((profileMat.cols / 2) + finalHeight + upBorderValue));
			centerX = (double)(512 - ((profileMat.cols / 2) + finalHeight + upBorderValue));
			centerY = (double)(512 - ((profileMat.rows / 2) + finalWidth + leftBorderValue));
			std::cout << "质心X坐标:" << centerX << endl;
			std::cout << "质心Y坐标:" << centerY << endl;

			int XiangDuiX = profileMat.cols / 2;
			int XiangDuiY = profileMat.rows / 2;

			std::cout << "相对X坐标：" << XiangDuiX << endl;
			std::cout << "相对Y坐标：" << XiangDuiY << endl;

			centerZ = (double)(profileMat.at<UINT16>(XiangDuiY, XiangDuiX));

			std::cout << "质心Z坐标:" << centerZ << endl;

			int result[200] = { 0 };
			int middle = ((profileMat.cols) / 2) - 5;
			if (middle < 0)
			{
				std::cout << "图像宽度太小了，识别失败！" << endl;
			}
			else
			{
				for (int j = 0; j < profileMat.rows; j++)
				{
					for (int i = middle; i < middle + 10; i++)
					{
						result[j] = result[j] + profileMat.at<UINT16>(j, i);
					}
					result[j] = result[j] / 10;
					//std::cout << j << ":" << result[j] << endl;
				}
			}


			cout << "找棱" << endl;
			//找出棱
			int errorLine[10] = { 0 };
			for (int i = 0; i < profileMat.rows - 10; i = i + 1)
			{
				errorLine[0] = result[i + 1] - result[i];
				errorLine[1] = result[i + 2] - result[i + 1];
				errorLine[2] = result[i + 3] - result[i + 2];
				errorLine[3] = result[i + 4] - result[i + 3];
				errorLine[4] = result[i + 5] - result[i + 4];
				errorLine[5] = result[i + 6] - result[i + 5];
				errorLine[6] = result[i + 7] - result[i + 6];
				errorLine[7] = result[i + 8] - result[i + 7];
				errorLine[8] = result[i + 9] - result[i + 8];
				int positiveNum = 0;
				int negativeNum = 0;
				for (int i = 0; i < 9; i++)
				{
					if (errorLine[i] > 0)
					{
						positiveNum++;
					}
					else
					{
						negativeNum++;
					}
				}
				if (positiveNum < negativeNum)
				{
					//std::cout << "减少中......" << endl;
				}
				else
				{
					std::cout << "增长中......边界可能是：" << i + 3 << endl;
					int borderValue = i + 3;
					int stepValue = profileMat.rows / 20;

					if ((borderValue - (stepValue * 9)) > 0 && (borderValue + (stepValue * 9)) < profileMat.rows)
					{
						x1 = (double)profileMat.cols / 2 + finalHeight + upBorderValue;
						y1 = (double)(borderValue - stepValue) + finalWidth + leftBorderValue;


						int x11 = profileMat.cols / 2;
						int y11 = borderValue - stepValue;
						z1 = (double)profileMat.at<UINT16>(y11, x11);

						x2 = (double)profileMat.cols / 2 + (double)profileMat.cols / 4 + finalHeight + upBorderValue;
						y2 = (double)(borderValue - (stepValue * 6)) + finalWidth + leftBorderValue;
						int x22 = profileMat.cols / 2 + (double)profileMat.cols / 4;
						int y22 = borderValue - (stepValue * 6);
						z2 = (double)profileMat.at<UINT16>(y22, x22);


						x3 = (double)profileMat.cols / 2 - (double)profileMat.cols / 4 + finalHeight + upBorderValue;
						y3 = (double)(borderValue - (stepValue * 6)) + finalWidth + leftBorderValue;
						int x33 = profileMat.cols / 2 - (double)profileMat.cols / 4;
						int y33 = borderValue - (stepValue * 6);
						z3 = (double)profileMat.at<UINT16>(y33, x33);


						x4 = (double)profileMat.cols / 2 + finalHeight + upBorderValue;
						y4 = (double)(borderValue + stepValue) + finalWidth + leftBorderValue;
						int x44 = profileMat.cols / 2;
						int y44 = borderValue + stepValue;
						z4 = (double)profileMat.at<UINT16>(y44, x44);


						x5 = (double)profileMat.cols / 2 + (double)profileMat.cols / 4 + finalHeight + upBorderValue;
						y5 = (double)(borderValue + (stepValue * 6)) + finalWidth + leftBorderValue;
						int x55 = profileMat.cols / 2 + (double)profileMat.cols / 4;
						int y55 = borderValue + (stepValue * 6);
						z5 = (double)profileMat.at<UINT16>(y55, x55);


						x6 = (double)profileMat.cols / 2 - (double)profileMat.cols / 4 + finalHeight + upBorderValue;
						y6 = (double)(borderValue + (stepValue * 6)) + finalWidth + leftBorderValue;
						int x66 = profileMat.cols / 2 - (double)profileMat.cols / 4;
						int y66 = borderValue + (stepValue * 6);
						z6 = (double)profileMat.at<UINT16>(y66, x66);


						cout << "x" << "\t" << "y" << "z" << endl;
						cout << x1 << "\t" << y1 << z1 << endl;
						cout << x2 << "\t" << y2 << z2 << endl;
						cout << x3 << "\t" << y3 << z3 << endl;
						cout << x4 << "\t" << y4 << z4 << endl;
						cout << x5 << "\t" << y5 << z5 << endl;
						cout << x6 << "\t" << y6 << z6 << endl;

					}
					//else if (i - 5 >= 0 && i + 9 < profileMat.rows)
					//{
					//	cout << "方法2" << endl;
					//	x1 = (double)profileMat.cols / 2 + finalHeight + upBorderValue;
					//	y1 = (double)(i + 1) + finalWidth + leftBorderValue;
					//	int x11 = profileMat.cols / 2;
					//	int y11 = i + 1;
					//	z1 = (double)profileMat.at<UINT16>(y11, x11);

					//	x2 = (double)profileMat.cols / 2 + (double)profileMat.cols / 4 + finalHeight + upBorderValue;
					//	y2 = (double)(i - 2) + finalWidth + leftBorderValue;
					//	int x22 = profileMat.cols / 2 + (double)profileMat.cols / 4;
					//	int y22 = i - 2;
					//	z2 = (double)profileMat.at<UINT16>(y22, x22);


					//	x3 = (double)profileMat.cols / 2 - (double)profileMat.cols / 4 + finalHeight + upBorderValue;
					//	y3 = (double)(i - 5) + finalWidth + leftBorderValue;
					//	int x33 = profileMat.cols / 2 - (double)profileMat.cols / 4;
					//	int y33 = i - 5;
					//	z3 = (double)profileMat.at<UINT16>(y33, x33);


					//	x4 = (double)profileMat.cols / 2 + finalHeight + upBorderValue;
					//	y4 = (double)(i + 3) + finalWidth + leftBorderValue;
					//	int x44 = profileMat.cols / 2;
					//	int y44 = i + 3;
					//	z4 = (double)profileMat.at<UINT16>(y44, x44);


					//	x5 = (double)profileMat.cols / 2 + (double)profileMat.cols / 4 + finalHeight + upBorderValue;
					//	y5 = (double)(i + 6) + finalWidth + leftBorderValue;
					//	int x55 = profileMat.cols / 2 + (double)profileMat.cols / 4;
					//	int y55 = i + 6;
					//	z5 = (double)profileMat.at<UINT16>(y55, x55);


					//	x6 = (double)profileMat.cols / 2 - (double)profileMat.cols / 4 + finalHeight + upBorderValue;
					//	y6 = (double)(i + 9) + finalWidth + leftBorderValue;
					//	int x66 = profileMat.cols / 2 - (double)profileMat.cols / 4;
					//	int y66 = i + 9;
					//	z6 = (double)profileMat.at<UINT16>(y66, x66);
					//	cout << "x" << "\t" << "y" << "z" << endl;
					//	cout << x1 << "\t" << y1 << z1 << endl;
					//	cout << x2 << "\t" << y2 << z2 << endl;
					//	cout << x3 << "\t" << y3 << z3 << endl;
					//	cout << x4 << "\t" << y4 << z4 << endl;
					//	cout << x5 << "\t" << y5 << z5 << endl;
					//	cout << x6 << "\t" << y6 << z6 << endl;
					//}

					else
					{
						cout << "识别边界线失败" << endl;
					}

					break;
				}
			}
			std::cout << "本次识别结束" << endl;
		}
	}
	
	
}



//绘制统计直方图，仅用于灰度值统计,暂时没什么用
void draw_gray_Histogram(Mat a)
{
	const int channels[1] = { 0 };
	int histSize[] = { 256 };
	float midRanges[] = { 0,256 };
	const float *ranges[] = { midRanges };
	MatND dstHist;
	calcHist(&a, 1, channels, Mat(), dstHist, 1, histSize, ranges, true, false);
	Mat drawImage = Mat::zeros(Size(256, 256), CV_8UC3);
	double HistMaxValue;
	minMaxLoc(dstHist, 0, &HistMaxValue, 0, 0);
	for (int i = 0; i < 256; i++)
	{
		int value = cvRound(256 * 0.9*(dstHist.at<float>(i) / HistMaxValue));
		line(drawImage, Point(i, drawImage.rows - 1), Point(i, drawImage.rows - 1 - value), Scalar(255, 0, 0));
	}
	imshow("直方图", drawImage);
}




//图像坐标->相机坐标
void image2cam(double u, double v, double Zc, double &camera1, double &camera2, double &camera3)
{
	//相机内参
	Mat Internal_reference = Mat::ones(3, 3, CV_64FC1);
	Internal_reference.at<double>(0, 0) = 373.740164527553;
	Internal_reference.at<double>(0, 1) = 0;
	Internal_reference.at<double>(0, 2) = 256.805373188569;
	Internal_reference.at<double>(1, 0) = 0;
	Internal_reference.at<double>(1, 1) = 376.645274107254;
	Internal_reference.at<double>(1, 2) = 207.525383038604;
	Internal_reference.at<double>(2, 0) = 0;
	Internal_reference.at<double>(2, 1) = 0;
	Internal_reference.at<double>(2, 2) = 1;

	//相机内参的逆
	Mat invert_Internal_reference;
	invert(Internal_reference, invert_Internal_reference, DECOMP_CHOLESKY);
	//打印相机内参逆矩阵
	//std::std::cout << "相机内参逆矩阵是：" << endl;
	//for (int i = 0; i < 3; i++)
	//{
	//	for (int j = 0; j < 3; j++)
	//	{
	//		std::std::cout << invert_Internal_reference.at<double>(i, j) << "\t";
	//	}
	//	std::std::cout << endl;
	//}
	std::cout << endl;

	//像素坐标矩阵
	Mat imageMatrix = Mat::ones(3, 1, CV_64FC1);
	imageMatrix.at<double>(0, 0) = u;
	imageMatrix.at<double>(1, 0) = v;
	imageMatrix.at<double>(2, 0) = 1;

	//相机坐标系中的目标位置
	Mat finalPosition;

	finalPosition = Zc * invert_Internal_reference*imageMatrix;

	//把坐标写成数组形式，传出本函数

	camera1 = finalPosition.at<double>(0, 0);
	camera2 = finalPosition.at<double>(1, 0);
	camera3 = finalPosition.at<double>(2, 0);
	//double matPosition[3];
	//for (int i = 0; i < 3; i++)
	//{
	//	matPosition[i] = finalPosition.at<double>(i, 0);
	//	std::cout << "相机坐标系" << i << ":" << matPosition[i] << endl;
	//}

	////打印相机坐标系中的目标位置
	////std::std::cout << "相机坐标系中的目标位置" << endl;
	////for (int i = 0; i < 3; i++)
	////{
	////	std::std::cout << finalPosition.at<double>(i, 0) << endl;
	////}
	//return matPosition;
}


//相机坐标系->世界坐标系
//暂未用到
void cam2world(double Xc, double Yc, double Zc,double &worldX,double &worldY,double &worldZ)
{
	//旋转平移矩阵
	Mat finalTransferMatrix = Mat::ones(4, 4, CV_64FC1);
	finalTransferMatrix.at<double>(0, 0) = -0.9896;
	finalTransferMatrix.at<double>(0, 1) = -0.0806;
	finalTransferMatrix.at<double>(0, 2) = 0.0133;
	finalTransferMatrix.at<double>(0, 3) = 351.2019;
	finalTransferMatrix.at<double>(1, 0) = -0.0807;
	finalTransferMatrix.at<double>(1, 1) = 0.9936;
	finalTransferMatrix.at<double>(1, 2) = -0.0439;
	finalTransferMatrix.at<double>(1, 3) = 460.8673;
	finalTransferMatrix.at<double>(2, 0) = -0.0097;
	finalTransferMatrix.at<double>(2, 1) = -0.0445;
	finalTransferMatrix.at<double>(2, 2) = -0.9897;
	finalTransferMatrix.at<double>(2, 3) = 713.1730;
	finalTransferMatrix.at<double>(3, 0) = 0;
	finalTransferMatrix.at<double>(3, 1) = 0;
	finalTransferMatrix.at<double>(3, 2) = 0;
	finalTransferMatrix.at<double>(3, 3) = 1;

	
	Mat finalPositionAddOne = Mat(4, 1, CV_64FC1);
	finalPositionAddOne.at<double>(0, 0) = Xc;
	finalPositionAddOne.at<double>(1, 0) = Yc;
	finalPositionAddOne.at<double>(2, 0) = Zc;
	finalPositionAddOne.at<double>(3, 0) = 1;
	//目标点在世界坐标系位置
	Mat finalWorldPosition;
	finalWorldPosition = finalTransferMatrix * finalPositionAddOne;

	//把坐标写成数组形式，传出本函数
	
		worldX = finalWorldPosition.at<double>(0, 0);
		worldY = finalWorldPosition.at<double>(1, 0);
		worldZ = finalWorldPosition.at<double>(2, 0);
	//打印最终坐标
	//std::std::cout << endl;
	//std::std::cout << "最终坐标" << endl;
	//for (int i = 0; i < 4; i++)
	//{
	//	std::std::cout << finalWorldPosition.at<double>(i, 0) << endl;
	//}
	//return matWorldPosition;
}

//计算角度
double CalculateAngle(double u1, double v1, double Zc1, double u2, double v2, double Zc2,
	double u3, double v3, double Zc3, double u4, double v4, double Zc4,
	double u5, double v5, double Zc5, double u6, double v6, double Zc6)
{
	double camaeraX1, camaeraY1, camaeraZ1;
	double camaeraX2, camaeraY2, camaeraZ2;
	double camaeraX3, camaeraY3, camaeraZ3;
	double camaeraX4, camaeraY4, camaeraZ4;
	double camaeraX5, camaeraY5, camaeraZ5;
	double camaeraX6, camaeraY6, camaeraZ6;

	//两个平面上各取3个点
	image2cam(u1, v1, Zc1, camaeraX1, camaeraY1, camaeraZ1);//x1,y1,z1
	image2cam(u2, v2, Zc2, camaeraX2, camaeraY2, camaeraZ2);//x2,y2,z2
	image2cam(u3, v3, Zc3, camaeraX3, camaeraY3, camaeraZ3);//x3,y3,z3
	image2cam(u4, v4, Zc4, camaeraX4, camaeraY4, camaeraZ4);//x4,y4,z4
	image2cam(u5, v5, Zc5, camaeraX5, camaeraY5, camaeraZ5);//x5,y5,z5
	image2cam(u6, v6, Zc6, camaeraX6, camaeraY6, camaeraZ6);//x6,y6,z6

	/*std::cout << camaeraX1 << " " << camaeraY1 << " " << camaeraZ1 << endl;
	std::cout << camaeraX2 << " " << camaeraY2 << " " << camaeraZ2 << endl;*/
	//两个平面的法向量
	double n1[3], n2[3];
	n1[0] = (camaeraY2 - camaeraY1)*(camaeraZ3 - camaeraZ1) - (camaeraZ2 - camaeraZ1)*(camaeraY3 - camaeraY1);
	n1[1] = (camaeraZ2 - camaeraZ1)*(camaeraX3 - camaeraX1) - (camaeraZ3 - camaeraZ1)*(camaeraX2 - camaeraX1);
	n1[2] = (camaeraX2 - camaeraX1)*(camaeraY3 - camaeraY1) - (camaeraX3 - camaeraX1)*(camaeraY2 - camaeraY1);
	n2[0] = (camaeraY5 - camaeraY4)*(camaeraZ6 - camaeraZ4) - (camaeraZ5 - camaeraZ4)*(camaeraY6 - camaeraY4);
	n2[1] = (camaeraZ5 - camaeraZ4)*(camaeraX6 - camaeraX4) - (camaeraZ6 - camaeraZ4)*(camaeraX5 - camaeraX4);
	n2[2] = (camaeraX5 - camaeraX4)*(camaeraY6 - camaeraY4) - (camaeraX6 - camaeraX4)*(camaeraY5 - camaeraY4);
	//for (int i = 0; i < 3; i++)
	//{
	//	std::cout << n1[i] << endl;
	//	std::cout << n2[i] << endl;
	//}
	double cosAngle = (n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2]) / (sqrt(pow(n1[0], 2) + pow(n1[1], 2) + pow(n1[2], 2))*sqrt(pow(n2[0], 2) + pow(n2[1], 2) + pow(n2[2],2)));
	double nAngle = 180 / pi * acos(cosAngle);
	return nAngle;
}



/*获得深度图像，彩色图像,合成图像并显示*/
void get_depth_image(string &outSendDta)
{
	cv::setUseOptimized(true);

	// Sensor
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hResult))
	{
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
	}

	hResult = pSensor->Open();
	if (FAILED(hResult))
	{
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
	}

	// Source
	IColorFrameSource* pColorSource;
	hResult = pSensor->get_ColorFrameSource(&pColorSource);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
	}

	IDepthFrameSource* pDepthSource;
	hResult = pSensor->get_DepthFrameSource(&pDepthSource);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IKinectSensor::get_DepthFrameSource()" << std::endl;
	}

	// Reader
	IColorFrameReader* pColorReader;
	hResult = pColorSource->OpenReader(&pColorReader);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
	}

	IDepthFrameReader* pDepthReader;
	hResult = pDepthSource->OpenReader(&pDepthReader);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IDepthFrameSource::OpenReader()" << std::endl;
	}

	// Description
	IFrameDescription* pColorDescription;
	hResult = pColorSource->get_FrameDescription(&pColorDescription);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IColorFrameSource::get_FrameDescription()" << std::endl;
	}

	int colorWidth = 0;
	int colorHeight = 0;
	pColorDescription->get_Width(&colorWidth); // 1920
	pColorDescription->get_Height(&colorHeight); // 1080
	unsigned int colorBufferSize = colorWidth * colorHeight * 4 * sizeof(unsigned char);

	cv::Mat colorBufferMat(colorHeight, colorWidth, CV_8UC4);
	cv::Mat colorMat(colorHeight / 2, colorWidth / 2, CV_8UC4);


	IFrameDescription* pDepthDescription;
	hResult = pDepthSource->get_FrameDescription(&pDepthDescription);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IDepthFrameSource::get_FrameDescription()" << std::endl;
	}

	int depthWidth = 0;
	int depthHeight = 0;
	pDepthDescription->get_Width(&depthWidth); // 512
	pDepthDescription->get_Height(&depthHeight); // 424
	unsigned int depthBufferSize = depthWidth * depthHeight * sizeof(unsigned short);

	cv::Mat depthBufferMat(depthHeight, depthWidth, CV_16UC1);
	cv::Mat depthMat(depthHeight, depthWidth, CV_8UC1);
	cv::Mat depthMatToTxt(depthHeight, depthWidth, CV_16UC1);


	// Coordinate Mapper
	ICoordinateMapper* pCoordinateMapper;
	hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
	if (FAILED(hResult))
	{
		std::cerr << "Error : IKinectSensor::get_CoordinateMapper()" << std::endl;
	}

	cv::Mat coordinateMapperMat(depthHeight, depthWidth, CV_8UC4);


	unsigned short minDepth, maxDepth;
	pDepthSource->get_DepthMinReliableDistance(&minDepth);
	pDepthSource->get_DepthMaxReliableDistance(&maxDepth);

	double x1 = 0, y1 = 0, z1 = 0, x2 = 0, y2 = 0, z2 = 0, x3 = 0, y3 = 0, z3 = 0, x4 = 0, y4 = 0, z4 = 0, x5 = 0, y5 = 0, z5 = 0, x6 = 0, y6 = 0, z6 = 0;
	double centerX, centerY, centerZ; //质心坐标
	int circleNum = 0; //记录有效循环次数
	double angleArray[100] = { 0 }; //保存角度的数组
	double centerXArray[100] = { 0 }; //保存X坐标的数组
	double centerYArray[100] = { 0 }; //保存Y坐标的数组
	double centerZArray[100] = { 0 }; //保存Z坐标的数组

	while (1)
	{
		std::cout << "开始捕获图像......" << endl;
		// Color Frame
		IColorFrame* pColorFrame = nullptr;
		hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
		if (SUCCEEDED(hResult))
		{
			hResult = pColorFrame->CopyConvertedFrameDataToArray(colorBufferSize, reinterpret_cast<BYTE*>(colorBufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
			if (SUCCEEDED(hResult))
			{
				cv::resize(colorBufferMat, colorMat, cv::Size(), 0.5, 0.5);
			}
		}

		// Depth Frame
		IDepthFrame* pDepthFrame = nullptr;
		hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
		if (SUCCEEDED(hResult))
		{
			hResult = pDepthFrame->AccessUnderlyingBuffer(&depthBufferSize, reinterpret_cast<UINT16**>(&depthBufferMat.data));
			if (SUCCEEDED(hResult))
			{
				depthBufferMat.convertTo(depthMat, CV_8U, -255.0f / 8000.0f, 255.0f);
			}
			pDepthFrame->CopyFrameDataToArray(depthWidth * depthHeight, (UINT16 *)depthMatToTxt.data);

			fstream backDepthMatSave;
			backDepthMatSave.open("backDepthMat.txt", ios::out);
			backDepthMatSave << depthMatToTxt;
			backDepthMatSave.close();

			Mat roi(depthMatToTxt, Rect(initialYValue, initialXValue, finalWidth, finalHeight));

			FileStorage fs("splitRealDepthMat.xml", FileStorage::WRITE);
			cv::write(fs, "data", roi);

			Mat testDepth(depthHeight, depthWidth, CV_8UC4);
			flip(depthMatToTxt, testDepth, 1);

			//cout << "1:" << testDepth.at<UINT16>(151, 204) << endl;
			//cout << "2:" << testDepth.at<UINT16>(151, 298) << endl;
			//cout << "3:" << testDepth.at<UINT16>(210, 298) << endl;
			/*fstream allDepthTxt;
			allDepthTxt.open("allDepthTxt.txt", ios::out);
			allDepthTxt << depthMatToTxt;
			allDepthTxt.close();
			fstream roiTxt;
			roiTxt.open("depth.txt", ios::out);
			roiTxt << roi;
			roiTxt.close();*/
		}


		// Mapping (Depth to Color)
		if (SUCCEEDED(hResult))
		{
			std::vector<ColorSpacePoint> colorSpacePoints(depthWidth * depthHeight);
			hResult = pCoordinateMapper->MapDepthFrameToColorSpace(depthWidth * depthHeight, reinterpret_cast<UINT16*>(depthBufferMat.data), depthWidth * depthHeight, &colorSpacePoints[0]);
			if (SUCCEEDED(hResult))
			{
				coordinateMapperMat = cv::Scalar(0, 0, 0, 0);
				for (int y = 0; y < depthHeight; y++)
				{
					for (int x = 0; x < depthWidth; x++)
					{
						unsigned int index = y * depthWidth + x;
						ColorSpacePoint point = colorSpacePoints[index];
						int colorX = static_cast<int>(std::floor(point.X + 0.5));
						int colorY = static_cast<int>(std::floor(point.Y + 0.5));
						unsigned short depth = depthBufferMat.at<unsigned short>(y, x);
						if ((colorX >= 0) && (colorX < colorWidth) && (colorY >= 0) && (colorY < colorHeight)/* && ( depth >= minDepth ) && ( depth <= maxDepth )*/)
						{
							coordinateMapperMat.at<cv::Vec4b>(y, x) = colorBufferMat.at<cv::Vec4b>(colorY, colorX);
						}
					}
				}
			}
			Mat test(depthHeight, depthWidth, CV_8UC4);
			flip(coordinateMapperMat, test, 1);
			cv::imwrite("test.jpg", test);



			Mat roi1(coordinateMapperMat, Rect(initialYValue,initialXValue, finalWidth, finalHeight));
			cv::imwrite("coorMap.jpg", roi1);
			cv::imwrite("coMap.jpg", coordinateMapperMat);
			cv::imshow("coorMap", roi1);
		}

		SafeRelease(pColorFrame);
		SafeRelease(pDepthFrame);


		//cv::namedWindow("Color");
		//cv::imshow("Color", colorMat);
		//cv::namedWindow("Depth");
		//cv::imshow("Depth", depthMat);
		cv::namedWindow("CoordinateMapper");
		cv::imshow("CoordinateMapper", coordinateMapperMat);




		if (SUCCEEDED(hResult))
		{
			//std::std::cout << "开始图像识别" << endl;
			get_split_depthImage(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, y6, z6, centerX, centerY, centerZ, circleNum);
			//std::cout << x1 << " " << y1 << " " << z1 << endl;
			//std::cout << x2 << " " << y2 << " " << z2 << endl;
			//std::cout << x3 << " " << y3 << " " << z3 << endl;
			//std::cout << x4 << " " << y4 << " " << z4 << endl;
			//std::cout << x5 << " " << y5 << " " << z5 << endl;
			//std::cout << x6 << " " << y6 << " " << z6 << endl;
			//double angle = CalculateAngle(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, y6, z6);
			//
			//angleArray[circleNum] = angle;

			//centerXArray[circleNum] = centerX;
			//centerYArray[circleNum] = centerY;
			//centerZArray[circleNum] = centerZ;


			
			std::cout << "循环次数:" << circleNum << endl;
			circleNum++;
			if (circleNum == circleNumOfAll)
			{
				double angle = CalculateAngle(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, y6, z6);
				
				cout << "角度是：" << angle << endl;
				//sort(angleArray, angleArray + 100);
				//std::cout << "中位数：" << (angleArray[49] + angleArray[50]) / 2 << endl;


				////角度平均值
				//double averageValue = 0;
				//for (int i = 0; i < 100; i++)
				//{
				//	averageValue = angleArray[i] + averageValue;
				//}
				//averageValue = averageValue / 100;
				//std::cout << "平均值：" << averageValue << endl;



				////坐标X平均值
				//double centerXvalue = 0;
				//for (int i = 0; i < 100; i++)
				//{
				//	centerXvalue = centerXArray[i] + centerXvalue;
				//}
				//centerXvalue = centerXvalue / 100;




				////坐标Y平均值
				//double centerYvalue = 0;
				//for (int i = 0; i < 100; i++)
				//{
				//	centerYvalue = centerYArray[i] + centerYvalue;
				//}
				//centerYvalue = centerYvalue / 100;



				////坐标Z平均值
				//double centerZvalue = 0;
				//for (int i = 0; i < 100; i++)
				//{
				//	centerZvalue = centerZArray[i] + centerZvalue;
				//}
				//centerZvalue = centerZvalue / 100;

				double centerXvalue = centerX;
				double centerYvalue = centerY;
				double centerZvalue = centerZ;


				double worldXfinal, worldYfinal, worldZfinal;
				double cameraXfinal, cameraYfinal, cameraZfinal;
				image2cam(centerXvalue, centerYvalue, centerZvalue, cameraXfinal, cameraYfinal, cameraZfinal);
				//cam2world(centerXvalue, centerYvalue, centerZvalue, worldXfinal, worldYfinal, worldZfinal);
				cam2world(cameraXfinal, cameraYfinal, cameraZfinal, worldXfinal, worldYfinal, worldZfinal);
				string centerXvalueString = to_string(worldXfinal);
				string centerYvalueString = to_string(worldYfinal);
				string centerZvalueString = to_string(worldZfinal);

				//拼接发送字符串

				string afterJingHao = "#0#-180#0#0#0#WCS#0#0#0#0#0\r\n";
				string JingHao = "#";
				outSendDta = centerXvalueString + JingHao + centerYvalueString + JingHao + centerZvalueString + afterJingHao;
				std::cout << "发送的数据为：" << outSendDta << endl;

				//if (abs((angleArray[49] + angleArray[50]) / 2 - 60) <= 15)
				//{
				//	std::cout << "识别到的物体是三棱柱" << endl;
				//}
				//else if (abs((angleArray[49] + angleArray[50]) / 2 - 120) <= 15)
				//{
				//	std::cout << "识别到的物体是六棱柱" << endl;
				//}
				//else if (abs((angleArray[49] + angleArray[50]) / 2 - 90) <= 15)
				//{
				//	std::cout << "识别到的物体是四棱柱" << endl;
				//}
				//else
				//{
				//	std::cout << "没有识别出来，暂不支持其余形状识别" << endl;
				//}

				if (abs(angle - 60) <= 15)
				{
					std::cout << "识别到的物体是三棱柱" << endl;
				}
				else if (abs(angle - 120) <= 15)
				{
					std::cout << "识别到的物体是六棱柱" << endl;
				}
				else if (abs(angle - 90) <= 15)
				{
					std::cout << "识别到的物体是四棱柱" << endl;
				}
				else
				{
					std::cout << "没有识别出来，暂不支持其余形状识别" << endl;
				}
				break;
			}
		}
		

		


		if (cv::waitKey(30) == VK_ESCAPE) 
		{
			break;
		}
	}

	SafeRelease(pColorSource);
	SafeRelease(pDepthSource);
	SafeRelease(pColorReader);
	SafeRelease(pDepthReader);
	SafeRelease(pColorDescription);
	SafeRelease(pDepthDescription);
	SafeRelease(pCoordinateMapper);
	if (pSensor) 
	{
		pSensor->Close();
	}
	SafeRelease(pSensor);
	cv::destroyAllWindows();
}


int main()
{
	string data_;
	get_depth_image(data_);
	std::cout << data_;
	server(data_);
	system("pause");
	return  0;
}