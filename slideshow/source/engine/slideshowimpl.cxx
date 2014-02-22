/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/exc_hlp.hxx>

#include <comphelper/anytostring.hxx>
#include <comphelper/make_shared_from_uno.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/optional.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/namecontainer.hxx>

#include <cppcanvas/spritecanvas.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>

#include <tools/debug.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/canvastools.hxx>

#include <vcl/font.hxx>
#include "rtl/ref.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include "com/sun/star/uno/Reference.hxx"
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>

#include "unoviewcontainer.hxx"
#include "transitionfactory.hxx"
#include "eventmultiplexer.hxx"
#include "usereventqueue.hxx"
#include "eventqueue.hxx"
#include "cursormanager.hxx"
#include "slideshowcontext.hxx"
#include "activitiesqueue.hxx"
#include "activitiesfactory.hxx"
#include "interruptabledelayevent.hxx"
#include "slide.hxx"
#include "shapemaps.hxx"
#include "slideview.hxx"
#include "tools.hxx"
#include "unoview.hxx"
#include "slidebitmap.hxx"
#include "rehearsetimingsactivity.hxx"
#include "waitsymbol.hxx"
#include "effectrewinder.hxx"
#include "framerate.hxx"
#include "pointersymbol.hxx"

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include <map>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <iostream>

using namespace com::sun::star;
using namespace ::slideshow::internal;

namespace {

/** During animations the update() method tells its caller to call it as
    soon as possible.  This gives us more time to render the next frame and
    still maintain a steady frame rate.  This class is responsible for
    synchronizing the display of new frames and thus keeping the frame rate
    steady.
*/
class FrameSynchronization
{
public:
    /** Create new object with a predefined duration between two frames.
        @param nFrameDuration
            The preferred duration between the display of two frames in
            seconds.
    */
    FrameSynchronization (const double nFrameDuration);

    /** Set the current time as the time at which the current frame is
        displayed.  From this the target time of the next frame is derived.
    */
    void MarkCurrentFrame (void);

    /** When there is time left until the next frame is due then wait.
        Otherwise return without delay.
    */
    void Synchronize (void);

    /** Activate frame synchronization when an animation is active and
        frames are to be displayed in a steady rate.  While active
        Synchronize() will wait until the frame duration time has passed.
    */
    void Activate (void);

    /** Deactivate frame sychronization when no animation is active and the
        time between frames depends on user actions and other external
        sources.  While deactivated Synchronize() will return without delay.
    */
    void Deactivate (void);

private:
    /** The timer that is used for synchronization is independent from the
        one used by SlideShowImpl: it is not paused or modified by
        animations.
    */
    canvas::tools::ElapsedTime maTimer;
    /** Time between the display of frames.  Enforced only when mbIsActive
        is <TRUE/>.
    */
    const double mnFrameDuration;
    /** Time (of maTimer) when the next frame shall be displayed.
        Synchronize() will wait until this time.
    */
    double mnNextFrameTargetTime;
    /** Synchronize() will wait only when this flag is <TRUE/>.  Otherwise
        it returns immediately.
    */
    bool mbIsActive;
};

/******************************************************************************

   SlideShowImpl

   This class encapsulates the slideshow presentation viewer.

   With an instance of this class, it is possible to statically
   and dynamically show a presentation, as defined by the
   constructor-provided draw model (represented by a sequence
   of ::com::sun::star::drawing::XDrawPage objects).

   It is possible to show the presentation on multiple views
   simultaneously (e.g. for a multi-monitor setup). Since this
   class also relies on user interaction, the corresponding
   XSlideShowView interface provides means to register some UI
   event listeners (mostly borrowed from awt::XWindow interface).

   Since currently (mid 2004), OOo isn't very well suited to
   multi-threaded rendering, this class relies on <em>very
   frequent</em> external update() calls, which will render the
   next frame of animations. This works as follows: after the
   displaySlide() has been successfully called (which setup and
   starts an actual slide show), the update() method must be
   called until it returns false.
   Effectively, this puts the burden of providing
   concurrency to the clients of this class, which, as noted
   above, is currently unavoidable with the current state of
   affairs (I've actually tried threading here, but failed
   miserably when using the VCL canvas as the render backend -
   deadlocked).

 ******************************************************************************/

typedef cppu::WeakComponentImplHelper1<presentation::XSlideShow> SlideShowImplBase;

typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;


typedef ::std::map< ::com::sun::star::uno::Reference<
                                    ::com::sun::star::drawing::XDrawPage>,
                                    PolyPolygonVector>  PolygonMap;

class SlideShowImpl : private cppu::BaseMutex,
                      public CursorManager,
                      public SlideShowImplBase
{
public:
    explicit SlideShowImpl(
        uno::Reference<uno::XComponentContext> const& xContext );

    /** Notify that the transition phase of the current slide
        has ended.

        The life of a slide has three phases: the transition
        phase, when the previous slide vanishes, and the
        current slide becomes visible, the shape animation
        phase, when shape effects are running, and the phase
        after the last shape animation has ended, but before
        the next slide transition starts.

        This method notifies the end of the first phase.

        @param bPaintSlide
        When true, Slide::show() is passed a true as well, denoting
        explicit paint of slide content. Pass false here, if e.g. a
        slide transition has already rendered the initial slide image.
    */
    void notifySlideTransitionEnded( bool bPaintSlide );

    /** Notify that the shape animation phase of the current slide
        has ended.

        The life of a slide has three phases: the transition
        phase, when the previous slide vanishes, and the
        current slide becomes visible, the shape animation
        phase, when shape effects are running, and the phase
        after the last shape animation has ended, but before
        the next slide transition starts.

        This method notifies the end of the second phase.
    */
    void notifySlideAnimationsEnded();

    /** Notify that the slide has ended.

        The life of a slide has three phases: the transition
        phase, when the previous slide vanishes, and the
        current slide becomes visible, the shape animation
        phase, when shape effects are running, and the phase
        after the last shape animation has ended, but before
        the next slide transition starts.

        This method notifies the end of the third phase.
    */
    void notifySlideEnded (const bool bReverse);

    /** Notification from eventmultiplexer that a hyperlink
        has been clicked.
    */
    bool notifyHyperLinkClicked( OUString const& hyperLink );

    /** Notification from eventmultiplexer that an animation event has occoured.
        This will be forewarded to all registered XSlideShowListener
     */
    bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode );

private:
    
