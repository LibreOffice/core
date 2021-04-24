#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/uno/Reference.h>

#include <cppcanvas/customsprite.hxx>
#include <transitionfactory.hxx>
#include <tools.hxx>
#include <memory>
#include <math.h>
#include <svx/svdobj.hxx>

#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>
#include <ZoomingAnimation.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <slide.hxx>
#include <iostream>
namespace slideshow::internal
{
using namespace com::sun::star;
double ZoomingAnimation::getUnderlyingValue() const { return 0.0; }
ZoomingAnimation::ZoomingAnimation(ZoomingParameters zp)
    : targetPosition(zp._targetPosition)
    , mbAnimationStarted(false)
    , mfDuration(zp._fDuration)
    , mfPreviousElapsedTime(0.00f)
    , mrViewContainer(zp._mrViewContainer)
    , maScreenUpdater(zp._maScreenUpdater)
    , mpCurrentSlide(zp._mpCurrentSlide)
    , maEventMultiplexer(zp._maEventMultiplexer)
    , maFrameSynchronization(zp._maFrameSynchronization)
    , currentScreenSize(zp._currentScreenSize)
    , isFirstTransformation(true)
    , isZooming(false)
    , requestedZoomLevel(zp._requestedZoomLevel)
{
    lastTimePoint = 0.0;
    differences.clear();
    startingDiffs.clear();
    differenceSumX = 0;
    differenceSumY = 0;
    isFirstTransformation = zp.isFirstZoom;
}
ZoomingAnimation::~ZoomingAnimation() { end_(); }

void ZoomingAnimation::prefetch() {}
using namespace basegfx;
///
/// \param targetPosition Target position uses units of rects. A rect is 100th of a millimeter
/// \param currentTransformation is acquired through pView->getCanvas()->getTransformation();
/// \param currentScreenSize
/// \return
basegfx::B2DPoint createDifference(basegfx::B2DPoint targetPosition,
                                   basegfx::B2DHomMatrix currentTransformation,
                                   basegfx::B2DVector currentScreenSize, std::ofstream& debugFile,
                                   bool isZooming, bool isFirstTransformation,
                                   double requestedZoomLevel)
{
    const B2DPoint screenTranslationPixelValues(
        B2DPoint(0, 0)); //currentTransformation * B2DPoint());
    B2DPoint aDifference(screenTranslationPixelValues);
    double x = targetPosition.getX() / 28000 * currentScreenSize.getX();
    aDifference -= B2DPoint(x, targetPosition.getY() / 15750 * currentScreenSize.getY() );
    B2DPoint middlePoint(B2DPoint(0, 0));
    if (isFirstTransformation)
    {
        middlePoint = B2DPoint((currentScreenSize.getX() * (requestedZoomLevel) - currentScreenSize.getX()) / 2, currentScreenSize.getY() * (requestedZoomLevel) / 2);
    }
    else
    {
//        middlePoint += B2DPoint(currentScreenSize.getX() * (1/requestedZoomLevel) / 2, currentScreenSize.getY() * (1/requestedZoomLevel) / 2);
//        double fX = -((currentScreenSize.getX() * (1 + requestedZoomLevel)) - currentScreenSize.getX()*(1+requestedZoomLevel)) / 2;
//        double fX = -(currentScreenSize.getX() * 1 / requestedZoomLevel);
//        double y = -((currentScreenSize.getY() * (1 + requestedZoomLevel)) - currentScreenSize.getY()*(1+requestedZoomLevel)) / 2;
//        double y = -(currentScreenSize.getY() * 1 / requestedZoomLevel);
    }
    if(requestedZoomLevel != 1) {
        SAL_DEBUG("Requested zoom level: " << requestedZoomLevel);
    }

    debugFile << "targetPosition.getX()=;" << targetPosition.getX() << ";"
              << "targetPosition.getY()=;" << targetPosition.getY() << ";" << std::endl;
    debugFile << "targetPosition.getX() / currentScreenSize.getX()=;"
              << targetPosition.getX() / currentScreenSize.getX() << ";"
              << "targetPosition.getY() / currentScreenSize.getX()=;"
              << targetPosition.getY() / currentScreenSize.getX() << ";" << std::endl;
    debugFile << "currentScreenSize.getX()=;" << currentScreenSize.getX() << ";"
              << "currentScreenSize.getY()=;" << currentScreenSize.getY() << ";" << std::endl;
    aDifference += middlePoint;
    /*{
        SAL_DEBUG("realTranslationOffset width " << realTranslationOffset.Width);
        SAL_DEBUG("realTranslationOffset height " << realTranslationOffset.Height);
        SAL_DEBUG("xValue " << xValue);
        SAL_DEBUG("yValue " << yValue);
        SAL_DEBUG("point.getX(): " << point.getX());
        SAL_DEBUG("point.getY(): " << point.getY());
        differenceScreenSides = (double) screenViewArea.Width / (double) screenViewArea.Height;
        SAL_DEBUG("differenceScreenSides " << differenceScreenSides);
        SAL_DEBUG("aOutputPosPixel: " << screenTranslationPixelValues);
        SAL_DEBUG("aDiff " << aDifference);
        SAL_DEBUG("--------");
    }*/
    return aDifference;
}

void ZoomingAnimation::start(const AnimatableShapeSharedPtr&, const ShapeAttributeLayerSharedPtr&)
{
    debugFile.open("logike02.csv");
    debugFile << "t;"
              << "timeTaken;"
              << "distanceTaken;"
              << "screenPositionX;"
              << "screenPositionY;"
              << "distanceTakenX;"
              << "distanceTakenY;" << std::endl;
    debugFile << "t;"
              << "screenSizeX=;" << currentScreenSize.getX() << ";"
              << "screenSizeY=;" << currentScreenSize.getY() << ";" << std::endl;
    maFrameSynchronization->Activate();
    if (!mbAnimationStarted)
    {
        mbAnimationStarted = true;
    }
    else
    {
        //        return;
    }
    for (const auto& pView : *mrViewContainer)
    {
        const awt::Rectangle rect = pView->getUnoView()->getCanvasArea();
        slideSizeAtAnimationStart = basegfx::B2DSize(rect.Width, rect.Height);
        debugFile << "screenSizeX=;" << rect.Width << ";"
                  << "screenSizeY=;" << rect.Height << ";" << std::endl;
        auto diff
            = createDifference(targetPosition, pView->getCanvas()->getTransformation(),
                               slideSizeAtAnimationStart, debugFile, false, isFirstTransformation,
                               requestedZoomLevel);
        debugFile << "diffX=;" << diff.getX() << ";"
                  << "diffY=;" << diff.getY() << ";" << std::endl;
        differences.push_back(diff);
    }
    /*{
        SAL_DEBUG("CurrentScreenSize" << currentScreenSize);
        SAL_DEBUG("slideSize" << slideSizeAtAnimationStart);
        SAL_DEBUG("targetCenterX: " << targetCenterX);
        SAL_DEBUG("targetCenterY: " << targetCenterY);
        SAL_DEBUG("targetCenterXConverted " << targetCenterXConverted);
        SAL_DEBUG("targetCenterYConverted " << targetCenterYConverted);
    }*/
    //    zoomingParameters.maFrameSynchronization->Synchronize();
}

B2ISize getEnteringSlideSizePixel(const UnoViewSharedPtr& pView,
                                  ::std::shared_ptr<Slide> mpCurrentSlide)
{
    return getSlideSizePixel(basegfx::B2DSize(mpCurrentSlide->getSlideSize()), pView);
}
B2DPoint getTakenTranslation(double timeTaken, B2DPoint difference)
{
    return B2DPoint(difference.getX() * timeTaken, difference.getY() * timeTaken);
}
long double getTakenZoom(double timeTaken, double requestedZoom){
    return timeTaken * requestedZoom;
}
/*I expect t to range from 0 to 1 where both are inclusive.*/
bool ZoomingAnimation::operator()(double t)
{
    if (zoomTargetObject == nullptr)
    {
        return false;
    }
    // TODO: t can actually start from -1 and this doesn't allow for it
    double timeTaken = t - lastTimePoint;

    /*if (abs(t - 0.5) < 0.03)
    {
        SAL_DEBUG("MIDPOINT");
        SAL_DEBUG("--------");
        SAL_DEBUG("time " << t);
        SAL_DEBUG("differences[0] " << differences[0]);
        SAL_DEBUG("--------");
    }*/
    lastTimePoint = t;
    int count = 0;
    int count2 = 0;
    int countScreenSizes = 0;
    for (const auto& pView : *mrViewContainer)
    {
        B2DPoint takenDiff;
        auto difference = differences[count++];
        basegfx::B2DHomMatrix transformation = pView->getCanvas()->getTransformation();
        /*auto aViewArea = pView->getUnoView()->getCanvasArea();
            SAL_DEBUG("transformation: " << transformation);
            SAL_DEBUG("dty " << differenceTakenY);
            SAL_DEBUG("dtx " << differenceTakenX);
            transformation.scale(1 - differenceTakenX + 0.5, 1 - differenceTakenX + 0.5);
            difference -= differenceTaken;//-differenceTaken.get(0, 2), -differenceTaken.get(1, 2));
            a portion of the difference between two objects has been moved in time taken.
        const geometry::IntegerSize2D realTranslationOffset ( pView->getTranslationOffset() );
        const auto val = basegfx::B2DPoint(
            realTranslationOffset.Width + ((aViewArea.Width - aViewArea.X) - 2 * realTranslationOffset.Width) * maPos.X,
            realTranslationOffset.Height + ((aViewArea.Height - aViewArea.Y) - 2 * realTranslationOffset.Height) * maPos.Y);*/
        long double zoomTaken = getTakenZoom(timeTaken, requestedZoomLevel);
        takenDiff = getTakenTranslation(timeTaken, difference);
        // Keeping translation correct when scaling
        double convertedDiffX = takenDiff.getX();
        double convertedDiffY = takenDiff.getY();

        long double zoomTakenAdded = 1.0 + zoomTaken;
        takenDiff.setX(convertedDiffX);
        takenDiff.setY(convertedDiffY);
        if(!isFirstTransformation){
//            takenDiff -= B2DPoint(-((currentScreenSize.getX() * (timeTaken)) - currentScreenSize.getX()) / 2, -((currentScreenSize.getY() * (timeTaken)) - currentScreenSize.getY()) / 2);
//            transformation.translate(currentScreenSize.getX() * zoomTaken / 2, currentScreenSize.getY() * zoomTaken / 2);
            transformation.translate(takenDiff);
            transformation.scale(zoomTakenAdded, zoomTakenAdded);
        } else {
            transformation.translate(takenDiff);
            transformation.scale(zoomTakenAdded, zoomTakenAdded);
        }
//        transformation.translate(currentScreenSize*zoomTakenAdded);
        pView->getCanvas()->setTransformation(transformation);
        debugFile << t << ";" << timeTaken << ";" << takenDiff << ";" << transformation.get(0, 2)
                  << ";" << transformation.get(1, 2) << ";" << takenDiff.getX() << ";"
                  << takenDiff.getY() << std::endl;
    }
    /*if (abs(t - 0.9) < 0.03)
    {
        SAL_DEBUG("NEAR ENDPOINT");
        SAL_DEBUG("--------");
        SAL_DEBUG("time " << t);
        SAL_DEBUG("differences[0] " << differences[0]);
        SAL_DEBUG("--------");
    }*/
    maEventMultiplexer->notifyViewsChanged();
    return true;
}
// NumberAnimation interface
void ZoomingAnimation::end() { end_(); }
void ZoomingAnimation::end_()
{
    debugFile.close();
    using namespace basegfx;
    B2DTuple rScale, rTransform;
    double _;
    maFrameSynchronization->Deactivate();
    if (!mbAnimationStarted)
        return;
    mbAnimationStarted = false;
}
}