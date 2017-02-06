/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <tools/diagnose_ex.h>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/exc_hlp.hxx>

#include <comphelper/anytostring.hxx>
#include <comphelper/make_shared_from_uno.hxx>
#include <comphelper/scopeguard.hxx>
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
#include <com/sun/star/lang/NoSupportException.hpp>
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
    explicit FrameSynchronization (const double nFrameDuration);

    /** Set the current time as the time at which the current frame is
        displayed.  From this the target time of the next frame is derived.
    */
    void MarkCurrentFrame();

    /** When there is time left until the next frame is due then wait.
        Otherwise return without delay.
    */
    void Synchronize();

    /** Activate frame synchronization when an animation is active and
        frames are to be displayed in a steady rate.  While active
        Synchronize() will wait until the frame duration time has passed.
    */
    void Activate();

    /** Deactivate frame sychronization when no animation is active and the
        time between frames depends on user actions and other external
        sources.  While deactivated Synchronize() will return without delay.
    */
    void Deactivate();

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
   of css::drawing::XDrawPage objects).

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

typedef cppu::WeakComponentImplHelper<presentation::XSlideShow> SlideShowImplBase;

typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;

/// Maps XDrawPage for annotations persistence
typedef ::std::map< css::uno::Reference<
                                    css::drawing::XDrawPage>,
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
    // XSlideShow:
    virtual sal_Bool SAL_CALL nextEffect() override;
    virtual sal_Bool SAL_CALL previousEffect() override;
    virtual sal_Bool SAL_CALL startShapeActivity(
        uno::Reference<drawing::XShape> const& xShape ) override;
    virtual sal_Bool SAL_CALL stopShapeActivity(
        uno::Reference<drawing::XShape> const& xShape ) override;
    virtual sal_Bool SAL_CALL pause( sal_Bool bPauseShow ) override;
    virtual uno::Reference<drawing::XDrawPage> SAL_CALL getCurrentSlide() override;
    virtual void SAL_CALL displaySlide(
        uno::Reference<drawing::XDrawPage> const& xSlide,
        uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
        uno::Reference<animations::XAnimationNode> const& xRootNode,
        uno::Sequence<beans::PropertyValue> const& rProperties ) override;
    virtual void SAL_CALL registerUserPaintPolygons( const css::uno::Reference< css::lang::XMultiServiceFactory >& xDocFactory ) override;
    virtual sal_Bool SAL_CALL setProperty(
        beans::PropertyValue const& rProperty ) override;
    virtual sal_Bool SAL_CALL addView(
        uno::Reference<presentation::XSlideShowView> const& xView ) override;
    virtual sal_Bool SAL_CALL removeView(
        uno::Reference<presentation::XSlideShowView> const& xView ) override;
    virtual sal_Bool SAL_CALL update( double & nNextTimeout ) override;
    virtual void SAL_CALL addSlideShowListener(
        uno::Reference<presentation::XSlideShowListener> const& xListener ) override;
    virtual void SAL_CALL removeSlideShowListener(
        uno::Reference<presentation::XSlideShowListener> const& xListener ) override;
    virtual void SAL_CALL addShapeEventListener(
        uno::Reference<presentation::XShapeEventListener> const& xListener,
        uno::Reference<drawing::XShape> const& xShape ) override;
    virtual void SAL_CALL removeShapeEventListener(
        uno::Reference<presentation::XShapeEventListener> const& xListener,
        uno::Reference<drawing::XShape> const& xShape ) override;
    virtual void SAL_CALL setShapeCursor(
        uno::Reference<drawing::XShape> const& xShape, sal_Int16 nPointerShape ) override;

    // CursorManager


    virtual bool requestCursor( sal_Int16 nCursorShape ) override;
    virtual void resetCursor() override;

    /** This is somewhat similar to displaySlide when called for the current
        slide.  It has been simplified to take advantage of that no slide
        change takes place.  Furthermore it does not show the slide
        transition.
    */
    void redisplayCurrentSlide();

protected:
    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing() override;

    bool isDisposed() const
    {
        return (rBHelper.bDisposed || rBHelper.bInDispose);
    }