    virtual sal_Bool SAL_CALL nextEffect() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL previousEffect() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL startShapeActivity(
        uno::Reference<drawing::XShape> const& xShape )
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL stopShapeActivity(
        uno::Reference<drawing::XShape> const& xShape )
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL pause( sal_Bool bPauseShow )
        throw (uno::RuntimeException);
    virtual uno::Reference<drawing::XDrawPage> SAL_CALL getCurrentSlide()
        throw (uno::RuntimeException);
    virtual void SAL_CALL displaySlide(
        uno::Reference<drawing::XDrawPage> const& xSlide,
        uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
        uno::Reference<animations::XAnimationNode> const& xRootNode,
        uno::Sequence<beans::PropertyValue> const& rProperties )
        throw (uno::RuntimeException);
    virtual void SAL_CALL registerUserPaintPolygons( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xDocFactory ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setProperty(
        beans::PropertyValue const& rProperty ) throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL addView(
        uno::Reference<presentation::XSlideShowView> const& xView )
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL removeView(
        uno::Reference<presentation::XSlideShowView> const& xView )
        throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL update( double & nNextTimeout )
        throw (uno::RuntimeException);
    virtual void SAL_CALL addSlideShowListener(
        uno::Reference<presentation::XSlideShowListener> const& xListener )
        throw (uno::RuntimeException);
    virtual void SAL_CALL removeSlideShowListener(
        uno::Reference<presentation::XSlideShowListener> const& xListener )
        throw (uno::RuntimeException);
    virtual void SAL_CALL addShapeEventListener(
        uno::Reference<presentation::XShapeEventListener> const& xListener,
        uno::Reference<drawing::XShape> const& xShape )
        throw (uno::RuntimeException);
    virtual void SAL_CALL removeShapeEventListener(
        uno::Reference<presentation::XShapeEventListener> const& xListener,
        uno::Reference<drawing::XShape> const& xShape )
        throw (uno::RuntimeException);
    virtual void SAL_CALL setShapeCursor(
        uno::Reference<drawing::XShape> const& xShape, sal_Int16 nPointerShape )
        throw (uno::RuntimeException);

    
    

    virtual bool requestCursor( sal_Int16 nCursorShape );
    virtual void resetCursor();

    /** This is somewhat similar to displaySlide when called for the current
        slide.  It has been simplified to take advantage of that no slide
        change takes place.  Furthermore it does not show the slide
        transition.
    */
    void redisplayCurrentSlide (void);

protected:
    
    virtual void SAL_CALL disposing();

    bool isDisposed() const
    {
        return (rBHelper.bDisposed || rBHelper.bInDispose);
    }

private:
    struct SeparateListenerImpl; friend struct SeparateListenerImpl;
    class PrefetchPropertiesFunc; friend class PrefetchPropertiesFunc;

    
    void stopShow();

    
    PolygonMap::iterator findPolygons( uno::Reference<drawing::XDrawPage> const& xDrawPage);

    
    SlideSharedPtr makeSlide(
        uno::Reference<drawing::XDrawPage> const& xDrawPage,
        uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
        uno::Reference<animations::XAnimationNode> const& xRootNode );

    
    static bool matches(
        SlideSharedPtr const& pSlide,
        uno::Reference<drawing::XDrawPage> const& xSlide,
        uno::Reference<animations::XAnimationNode> const& xNode )
    {
        if (pSlide)
            return (pSlide->getXDrawPage() == xSlide &&
                    pSlide->getXAnimationNode() == xNode);
        else
            return (!xSlide.is() && !xNode.is());
    }

    
    SoundPlayerSharedPtr resetSlideTransitionSound(
        uno::Any const& url = uno::Any(), bool bLoopSound = false );

    
    void stopSlideTransitionSound();

    /** Prepare a slide transition

        This method registers all necessary events and
        activities for a slide transition.

        @return the slide change activity, or NULL for no transition effect
    */
    ActivitySharedPtr createSlideTransition(
        const uno::Reference< drawing::XDrawPage >&    xDrawPage,
        const SlideSharedPtr&                          rLeavingSlide,
        const SlideSharedPtr&                          rEnteringSlide,
        const EventSharedPtr&                          rTransitionEndEvent );

    /** Request/release the wait symbol.  The wait symbol is displayed when
        there are more requests then releases.  Locking the wait symbol
        helps to avoid intermediate repaints.

        Do not call this method directly.  Use WaitSymbolLock instead.
    */
    void requestWaitSymbol (void);
    void releaseWaitSymbol (void);

    class WaitSymbolLock {public:
        WaitSymbolLock(SlideShowImpl& rSlideShowImpl) : mrSlideShowImpl(rSlideShowImpl)
            { mrSlideShowImpl.requestWaitSymbol(); }
        ~WaitSymbolLock(void)
            { mrSlideShowImpl.releaseWaitSymbol(); }
    private: SlideShowImpl& mrSlideShowImpl;
    };

    
    sal_Int16 calcActiveCursor( sal_Int16 nCursorShape ) const;

    /** This method is called asynchronously to finish the rewinding of an
        effect to the previous slide that was initiated earlier.
    */
    void rewindEffectToPreviousSlide (void);

    
    UnoViewContainer                        maViewContainer;

    
    cppu::OInterfaceContainerHelper         maListenerContainer;

    
    ShapeEventListenerMap                   maShapeEventListeners;
    
    ShapeCursorMap                          maShapeCursors;

    
    PolygonMap                              maPolygons;

    boost::optional<RGBColor>               maUserPaintColor;

    double                                  maUserPaintStrokeWidth;

    
    boost::optional<bool>           maEraseAllInk;
    boost::optional<bool>           maSwitchPenMode;
    boost::optional<bool>           maSwitchEraserMode;
    boost::optional<sal_Int32>          maEraseInk;
    

    boost::shared_ptr<canvas::tools::ElapsedTime> mpPresTimer;
    ScreenUpdater                           maScreenUpdater;
    EventQueue                              maEventQueue;
    EventMultiplexer                        maEventMultiplexer;
    ActivitiesQueue                         maActivitiesQueue;
    UserEventQueue                          maUserEventQueue;
    SubsettableShapeManagerSharedPtr        mpDummyPtr;

    boost::shared_ptr<SeparateListenerImpl> mpListener;

    boost::shared_ptr<RehearseTimingsActivity> mpRehearseTimingsActivity;
    boost::shared_ptr<WaitSymbol>           mpWaitSymbol;

    boost::shared_ptr<PointerSymbol>        mpPointerSymbol;

    
    SoundPlayerSharedPtr                    mpCurrentSlideTransitionSound;

    uno::Reference<uno::XComponentContext>  mxComponentContext;
    uno::Reference<
        presentation::XTransitionFactory>   mxOptionalTransitionFactory;

    
    SlideSharedPtr                          mpPreviousSlide;
    
    SlideSharedPtr                          mpCurrentSlide;
    
    SlideSharedPtr                          mpPrefetchSlide;
    
    uno::Reference<drawing::XDrawPage>      mxPrefetchSlide;
    
    uno::Reference<drawing::XDrawPagesSupplier>  mxDrawPagesSupplier;
    
    uno::Reference<animations::XAnimationNode> mxPrefetchAnimationNode;

    sal_Int16                               mnCurrentCursor;

    sal_Int32                               mnWaitSymbolRequestCount;
    bool                                    mbAutomaticAdvancementMode;
    bool                                    mbImageAnimationsAllowed;
    bool                                    mbNoSlideTransitions;
    bool                                    mbMouseVisible;
    bool                                    mbForceManualAdvance;
    bool                                    mbShowPaused;
    bool                                    mbSlideShowIdle;
    bool                                    mbDisableAnimationZOrder;

    EffectRewinder                          maEffectRewinder;
    FrameSynchronization                    maFrameSynchronization;
};

/** Separate event listener for animation, view and hyperlink events.

    This handler is registered for slide animation end, view and
    hyperlink events at the global EventMultiplexer, and forwards
    notifications to the SlideShowImpl
*/
struct SlideShowImpl::SeparateListenerImpl : public EventHandler,
                                             public ViewRepaintHandler,
                                             public HyperlinkHandler,
                                             public AnimationEventHandler,
                                             private boost::noncopyable
{
    SlideShowImpl& mrShow;
    ScreenUpdater& mrScreenUpdater;
    EventQueue&    mrEventQueue;

    SeparateListenerImpl( SlideShowImpl& rShow,
                          ScreenUpdater& rScreenUpdater,
                          EventQueue&    rEventQueue ) :
        mrShow( rShow ),
        mrScreenUpdater( rScreenUpdater ),
        mrEventQueue( rEventQueue )
    {}

    
    virtual bool handleEvent()
    {
        
        
        
        
        
        
        
        
        mrEventQueue.addEventForNextRound(
            makeEvent(
                boost::bind( &SlideShowImpl::notifySlideAnimationsEnded, boost::ref(mrShow) ),
                "SlideShowImpl::notifySlideAnimationsEnded"));
        return true;
    }

    
    virtual void viewClobbered( const UnoViewSharedPtr& rView )
    {
        
        mrScreenUpdater.notifyUpdate(rView, true);
    }

    
    virtual bool handleHyperlink( OUString const& rLink )
    {
        return mrShow.notifyHyperLinkClicked(rLink);
    }

    
    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode )
    {
        return mrShow.handleAnimationEvent(rNode);
    }
};

