# 16.12.9
* Feature Detection
SURF와 ORB를 이용하여 이미지 특성 매칭을 하였다.
두 이미지에서 특성을 비교하여, 일정범위 내에서 동일하다고 생각되는 키포인트 부분을 선으로 연결하여 매칭시킨다.

image와 video 소스를 활용하여, 매칭의 변화되는 모습을 확인해볼 수 있도록 한다.
두 이미지의 특성을 비교하는 단순한 방법은 전수조사 방법으로 BF(Brute-Force)매칭이 있다.
이것은 만약 두 이미지 A,B가 있다고 한다면, 이미지A에서 하나의 특성 디스크립터를 취하고, 이 디스크립터를 이미지B의 모든 특성 디스크립터와 거리 계산 방법을 통해 하나하나 비교하고, 이렇게 해서 나온 값 중 가장 비슷한 값을 리턴하는 방식이다. opencv에서는 BFMatcher라는 메소드를 제공하는데, 여기서 설정하는 normType은 BF매칭에 사용할 거리 계산 방법을 지정한다. 보통 SIFT나 SURF는 NORM_L2를 사용하고, ORB,BRIEF는 NORM_HAMMING을 사용하는 것이 좋다고 한다. crossCheck는 두 이미지 양방향으로 디스크립터를 비교하는 방법이다.
결과적으로 FLANN Matcher보다 BF Matcher가 더욱 빠르고, 정확한 결과값을 출력하였다.

* 참고사이트
opencv feature detection 소스코드를 참고하였다.
http://docs.opencv.org/2.4/doc/tutorials/features2d/feature_homography/feature_homography.html

이미지 매칭 방법에 대한 세부속성을 참고하였다.
http://m.blog.naver.com/samsjang/220657424078