private:
    struct SeparateListenerImpl; friend struct SeparateListenerImpl;
    class PrefetchPropertiesFunc; friend class PrefetchPropertiesFunc;

    /// Stop currently running show.
    void stopShow();

    ///Find a polygons vector in maPolygons (map)
    PolygonMap::iterator findPolygons( uno::Reference<drawing::XDrawPage> const& xDrawPage);

    /// Creates a new slide.
    SlideSharedPtr makeSlide(
        uno::Reference<drawing::XDrawPage> const& xDrawPage,
        uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
        uno::Reference<animations::XAnimationNode> const& xRootNode );

    /// Checks whether the given slide/animation node matches mpPrefetchSlide
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

    /// Resets the current slide transition sound object with a new one:
    SoundPlayerSharedPtr resetSlideTransitionSound(
        uno::Any const& url, bool bLoopSound );

    /// stops the current slide transition sound
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
    void requestWaitSymbol();
    void releaseWaitSymbol();

    class WaitSymbolLock {public:
        explicit WaitSymbolLock(SlideShowImpl& rSlideShowImpl) : mrSlideShowImpl(rSlideShowImpl)
            { mrSlideShowImpl.requestWaitSymbol(); }
        ~WaitSymbolLock()
            { mrSlideShowImpl.releaseWaitSymbol(); }
    private: SlideShowImpl& mrSlideShowImpl;
    };

    /// Filter requested cursor shape against hard slideshow cursors (wait, etc.)
    sal_Int16 calcActiveCursor( sal_Int16 nCursorShape ) const;

    /** This method is called asynchronously to finish the rewinding of an
        effect to the previous slide that was initiated earlier.
    */
    void rewindEffectToPreviousSlide();

    /// all registered views
    UnoViewContainer                        maViewContainer;

    /// all registered slide show listeners
    comphelper::OInterfaceContainerHelper2         maListenerContainer;

    /// map of vectors, containing all registered listeners for a shape
    ShapeEventListenerMap                   maShapeEventListeners;
    /// map of sal_Int16 values, specifying the mouse cursor for every shape
    ShapeCursorMap                          maShapeCursors;

    //map of vector of Polygons, containing polygons drawn on each slide.
    PolygonMap                              maPolygons;

    boost::optional<RGBColor>               maUserPaintColor;

    double                                  maUserPaintStrokeWidth;

    //changed for the eraser project
    boost::optional<bool>           maEraseAllInk;
    boost::optional<bool>           maSwitchPenMode;
    boost::optional<bool>           maSwitchEraserMode;
    boost::optional<sal_Int32>          maEraseInk;
    //end changed

    std::shared_ptr<canvas::tools::ElapsedTime> mpPresTimer;
    ScreenUpdater                           maScreenUpdater;
    EventQueue                              maEventQueue;
    EventMultiplexer                        maEventMultiplexer;
    ActivitiesQueue                         maActivitiesQueue;
    UserEventQueue                          maUserEventQueue;
    SubsettableShapeManagerSharedPtr        mpDummyPtr;

    std::shared_ptr<SeparateListenerImpl> mpListener;

    std::shared_ptr<RehearseTimingsActivity> mpRehearseTimingsActivity;
    std::shared_ptr<WaitSymbol>           mpWaitSymbol;

    std::shared_ptr<PointerSymbol>        mpPointerSymbol;

    /// the current slide transition sound object:
    SoundPlayerSharedPtr                    mpCurrentSlideTransitionSound;

    uno::Reference<uno::XComponentContext>  mxComponentContext;
    uno::Reference<
        presentation::XTransitionFactory>   mxOptionalTransitionFactory;

    /// the previously running slide
    SlideSharedPtr                          mpPreviousSlide;
    /// the currently running slide
    SlideSharedPtr                          mpCurrentSlide;
    /// the already prefetched slide: best candidate for upcoming slide
    SlideSharedPtr                          mpPrefetchSlide;
    /// slide to be prefetched: best candidate for upcoming slide
    uno::Reference<drawing::XDrawPage>      mxPrefetchSlide;
    ///  save the XDrawPagesSupplier to retrieve polygons
    uno::Reference<drawing::XDrawPagesSupplier>  mxDrawPagesSupplier;
    /// slide animation to be prefetched:
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
                                             public AnimationEventHandler
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

    SeparateListenerImpl( const SeparateListenerImpl& ) = delete;
    SeparateListenerImpl& operator=( const SeparateListenerImpl& ) = delete;

    // EventHandler
    virtual bool handleEvent() override
    {
        // DON't call notifySlideAnimationsEnded()
        // directly, but queue an event. handleEvent()
        // might be called from e.g.
        // showNext(), and notifySlideAnimationsEnded() must not be called
        // in recursion.  Note that the event is scheduled for the next
        // frame so that its expensive execution does not come in between
        // sprite hiding and shape redraw (at the end of the animation of a
        // shape), which would cause a flicker.
        mrEventQueue.addEventForNextRound(
            makeEvent( [this] () { this->mrShow.notifySlideAnimationsEnded(); },
                "SlideShowImpl::notifySlideAnimationsEnded"));
        return true;
    }

    // ViewRepaintHandler
    virtual void viewClobbered( const UnoViewSharedPtr& rView ) override
    {
        // given view needs repaint, request update
        mrScreenUpdater.notifyUpdate(rView, true);
    }

    // HyperlinkHandler
    virtual bool handleHyperlink( OUString const& rLink ) override
    {
        return mrShow.notifyHyperLinkClicked(rLink);
    }

    // AnimationEventHandler
    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode ) override
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
    // keep care not constructing any UNO references to this inside ctor,
    // shift that code to create()!

    uno::Reference<lang::XMultiComponentFactory> xFactory(
        mxComponentContext->getServiceManager() );

    if( xFactory.is() )
    {
        try
    {
            // #i82460# try to retrieve special transition factory
            mxOptionalTransitionFactory.set(
                xFactory->createInstanceWithContext(
                    "com.sun.star.presentation.TransitionFactory",
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

// we are about to be disposed (someone call dispose() on us)
void SlideShowImpl::disposing()
{
    osl::MutexGuard const guard( m_aMutex );

    maEffectRewinder.dispose();

    // stop slide transition sound, if any:
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

    // send all listeners a disposing() that we are going down:
    maListenerContainer.disposeAndClear(
        lang::EventObject( static_cast<cppu::OWeakObject *>(this) ) );

    maViewContainer.dispose();

    // release slides:
    mxPrefetchAnimationNode.clear();
    mxPrefetchSlide.clear();
    mpPrefetchSlide.reset();
    mpCurrentSlide.reset();
    mpPreviousSlide.reset();
}

/// stops the current slide transition sound
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
    bool bStopSound = false;
    OUString url;

    if( !(rSound >>= bStopSound) )
        bStopSound = false;
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
            // catch possible exceptions from SoundPlayer, since
            // being not able to playback the sound is not a hard
            // error here (still, the slide transition should be
            // shown).
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

    // return empty transition, if slide transitions
    // are disabled.
    if (mbNoSlideTransitions)
        return ActivitySharedPtr();

    // retrieve slide change parameters from XDrawPage
    uno::Reference< beans::XPropertySet > xPropSet( xDrawPage,
                                                    uno::UNO_QUERY );

    if( !xPropSet.is() )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Slide has no PropertySet - assuming no transition" );
        return ActivitySharedPtr();
    }

    sal_Int16 nTransitionType(0);
    if( !getPropertyValue( nTransitionType,
                           xPropSet,
                           "TransitionType") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Could not extract slide transition type from XDrawPage - assuming no transition" );
        return ActivitySharedPtr();
    }

    sal_Int16 nTransitionSubType(0);
    if( !getPropertyValue( nTransitionSubType,
                           xPropSet,
                           "TransitionSubtype") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Could not extract slide transition subtype from XDrawPage - assuming no transition" );
        return ActivitySharedPtr();
    }

    bool bTransitionDirection(false);
    if( !getPropertyValue( bTransitionDirection,
                           xPropSet,
                           "TransitionDirection") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Could not extract slide transition direction from XDrawPage - assuming default direction" );
    }

    sal_Int32 aUnoColor(0);
    if( !getPropertyValue( aUnoColor,
                           xPropSet,
                           "TransitionFadeColor") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Could not extract slide transition fade color from XDrawPage - assuming black" );
    }

    const RGBColor aTransitionFadeColor( unoColor2RGBColor( aUnoColor ));

    uno::Any aSound;
    bool bLoopSound = false;

    if( !getPropertyValue( aSound, xPropSet, "Sound") )
        SAL_INFO("slideshow", "createSlideTransition(): Could not determine transition sound effect URL from XDrawPage - using no sound" );

    if( !getPropertyValue( bLoopSound, xPropSet, "LoopSound" ) )
        SAL_INFO("slideshow", "createSlideTransition(): Could not get slide property 'LoopSound' - using no sound" );

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
        return ActivitySharedPtr(); // no transition effect has been
                                    // generated. Normally, that means
                                    // that simply no transition is
                                    // set on this slide.

    double nTransitionDuration(0.0);
    if( !getPropertyValue( nTransitionDuration,
                           xPropSet,
                           "TransitionDuration") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "Could not extract slide transition duration from XDrawPage - assuming no transition" );
        return ActivitySharedPtr();
    }

    sal_Int32 nMinFrames(5);
    if( !getPropertyValue( nMinFrames,
                           xPropSet,
                           "MinimalFrameNumber") )
    {
        SAL_INFO("slideshow", "createSlideTransition(): "
                   "No minimal number of frames given - assuming 5" );
    }

    // prefetch slide transition bitmaps, but postpone it after
    // displaySlide() has finished - sometimes, view size has not yet
    // reached final size
    maEventQueue.addEvent(
        makeEvent( [pTransition] () {
                        pTransition->prefetch(
                            AnimatableShapeSharedPtr(),
                            ShapeAttributeLayerSharedPtr()); },
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
    // TODO(P2): optimize research in the map.
    PolygonMap::iterator aEnd = maPolygons.end();
    for( PolygonMap::iterator aIter = maPolygons.begin();
         aIter != aEnd;
         ++aIter )
        if( aIter->first == xDrawPage )
            return aIter;

    return aEnd;
}

SlideSharedPtr SlideShowImpl::makeSlide(
    uno::Reference<drawing::XDrawPage> const&          xDrawPage,
    uno::Reference<drawing::XDrawPagesSupplier> const& xDrawPages,
    uno::Reference<animations::XAnimationNode> const&  xRootNode )
{
    if( !xDrawPage.is() )
        return SlideSharedPtr();

    //Retrieve polygons for the current slide
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

    // prefetch show content (reducing latency for slide
    // bitmap and effect start later on)
    pSlide->prefetch();

    return pSlide;
}

void SlideShowImpl::requestWaitSymbol()
{
    ++mnWaitSymbolRequestCount;
    OSL_ASSERT(mnWaitSymbolRequestCount>0);

    if (mnWaitSymbolRequestCount == 1)
    {
        if( !mpWaitSymbol )
        {
            // fall back to cursor
            requestCursor(calcActiveCursor(mnCurrentCursor));
        }
        else
            mpWaitSymbol->show();
    }
}

void SlideShowImpl::releaseWaitSymbol()
{
    --mnWaitSymbolRequestCount;
    OSL_ASSERT(mnWaitSymbolRequestCount>=0);

    if (mnWaitSymbolRequestCount == 0)
    {
        if( !mpWaitSymbol )
        {
            // fall back to cursor
            requestCursor(calcActiveCursor(mnCurrentCursor));
        }
        else
            mpWaitSymbol->hide();
    }
}

sal_Int16 SlideShowImpl::calcActiveCursor( sal_Int16 nCursorShape ) const
{
    if( mnWaitSymbolRequestCount>0 && !mpWaitSymbol ) // enforce wait cursor
        nCursorShape = awt::SystemPointer::WAIT;
    else if( !mbMouseVisible ) // enforce INVISIBLE
        nCursorShape = awt::SystemPointer::INVISIBLE;
    else if( maUserPaintColor &&
             nCursorShape == awt::SystemPointer::ARROW )
        nCursorShape = awt::SystemPointer::PEN;

    return nCursorShape;
}

void SlideShowImpl::stopShow()
{
    // Force-end running animation
    // ===========================
    if (mpCurrentSlide)
    {
        mpCurrentSlide->hide();
        //Register polygons in the map
        if(findPolygons(mpCurrentSlide->getXDrawPage()) != maPolygons.end())
            maPolygons.erase(mpCurrentSlide->getXDrawPage());

        maPolygons.insert(make_pair(mpCurrentSlide->getXDrawPage(),mpCurrentSlide->getPolygons()));
    }

    // clear all queues
    maEventQueue.clear();
    maActivitiesQueue.clear();

    // Attention: we MUST clear the user event queue here,
    // this is because the current slide might have registered
    // shape events (click or enter/leave), which might
    // otherwise dangle forever in the queue (because of the
    // shared ptr nature). If someone needs to change this:
    // somehow unregister those shapes at the user event queue
    // on notifySlideEnded().
    maUserEventQueue.clear();

    // re-enable automatic effect advancement
    // (maEventQueue.clear() above might have killed
    // maEventMultiplexer's tick events)
    if (mbAutomaticAdvancementMode)
    {
        // toggle automatic mode (enabling just again is
        // ignored by EventMultiplexer)
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
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    maEffectRewinder.setRootAnimationNode(xRootNode);

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    mxDrawPagesSupplier = xDrawPages;

    stopShow();  // MUST call that: results in
    // maUserEventQueue.clear(). What's more,
    // stopShow()'s currSlide->hide() call is
    // now also required, notifySlideEnded()
    // relies on that
    // unconditionally. Otherwise, genuine
    // shape animations (drawing layer and
    // GIF) will not be stopped.

    bool bSkipAllMainSequenceEffects (false);
    bool bSkipSlideTransition (false);
    std::for_each( rProperties.getConstArray(),
                   rProperties.getConstArray() + rProperties.getLength(),
        PrefetchPropertiesFunc(this, bSkipAllMainSequenceEffects, bSkipSlideTransition) );

    OSL_ENSURE( !maViewContainer.empty(), "### no views!" );
    if (maViewContainer.empty())
        return;

    // this here might take some time
    {
        WaitSymbolLock aLock (*this);

        mpPreviousSlide = mpCurrentSlide;
        mpCurrentSlide.reset();

        if (matches( mpPrefetchSlide, xSlide, xRootNode ))
        {
            // prefetched slide matches:
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

            // push new transformation to all views, if size changed
            if( !mpPreviousSlide || oldSlideSize != slideSize )
            {
                for( const auto& pView : maViewContainer )
                    pView->setViewSize( slideSize );

                // explicitly notify view change here,
                // because transformation might have changed:
                // optimization, this->notifyViewChange() would
                // repaint slide which is not necessary.
                maEventMultiplexer.notifyViewsChanged();
            }

            // create slide transition, and add proper end event
            // (which then starts the slide effects
            // via CURRENT_SLIDE.show())
            ActivitySharedPtr pSlideChangeActivity (
                createSlideTransition(
                    mpCurrentSlide->getXDrawPage(),
                    mpPreviousSlide,
                    mpCurrentSlide,
                    makeEvent(
                        [this] () { this->notifySlideTransitionEnded(false); },
                        "SlideShowImpl::notifySlideTransitionEnded")));

            if (bSkipSlideTransition)
            {
                // The transition activity was created for the side effects
                // (like sound transitions).  Because we want to skip the
                // actual transition animation we do not need the activity
                // anymore.
                pSlideChangeActivity.reset();
            }

            if (pSlideChangeActivity)
            {
                // factory generated a slide transition - activate it!
                maActivitiesQueue.addActivity( pSlideChangeActivity );
            }
            else
            {
                // no transition effect on this slide - schedule slide
                // effect start event right away.
                maEventQueue.addEvent(
                    makeEvent(
                        [this] () { this->notifySlideTransitionEnded(true); },
                        "SlideShowImpl::notifySlideTransitionEnded"));
            }
        }
    } // finally

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionStarted();
        });

    // We are currently rewinding an effect.  This lead us from the next
    // slide to this one.  To complete this we have to play back all main
    // sequence effects on this slide.
    if (bSkipAllMainSequenceEffects)
        maEffectRewinder.skipAllMainSequenceEffects();
}

void SlideShowImpl::redisplayCurrentSlide()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();
    stopShow();

    OSL_ENSURE( !maViewContainer.empty(), "### no views!" );
    if (maViewContainer.empty())
        return;

    // No transition effect on this slide - schedule slide
    // effect start event right away.
    maEventQueue.addEvent(
        makeEvent( [this] () { this->notifySlideTransitionEnded(true); },
            "SlideShowImpl::notifySlideTransitionEnded"));

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideTransitionStarted();
        });
}