SlideShowImpl::SlideShowImpl(
    uno::Reference<uno::XComponentContext> const& xContext )
    : SlideShowImplBase(m_aMutex),
      maViewContainer(),
      maListenerContainer( m_aMutex ),
      maShapeEventListeners(),
      maShapeCursors(),
      maUserPaintColor(),
      maUserPaintStrokeWidth(4.0),
      mpPresTimer( new canvas::tools::ElapsedTime ),
      maScreenUpdater(maViewContainer),
      maEventQueue( mpPresTimer ),
      maEventMultiplexer( maEventQueue,
                          maViewContainer ),
      maActivitiesQueue( mpPresTimer ),
      maUserEventQueue( maEventMultiplexer,
                        maEventQueue,
                        *this ),
      mpDummyPtr(),
      mpListener(),
      mpRehearseTimingsActivity(),
      mpWaitSymbol(),
      mpPointerSymbol(),
      mpCurrentSlideTransitionSound(),
      mxComponentContext( xContext ),
      mxOptionalTransitionFactory(),
      mpCurrentSlide(),
      mpPrefetchSlide(),
      mxPrefetchSlide(),
      mxDrawPagesSupplier(),
      mxPrefetchAnimationNode(),
      mnCurrentCursor(awt::SystemPointer::ARROW),
      mnWaitSymbolRequestCount(0),
      mbAutomaticAdvancementMode(false),
      mbImageAnimationsAllowed( true ),
      mbNoSlideTransitions( false ),
      mbMouseVisible( true ),
      mbForceManualAdvance( false ),
      mbShowPaused( false ),
      mbSlideShowIdle( true ),
      mbDisableAnimationZOrder( false ),
      maEffectRewinder(maEventMultiplexer, maEventQueue, maUserEventQueue),
      maFrameSynchronization(1.0 / FrameRate::PreferredFramesPerSecond)

{
    
    

    uno::Reference<lang::XMultiComponentFactory> xFactory(
        mxComponentContext->getServiceManager() );

    if( xFactory.is() )
    {
        try
    {
            
            mxOptionalTransitionFactory.set(
                xFactory->createInstanceWithContext(
                    OUString("com.sun.star.presentation.TransitionFactory" ),
                    mxComponentContext ),
                uno::UNO_QUERY );
        }
        catch (loader::CannotActivateFactoryException const&)
    {
    }
    }

    mpListener.reset( new SeparateListenerImpl(
                          *this,
                          maScreenUpdater,
                          maEventQueue ));
    maEventMultiplexer.addSlideAnimationsEndHandler( mpListener );
    maEventMultiplexer.addViewRepaintHandler( mpListener );
    maEventMultiplexer.addHyperlinkHandler( mpListener, 0.0 );
    maEventMultiplexer.addAnimationStartHandler( mpListener );
    maEventMultiplexer.addAnimationEndHandler( mpListener );
}


void SlideShowImpl::disposing()
{
    osl::MutexGuard const guard( m_aMutex );

    maEffectRewinder.dispose();

    
    stopSlideTransitionSound();

    mxComponentContext.clear();

    if( mpCurrentSlideTransitionSound )
    {
        mpCurrentSlideTransitionSound->dispose();
        mpCurrentSlideTransitionSound.reset();
    }

    mpWaitSymbol.reset();
    mpPointerSymbol.reset();

    if( mpRehearseTimingsActivity )
    {
        mpRehearseTimingsActivity->dispose();
        mpRehearseTimingsActivity.reset();
    }

    if( mpListener )
    {
        maEventMultiplexer.removeSlideAnimationsEndHandler(mpListener);
        maEventMultiplexer.removeViewRepaintHandler(mpListener);
        maEventMultiplexer.removeHyperlinkHandler(mpListener);
        maEventMultiplexer.removeAnimationStartHandler( mpListener );
        maEventMultiplexer.removeAnimationEndHandler( mpListener );

        mpListener.reset();
    }

    maUserEventQueue.clear();
    maActivitiesQueue.clear();
    maEventMultiplexer.clear();
    maEventQueue.clear();
    mpPresTimer.reset();
    maShapeCursors.clear();
    maShapeEventListeners.clear();

    
    maListenerContainer.disposeAndClear(
        lang::EventObject( static_cast<cppu::OWeakObject *>(this) ) );

    maViewContainer.dispose();

    
    mxPrefetchAnimationNode.clear();
    mxPrefetchSlide.clear();
    mpPrefetchSlide.reset();
    mpCurrentSlide.reset();
    mpPreviousSlide.reset();
}


void SlideShowImpl::stopSlideTransitionSound()
{
    if (mpCurrentSlideTransitionSound)
    {
        mpCurrentSlideTransitionSound->stopPlayback();
        mpCurrentSlideTransitionSound->dispose();
        mpCurrentSlideTransitionSound.reset();
    }
 }

SoundPlayerSharedPtr SlideShowImpl::resetSlideTransitionSound( const uno::Any& rSound, bool bLoopSound )
{
    sal_Bool bStopSound = sal_False;
    OUString url;

    if( !(rSound >>= bStopSound) )
        bStopSound = sal_False;
    rSound >>= url;

    if( !bStopSound && url.isEmpty() )
        return SoundPlayerSharedPtr();

    stopSlideTransitionSound();

    if (!url.isEmpty())
    {
        try
        {
            mpCurrentSlideTransitionSound = SoundPlayer::create(
                maEventMultiplexer, url, mxComponentContext );
            mpCurrentSlideTransitionSound->setPlaybackLoop( bLoopSound );
        }
        catch (lang::NoSupportException const&)
        {
            
            
            
            
        }
    }
    return mpCurrentSlideTransitionSound;
}

ActivitySharedPtr SlideShowImpl::createSlideTransition(
    const uno::Reference< drawing::XDrawPage >& xDrawPage,
    const SlideSharedPtr&                       rLeavingSlide,
    const SlideSharedPtr&                       rEnteringSlide,
    const EventSharedPtr&                       rTransitionEndEvent)
{
    ENSURE_OR_THROW( !maViewContainer.empty(),
                      "createSlideTransition(): No views" );
    ENSURE_OR_THROW( rEnteringSlide,
                      "createSlideTransition(): No entering slide" );

    
    
    if (mbNoSlideTransitions)
        return ActivitySharedPtr();

    
    uno::Reference< beans::XPropertySet > xPropSet( xDrawPage,
                                                    uno::UNO_QUERY );

    if( !xPropSet.is() )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Slide has no PropertySet - assuming no transition\n" );
        return ActivitySharedPtr();
    }

    sal_Int16 nTransitionType(0);
    if( !getPropertyValue( nTransitionType,
                           xPropSet,
                           "TransitionType") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Could not extract slide transition type from XDrawPage - assuming no transition\n" );
        return ActivitySharedPtr();
    }

    sal_Int16 nTransitionSubType(0);
    if( !getPropertyValue( nTransitionSubType,
                           xPropSet,
                           "TransitionSubtype") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Could not extract slide transition subtype from XDrawPage - assuming no transition\n" );
        return ActivitySharedPtr();
    }

    bool bTransitionDirection(false);
    if( !getPropertyValue( bTransitionDirection,
                           xPropSet,
                           "TransitionDirection") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Could not extract slide transition direction from XDrawPage - assuming default direction\n" );
    }

    sal_Int32 aUnoColor(0);
    if( !getPropertyValue( aUnoColor,
                           xPropSet,
                           "TransitionFadeColor") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Could not extract slide transition fade color from XDrawPage - assuming black\n" );
    }

    const RGBColor aTransitionFadeColor( unoColor2RGBColor( aUnoColor ));

    uno::Any aSound;
    sal_Bool bLoopSound = sal_False;

    if( !getPropertyValue( aSound, xPropSet, "Sound") )
        OSL_TRACE( "createSlideTransition(): Could not determine transition sound effect URL from XDrawPage - using no sound" );

    if( !getPropertyValue( bLoopSound, xPropSet, "LoopSound" ) )
        OSL_TRACE( "createSlideTransition(): Could not get slide property 'LoopSound' - using no sound" );

    NumberAnimationSharedPtr pTransition(
        TransitionFactory::createSlideTransition(
            rLeavingSlide,
            rEnteringSlide,
            maViewContainer,
            maScreenUpdater,
            maEventMultiplexer,
            mxOptionalTransitionFactory,
            nTransitionType,
            nTransitionSubType,
            bTransitionDirection,
            aTransitionFadeColor,
            resetSlideTransitionSound( aSound, bLoopSound ) ));

    if( !pTransition )
        return ActivitySharedPtr(); 
                                    
                                    
                                    

    double nTransitionDuration(0.0);
    if( !getPropertyValue( nTransitionDuration,
                           xPropSet,
                           "TransitionDuration") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "Could not extract slide transition duration from XDrawPage - assuming no transition\n" );
        return ActivitySharedPtr();
    }

    sal_Int32 nMinFrames(5);
    if( !getPropertyValue( nMinFrames,
                           xPropSet,
                           "MinimalFrameNumber") )
    {
        OSL_TRACE( "createSlideTransition(): "
                   "No minimal number of frames given - assuming 5\n" );
    }

    
    
    
    maEventQueue.addEvent(
        makeEvent(
            boost::bind(
                &::slideshow::internal::Animation::prefetch,
                pTransition,
                AnimatableShapeSharedPtr(),
                ShapeAttributeLayerSharedPtr()),
            "Animation::prefetch"));

    return ActivitySharedPtr(
        ActivitiesFactory::createSimpleActivity(
            ActivitiesFactory::CommonParameters(
                rTransitionEndEvent,
                maEventQueue,
                maActivitiesQueue,
                nTransitionDuration,
                nMinFrames,
                false,
                boost::optional<double>(1.0),
                0.0,
                0.0,
                ShapeSharedPtr(),
                basegfx::B2DSize( rEnteringSlide->getSlideSize() ) ),
            pTransition,
            true ));
}

