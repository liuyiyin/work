// work.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//去除干扰		
void clear_disturb(Mat& src)
{
	//除左边的字
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < 150; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//除小窗口
	for (int i = 0; i < 100; i++)
	{
		for (int j = 310; j < src.cols; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//除火
	for (int i = 150; i < src.rows; i++)
	{
		for (int j = 420; j < src.cols; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//除水反光
	for (int i = 100; i < src.rows; i++)
	{
		for (int j = 0; j < 300; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
}

//确定大致水柱轮廓
void point(Mat& src, std::vector<Point>& points)
{
	for (int i = 150; i < 450; i++)
	{
		for (int j = 30; j < 230; j++)
		{
			if (src.at<uchar>(j, i) == 255)
			{
				points.push_back(Point(i, j));
				//检测坐标
				//circle(src_color, Point(j, i), 1, Scalar(255, 0, 0), 1, 1, 0);
				break;
			}
		}
	}
	//打印坐标
	/*int N = points.size();
	for (int n = 0; n < N; n++)
	{
	cout << "point" << points[n].x << " " << points[n].y << endl;
	}*/
}

//拟合曲线
bool fit(int n, Mat &A, std::vector<Point>& point)
{
	int N = point.size();
	//矩阵X
	cv::Mat X = cv::Mat::zeros(n + 1, n + 1, CV_64FC1);
	for (int i = 0; i < n + 1; i++)
	{
		for (int j = 0; j < n + 1; j++)
		{
			for (int k = 0; k < N; k++)
			{
				X.at<double>(i, j) = X.at<double>(i, j) + std::pow(point[k].x, i + j);
			}
		}
	}
	//矩阵Y
	cv::Mat Y = cv::Mat::zeros(n + 1, 1, CV_64FC1);
	for (int i = 0; i < n + 1; i++)
	{
		for (int k = 0; k < N; k++)
		{
			Y.at<double>(i, 0) = Y.at<double>(i, 0) + std::pow(point[k].x, i) * point[k].y;
		}
	}
	A = cv::Mat::zeros(n + 1, 1, CV_64FC1);
	//求矩阵A
	cv::solve(X, Y, A, cv::DECOMP_LU);
	return true;
}

//背景差分
void bgSub_demo(Mat& src, Mat& bny_subMat, Mat& bgMat)
{
	cv::Mat subMat;
	absdiff(src, bgMat, subMat);
	threshold(subMat, bny_subMat, 90, 255, CV_THRESH_BINARY);
	clear_disturb(bny_subMat);	//去除干扰
	//imshow("b_subMat", bny_subMat);
}

//火焰标记
void BlobAnalysis(Mat src_color)
{
	//筛选
	int width_th = 20;
	int height_th = 14;

	double i_minH = 0;
	double i_maxH = 30;

	double i_minH2 = 150;
	double i_maxH2 = 190;

	double i_minS = 110;
	double i_maxS = 255;

	double i_minV = 50;
	double i_maxV = 255;

	Mat hsvMat;
	Mat disMat;
	Mat rangeMat1;
	Mat rangeMat2;
	Mat bnyMat;
	Mat lblMat, sttMat, cntMat;

	//转换为hsv模式
	cvtColor(src_color, hsvMat, COLOR_BGR2HSV);
	src_color.copyTo(disMat);
	cv::inRange(hsvMat, Scalar(i_minH, i_minS, i_minV), Scalar(i_maxH, i_maxS, i_maxV), rangeMat1);
	cv::inRange(hsvMat, Scalar(i_minH2, i_minS, i_minV), Scalar(i_maxH2, i_maxS, i_maxV), rangeMat2);
	//与运算，合并两个范围的筛选结果
	bnyMat = rangeMat1 + rangeMat2;
	//连通域
	int nComp = connectedComponentsWithStats(bnyMat, lblMat, sttMat, cntMat);
	//0号为背景，跳过，i=1开始循环
	for (int i = 1; i < nComp; i++) {
		Rect bbox;
		//bounding box左上角坐标					
		bbox.x = sttMat.at<int>(i, 0);
		bbox.y = sttMat.at<int>(i, 1);
		//bouding box的宽和长 					
		bbox.width = sttMat.at<int>(i, 2);
		bbox.height = sttMat.at<int>(i, 3);
		//绘制					
		if (bbox.width > width_th	 &&	bbox.height > height_th)
		{
			rectangle(src_color, bbox, CV_RGB(255, 255, 0), 2, 8, 0);
		}
	}
}

int main()
{

	VideoCapture cap("..\\testImage\\shuizhu.mp4", 0);
	VideoCapture cap_color("..\\testImage\\shuizhu.mp4");

	vector<Point> points;
	cv::Mat A;
	cv::Mat frame;
	cv::Mat frame_color;
	cv::Mat bgMat;
	cv::Mat bny_subMat;
	int cnt = 0;

	if (!cap.isOpened())
	{
		std::cout << "不能打开视频文件" << std::endl;
		return-1;
	}
	if (!cap_color.isOpened())
	{
		std::cout << "不能打开视频文件" << std::endl;
		return-1;
	}

	while (1)
	{
		cap >> frame;
		cap_color >> frame_color;

		cvtColor(frame, frame, COLOR_BGR2GRAY);

		if (cnt == 0) {
			frame.copyTo(bgMat);
		}

		else if (cnt >= 2 && cnt < 70)
		{
			//火焰标记
			BlobAnalysis(frame_color);
		}

		else if (cnt >= 70 && cnt < 240)
		{
			//背景差分,消除干扰
			bgSub_demo(frame, bny_subMat, bgMat);
			//确定大致水柱轮廓
			point(bny_subMat, points);
			//拟合曲线
			fit(3, A, points);
			std::vector<cv::Point> points_fitted;
			for (int x = 173; x < 430; x++)
			{
				double y = A.at<double>(0, 0) + A.at<double>(1, 0) * x + A.at<double>(2, 0) * std::pow(x, 2) + A.at<double>(3, 0) * std::pow(x, 3);
				points_fitted.push_back(cv::Point(x, y));
			}
			//绘制
			cv::polylines(frame_color, points_fitted, false, cv::Scalar(255, 0, 0), 1, 8, 0);
			//火焰标记
			BlobAnalysis(frame_color);
		}
		else if (cnt == 240)
		{
			break;
		}
		imshow("frame_color", frame_color);
		cnt++;
		waitKey(30);
	}
	return 0;
}