sal_Bool SlideShowImpl::nextEffect()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    if (mbShowPaused)
        return true;
    else
        return maEventMultiplexer.notifyNextEffect();
}

sal_Bool SlideShowImpl::previousEffect()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    if (mbShowPaused)
        return true;
    else
    {
        return maEffectRewinder.rewind(
            maScreenUpdater.createLock(),
            [this]() { return this->redisplayCurrentSlide(); },
            [this]() { return this->rewindEffectToPreviousSlide(); } );
    }
}

void SlideShowImpl::rewindEffectToPreviousSlide()
{
    // Show the wait symbol now and prevent it from showing temporary slide
    // content while effects are played back.
    WaitSymbolLock aLock (*this);

    // A previous call to EffectRewinder::Rewind could not rewind the current
    // effect because there are no effects on the current slide or none has
    // yet been displayed.  Go to the previous slide.
    notifySlideEnded(true);

    // Process pending events once more in order to have the following
    // screen update show the last effect.  Not sure whether this should be
    // necessary.
    maEventQueue.forceEmpty();

    // We have to call the screen updater before the wait symbol is turned
    // off.  Otherwise the wait symbol would force the display of an
    // intermediate state of the slide (before the effects are replayed.)
    maScreenUpdater.commitUpdates();
}

sal_Bool SlideShowImpl::startShapeActivity(
    uno::Reference<drawing::XShape> const& /*xShape*/ )
{
    osl::MutexGuard const guard( m_aMutex );

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    // TODO(F3): NYI
    OSL_FAIL( "not yet implemented!" );
    return false;
}