PolygonMap::iterator SlideShowImpl::findPolygons( uno::Reference<drawing::XDrawPage> const& xDrawPage)
{
    
    bool bFound = false;
    PolygonMap::iterator aIter=maPolygons.begin();

    while(aIter!=maPolygons.end() && !bFound)
    {
        if(aIter->first == xDrawPage)
            bFound = true;
        else
            ++aIter;
    }

    return aIter;
}

SlideSharedPtr SlideShowImpl::makeSlide(
    uno::Reference<drawing::XDrawPage> const&          xDrawPage,
    uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
    uno::Reference<animations::XAnimationNode> const&  xRootNode )
{
    if( !xDrawPage.is() )
        return SlideSharedPtr();

    
    PolygonMap::iterator aIter;
    aIter = findPolygons(xDrawPage);

    const SlideSharedPtr pSlide( createSlide(xDrawPage,
                                             xDrawPages,
                                             xRootNode,
                                             maEventQueue,
                                             maEventMultiplexer,
                                             maScreenUpdater,
                                             maActivitiesQueue,
                                             maUserEventQueue,
                                             *this,
                                             maViewContainer,
                                             mxComponentContext,
                                             maShapeEventListeners,
                                             maShapeCursors,
                                             (aIter != maPolygons.end()) ? aIter->second :  PolyPolygonVector(),
                                             maUserPaintColor ? *maUserPaintColor : RGBColor(),
                                             maUserPaintStrokeWidth,
                                             !!maUserPaintColor,
                                             mbImageAnimationsAllowed,
                                             mbDisableAnimationZOrder) );

    
    
    pSlide->prefetch();

    return pSlide;
}

void SlideShowImpl::requestWaitSymbol (void)
{
    ++mnWaitSymbolRequestCount;
    OSL_ASSERT(mnWaitSymbolRequestCount>0);

    if (mnWaitSymbolRequestCount == 1)
    {
        if( !mpWaitSymbol )
        {
            
            requestCursor(calcActiveCursor(mnCurrentCursor));
        }
        else
            mpWaitSymbol->show();
    }
}

void SlideShowImpl::releaseWaitSymbol (void)
{
    --mnWaitSymbolRequestCount;
    OSL_ASSERT(mnWaitSymbolRequestCount>=0);

    if (mnWaitSymbolRequestCount == 0)
    {
        if( !mpWaitSymbol )
        {
            
            requestCursor(calcActiveCursor(mnCurrentCursor));
        }
        else
            mpWaitSymbol->hide();
    }
}

sal_Int16 SlideShowImpl::calcActiveCursor( sal_Int16 nCursorShape ) const
{
    if( mnWaitSymbolRequestCount>0 && !mpWaitSymbol ) 
        nCursorShape = awt::SystemPointer::WAIT;
    else if( !mbMouseVisible ) 
        nCursorShape = awt::SystemPointer::INVISIBLE;
    else if( maUserPaintColor &&
             nCursorShape == awt::SystemPointer::ARROW )
        nCursorShape = awt::SystemPointer::PEN;

    return nCursorShape;
}

void SlideShowImpl::stopShow()
{
    
    
    if (mpCurrentSlide)
    {
        mpCurrentSlide->hide();
        
        if(findPolygons(mpCurrentSlide->getXDrawPage()) != maPolygons.end())
            maPolygons.erase(mpCurrentSlide->getXDrawPage());

        maPolygons.insert(make_pair(mpCurrentSlide->getXDrawPage(),mpCurrentSlide->getPolygons()));
    }

    
    maEventQueue.clear();
    maActivitiesQueue.clear();

    
    
    
    
    
    
    
    maUserEventQueue.clear();

    
    
    
    if (mbAutomaticAdvancementMode)
    {
        
        
        maEventMultiplexer.setAutomaticMode( false );
        maEventMultiplexer.setAutomaticMode( true );
    }
}

class SlideShowImpl::PrefetchPropertiesFunc
{
public:
    PrefetchPropertiesFunc( SlideShowImpl * that_,
        bool& rbSkipAllMainSequenceEffects,
        bool& rbSkipSlideTransition)
        : mpSlideShowImpl(that_),
          mrbSkipAllMainSequenceEffects(rbSkipAllMainSequenceEffects),
          mrbSkipSlideTransition(rbSkipSlideTransition)
    {}

    void operator()( beans::PropertyValue const& rProperty ) const {
        if (rProperty.Name == "Prefetch" )
        {
            uno::Sequence<uno::Any> seq;
            if ((rProperty.Value >>= seq) && seq.getLength() == 2)
            {
                seq[0] >>= mpSlideShowImpl->mxPrefetchSlide;
                seq[1] >>= mpSlideShowImpl->mxPrefetchAnimationNode;
            }
        }
        else if ( rProperty.Name == "SkipAllMainSequenceEffects" )
        {
            rProperty.Value >>= mrbSkipAllMainSequenceEffects;
        }
        else if ( rProperty.Name == "SkipSlideTransition" )
        {
            rProperty.Value >>= mrbSkipSlideTransition;
        }
        else
        {
            OSL_FAIL( OUStringToOString(
                            rProperty.Name, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
private:
    SlideShowImpl *const mpSlideShowImpl;
    bool& mrbSkipAllMainSequenceEffects;
    bool& mrbSkipSlideTransition;
};

void SlideShowImpl::displaySlide(
    uno::Reference<drawing::XDrawPage> const& xSlide,
    uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
    uno::Reference<animations::XAnimationNode> const& xRootNode,
    uno::Sequence<beans::PropertyValue> const& rProperties )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    maEffectRewinder.setRootAnimationNode(xRootNode);

    
    DBG_TESTSOLARMUTEX();

    mxDrawPagesSupplier = xDrawPages;

    stopShow();  
    
    
    
    
    
    
    

    bool bSkipAllMainSequenceEffects (false);
    bool bSkipSlideTransition (false);
    std::for_each( rProperties.getConstArray(),
                   rProperties.getConstArray() + rProperties.getLength(),
        PrefetchPropertiesFunc(this, bSkipAllMainSequenceEffects, bSkipSlideTransition) );

    OSL_ENSURE( !maViewContainer.empty(), "### no views!" );
    if (maViewContainer.empty())
        return;

    
    {
        WaitSymbolLock aLock (*this);

        mpPreviousSlide = mpCurrentSlide;
        mpCurrentSlide.reset();

        if (matches( mpPrefetchSlide, xSlide, xRootNode ))
        {
            
            mpCurrentSlide = mpPrefetchSlide;
        }
        else
            mpCurrentSlide = makeSlide( xSlide, xDrawPages, xRootNode );

        OSL_ASSERT( mpCurrentSlide );
        if (mpCurrentSlide)
        {
            basegfx::B2DSize oldSlideSize;
            if( mpPreviousSlide )
                oldSlideSize = basegfx::B2DSize( mpPreviousSlide->getSlideSize() );

            basegfx::B2DSize const slideSize( mpCurrentSlide->getSlideSize() );

            
            if( !mpPreviousSlide || oldSlideSize != slideSize )
            {
                std::for_each( maViewContainer.begin(),
                               maViewContainer.end(),
                               boost::bind( &View::setViewSize, _1,
                                            boost::cref(slideSize) ));

                
                
                
                
                maEventMultiplexer.notifyViewsChanged();
            }

            
            
            
            ActivitySharedPtr pSlideChangeActivity (
                createSlideTransition(
                    mpCurrentSlide->getXDrawPage(),
                    mpPreviousSlide,
                    mpCurrentSlide,
                    makeEvent(
                        boost::bind(
                            &SlideShowImpl::notifySlideTransitionEnded,
                            this,
                            false ),
                        "SlideShowImpl::notifySlideTransitionEnded")));

            if (bSkipSlideTransition)
            {
                
                
                
                
                pSlideChangeActivity.reset();
            }

            if (pSlideChangeActivity)
            {
                
                maActivitiesQueue.addActivity( pSlideChangeActivity );
            }
            else
            {
                
                
                maEventQueue.addEvent(
                    makeEvent(
                        boost::bind(
                            &SlideShowImpl::notifySlideTransitionEnded,
                            this,
                            true ),
                        "SlideShowImpl::notifySlideTransitionEnded"));
            }
        }
    } 

    maEventMultiplexer.notifySlideTransitionStarted();
    maListenerContainer.forEach<presentation::XSlideShowListener>(
        boost::mem_fn( &presentation::XSlideShowListener::slideTransitionStarted ) );

    
    
    
    if (bSkipAllMainSequenceEffects)
        maEffectRewinder.skipAllMainSequenceEffects();
}

void SlideShowImpl::redisplayCurrentSlide (void)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    
    DBG_TESTSOLARMUTEX();
    stopShow();

    OSL_ENSURE( !maViewContainer.empty(), "### no views!" );
    if (maViewContainer.empty())
        return;

    
    
    maEventQueue.addEvent(
        makeEvent(
            boost::bind(
                &SlideShowImpl::notifySlideTransitionEnded,
                this,
                true ),
            "SlideShowImpl::notifySlideTransitionEnded"));

    maEventMultiplexer.notifySlideTransitionStarted();
    maListenerContainer.forEach<presentation::XSlideShowListener>(
        boost::mem_fn( &presentation::XSlideShowListener::slideTransitionStarted ) );
}

sal_Bool SlideShowImpl::nextEffect() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    DBG_TESTSOLARMUTEX();

    if (mbShowPaused)
        return true;
    else
        return maEventMultiplexer.notifyNextEffect();
}

sal_Bool SlideShowImpl::previousEffect() throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    DBG_TESTSOLARMUTEX();

