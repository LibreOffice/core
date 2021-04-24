#include <vector>
#include <unoview.hxx>
#include <slideshow/FrameSynchronization.hxx>
#include <fstream>
namespace slideshow::internal
{
// Used underscores because this is a temporary object for constructor use
struct ZoomingParameters
{
    // This is saved from void DrawViewShell::AddToTransitionList(SfxRequest& rReq){
    const basegfx::B2DPoint _targetPosition;
    // Duration of the zooming animation
    const double _fDuration;
    // A slide is a view, they are contained in this
    UnoViewContainer* _mrViewContainer;
    // A pointer to the current slide
    ::std::shared_ptr<Slide> _mpCurrentSlide;
    // An event needs to be fired when a view is updated to display in on the screen
    EventMultiplexer* _maEventMultiplexer;
    // This is no longer used, wasn't necessary
    ScreenUpdater* _maScreenUpdater;
    // Frame synchronisation
    FrameSynchronization* _maFrameSynchronization;
    // Current screen size is saved to aim properly at screen objects
    basegfx::B2DVector _currentScreenSize;
    bool isFirstZoom;
    long double _requestedZoomLevel;
};
struct ViewEntry
{
    explicit ViewEntry(const UnoViewSharedPtr& rView)
        : mpView(rView)
    {
    }
    /// The view this entry is for
    UnoViewSharedPtr mpView;
    /// outgoing slide sprite
    std::shared_ptr<cppcanvas::CustomSprite> mpOutSprite;
    /// incoming slide sprite
    std::shared_ptr<cppcanvas::CustomSprite> mpInSprite;
    /// outgoing slide bitmap
    mutable SlideBitmapSharedPtr mpLeavingBitmap;
    /// incoming slide bitmap
    mutable SlideBitmapSharedPtr mpEnteringBitmap;

    // for algo access
    const UnoViewSharedPtr& getView() const { return mpView; }
};
typedef ::std::vector<ViewEntry> ViewsVec;
class ZoomingAnimation : public NumberAnimation
{
public:
    ZoomingAnimation(ZoomingParameters zp);
    ~ZoomingAnimation();
    // Animation interface
    void prefetch();
    void start(const AnimatableShapeSharedPtr& /* rShape */,
               const ShapeAttributeLayerSharedPtr& /* rAttrLayer */);
    void end();
    void end_();
    // NumberAnimation interface
    bool operator()(double nValue);
    double getUnderlyingValue() const;
    UnoViewContainer* mrViewContainer;

private:
    // difference sums are only used for debugging purposes
    bool isZooming;
    std::ofstream debugFile;
    double differenceSumX;
    double differenceSumY;
    bool isFirstTransformation;
    double differenceScreenSides;
    basegfx::B2DPoint targetPosition;
    basegfx::B2DVector currentScreenSize;
    double lastTimePoint;
    std::vector<basegfx::B2DPoint> screenSizesPerView;
    std::vector<basegfx::B2DPoint> startingDiffs;
    std::vector<basegfx::B2DPoint> differences;
    ScreenUpdater* maScreenUpdater;
    FrameSynchronization* maFrameSynchronization;
    EventMultiplexer* maEventMultiplexer;
    basegfx::B2DSize slideSizeAtAnimationStart;
    const SdrObject* zoomTargetObject;
    bool mbAnimationStarted;
    double mfDuration;
    double mfPreviousElapsedTime;
    ::std::shared_ptr<Slide> mpCurrentSlide;
    double requestedZoomLevel;
};
}