sal_Bool SlideShowImpl::stopShapeActivity(
    uno::Reference<drawing::XShape> const& /*xShape*/ )
{
    osl::MutexGuard const guard( m_aMutex );

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    // TODO(F3): NYI
    OSL_FAIL( "not yet implemented!" );
    return false;
}

sal_Bool SlideShowImpl::pause( sal_Bool bPauseShow )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: must only be called from the main thread!
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
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return uno::Reference<drawing::XDrawPage>();

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    if (mpCurrentSlide)
        return mpCurrentSlide->getXDrawPage();
    else
        return uno::Reference<drawing::XDrawPage>();
}

sal_Bool SlideShowImpl::addView(
    uno::Reference<presentation::XSlideShowView> const& xView )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    // first of all, check if view has a valid canvas
    ENSURE_OR_RETURN_FALSE( xView.is(), "addView(): Invalid view" );
    ENSURE_OR_RETURN_FALSE( xView->getCanvas().is(),
                       "addView(): View does not provide a valid canvas" );

    UnoViewSharedPtr const pView( createSlideView(
                                      xView,
                                      maEventQueue,
                                      maEventMultiplexer ));
    if (!maViewContainer.addView( pView ))
        return false; // view already added

    // initialize view content
    // =======================

    if (mpCurrentSlide)
    {
        // set view transformation
        const basegfx::B2ISize slideSize = mpCurrentSlide->getSlideSize();
        pView->setViewSize( basegfx::B2DSize( slideSize.getX(),
                                              slideSize.getY() ) );
    }

    // clear view area (since its newly added,
    // we need a clean slate)
    pView->clearAll();

    // broadcast newly added view
    maEventMultiplexer.notifyViewAdded( pView );

    // set current mouse ptr
    pView->setCursorShape( calcActiveCursor(mnCurrentCursor) );

    return true;
}