    if (mbShowPaused)
        return true;
    else
    {
        return maEffectRewinder.rewind(
            maScreenUpdater.createLock(false),
            ::boost::bind<void>(::boost::mem_fn(&SlideShowImpl::redisplayCurrentSlide), this),
            ::boost::bind<void>(::boost::mem_fn(&SlideShowImpl::rewindEffectToPreviousSlide), this));
    }
}

void SlideShowImpl::rewindEffectToPreviousSlide (void)
{
    
    
    WaitSymbolLock aLock (*this);

    
    
    
    notifySlideEnded(true);

    
    
    
    maEventQueue.forceEmpty();

    
    
    
    maScreenUpdater.commitUpdates();
}

sal_Bool SlideShowImpl::startShapeActivity(
    uno::Reference<drawing::XShape> const& /*xShape*/ )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    
    DBG_TESTSOLARMUTEX();

    
    OSL_FAIL( "not yet implemented!" );
    return false;
}

sal_Bool SlideShowImpl::stopShapeActivity(
    uno::Reference<drawing::XShape> const& /*xShape*/ )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    
    DBG_TESTSOLARMUTEX();

    
    OSL_FAIL( "not yet implemented!" );
    return false;
}

sal_Bool SlideShowImpl::pause( sal_Bool bPauseShow )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    DBG_TESTSOLARMUTEX();

    if (bPauseShow)
        mpPresTimer->pauseTimer();
    else
        mpPresTimer->continueTimer();

    maEventMultiplexer.notifyPauseMode(bPauseShow);

    mbShowPaused = bPauseShow;
    return true;
}

uno::Reference<drawing::XDrawPage> SlideShowImpl::getCurrentSlide()
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return uno::Reference<drawing::XDrawPage>();

    
    DBG_TESTSOLARMUTEX();

    if (mpCurrentSlide)
        return mpCurrentSlide->getXDrawPage();
    else
        return uno::Reference<drawing::XDrawPage>();
}

sal_Bool SlideShowImpl::addView(
    uno::Reference<presentation::XSlideShowView> const& xView )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    DBG_TESTSOLARMUTEX();

    
    ENSURE_OR_RETURN_FALSE( xView.is(), "addView(): Invalid view" );
    ENSURE_OR_RETURN_FALSE( xView->getCanvas().is(),
                       "addView(): View does not provide a valid canvas" );

    UnoViewSharedPtr const pView( createSlideView(
                                      xView,
                                      maEventQueue,
                                      maEventMultiplexer ));
    if (!maViewContainer.addView( pView ))
        return false; 

    
    

    if (mpCurrentSlide)
    {
        
        const basegfx::B2ISize slideSize = mpCurrentSlide->getSlideSize();
        pView->setViewSize( basegfx::B2DSize( slideSize.getX(),
                                              slideSize.getY() ) );
    }

    
    
    pView->clearAll();

    
    maEventMultiplexer.notifyViewAdded( pView );

    
    pView->setCursorShape( calcActiveCursor(mnCurrentCursor) );

    return true;
}

sal_Bool SlideShowImpl::removeView(
    uno::Reference<presentation::XSlideShowView> const& xView )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    
    DBG_TESTSOLARMUTEX();

    ENSURE_OR_RETURN_FALSE( xView.is(), "removeView(): Invalid view" );

    UnoViewSharedPtr const pView( maViewContainer.removeView( xView ) );
    if( !pView )
        return false; 

    
    maEventMultiplexer.notifyViewRemoved( pView );

    pView->_dispose();

    return true;
}

