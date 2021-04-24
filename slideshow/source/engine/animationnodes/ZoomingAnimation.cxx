#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
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
namespace slideshow::internal {
    ZoomingAnimation::~ZoomingAnimation()
    {
        end_();
    }

    // Animation interface

    void ZoomingAnimation::prefetch()
    {}
    

    void ZoomingAnimation::start( const AnimatableShapeSharedPtr&     /* rShape */,
                            const ShapeAttributeLayerSharedPtr& /* rAttrLayer */ )
    {
        SAL_DEBUG("Animation started");
        maFrameSynchronization->Activate();
        slideSizeAtAnimationStart = basegfx::B2DSize(mpCurrentSlide->getSlideSize());
        if( !mbAnimationStarted )
        {
            mbAnimationStarted = true;
        }
    }

    
    // NumberAnimation interface
    void ZoomingAnimation::end() { end_(); } 
    void ZoomingAnimation::end_()
    {
        maFrameSynchronization->Deactivate();
        if( !mbAnimationStarted )
            return;
        mbAnimationStarted = false;
    }
    cppcanvas::CustomSpriteSharedPtr createSprite(UnoViewSharedPtr const & pView, basegfx::B2DSize const & rSpriteSize, double nPrio ) {
        // TODO(P2): change to bitmapsprite once that's working
        const cppcanvas::CustomSpriteSharedPtr pSprite(pView->createSprite( rSpriteSize,nPrio ));
        // alpha default is 0.0, which seems to be
        // a bad idea when viewing content...
        pSprite->setAlpha( 1.0 );
        // if (mbSpritesVisible)
        pSprite->show();
        return pSprite;
    }
    ::basegfx::B2ISize getEnteringSlideSizePixel(const UnoViewSharedPtr& pView, ::std::shared_ptr<Slide> mpCurrentSlide)
    {
        return getSlideSizePixel(basegfx::B2DSize(mpCurrentSlide->getSlideSize()), pView);
    }
    ::cppcanvas::CustomSpriteSharedPtr createSprite(ViewEntry& rEntry, ::std::shared_ptr<Slide> mpCurrentSlide)
    {
            // create entering sprite:
            const basegfx::B2ISize enteringSlideSizePixel(getSlideSizePixel( basegfx::B2DSize( mpCurrentSlide->getSlideSize() ), rEntry.mpView));
            return createSprite(rEntry.mpView,basegfx::B2DSize( enteringSlideSizePixel ),101);
    }
    using namespace com::sun::star;
    bool ZoomingAnimation::operator()( double t )
    {
        SAL_DEBUG("Y");
        ViewsVec maViewData;
        for (const auto& pView : *mrViewContainer)
        {
            //pView->getUnoView()->acquire();
            pView->setViewSize(slideSizeAtAnimationStart / 2 * (t + 1));
            //pView->getUnoView()->release();
            pView->paintScreen();
            pView->updateScreen();
            /*try
            {
                uno::Reference<presentation::XSlideShowView> xView(pView->getUnoView(), uno::UNO_SET_THROW);
                uno::Reference<util::XUpdatable> const xUpdatable(
                    xView->getCanvas(), uno::UNO_QUERY);
                if (xUpdatable.is()) // not supported in PresenterCanvas
                {
                    xUpdatable->update();
                }
            }
            catch( uno::RuntimeException& )
            {
                throw;
            }
            catch( uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION( "slideshow", "" );
            }*/


//            maEventMultiplexer->notifyViewsChanged();
//            maViewData.emplace_back(pView);
        }
/*        const std::size_t nEntries = maViewData.size();
        for(::std::size_t i=0; i<nEntries; ++i)
        {
            ViewEntry& rViewEntry(maViewData[i]);
            const double helper = t*5+1;
            rViewEntry.mpView->getCanvas()->setTransformation(
                basegfx::B2DHomMatrix(
                    helper,helper,helper,helper,helper,helper));
            const double sizeConst = 10;
            const ::cppcanvas::CanvasSharedPtr rDestinationCanvas(rViewEntry.mpView->getCanvas());
            ::basegfx::B2DHomMatrix aViewTransform(rDestinationCanvas->getTransformation());
            aViewTransform.scale(10 + t * sizeConst, 10 + t * sizeConst);
            ::cppcanvas::CustomSpriteSharedPtr rSprite(createSprite(rViewEntry, mpCurrentSlide));
            //
            const ::basegfx::B2DPoint aPageOrigin(aViewTransform * ::basegfx::B2DPoint());
            rSprite->movePixel(aPageOrigin + ((t - 1.0) * ::basegfx::B2DSize(getEnteringSlideSizePixel(rViewEntry.mpView, mpCurrentSlide))));
            rSprite->transform(aViewTransform);
        }*/
        maFrameSynchronization->Synchronize();
        maEventMultiplexer->notifyViewsChanged();
//        maEventMultiplexer->notifyNextEffect();
//        maEventMultiplexer->notifyPauseMode(false);
        maEventMultiplexer->notifySlideStartEvent();
        //maScreenUpdater->commitUpdates();

        return true;
    }

    double ZoomingAnimation::getUnderlyingValue() const
    {
        return 0.0;
    }
    ZoomingAnimation::ZoomingAnimation(const SdrObject* zoomTargetObject, const double fDuration,
                                       UnoViewContainer* mrViewContainer,
                                       ::std::shared_ptr<Slide> mpCurrentSlide,
                                       EventMultiplexer* maEventMultiplexer, ScreenUpdater* maScreenUpdater,
                                       FrameSynchronization* maFrameSynchronization) :
            zoomTargetObject(zoomTargetObject),
            mbAnimationStarted( false ),
            mfDuration(fDuration),
            mfPreviousElapsedTime(0.00f),
            mrViewContainer(mrViewContainer),
            mpCurrentSlide(mpCurrentSlide),
            maEventMultiplexer(maEventMultiplexer),
            maScreenUpdater(maScreenUpdater),
            maFrameSynchronization(maFrameSynchronization)
            {}
}