sal_Bool SlideShowImpl::removeView(
    uno::Reference<presentation::XSlideShowView> const& xView )
{
    osl::MutexGuard const guard( m_aMutex );

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    ENSURE_OR_RETURN_FALSE( xView.is(), "removeView(): Invalid view" );

    UnoViewSharedPtr const pView( maViewContainer.removeView( xView ) );
    if( !pView )
        return false; // view was not added in the first place

    // remove view from EventMultiplexer (mouse events etc.)
    maEventMultiplexer.notifyViewRemoved( pView );

    pView->_dispose();

    return true;
}

void SlideShowImpl::registerUserPaintPolygons( const uno::Reference< lang::XMultiServiceFactory >& xDocFactory )
{
    //Retrieve Polygons if user ends presentation by context menu
    if (mpCurrentSlide)
    {
        if(findPolygons(mpCurrentSlide->getXDrawPage()) != maPolygons.end())
            maPolygons.erase(mpCurrentSlide->getXDrawPage());

        maPolygons.insert(make_pair(mpCurrentSlide->getXDrawPage(),mpCurrentSlide->getPolygons()));
    }

    //Creating the layer for shapes
    // query for the XLayerManager
    uno::Reference< drawing::XLayerSupplier > xLayerSupplier(xDocFactory, uno::UNO_QUERY);
    uno::Reference< container::XNameAccess > xNameAccess = xLayerSupplier->getLayerManager();

    uno::Reference< drawing::XLayerManager > xLayerManager(xNameAccess, uno::UNO_QUERY);
    // create a layer and set its properties
    uno::Reference< drawing::XLayer > xDrawnInSlideshow = xLayerManager->insertNewByIndex(xLayerManager->getCount());
    uno::Reference< beans::XPropertySet > xLayerPropSet(xDrawnInSlideshow, uno::UNO_QUERY);

    //Layer Name which enables to catch annotations
    OUString layerName = "DrawnInSlideshow";
    uno::Any aPropLayer;

    aPropLayer <<= layerName;
    xLayerPropSet->setPropertyValue("Name", aPropLayer);

    aPropLayer <<= true;
    xLayerPropSet->setPropertyValue("IsVisible", aPropLayer);

    aPropLayer <<= false;
    xLayerPropSet->setPropertyValue("IsLocked", aPropLayer);

    //Register polygons for each slide
    for( const auto& rPoly : maPolygons )
    {
        PolyPolygonVector aPolygons = rPoly.second;
        //Get shapes for the slide
        css::uno::Reference< css::drawing::XShapes > Shapes(rPoly.first, css::uno::UNO_QUERY);
        //Retrieve polygons for one slide
        for( const auto& pPolyPoly : aPolygons )
        {
            ::basegfx::B2DPolyPolygon b2DPolyPoly = ::basegfx::unotools::b2DPolyPolygonFromXPolyPolygon2D(pPolyPoly->getUNOPolyPolygon());

            //Normally there is only one polygon
            for(sal_uInt32 i=0; i< b2DPolyPoly.count();i++)
            {
                const ::basegfx::B2DPolygon& aPoly =  b2DPolyPoly.getB2DPolygon(i);
                sal_uInt32 nPoints = aPoly.count();

                if( nPoints > 1)
                {
                    //create the PolyLineShape
                    uno::Reference< uno::XInterface > polyshape(xDocFactory->createInstance(
                                                                    "com.sun.star.drawing.PolyLineShape" ) );
                    uno::Reference< drawing::XShape > rPolyShape(polyshape, uno::UNO_QUERY);

                    //Add the shape to the slide
                    Shapes->add(rPolyShape);

                    //Retrieve shape properties
                    uno::Reference< beans::XPropertySet > aXPropSet( rPolyShape, uno::UNO_QUERY );
                    //Construct a sequence of points sequence
                    drawing::PointSequenceSequence aRetval;
                    //Create only one sequence for one polygon
                    aRetval.realloc( 1 );
                    // Retrieve the sequence of points from aRetval
                    drawing::PointSequence* pOuterSequence = aRetval.getArray();
                    // Create 2 points in this sequence
                    pOuterSequence->realloc(nPoints);
                    // Get these points which are in an array
                    awt::Point* pInnerSequence = pOuterSequence->getArray();
                    for( sal_uInt32 n = 0; n < nPoints; n++ )
                    {
                        //Create a point from the polygon
                        *pInnerSequence++ = awt::Point(
                            basegfx::fround(aPoly.getB2DPoint(n).getX()),
                            basegfx::fround(aPoly.getB2DPoint(n).getY()));
                    }

                    //Fill the properties
                    //Give the built PointSequenceSequence.
                    uno::Any aParam;
                    aParam <<= aRetval;
                    aXPropSet->setPropertyValue("PolyPolygon", aParam );

                    //LineStyle : SOLID by default
                    drawing::LineStyle  eLS;
                    eLS = drawing::LineStyle_SOLID;
                    aXPropSet->setPropertyValue("LineStyle", uno::Any(eLS) );

                    //LineColor
                    sal_uInt32          nLineColor;
                    nLineColor = pPolyPoly->getRGBALineColor();
                    //Transform polygon color from RRGGBBAA to AARRGGBB
                    aXPropSet->setPropertyValue("LineColor", uno::Any(RGBAColor2UnoColor(nLineColor)) );

                    //LineWidth
                    double              fLineWidth;
                    fLineWidth = pPolyPoly->getStrokeWidth();
                    aXPropSet->setPropertyValue("LineWidth", uno::Any((sal_Int32)fLineWidth) );

                    // make polygons special
                    xLayerManager->attachShapeToLayer(rPolyShape, xDrawnInSlideshow);
                }
            }
        }
    }
}

