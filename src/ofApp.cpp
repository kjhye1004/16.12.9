#include "ofApp.h"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
using namespace cv;
using namespace std;

ofImage image1;
ofVideoPlayer video;
Mat img_object, img_scene;
Mat img_matches;

void myFeatureDetector(Mat img_scene, Mat img_object){
    //-- 1.Detector를 이용하여 keypoint를 찾는다.
    int minHessian = 400;
    
    //SurfFeatureDetector detector( minHessian );
    OrbFeatureDetector detector( minHessian );
    
    vector<KeyPoint> keypoints_object, keypoints_scene;
    
    detector.detect( img_object, keypoints_object );
    detector.detect( img_scene, keypoints_scene );

    //-- 2.descriptors를 찾는다.
    //SurfDescriptorExtractor extractor;
    OrbDescriptorExtractor extractor;
    
    Mat descriptors_object, descriptors_scene;
    
    extractor.compute( img_object, keypoints_object, descriptors_object );
    extractor.compute( img_scene, keypoints_scene, descriptors_scene );
    
    //-- 3.keypoints와 descriptors를 BF 또는 FLANN matcher으로 매칭한다.
    //FlannBasedMatcher matcher;
    BFMatcher matcher(NORM_L2); //(NORM_HAMMING);
    vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );
    
    double max_dist = 0; double min_dist = 100;
    
    //-- 4.keypoint들 사이의 거리의 최소값과 최대값을 계산한다.
    for( int i = 0; i < descriptors_object.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    
    //-- 5.결과값이 좋은 것만 그린다. (최소거리의 3배 이상보다 작은 거리의 것들만 그린다.)
    vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_object.rows; i++ )
    { if( matches[i].distance < 3*min_dist )
    { good_matches.push_back( matches[i]); }
    }
    
    drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    
    //-- 6.매칭이 잘된 것들로부터 키포인트를 구한다.
    vector<Point2f> obj;
    vector<Point2f> scene;
    
    for( int i = 0; i < good_matches.size(); i++ )
    {
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }
    
    Mat H = findHomography( obj, scene, CV_RANSAC );
    
    //-- 7.동영상에서 검출될 책부분의 네 모서리를 찾는다.
    vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
    obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
    std::vector<Point2f> scene_corners(4);
    
    perspectiveTransform( obj_corners, scene_corners, H);
    
    //-- 8.책의 네 모서리끼리 선을 그어 사각형을 그린다.
    line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    
    //-- 9.결과물 출력
    imshow( "Good Matches & Object detection", img_matches );
}

void ofApp::setup(){
    /* camera setup */
//    vidGrabber.setDeviceID(0);
//    vidGrabber.setDesiredFrameRate(60);
//    vidGrabber.initGrabber(camWidth, camHeight);
//    videoInverted.allocate(camWidth, camHeight, OF_PIXELS_RGB);
//    videoTexture.allocate(videoInverted);
//    ofSetVerticalSync(true);

    image1.load("/Applications/of_v0.9.3_osx_release/apps/myApps/16.12.9/bin/data/image.jpg"); //이미지 불러들이기
    img_object = Mat(image1.getHeight(), image1.getWidth(), CV_8UC3, image1.getPixels().getData()); //Mat에 이미지 저장
    cvtColor(img_object, img_object, CV_BGR2GRAY); //그레이스케일 이미지로 변환

    /* video */
    String OutputFile = "/Applications/of_v0.9.3_osx_release/apps/myApps/16.12.9/bin/data/save.mov";
    VideoCapture vc("/Applications/of_v0.9.3_osx_release/apps/myApps/16.12.9/bin/data/video.mov");
    
    int w = vc.get(CV_CAP_PROP_FRAME_WIDTH);
    int h = vc.get(CV_CAP_PROP_FRAME_HEIGHT);
    int nr_superpixels = 100;
    int nc = 50;
    double step = sqrt((w * h) / (double) nr_superpixels);
    
    //-- 결과 비디오 저장
    float out_fps = vc.get(CV_CAP_PROP_FPS);
    int out_cols = w;
    int out_rows = h;
    Size2i out_frameSize(out_cols, out_rows);
    int out_codec = CV_FOURCC('m','p','4','v'); //mov format
    VideoWriter OutVideo(OutputFile, out_codec, out_fps, out_frameSize);
    
    Mat frame, lab_image;
    int nframe = vc.get(CV_CAP_PROP_FRAME_COUNT);
    vector<Mat> video;
    for(int k=0; k<nframe; k++)
    {
        vc.set(CV_CAP_PROP_POS_FRAMES, k);
        vc >> frame;
        myFeatureDetector(frame, img_object);
        //OutVideo << save; //결과 비디오 저장
        if(waitKey(5)==27)
        {
            break;
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    /* camera update */
//    vidGrabber.update();
//    if(vidGrabber.isFrameNew()){
//        ofPixels & pixels = vidGrabber.getPixels();
//        for(int i = 0; i < pixels.size(); i++){
//        }
//    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    /* camera draw */
//    ofSetHexColor(0xffffff);
//    vidGrabber.draw(20, 20);
//    videoTexture.draw(20 + camWidth, 20, camWidth, camHeight);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}



