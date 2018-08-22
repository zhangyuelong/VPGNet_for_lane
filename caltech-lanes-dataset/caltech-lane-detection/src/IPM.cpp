/***
Created by Rui Wang @ Tsinghua University
08/14/2018
-This file utilizes caltech dataset and performs Inverse Perspective Mapping on it
-usage:
input.png(RGB) -> IPM according to configs -> output.png(gray scale image)

--NOTICE:
to compile the project use the following command:
g++ IPM.cpp InversePerspectiveMapping.cc mcv.cc -o a `pkg-config --libs opencv`
***/

#include <opencv2/opencv.hpp>
#include <list>
#include <vector>
#include "InversePerspectiveMapping.hh"
#include "mcv.hh"
#include <cstring>
#include <fstream>

using namespace std;
using namespace cv;

void parse_config(string filename, int &ipmWidth, int &ipmHeight, LaneDetector::CameraInfo* cameraInfo, LaneDetector::IPMInfo &ipmInfo){
    ifstream config_file;
    config_file.open(filename.c_str());
    string line, var_name;
    float number;
    struct{
        int ipmWidth, ipmHeight, ipmLeft, ipmRight, ipmTop, ipmBottom, ipmInterpolation, imageWidth, imageHeight;
        float vpPortion, focalLengthX, focalLengthY, opticalCenterX, opticalCenterY, cameraHeight, pitch, yaw;
    } configurations;
    while (getline(config_file, line)){
        if (line[0] != '#') // this line not commented
        {
            istringstream iss1(line);
            iss1 >> var_name >> number;
            if (var_name == "ipmWidth")
                configurations.ipmWidth = number;
            else if (var_name == "ipmHeight")
                configurations.ipmHeight = number;
            else if (var_name == "vpPortion")
                configurations.vpPortion = number;
            else if (var_name == "ipmLeft")
                configurations.ipmLeft = number;
            else if (var_name == "ipmRight")
                configurations.ipmRight = number;
            else if (var_name == "ipmTop")
                configurations.ipmTop = number;
            else if (var_name == "ipmBottom")
                configurations.ipmBottom = number;
            else if (var_name == "ipmInterpolation")
                configurations.ipmInterpolation = number;
            else if (var_name == "focalLengthX")
                configurations.focalLengthX = number;
            else if (var_name == "focalLengthY")
                configurations.focalLengthY = number;
            else if (var_name == "opticalCenterX")
                configurations.opticalCenterX = number;
            else if (var_name == "opticalCenterY")
                configurations.opticalCenterY = number;
            else if (var_name == "cameraHeight")
                configurations.cameraHeight = number;
            else if (var_name == "pitch")
                configurations.pitch = number;
            else if (var_name == "yaw")
                configurations.yaw = number;
            else if (var_name == "imageWidth")
                configurations.imageWidth = number;
            else if (var_name == "imageHeight")
                configurations.imageHeight = number;
        }
    }
    // sizes: the output size, can be arbitrary
    ipmWidth = configurations.ipmWidth; // output size!
    ipmHeight = configurations.ipmHeight;

    // IPM info: define the pixel range
    ipmInfo.vpPortion = configurations.vpPortion; // how far is the image top from vanishing point (bc vp is too far, can't display all)
    ipmInfo.ipmLeft = configurations.ipmLeft;
    ipmInfo.ipmRight = configurations.ipmRight;
    ipmInfo.ipmTop = configurations.ipmTop;
    ipmInfo.ipmBottom = configurations.ipmBottom;// 380;
    ipmInfo.ipmInterpolation = configurations.ipmInterpolation;

    // focal length
    float focalLengthX = configurations.focalLengthX;
    float focalLengthY = configurations.focalLengthY;
    cameraInfo->focalLength = cvPoint2D32f(focalLengthX, focalLengthY);
    // optical center coordinates in image frame (origin is (0,0) at top left)
    float opticalCenterX = configurations.opticalCenterX;
    float opticalCenterY = configurations.opticalCenterY;
    cameraInfo->opticalCenter = cvPoint2D32f(opticalCenterX, opticalCenterY);
    // height of the camera in mm
    cameraInfo->cameraHeight = configurations.cameraHeight; //# 393.7 + 1786.1
    // pitch of the camera
    cameraInfo->pitch = configurations.pitch * CV_PI / 180.0; // in radius!
    // yaw of the camera
    cameraInfo->yaw  = configurations.yaw * CV_PI / 180.0;
    // imag width and height
    cameraInfo->imageWidth = configurations.imageWidth; // camera photo size! input size.
    cameraInfo->imageHeight = configurations.imageHeight;
    
    config_file.close();
    return;
}