sal_Bool SlideShowImpl::setProperty( beans::PropertyValue const& rProperty )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: must only be called from the main thread!
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

            // enable user paint
            maUserPaintColor.reset( unoColor2RGBColor( nColor ) );
            if( mpCurrentSlide && !mpCurrentSlide->isPaintOverlayActive() )
                mpCurrentSlide->enablePaintOverlay();

            maEventMultiplexer.notifyUserPaintColor( *maUserPaintColor );
        }
        else
        {
            // disable user paint
            maUserPaintColor.reset();
            maEventMultiplexer.notifyUserPaintDisabled();
            if( mpCurrentSlide )
                mpCurrentSlide->disablePaintOverlay();
        }

        resetCursor();

        return true;
    }

    //adding support for erasing features in UserPaintOverlay
    if ( rProperty.Name == "EraseAllInk" )
    {
        bool bEraseAllInk(false);
        if (rProperty.Value >>= bEraseAllInk)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            // enable user paint
            maEraseAllInk.reset( bEraseAllInk );
            maEventMultiplexer.notifyEraseAllInk( *maEraseAllInk );
        }

        return true;
    }

    if ( rProperty.Name == "SwitchPenMode" )
    {
        bool bSwitchPenMode(false);
        if (rProperty.Value >>= bSwitchPenMode)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );

            if(bSwitchPenMode){
            // Switch to Pen Mode
            maSwitchPenMode.reset( bSwitchPenMode );
            maEventMultiplexer.notifySwitchPenMode();
            }
        }
        return true;
    }

    if ( rProperty.Name == "SwitchEraserMode" )
    {
        bool bSwitchEraserMode(false);
        if (rProperty.Value >>= bSwitchEraserMode)
        {
            OSL_ENSURE( mbMouseVisible,
                        "setProperty(): User paint overrides invisible mouse" );
            if(bSwitchEraserMode){
            // switch to Eraser mode
            maSwitchEraserMode.reset( bSwitchEraserMode );
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

            // enable user paint
            maEraseInk.reset( nEraseInk );
            maEventMultiplexer.notifyEraseInkWidth( *maEraseInk );
        }

        return true;
    }

    // new Property for pen's width
    if ( rProperty.Name == "UserPaintStrokeWidth" )
    {
        double nWidth(4.0);
        if (rProperty.Value >>= nWidth)
        {
            OSL_ENSURE( mbMouseVisible,"setProperty(): User paint overrides invisible mouse" );
            // enable user paint stroke width
            maUserPaintStrokeWidth = nWidth;
            maEventMultiplexer.notifyUserPaintStrokeWidth( maUserPaintStrokeWidth );
        }

        return true;
    }

    if ( rProperty.Name == "AdvanceOnClick" )
    {
        bool bAdvanceOnClick = false;
        if (! (rProperty.Value >>= bAdvanceOnClick))
            return false;
        maUserEventQueue.setAdvanceOnClick( bAdvanceOnClick );
        return true;
    }

    if ( rProperty.Name == "DisableAnimationZOrder" )
    {
        bool bDisableAnimationZOrder = false;
        if (! (rProperty.Value >>= bDisableAnimationZOrder))
            return false;
        mbDisableAnimationZOrder = bDisableAnimationZOrder;
        return true;
    }

    if ( rProperty.Name == "ImageAnimationsAllowed" )
    {
        if (! (rProperty.Value >>= mbImageAnimationsAllowed))
            return false;

        // TODO(F3): Forward to slides!
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
            // TODO(Q3): Move to slide
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
            // removes timer from all views:
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
        css::geometry::RealPoint2D pos;
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
        bool bValue (false);
        if ((rProperty.Value >>= aValues)
            && aValues.getLength()==2
            && (aValues[0] >>= xView)
            && (aValues[1] >>= bValue))
        {
            // Look up the view.
            for (UnoViewVector::const_iterator
                     iView (maViewContainer.begin()),
                     iEnd (maViewContainer.end());
                 iView!=iEnd;
                 ++iView)
            {
                if (*iView && (*iView)->getUnoView()==xView)
                {
                    // Store the flag at the view so that media shapes have
                    // access to it.
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
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    // container syncs with passed mutex ref
    maListenerContainer.addInterface(xListener);
}

void SlideShowImpl::removeSlideShowListener(
    uno::Reference<presentation::XSlideShowListener> const& xListener )
{
    osl::MutexGuard const guard( m_aMutex );

    // container syncs with passed mutex ref
    maListenerContainer.removeInterface(xListener);
}

void SlideShowImpl::addShapeEventListener(
    uno::Reference<presentation::XShapeEventListener> const& xListener,
    uno::Reference<drawing::XShape> const& xShape )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    ShapeEventListenerMap::iterator aIter;
    if( (aIter=maShapeEventListeners.find( xShape )) ==
        maShapeEventListeners.end() )
    {
        // no entry for this shape -> create one
        aIter = maShapeEventListeners.insert(
            ShapeEventListenerMap::value_type(
                xShape,
                std::make_shared<comphelper::OInterfaceContainerHelper2>(
                    m_aMutex))).first;
    }

    // add new listener to broadcaster
    if( aIter->second.get() )
        aIter->second->addInterface( xListener );

    maEventMultiplexer.notifyShapeListenerAdded(xListener,
                                                xShape);
}

void SlideShowImpl::removeShapeEventListener(
    uno::Reference<presentation::XShapeEventListener> const& xListener,
    uno::Reference<drawing::XShape> const& xShape )
{
    osl::MutexGuard const guard( m_aMutex );

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    ShapeEventListenerMap::iterator aIter;
    if( (aIter = maShapeEventListeners.find( xShape )) !=
        maShapeEventListeners.end() )
    {
        // entry for this shape found -> remove listener from
        // helper object
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
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    // precondition: must only be called from the main thread!
    DBG_TESTSOLARMUTEX();

    ShapeCursorMap::iterator aIter;
    if( (aIter=maShapeCursors.find( xShape )) == maShapeCursors.end() )
    {
        // no entry for this shape -> create one
        if( nPointerShape != awt::SystemPointer::ARROW )
        {
            // add new entry, unless shape shall display
            // normal pointer arrow -> no need to handle that
            // case
            maShapeCursors.insert(
                ShapeCursorMap::value_type(xShape,
                                           nPointerShape) );
        }
    }
    else if( nPointerShape == awt::SystemPointer::ARROW )
    {
        // shape shall display normal cursor -> can disable
        // the cursor and clear the entry
        maShapeCursors.erase( xShape );
    }
    else
    {
        // existing entry found, update with new cursor ID
        aIter->second = nPointerShape;
    }
}

bool SlideShowImpl::requestCursor( sal_Int16 nCursorShape )
{
    mnCurrentCursor = nCursorShape;

    const sal_Int16 nActualCursor = calcActiveCursor(mnCurrentCursor);

    // change all views to the requested cursor ID
    for( const auto& pView : maViewContainer )
        pView->setCursorShape( nActualCursor );

    return nActualCursor==nCursorShape;
}

void SlideShowImpl::resetCursor()
{
    mnCurrentCursor = awt::SystemPointer::ARROW;

    const sal_Int16 nActualCursor = calcActiveCursor( mnCurrentCursor );
    // change all views to the default cursor ID
    for( const auto& pView : maViewContainer )
        pView->setCursorShape( nActualCursor );
}

sal_Bool SlideShowImpl::update( double & nNextTimeout )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    // precondition: update() must only be called from the
    // main thread!
    DBG_TESTSOLARMUTEX();

    if( mbShowPaused )
    {
        // commit frame (might be repaints pending)
        maScreenUpdater.commitUpdates();

        return false;
    }
    else
    {
        // TODO(F2): re-evaluate whether that timer lagging makes
        // sense.

        // hold timer, while processing the queues:
        // 1. when there is more than one active activity this ensures the
        //    same time for all activities and events
        // 2. processing of events may lead to creation of further events
        //    that have zero delay.  While the timer is stopped these events
        //    are processed in the same run.
        {
            //Get a shared-ptr that outlives the scope-guard which will
            //ensure that the pointed-to-item exists in the case of a
            //::dispose clearing mpPresTimer
            std::shared_ptr<canvas::tools::ElapsedTime> xTimer(mpPresTimer);
            comphelper::ScopeGuard scopeGuard(
                [&xTimer]() { return xTimer->releaseTimer(); } );
            xTimer->holdTimer();

            // process queues
            maEventQueue.process();

            // #i118671# the call above may execute a macro bound to an object. In
            // that case this macro may have destroyed this local sliseshow so that it
            // is disposed (see bugdoc at task). In that case, detect this and exit
            // gently from this slideshow. Do not forget to disable the scoped
            // call to mpPresTimer, this will be deleted if we are disposed.
            if (isDisposed())
            {
                scopeGuard.dismiss();
                return false;
            }

            maActivitiesQueue.process();

            // commit frame to screen
            maFrameSynchronization.Synchronize();
            maScreenUpdater.commitUpdates();

            // TODO(Q3): remove need to call dequeued() from
            // activities. feels like a wart.

            // Rationale for ActivitiesQueue::processDequeued(): when
            // an activity ends, it usually pushed the end state to
            // the animated shape in question, and ends the animation
            // (which, in turn, will usually disable shape sprite
            // mode). Disabling shape sprite mode causes shape
            // repaint, which, depending on slide content, takes
            // considerably more time than sprite updates. Thus, the
            // last animation step tends to look delayed. To
            // camouflage this, reaching end position and disabling
            // sprite mode is split into two (normal Activity::end(),
            // and Activity::dequeued()). Now, the reason to call
            // commitUpdates() twice here is caused by the unrelated
            // fact that during wait cursor display/hide, the screen
            // is updated, and shows hidden sprites, but, in case of
            // leaving the second commitUpdates() call out and punting
            // that to the next round, no updated static slide
            // content. In short, the last shape animation of a slide
            // tends to blink at its end.

            // process dequeued activities _after_ commit to screen
            maActivitiesQueue.processDequeued();

            // commit frame to screen
            maScreenUpdater.commitUpdates();
        }
        // Time held until here

        const bool bActivitiesLeft = (! maActivitiesQueue.isEmpty());
        const bool bTimerEventsLeft = (! maEventQueue.isEmpty());
        const bool bRet = (bActivitiesLeft || bTimerEventsLeft);

        if (bRet)
        {
            // calc nNextTimeout value:
            if (bActivitiesLeft)
            {
                // Activity queue is not empty.  Tell caller that we would
                // like to render another frame.

                // Return a zero time-out to signal our caller to call us
                // back as soon as possible.  The actual timing, waiting the
                // appropriate amount of time between frames, is then done
                // by the maFrameSynchronization object.
                nNextTimeout = 0;
                maFrameSynchronization.Activate();
            }
            else
            {
                // timer events left:
                // difference from current time (nota bene:
                // time no longer held here!) to the next event in
                // the event queue.

                // #i61190# Retrieve next timeout only _after_
                // processing activity queue

                // ensure positive value:
                nNextTimeout = std::max( 0.0, maEventQueue.nextTimeout() );

                // There is no active animation so the frame rate does not
                // need to be synchronized.
                maFrameSynchronization.Deactivate();
            }

            mbSlideShowIdle = false;
        }

#if defined(DBG_UTIL)
        // when slideshow is idle, issue an XUpdatable::update() call
        // exactly once after a previous animation sequence finished -
        // this might trigger screen dumps on some canvas
        // implementations
        if( !mbSlideShowIdle &&
            (!bRet ||
             nNextTimeout > 1.0) )
        {
            for( const auto& pView : maViewContainer )
            {
                try
                {
                    uno::Reference< presentation::XSlideShowView > xView( pView->getUnoView(),
                                                                          uno::UNO_QUERY_THROW );
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
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString( cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
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

        // first init show, to give the animations
        // the chance to register SlideStartEvents
        const bool bBackgroundLayerRendered( !bPaintSlide );
        mpCurrentSlide->show( bBackgroundLayerRendered );
        maEventMultiplexer.notifySlideStartEvent();
    }
}

void queryAutomaticSlideTransition( uno::Reference<drawing::XDrawPage> const& xDrawPage,
                                    double&                                   nAutomaticNextSlideTimeout,
                                    bool&                                     bHasAutomaticNextSlide )
{
    // retrieve slide change parameters from XDrawPage
    // ===============================================

    uno::Reference< beans::XPropertySet > xPropSet( xDrawPage,
                                                    uno::UNO_QUERY );

    sal_Int32 nChange(0);
    if( !xPropSet.is() ||
        !getPropertyValue( nChange,
                           xPropSet,
                           "Change") )
    {
        SAL_INFO("slideshow",
            "queryAutomaticSlideTransition(): "
            "Could not extract slide change mode from XDrawPage - assuming <none>" );
    }

    bHasAutomaticNextSlide = nChange == 1;

    if( !xPropSet.is() ||
        !getPropertyValue( nAutomaticNextSlideTimeout,
                           xPropSet,
                           "HighResDuration") )
    {
        SAL_INFO("slideshow",
            "queryAutomaticSlideTransition(): "
            "Could not extract slide transition timeout from "
            "XDrawPage - assuming 1 sec" );
    }
}

void SlideShowImpl::notifySlideAnimationsEnded()
{
    osl::MutexGuard const guard( m_aMutex );

    //Draw polygons above animations
    mpCurrentSlide->drawPolygons();

    OSL_ENSURE( !isDisposed(), "### already disposed!" );

    // This struct will receive the (interruptable) event,
    // that triggers the notifySlideEnded() method.
    InterruptableEventPair aNotificationEvents;

    if( maEventMultiplexer.getAutomaticMode() )
    {
        OSL_ENSURE( ! mpRehearseTimingsActivity,
                    "unexpected: RehearseTimings mode!" );

        // schedule a slide end event, with automatic mode's
        // delay
        aNotificationEvents = makeInterruptableDelay(
            [this]() { return this->notifySlideEnded( false ); },
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

        // check whether slide transition should happen
        // 'automatically'. If yes, simply schedule the
        // specified timeout.
        // NOTE: mbForceManualAdvance and mpRehearseTimingsActivity
        // override any individual slide setting, to always
        // step slides manually.
        if( !mbForceManualAdvance &&
            !mpRehearseTimingsActivity &&
            bHasAutomaticNextSlide )
        {
            aNotificationEvents = makeInterruptableDelay(
                [this]() { return this->notifySlideEnded( false ); },
                nAutomaticNextSlideTimeout);

            // TODO(F2): Provide a mechanism to let the user override
            // this automatic timeout via next()
        }
        else
        {
            if (mpRehearseTimingsActivity)
                mpRehearseTimingsActivity->start();

            // generate click event. Thus, the user must
            // trigger the actual end of a slide. No need to
            // generate interruptable event here, there's no
            // timeout involved.
            aNotificationEvents.mpImmediateEvent =
                makeEvent( [this] () { this->notifySlideEnded(false); },
                    "SlideShowImpl::notifySlideEnded");
        }
    }

    // register events on the queues. To make automatic slide
    // changes interruptable, register the interruption event
    // as a nextEffectEvent target. Note that the timeout
    // event is optional (e.g. manual slide changes don't
    // generate a timeout)
    maUserEventQueue.registerNextEffectEvent(
        aNotificationEvents.mpImmediateEvent );

    if( aNotificationEvents.mpTimeoutEvent )
        maEventQueue.addEvent( aNotificationEvents.mpTimeoutEvent );

    // current slide's main sequence is over. Now should be
    // the time to prefetch the next slide (if any), and
    // prepare the initial slide bitmap (speeds up slide
    // change setup time a lot). Show the wait cursor, this
    // indeed might take some seconds.
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
            // ignore return value, this is just to populate
            // Slide's internal bitmap buffer, such that the time
            // needed to generate the slide bitmap is not spent
            // when the slide change is requested.
            mpPrefetchSlide->getCurrentSlideBitmap( *maViewContainer.begin() );
        }
    } // finally

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        [](uno::Reference<presentation::XSlideShowListener> const& xListener)
        {
            xListener->slideAnimationsEnded();
        });
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
            // save time at current drawpage:
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

    stopShow();  // MUST call that: results in
                 // maUserEventQueue.clear(). What's more,
                 // stopShow()'s currSlide->hide() call is
                 // now also required, notifySlideEnded()
                 // relies on that
                 // unconditionally. Otherwise, genuine
                 // shape animations (drawing layer and
                 // GIF) will not be stopped.

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        [&bReverse]( const uno::Reference< presentation::XSlideShowListener >& xListener )
        { return xListener->slideEnded( bReverse ); } );
}

bool SlideShowImpl::notifyHyperLinkClicked( OUString const& hyperLink )
{
    osl::MutexGuard const guard( m_aMutex );

    maListenerContainer.forEach<presentation::XSlideShowListener>(
        [&hyperLink]( const uno::Reference< presentation::XSlideShowListener >& xListener )
        { return xListener->hyperLinkClicked( hyperLink ); } );
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
            [&xNode]( const uno::Reference< animations::XAnimationListener >& xListener )
            { return xListener->beginEvent( xNode ); } );
        break;

    case AnimationNode::FROZEN:
    case AnimationNode::ENDED:
        maListenerContainer.forEach<presentation::XSlideShowListener>(
            [&xNode]( const uno::Reference< animations::XAnimationListener >& xListener )
            { return xListener->endEvent( xNode ); } );
        if(mpCurrentSlide->isPaintOverlayActive())
           mpCurrentSlide->drawPolygons();
        break;
    default:
        break;
    }

    return true;
}

//===== FrameSynchronization ==================================================

FrameSynchronization::FrameSynchronization (const double nFrameDuration)
    : maTimer(),
      mnFrameDuration(nFrameDuration),
      mnNextFrameTargetTime(0),
      mbIsActive(false)
{
    MarkCurrentFrame();
}

void FrameSynchronization::MarkCurrentFrame()
{
    mnNextFrameTargetTime = maTimer.getElapsedTime() + mnFrameDuration;
}

void FrameSynchronization::Synchronize()
{
    if (mbIsActive)
    {
        // Do busy waiting for now.
        while (maTimer.getElapsedTime() < mnNextFrameTargetTime)
            ;
    }

    MarkCurrentFrame();
}

void FrameSynchronization::Activate()
{
    mbIsActive = true;
}

void FrameSynchronization::Deactivate()
{
    mbIsActive = false;
}

} // anon namespace

namespace sdecl = comphelper::service_decl;
const sdecl::ServiceDecl slideShowDecl(
     sdecl::class_<SlideShowImpl>(),
    "com.sun.star.comp.presentation.SlideShow",
    "com.sun.star.presentation.SlideShow" );

// The C shared lib entry points
extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL slideshow_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&slideShowDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