void SlideShowImpl::registerUserPaintPolygons( const uno::Reference< lang::XMultiServiceFactory >& xDocFactory ) throw (uno::RuntimeException)
{
    
    if (mpCurrentSlide)
    {
        if(findPolygons(mpCurrentSlide->getXDrawPage()) != maPolygons.end())
            maPolygons.erase(mpCurrentSlide->getXDrawPage());

        maPolygons.insert(make_pair(mpCurrentSlide->getXDrawPage(),mpCurrentSlide->getPolygons()));
    }

    
    
    uno::Reference< drawing::XLayerSupplier > xLayerSupplier(xDocFactory, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess > xNameAccess = xLayerSupplier->getLayerManager();

    uno::Reference< drawing::XLayerManager > xLayerManager(xNameAccess, uno::UNO_QUERY);
    
    uno::Reference< drawing::XLayer > xDrawnInSlideshow = xLayerManager->insertNewByIndex(xLayerManager->getCount());
    uno::Reference< beans::XPropertySet > xLayerPropSet(xDrawnInSlideshow, uno::UNO_QUERY);

    
    OUString layerName = "DrawnInSlideshow";
    uno::Any aPropLayer;

    aPropLayer <<= layerName;
    xLayerPropSet->setPropertyValue("Name", aPropLayer);

    aPropLayer <<= true;
    xLayerPropSet->setPropertyValue("IsVisible", aPropLayer);

    aPropLayer <<= false;
    xLayerPropSet->setPropertyValue("IsLocked", aPropLayer);

    PolygonMap::iterator aIter=maPolygons.begin();

    PolyPolygonVector aPolygons;
    ::cppcanvas::PolyPolygonSharedPtr pPolyPoly;
    ::basegfx::B2DPolyPolygon b2DPolyPoly;

    
    while(aIter!=maPolygons.end())
    {
        aPolygons = aIter->second;
        
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > Shapes(aIter->first, ::com::sun::star::uno::UNO_QUERY);
        
        for( PolyPolygonVector::iterator aIterPoly=aPolygons.begin(),
                 aEnd=aPolygons.end();
             aIterPoly!=aEnd; ++aIterPoly )
        {
            pPolyPoly = (*aIterPoly);
            b2DPolyPoly = ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(pPolyPoly->getUNOPolyPolygon());

            
            for(sal_uInt32 i=0; i< b2DPolyPoly.count();i++)
            {
                const ::basegfx::B2DPolygon& aPoly =  b2DPolyPoly.getB2DPolygon(i);
                sal_uInt32 nPoints = aPoly.count();

                if( nPoints > 1)
                {
                    
                    uno::Reference< uno::XInterface > polyshape(xDocFactory->createInstance(
                                                                    OUString("com.sun.star.drawing.PolyLineShape") ) );
                    uno::Reference< drawing::XShape > rPolyShape(polyshape, uno::UNO_QUERY);

                    
                    Shapes->add(rPolyShape);

                    
                    uno::Reference< beans::XPropertySet > aXPropSet = uno::Reference< beans::XPropertySet >( rPolyShape, uno::UNO_QUERY );
                    
                    drawing::PointSequenceSequence aRetval;
                    
                    aRetval.realloc( 1 );
                    
                    drawing::PointSequence* pOuterSequence = aRetval.getArray();
                    
                    pOuterSequence->realloc(nPoints);
                    
                    awt::Point* pInnerSequence = pOuterSequence->getArray();
                    for( sal_uInt32 n = 0; n < nPoints; n++ )
                    {
                        
                        *pInnerSequence++ = awt::Point(
                            basegfx::fround(aPoly.getB2DPoint(n).getX()),
                            basegfx::fround(aPoly.getB2DPoint(n).getY()));
                    }

                    
                    
                    uno::Any aParam;
                    aParam <<= aRetval;
                    aXPropSet->setPropertyValue("PolyPolygon", aParam );

                    
                    uno::Any            aAny;
                    drawing::LineStyle  eLS;
                    eLS = drawing::LineStyle_SOLID;
                    aAny <<= eLS;
                    aXPropSet->setPropertyValue("LineStyle", aAny );

                    
                    sal_uInt32          nLineColor;
                    nLineColor = pPolyPoly->getRGBALineColor();
                    
                    aAny <<= RGBAColor2UnoColor(nLineColor);
                    aXPropSet->setPropertyValue("LineColor", aAny );

                    
                    double              fLineWidth;
                    fLineWidth = pPolyPoly->getStrokeWidth();
                    aAny <<= (sal_Int32)fLineWidth;
                    aXPropSet->setPropertyValue("LineWidth", aAny );

                    
                    xLayerManager->attachShapeToLayer(rPolyShape, xDrawnInSlideshow);
                }
            }
        }
        ++aIter;
    }
}

sal_Bool SlideShowImpl::setProperty( beans::PropertyValue const& rProperty )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    DBG_TESTSOLARMUTEX();

    if ( rProperty.Name == "AutomaticAdvancement" )
    {
        double nTimeout(0.0);
        mbAutomaticAdvancementMode = (rProperty.Value >>= nTimeout);
        if (mbAutomaticAdvancementMode)
        {
            maEventMultiplexer.setAutomaticTimeout( nTimeout );
        }
        maEventMultiplexer.setAutomaticMode( mbAutomaticAdvancementMode );
        return true;
    }

    if ( rProperty.Name == "UserPaintColor" )
    {
        sal_Int32 nColor(0);
        if (rProperty.Value >>= nColor)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            
            maUserPaintColor.reset( unoColor2RGBColor( nColor ) );
            if( mpCurrentSlide && !mpCurrentSlide->isPaintOverlayActive() )
                mpCurrentSlide->enablePaintOverlay();

            maEventMultiplexer.notifyUserPaintColor( *maUserPaintColor );
        }
        else
        {
            
            maUserPaintColor.reset();
            maEventMultiplexer.notifyUserPaintDisabled();
            if( mpCurrentSlide )
                mpCurrentSlide->disablePaintOverlay();
        }

        resetCursor();

        return true;
    }

    
    if ( rProperty.Name == "EraseAllInk" )
    {
        bool nEraseAllInk(false);
        if (rProperty.Value >>= nEraseAllInk)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            
            maEraseAllInk.reset( nEraseAllInk );
            maEventMultiplexer.notifyEraseAllInk( *maEraseAllInk );
        }

        return true;
    }

    if ( rProperty.Name == "SwitchPenMode" )
    {
        bool nSwitchPenMode(false);
        if (rProperty.Value >>= nSwitchPenMode)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            if(nSwitchPenMode == true){
            
            maSwitchPenMode.reset( nSwitchPenMode );
            maEventMultiplexer.notifySwitchPenMode();
            }
        }
        return true;
    }

    if ( rProperty.Name == "SwitchEraserMode" )
    {
        bool nSwitchEraserMode(false);
        if (rProperty.Value >>= nSwitchEraserMode)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );
            if(nSwitchEraserMode == true){
            
            maSwitchEraserMode.reset( nSwitchEraserMode );
            maEventMultiplexer.notifySwitchEraserMode();
            }
        }

        return true;
    }

    if ( rProperty.Name == "EraseInk" )
    {
        sal_Int32 nEraseInk(100);
        if (rProperty.Value >>= nEraseInk)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            
            maEraseInk.reset( nEraseInk );
            maEventMultiplexer.notifyEraseInkWidth( *maEraseInk );
        }

        return true;
    }

    
    if ( rProperty.Name == "UserPaintStrokeWidth" )
    {
        double nWidth(4.0);
        if (rProperty.Value >>= nWidth)
        {
            OSL_ENSURE( mbMouseVisible,"setProperty(): User paint overrides invisible mouse" );
            
            maUserPaintStrokeWidth = nWidth;
            maEventMultiplexer.notifyUserPaintStrokeWidth( maUserPaintStrokeWidth );
        }

        return true;
    }

    if ( rProperty.Name == "AdvanceOnClick" )
    {
        sal_Bool bAdvanceOnClick = sal_False;
        if (! (rProperty.Value >>= bAdvanceOnClick))
            return false;
        maUserEventQueue.setAdvanceOnClick( bAdvanceOnClick );
        return true;
    }

    if ( rProperty.Name == "DisableAnimationZOrder" )
    {
        sal_Bool bDisableAnimationZOrder = sal_False;
        if (! (rProperty.Value >>= bDisableAnimationZOrder))
            return false;
        mbDisableAnimationZOrder = bDisableAnimationZOrder == sal_True;
        return true;
    }

    if ( rProperty.Name == "ImageAnimationsAllowed" )
    {
        if (! (rProperty.Value >>= mbImageAnimationsAllowed))
            return false;

        
        return true;
    }

    if ( rProperty.Name == "MouseVisible" )
    {
        if (! (rProperty.Value >>= mbMouseVisible))
            return false;

        requestCursor(mnCurrentCursor);

        return true;
    }

    if ( rProperty.Name == "ForceManualAdvance" )
    {
        return (rProperty.Value >>= mbForceManualAdvance);
    }

    if ( rProperty.Name == "RehearseTimings" )
    {
        bool bRehearseTimings = false;
        if (! (rProperty.Value >>= bRehearseTimings))
            return false;

        if (bRehearseTimings)
        {
            
            mpRehearseTimingsActivity = RehearseTimingsActivity::create(
                SlideShowContext(
                    mpDummyPtr,
                    maEventQueue,
                    maEventMultiplexer,
                    maScreenUpdater,
                    maActivitiesQueue,
                    maUserEventQueue,
                    *this,
                    maViewContainer,
                    mxComponentContext) );
        }
        else if (mpRehearseTimingsActivity)
        {
            
            mpRehearseTimingsActivity->dispose();
            mpRehearseTimingsActivity.reset();
        }
        return true;
    }

    if ( rProperty.Name == "WaitSymbolBitmap" )
    {
        uno::Reference<rendering::XBitmap> xBitmap;
        if (! (rProperty.Value >>= xBitmap))
            return false;

        mpWaitSymbol = WaitSymbol::create( xBitmap,
                                           maScreenUpdater,
                                           maEventMultiplexer,
                                           maViewContainer );

        return true;
    }

    if ( rProperty.Name == "PointerSymbolBitmap" )
    {
        uno::Reference<rendering::XBitmap> xBitmap;
        if (! (rProperty.Value >>= xBitmap))
            return false;

        mpPointerSymbol = PointerSymbol::create( xBitmap,
                                           maScreenUpdater,
                                           maEventMultiplexer,
                                           maViewContainer );

        return true;
    }

    if ( rProperty.Name == "PointerVisible" )
    {
        bool visible;
        if (!(rProperty.Value >>= visible))
            return false;

        mpPointerSymbol->setVisible(visible);
        return true;
    }

    if ( rProperty.Name == "PointerPosition")
    {
        ::com::sun::star::geometry::RealPoint2D pos;
        if (! (rProperty.Value >>= pos))
            return false;

        mpPointerSymbol->viewsChanged(pos);
        return true;
    }

    if (rProperty.Name == "NoSlideTransitions" )
    {
        return (rProperty.Value >>= mbNoSlideTransitions);
    }

    if ( rProperty.Name == "IsSoundEnabled" )
    {
        uno::Sequence<uno::Any> aValues;
        uno::Reference<presentation::XSlideShowView> xView;
        sal_Bool bValue (false);
        if ((rProperty.Value >>= aValues)
            && aValues.getLength()==2
            && (aValues[0] >>= xView)
            && (aValues[1] >>= bValue))
        {
            
            for (UnoViewVector::const_iterator
                     iView (maViewContainer.begin()),
                     iEnd (maViewContainer.end());
                 iView!=iEnd;
                 ++iView)
            {
                if (*iView && (*iView)->getUnoView()==xView)
                {
                    
                    
                    (*iView)->setIsSoundEnabled(bValue);
                    return true;
                }
            }
        }
    }

    return false;
}