void points_image2ground(int n, int *points_x, int m, int *points_y){ // n == m is the number of points, for python interface

    for (int i = 0; i < n; i ++)
    {
        cout << points_x[i] << " " << points_y[i] << endl;
    }
    LaneDetector::CameraInfo *cameraInfo = new LaneDetector::CameraInfo();
    LaneDetector::IPMInfo ipmInfo;
    int ipmWidth = 640; // default, to be changed by parse_config function
    int ipmHeight = 480;
    string filename = "camera.conf";
    parse_config(filename, ipmWidth, ipmHeight, cameraInfo, ipmInfo);

    // FLOAT_MAT_ELEM_TYPE uv[] = {pt1.x, pt2.x, pt1.y, pt2.y};
    FLOAT_MAT_ELEM_TYPE uv[2 * n];
    for (int i = 0; i < n; i ++)
    {
        uv[i] = points_x[i]; // change the format of points!
        uv[2 * i + 1] = points_y[i];
    }
    CvMat uv_cvmat = cvMat(2, n, FLOAT_MAT_TYPE, uv);
    CvMat * xy = cvCreateMat(2, n, FLOAT_MAT_TYPE);
    CvMat xy_cvmat = *xy;
    mcvTransformImage2Ground(&uv_cvmat, &xy_cvmat, cameraInfo);
    for (int i = 0; i < n; i ++)
    {
        points_x[i] = CV_MAT_ELEM(xy_cvmat, float, 0, i);
        points_y[i] = CV_MAT_ELEM(xy_cvmat, float, 1, i);
    }
    return;
}

int main(){

    Mat image = imread("input.png");
    Mat gray_img;
    cvtColor(image, gray_img, COLOR_BGR2GRAY);
    CvMat converted_img = CvMat(gray_img);
    CvMat *int_image = &converted_img;
    CvMat *inImage = cvCreateMat(int_image->height, int_image->width, FLOAT_MAT_TYPE);
    cvConvertScale(int_image, inImage, 1./255);

    Mat output_img = cvarrToMat(&converted_img);
    imwrite("gray.png", output_img);

    LaneDetector::CameraInfo *cameraInfo = new LaneDetector::CameraInfo();
    LaneDetector::IPMInfo ipmInfo;
    int ipmWidth = 640; // default, can be changed by function
    int ipmHeight = 480;
    string filename = "camera.conf";

    parse_config(filename, ipmWidth, ipmHeight, cameraInfo, ipmInfo);

    CvMat * ipm = cvCreateMat(ipmHeight, ipmWidth, inImage->type); // the picture after IPM will be stored in ipm
    // execute GetIPM, new image is ipm
    list<CvPoint> outPixels;
    LaneDetector::mcvGetIPM(inImage, ipm, &ipmInfo, cameraInfo);
    printf("Press any key to continue!\n");
    LaneDetector::SHOW_IMAGE(ipm, "IPM_image");
    cvConvertScale(ipm, ipm, 255);
    output_img = cvarrToMat(ipm);
    cv::imwrite("output.png", output_img);

    int test_x[2] = {200, 300};
    int test_y[2] = {400, 500};
    points_image2ground(2, test_x, 2, test_y);

    return 0;
}
