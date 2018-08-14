#include <opencv2/opencv.hpp>
#include <list>
#include <vector>
#include "InversePerspectiveMapping.hh"
#include "mcv.hh"

using namespace std;
using namespace cv;

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

    // LaneDetectorConf *stopLineConf;

    // init stopLineConf
    // in LaneDetector.cc
    // mcvInitLaneDetectorConf("Stoplines.conf", // stopLineConf config file
    //     stopLineConf)

    // ipmVpPortion = .2//#.075#0.1 #.05
    // ipmLeft = 85
    // ipmRight = 550
    // ipmTop = 50
    // ipmBottom = 380 #350 #300 for latest St-lukes data
    // ipmInterpolation = 0;
    int ipmWidth = 160; // 160 by default
    int ipmHeight = 120; // 120 by default
    CvMat * ipm = cvCreateMat(ipmHeight, ipmWidth, inImage->type);

    LaneDetector::IPMInfo ipmInfo;
    ipmInfo.vpPortion = .2;
    ipmInfo.ipmLeft = 85;
    ipmInfo.ipmRight = 550;
    ipmInfo.ipmTop = 50;
    ipmInfo.ipmBottom = 380;
    ipmInfo.ipmInterpolation = 0;

    list<CvPoint> outPixels;

    LaneDetector::CameraInfo *cameraInfo = new LaneDetector::CameraInfo();
    // focal length
    float focalLengthX = 309.4362;
    float focalLengthY = 344.2161;
    cameraInfo->focalLength = cvPoint2D32f(focalLengthX, focalLengthY);
    // optical center coordinates in image frame (origin is (0,0) at top left)
    float opticalCenterX = 317.9034;
    float opticalCenterY = 256.5352;
    cameraInfo->opticalCenter = cvPoint2D32f(opticalCenterX, opticalCenterY);
    // height of the camera in mm
    cameraInfo->cameraHeight = 2179.8; //# 393.7 + 1786.1
    // pitch of the camera
    cameraInfo->pitch = 14.0;
    // yaw of the camera
    cameraInfo->yaw  = 0.0;
    // imag width and height
    cameraInfo->imageWidth = 640;
    cameraInfo->imageHeight = 480;

    // execute GetIPM, new image is ipm
    list<CvPoint>* out_pt = &outPixels;
    LaneDetector::mcvGetIPM(inImage, ipm, &ipmInfo, cameraInfo);
    LaneDetector::SHOW_IMAGE(ipm, "IPM_image");
    cvConvertScale(ipm, ipm, 255);
    output_img = cvarrToMat(ipm);
    cv::imwrite("output.png", output_img);

    return 0;
}