void SlideShowImpl::addSlideShowListener(
    uno::Reference<presentation::XSlideShowListener> const& xListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    
    maListenerContainer.addInterface(xListener);
}

void SlideShowImpl::removeSlideShowListener(
    uno::Reference<presentation::XSlideShowListener> const& xListener )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    
    maListenerContainer.removeInterface(xListener);
}

void SlideShowImpl::addShapeEventListener(
    uno::Reference<presentation::XShapeEventListener> const& xListener,
    uno::Reference<drawing::XShape> const& xShape )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    
    DBG_TESTSOLARMUTEX();

    ShapeEventListenerMap::iterator aIter;
    if( (aIter=maShapeEventListeners.find( xShape )) ==
        maShapeEventListeners.end() )
    {
        
        aIter = maShapeEventListeners.insert(
            ShapeEventListenerMap::value_type(
                xShape,
                boost::shared_ptr<cppu::OInterfaceContainerHelper>(
                    new cppu::OInterfaceContainerHelper(m_aMutex)))).first;
    }

    
    if( aIter->second.get() )
        aIter->second->addInterface( xListener );

    maEventMultiplexer.notifyShapeListenerAdded(xListener,
                                                xShape);
}

void SlideShowImpl::removeShapeEventListener(
    uno::Reference<presentation::XShapeEventListener> const& xListener,
    uno::Reference<drawing::XShape> const& xShape )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    
    DBG_TESTSOLARMUTEX();

    ShapeEventListenerMap::iterator aIter;
    if( (aIter = maShapeEventListeners.find( xShape )) !=
        maShapeEventListeners.end() )
    {
        
        
        ENSURE_OR_THROW(
            aIter->second.get(),
            "SlideShowImpl::removeShapeEventListener(): "
            "listener map contains NULL broadcast helper" );

        aIter->second->removeInterface( xListener );
    }

    maEventMultiplexer.notifyShapeListenerRemoved(xListener,
                                                  xShape);
}

void SlideShowImpl::setShapeCursor(
    uno::Reference<drawing::XShape> const& xShape, sal_Int16 nPointerShape )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    
    DBG_TESTSOLARMUTEX();

    ShapeCursorMap::iterator aIter;
    if( (aIter=maShapeCursors.find( xShape )) == maShapeCursors.end() )
    {
        
        if( nPointerShape != awt::SystemPointer::ARROW )
        {
            
            
            
            maShapeCursors.insert(
                ShapeCursorMap::value_type(xShape,
                                           nPointerShape) );
        }
    }
    else if( nPointerShape == awt::SystemPointer::ARROW )
    {
        
        
        maShapeCursors.erase( xShape );
    }
    else
    {
        
        aIter->second = nPointerShape;
    }

    maEventMultiplexer.notifyShapeCursorChange(xShape,
                                               nPointerShape);
}

bool SlideShowImpl::requestCursor( sal_Int16 nCursorShape )
{
    mnCurrentCursor = nCursorShape;

    const sal_Int16 nActualCursor = calcActiveCursor(mnCurrentCursor);

    
    std::for_each( maViewContainer.begin(),
                   maViewContainer.end(),
                   boost::bind( &View::setCursorShape,
                                _1,
                                nActualCursor ));

    return nActualCursor==nCursorShape;
}

void SlideShowImpl::resetCursor()
{
    mnCurrentCursor = awt::SystemPointer::ARROW;

    
    std::for_each( maViewContainer.begin(),
                   maViewContainer.end(),
                   boost::bind( &View::setCursorShape,
                                _1,
                                calcActiveCursor(mnCurrentCursor) ));
}

sal_Bool SlideShowImpl::update( double & nNextTimeout )
    throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    
    
    DBG_TESTSOLARMUTEX();

    if( mbShowPaused )
    {
        
        maScreenUpdater.commitUpdates();

        return false;
    }
    else
    {
        
        

        
        
        
        
        
        
        {
            
            
            
            boost::shared_ptr<canvas::tools::ElapsedTime> xTimer(mpPresTimer);
            comphelper::ScopeGuard scopeGuard(
                boost::bind( &canvas::tools::ElapsedTime::releaseTimer,
                             boost::cref(xTimer) ) );
            xTimer->holdTimer();

            
            maEventQueue.process();

            
            
            
            
            
            if (isDisposed())
            {
                scopeGuard.dismiss();
                return false;
            }

            maActivitiesQueue.process();

            
            maFrameSynchronization.Synchronize();
            maScreenUpdater.commitUpdates();

            
            
            //
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            

            
            maActivitiesQueue.processDequeued();

            
            maScreenUpdater.commitUpdates();
        }
        

        const bool bActivitiesLeft = (! maActivitiesQueue.isEmpty());
        const bool bTimerEventsLeft = (! maEventQueue.isEmpty());
        const bool bRet = (bActivitiesLeft || bTimerEventsLeft);

        if (bRet)
        {
            
            if (bActivitiesLeft)
            {
                
                

                
                
                
                
                nNextTimeout = 0;
                maFrameSynchronization.Activate();
            }
            else
            {
                
                
                
                

                
                

                
                nNextTimeout = std::max( 0.0, maEventQueue.nextTimeout() );

                
                
                maFrameSynchronization.Deactivate();
            }

            mbSlideShowIdle = false;
        }

#if OSL_DEBUG_LEVEL >= 2 && defined(DBG_UTIL)
        
        
        
        
        if( !mbSlideShowIdle &&
            (!bRet ||
             nNextTimeout > 1.0) )
        {
            UnoViewVector::const_iterator       aCurr(maViewContainer.begin());
            const UnoViewVector::const_iterator aEnd(maViewContainer.end());
            while( aCurr != aEnd )
            {
                try
                {
                    uno::Reference< presentation::XSlideShowView > xView( (*aCurr)->getUnoView(),
                                                                          uno::UNO_QUERY_THROW );
                    uno::Reference< util::XUpdatable >             xUpdatable( xView->getCanvas(),
                                                                               uno::UNO_QUERY_THROW );
                    xUpdatable->update();
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( uno::Exception& )
                {
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }

                ++aCurr;
            }

            mbSlideShowIdle = true;
        }
#endif

        return bRet;
    }
}

