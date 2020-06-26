// work.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

//ȥ������		
void clear_disturb(Mat& src)
{
	//����ߵ���
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < 150; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//��С����
	for (int i = 0; i < 100; i++)
	{
		for (int j = 310; j < src.cols; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//����
	for (int i = 150; i < src.rows; i++)
	{
		for (int j = 420; j < src.cols; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
	//��ˮ����
	for (int i = 100; i < src.rows; i++)
	{
		for (int j = 0; j < 300; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
}

//ȷ������ˮ������
void point(Mat& src, std::vector<Point>& points)
{
	for (int i = 150; i < 450; i++)
	{
		for (int j = 30; j < 230; j++)
		{
			if (src.at<uchar>(j, i) == 255)
			{
				points.push_back(Point(i, j));
				//�������
				//circle(src_color, Point(j, i), 1, Scalar(255, 0, 0), 1, 1, 0);
				break;
			}
		}
	}
	//��ӡ����
	/*int N = points.size();
	for (int n = 0; n < N; n++)
	{
	cout << "point" << points[n].x << " " << points[n].y << endl;
	}*/
}

//�������
bool fit(int n, Mat &A, std::vector<Point>& point)
{
	int N = point.size();
	//����X
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
	//����Y
	cv::Mat Y = cv::Mat::zeros(n + 1, 1, CV_64FC1);
	for (int i = 0; i < n + 1; i++)
	{
		for (int k = 0; k < N; k++)
		{
			Y.at<double>(i, 0) = Y.at<double>(i, 0) + std::pow(point[k].x, i) * point[k].y;
		}
	}
	A = cv::Mat::zeros(n + 1, 1, CV_64FC1);
	//�����A
	cv::solve(X, Y, A, cv::DECOMP_LU);
	return true;
}

//�������
void bgSub_demo(Mat& src, Mat& bny_subMat, Mat& bgMat)
{
	cv::Mat subMat;
	absdiff(src, bgMat, subMat);
	threshold(subMat, bny_subMat, 90, 255, CV_THRESH_BINARY);
	clear_disturb(bny_subMat);	//ȥ������
	//imshow("b_subMat", bny_subMat);
}

//������
void BlobAnalysis(Mat src_color)
{
	//ɸѡ
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

	//ת��Ϊhsvģʽ
	cvtColor(src_color, hsvMat, COLOR_BGR2HSV);
	src_color.copyTo(disMat);
	cv::inRange(hsvMat, Scalar(i_minH, i_minS, i_minV), Scalar(i_maxH, i_maxS, i_maxV), rangeMat1);
	cv::inRange(hsvMat, Scalar(i_minH2, i_minS, i_minV), Scalar(i_maxH2, i_maxS, i_maxV), rangeMat2);
	//�����㣬�ϲ�������Χ��ɸѡ���
	bnyMat = rangeMat1 + rangeMat2;
	//��ͨ��
	int nComp = connectedComponentsWithStats(bnyMat, lblMat, sttMat, cntMat);
	//0��Ϊ������������i=1��ʼѭ��
	for (int i = 1; i < nComp; i++) {
		Rect bbox;
		//bounding box���Ͻ�����					
		bbox.x = sttMat.at<int>(i, 0);
		bbox.y = sttMat.at<int>(i, 1);
		//bouding box�Ŀ�ͳ� 					
		bbox.width = sttMat.at<int>(i, 2);
		bbox.height = sttMat.at<int>(i, 3);
		//����					
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
		std::cout << "���ܴ���Ƶ�ļ�" << std::endl;
		return-1;
	}
	if (!cap_color.isOpened())
	{
		std::cout << "���ܴ���Ƶ�ļ�" << std::endl;
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
			//������
			BlobAnalysis(frame_color);
		}

		else if (cnt >= 70 && cnt < 240)
		{
			//�������,��������
			bgSub_demo(frame, bny_subMat, bgMat);
			//ȷ������ˮ������
			point(bny_subMat, points);
			//�������
			fit(3, A, points);
			std::vector<cv::Point> points_fitted;
			for (int x = 173; x < 430; x++)
			{
				double y = A.at<double>(0, 0) + A.at<double>(1, 0) * x + A.at<double>(2, 0) * std::pow(x, 2) + A.at<double>(3, 0) * std::pow(x, 3);
				points_fitted.push_back(cv::Point(x, y));
			}
			//����
			cv::polylines(frame_color, points_fitted, false, cv::Scalar(255, 0, 0), 1, 8, 0);
			//������
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