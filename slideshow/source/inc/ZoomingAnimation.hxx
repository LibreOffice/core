#include <vector>
#include <unoview.hxx>
#include <slideshow/FrameSynchronization.hxx>
namespace slideshow::internal {
    struct ViewEntry
    {
        explicit ViewEntry( const UnoViewSharedPtr& rView ) : mpView( rView ){}
        /// The view this entry is for
        UnoViewSharedPtr                              mpView;
        /// outgoing slide sprite
        std::shared_ptr<cppcanvas::CustomSprite>    mpOutSprite;
        /// incoming slide sprite
        std::shared_ptr<cppcanvas::CustomSprite>    mpInSprite;
        /// outgoing slide bitmap
        mutable SlideBitmapSharedPtr                  mpLeavingBitmap;
        /// incoming slide bitmap
        mutable SlideBitmapSharedPtr                  mpEnteringBitmap;

        // for algo access
        const UnoViewSharedPtr& getView() const { return mpView; }
    };
    typedef ::std::vector<ViewEntry> ViewsVec;
    class ZoomingAnimation : public NumberAnimation
    {
    public:
        ZoomingAnimation( const SdrObject* zoomTargetObject, const double fDuration,
                         UnoViewContainer* mrViewContainer, ::std::shared_ptr<Slide> mpCurrentSlide,
                          EventMultiplexer* maEventMultiplexer, ScreenUpdater* maScreenUpdater,
                         FrameSynchronization* maFrameSynchronization);
        ~ZoomingAnimation();
        // Animation interface
        void prefetch();
        void start(const AnimatableShapeSharedPtr& /* rShape */, const ShapeAttributeLayerSharedPtr& /* rAttrLayer */);
        void end();
        void end_();
        // NumberAnimation interface
        bool operator()(double nValue);
        double getUnderlyingValue() const;
        UnoViewContainer* mrViewContainer;
    private:
        ScreenUpdater* maScreenUpdater;
        FrameSynchronization* maFrameSynchronization;
        EventMultiplexer* maEventMultiplexer;
        basegfx::B2DSize slideSizeAtAnimationStart;
        const SdrObject* zoomTargetObject;
        bool mbAnimationStarted;
        double mfDuration;
        double mfPreviousElapsedTime;
        ::std::shared_ptr<Slide> mpCurrentSlide;
    };
}