void SlideShowImpl::notifySlideTransitionEnded( bool bPaintSlide )
{
    osl::MutexGuard const guard( m_aMutex );

    OSL_ENSURE( !isDisposed(), "### already disposed!" );
    OSL_ENSURE( mpCurrentSlide,
                "notifySlideTransitionEnded(): Invalid current slide" );
    if (mpCurrentSlide)
    {
        mpCurrentSlide->update_settings( !!maUserPaintColor, maUserPaintColor ? *maUserPaintColor : RGBColor(), maUserPaintStrokeWidth );

        
        
        const bool bBackgroundLayerRendered( !bPaintSlide );
        mpCurrentSlide->show( bBackgroundLayerRendered );
        maEventMultiplexer.notifySlideStartEvent();
    }
}

void queryAutomaticSlideTransition( uno::Reference<drawing::XDrawPage> const& xDrawPage,
                                    double&                                   nAutomaticNextSlideTimeout,
                                    bool&                                     bHasAutomaticNextSlide )
{
    
    

    uno::Reference< beans::XPropertySet > xPropSet( xDrawPage,
                                                    uno::UNO_QUERY );

    sal_Int32 nChange(0);
    if( !xPropSet.is() ||
        !getPropertyValue( nChange,
                           xPropSet,
                           OUString(
                               "Change")) )
    {
        OSL_TRACE(
            "queryAutomaticSlideTransition(): "
            "Could not extract slide change mode from XDrawPage - assuming <none>\n" );
    }

    bHasAutomaticNextSlide = nChange == 1;

    if( !xPropSet.is() ||
        !getPropertyValue( nAutomaticNextSlideTimeout,
                           xPropSet,
                           OUString(
                               "HighResDuration")) )
    {
        OSL_TRACE(
            "queryAutomaticSlideTransition(): "
            "Could not extract slide transition timeout from "
            "XDrawPage - assuming 1 sec\n" );
    }
}

void SlideShowImpl::notifySlideAnimationsEnded()
{
    osl::MutexGuard const guard( m_aMutex );

    
    mpCurrentSlide->drawPolygons();

    OSL_ENSURE( !isDisposed(), "### already disposed!" );

    
    
    InterruptableEventPair aNotificationEvents;

    if( maEventMultiplexer.getAutomaticMode() )
    {
        OSL_ENSURE( ! mpRehearseTimingsActivity,
                    "unexpected: RehearseTimings mode!" );

        
        
        aNotificationEvents = makeInterruptableDelay(
            boost::bind<void>( boost::mem_fn(&SlideShowImpl::notifySlideEnded), this, false ),
            maEventMultiplexer.getAutomaticTimeout() );
    }
    else
    {
        OSL_ENSURE( mpCurrentSlide,
                    "notifySlideAnimationsEnded(): Invalid current slide!" );

        bool   bHasAutomaticNextSlide=false;
        double nAutomaticNextSlideTimeout=0.0;
        queryAutomaticSlideTransition(mpCurrentSlide->getXDrawPage(),
                                      nAutomaticNextSlideTimeout,
                                      bHasAutomaticNextSlide);

        
        
        
        
        
        
        if( !mbForceManualAdvance &&
            !mpRehearseTimingsActivity &&
            bHasAutomaticNextSlide )
        {
            aNotificationEvents = makeInterruptableDelay(
                boost::bind<void>( boost::mem_fn(&SlideShowImpl::notifySlideEnded), this, false ),
                nAutomaticNextSlideTimeout);

            
            
        }
        else
        {
            if (mpRehearseTimingsActivity)
                mpRehearseTimingsActivity->start();

            
            
            
            
            aNotificationEvents.mpImmediateEvent =
                makeEvent( boost::bind<void>(
                    boost::mem_fn(&SlideShowImpl::notifySlideEnded), this, false ),
                    "SlideShowImpl::notifySlideEnded");
        }
    }

    
    
    
    
    
    maUserEventQueue.registerNextEffectEvent(
        aNotificationEvents.mpImmediateEvent );

    if( aNotificationEvents.mpTimeoutEvent )
        maEventQueue.addEvent( aNotificationEvents.mpTimeoutEvent );

    
    
    
    
    
    {
        WaitSymbolLock aLock (*this);

        if (! matches( mpPrefetchSlide,
                       mxPrefetchSlide, mxPrefetchAnimationNode ))
        {
            mpPrefetchSlide = makeSlide( mxPrefetchSlide, mxDrawPagesSupplier,
                                         mxPrefetchAnimationNode );
        }
        if (mpPrefetchSlide)
        {
            
            
            
            
            mpPrefetchSlide->getCurrentSlideBitmap( *maViewContainer.begin() );
        }
    } 

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        boost::mem_fn( &presentation::XSlideShowListener::slideAnimationsEnded ) );
}

void SlideShowImpl::notifySlideEnded (const bool bReverse)
{
    osl::MutexGuard const guard( m_aMutex );

    OSL_ENSURE( !isDisposed(), "### already disposed!" );

    if (mpRehearseTimingsActivity && !bReverse)
    {
        const double time = mpRehearseTimingsActivity->stop();
        if (mpRehearseTimingsActivity->hasBeenClicked())
        {
            
            uno::Reference<beans::XPropertySet> xPropSet(
                mpCurrentSlide->getXDrawPage(), uno::UNO_QUERY );
            OSL_ASSERT( xPropSet.is() );
            if (xPropSet.is())
            {
                xPropSet->setPropertyValue(
                    "Change",
                    uno::Any( static_cast<sal_Int32>(1) ) );
                xPropSet->setPropertyValue(
                    "Duration",
                    uno::Any( static_cast<sal_Int32>(time) ) );
            }
        }
    }

    if (bReverse)
        maEventMultiplexer.notifySlideEndEvent();

    stopShow();  
                 
                 
                 
                 
                 
                 
                 

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        boost::bind<void>(
            ::boost::mem_fn(&presentation::XSlideShowListener::slideEnded),
            _1,
            sal_Bool(bReverse)));
}

bool SlideShowImpl::notifyHyperLinkClicked( OUString const& hyperLink )
{
    osl::MutexGuard const guard( m_aMutex );

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        boost::bind( &presentation::XSlideShowListener::hyperLinkClicked,
                     _1,
                     boost::cref(hyperLink) ));
    return true;
}

/** Notification from eventmultiplexer that an animation event has occoured.
    This will be forewarded to all registered XSlideShoeListener
 */
bool SlideShowImpl::handleAnimationEvent( const AnimationNodeSharedPtr& rNode )
{
    osl::MutexGuard const guard( m_aMutex );

    uno::Reference<animations::XAnimationNode> xNode( rNode->getXAnimationNode() );

    switch( rNode->getState() )
    {
    case AnimationNode::ACTIVE:
        maListenerContainer.forEach<presentation::XSlideShowListener>(
            boost::bind( &animations::XAnimationListener::beginEvent,
                         _1,
                         boost::cref(xNode) ));
        break;

    case AnimationNode::FROZEN:
    case AnimationNode::ENDED:
        maListenerContainer.forEach<presentation::XSlideShowListener>(
            boost::bind( &animations::XAnimationListener::endEvent,
                         _1,
                         boost::cref(xNode) ));
        if(mpCurrentSlide->isPaintOverlayActive())
           mpCurrentSlide->drawPolygons();
        break;
    default:
        break;
    }

    return true;
}



FrameSynchronization::FrameSynchronization (const double nFrameDuration)
    : maTimer(),
      mnFrameDuration(nFrameDuration),
      mnNextFrameTargetTime(0),
      mbIsActive(false)
{
    MarkCurrentFrame();
}

void FrameSynchronization::MarkCurrentFrame (void)
{
    mnNextFrameTargetTime = maTimer.getElapsedTime() + mnFrameDuration;
}

void FrameSynchronization::Synchronize (void)
{
    if (mbIsActive)
    {
        
        while (maTimer.getElapsedTime() < mnNextFrameTargetTime)
            ;
    }

    MarkCurrentFrame();
}

void FrameSynchronization::Activate (void)
{
    mbIsActive = true;
}

void FrameSynchronization::Deactivate (void)
{
    mbIsActive = false;
}

} 

namespace sdecl = comphelper::service_decl;
const sdecl::ServiceDecl slideShowDecl(
     sdecl::class_<SlideShowImpl>(),
    "com.sun.star.comp.presentation.SlideShow",
    "com.sun.star.presentation.SlideShow" );


COMPHELPER_SERVICEDECL_EXPORTS1(slideshow, slideShowDecl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
