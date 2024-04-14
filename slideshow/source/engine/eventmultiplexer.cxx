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


#include <comphelper/diagnose_ex.hxx>

#include <rtl/ref.hxx>
#include <comphelper/compbase.hxx>

#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include <eventqueue.hxx>
#include <eventmultiplexer.hxx>
#include <listenercontainer.hxx>
#include <delayevent.hxx>
#include <unoview.hxx>
#include <unoviewcontainer.hxx>

#include <functional>
#include <memory>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

using namespace ::com::sun::star;

namespace
{
    // add operator== for weak_ptr, so we can use std::find over lists of them
    struct ViewEventHandlerWeakPtrWrapper final {
        slideshow::internal::ViewEventHandlerWeakPtr ptr;

        ViewEventHandlerWeakPtrWrapper(slideshow::internal::ViewEventHandlerWeakPtr thePtr):
            ptr(std::move(thePtr)) {}

        bool operator ==(ViewEventHandlerWeakPtrWrapper const & other) const
        { return ptr.lock().get() == other.ptr.lock().get(); }
    };
}

// Needed by ImplViewHandlers; see the ListenerOperations<std::weak_ptr<ListenerTargetT>> partial
// specialization in slideshow/source/inc/listenercontainer.hxx:
template<>
struct slideshow::internal::ListenerOperations<ViewEventHandlerWeakPtrWrapper>
{
    template< typename ContainerT,
              typename FuncT >
    static bool notifySingleListener( ContainerT& rContainer,
                                      FuncT       func )
    {
        for( const auto& rCurr : rContainer )
        {
            std::shared_ptr<ViewEventHandler> pListener( rCurr.ptr.lock() );

            if( pListener && func(pListener) )
                return true;
        }

        return false;
    }

    template< typename ContainerT,
              typename FuncT >
    static bool notifyAllListeners( ContainerT& rContainer,
                                    FuncT       func )
    {
        bool bRet(false);
        for( const auto& rCurr : rContainer )
        {
            std::shared_ptr<ViewEventHandler> pListener( rCurr.ptr.lock() );

            if( pListener.get() &&
                FunctionApply<typename ::std::invoke_result<FuncT, std::shared_ptr<ViewEventHandler> const&>::type,
                               std::shared_ptr<ViewEventHandler> >::apply(func,pListener) )
            {
                bRet = true;
            }
        }

        return bRet;
    }
    template< typename ContainerT >
    static void pruneListeners( ContainerT& rContainer,
                                size_t      nSizeThreshold )
    {
        if( rContainer.size() <= nSizeThreshold )
            return;

        ContainerT aAliveListeners;
        aAliveListeners.reserve(rContainer.size());

        for( const auto& rCurr : rContainer )
        {
            if( !rCurr.ptr.expired() )
                aAliveListeners.push_back( rCurr );
        }

        std::swap( rContainer, aAliveListeners );
    }
};

namespace slideshow::internal {

namespace {

template <typename HandlerT>
class PrioritizedHandlerEntry
{
    typedef std::shared_ptr<HandlerT> HandlerSharedPtrT;
    HandlerSharedPtrT mpHandler;
    double            mnPrio;

public:
    PrioritizedHandlerEntry( HandlerSharedPtrT pHandler,
                             double                   nPrio ) :
        mpHandler(std::move(pHandler)),
        mnPrio(nPrio)
    {}

    HandlerSharedPtrT const& getHandler() const { return mpHandler; }

    /// To sort according to priority
    bool operator<( PrioritizedHandlerEntry const& rRHS ) const
    {
        // reversed order - high prioritized entries
        // should be at the beginning of the queue
        return mnPrio > rRHS.mnPrio;
    }

    /// To permit std::remove in removeHandler template
    bool operator==( PrioritizedHandlerEntry const& rRHS ) const
    {
        // ignore prio, for removal, only the handler ptr matters
        return mpHandler == rRHS.mpHandler;
    }
};

}

typedef comphelper::WeakComponentImplHelper<
    awt::XMouseListener,
    awt::XMouseMotionListener > Listener_UnoBase;

namespace {

/** Listener class, to decouple UNO lifetime from EventMultiplexer

    This class gets registered as the XMouse(Motion)Listener on the
    XSlideViews, and passes on the events to the EventMultiplexer (via
    EventQueue indirection, to force the events into the main thread)
 */
class EventMultiplexerListener : public Listener_UnoBase
{
public:
    EventMultiplexerListener( EventQueue&           rEventQueue,
                              EventMultiplexerImpl& rEventMultiplexer ) :
        mpEventQueue( &rEventQueue ),
        mpEventMultiplexer( &rEventMultiplexer )
    {
    }

    EventMultiplexerListener( const EventMultiplexerListener& ) = delete;
    EventMultiplexerListener& operator=( const EventMultiplexerListener& ) = delete;

    // WeakComponentImplHelperBase::disposing
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

private:
    virtual void SAL_CALL disposing( const lang::EventObject& Source ) override;

    // XMouseListener implementation
    virtual void SAL_CALL mousePressed( const awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseReleased( const awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseEntered( const awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseExited( const awt::MouseEvent& e ) override;

    // XMouseMotionListener implementation
    virtual void SAL_CALL mouseDragged( const awt::MouseEvent& e ) override;
    virtual void SAL_CALL mouseMoved( const awt::MouseEvent& e ) override;


    EventQueue*           mpEventQueue;
    EventMultiplexerImpl* mpEventMultiplexer;
};

}

struct EventMultiplexerImpl
{
    EventMultiplexerImpl( EventQueue&             rEventQueue,
                          UnoViewContainer const& rViewContainer ) :
        mrEventQueue(rEventQueue),
        mrViewContainer(rViewContainer),
        mxListener( new EventMultiplexerListener(rEventQueue,
                                                 *this) ),
        maNextEffectHandlers(),
        maSlideStartHandlers(),
        maSlideEndHandlers(),
        maAnimationStartHandlers(),
        maAnimationEndHandlers(),
        maSlideAnimationsEndHandlers(),
        maAudioStoppedHandlers(),
        maCommandStopAudioHandlers(),
        maPauseHandlers(),
        maViewHandlers(),
        maViewRepaintHandlers(),
        maShapeListenerHandlers(),
        maUserPaintEventHandlers(),
        maMouseClickHandlers(),
        maMouseDoubleClickHandlers(),
        maMouseMoveHandlers(),
        maHyperlinkHandlers(),
        mnTimeout(0.0),
        mpTickEvent(),
        mbIsAutoMode(false)
    {}

    ~EventMultiplexerImpl()
    {
        if( mxListener.is() )
            mxListener->dispose();
    }

    /// Remove all handlers
    void clear();

    // actual handler callbacks (get called from the UNO interface
    // listeners via event queue)
    void mousePressed( const awt::MouseEvent& e );
    void mouseReleased( const awt::MouseEvent& e );
    void mouseDragged( const awt::MouseEvent& e );
    void mouseMoved( const awt::MouseEvent& e );

    bool isMouseListenerRegistered() const;

    typedef ThreadUnsafeListenerContainer<
        PrioritizedHandlerEntry<EventHandler>,
        std::vector<
            PrioritizedHandlerEntry<EventHandler> > >     ImplNextEffectHandlers;
    typedef PrioritizedHandlerEntry<MouseEventHandler>    ImplMouseHandlerEntry;
    typedef ThreadUnsafeListenerContainer<
        ImplMouseHandlerEntry,
        std::vector<ImplMouseHandlerEntry> >              ImplMouseHandlers;
    typedef ThreadUnsafeListenerContainer<
        EventHandlerSharedPtr,
        std::vector<EventHandlerSharedPtr> >              ImplEventHandlers;
    typedef ThreadUnsafeListenerContainer<
        AnimationEventHandlerSharedPtr,
        std::vector<AnimationEventHandlerSharedPtr> >     ImplAnimationHandlers;
    typedef ThreadUnsafeListenerContainer<
        PauseEventHandlerSharedPtr,
        std::vector<PauseEventHandlerSharedPtr> >         ImplPauseHandlers;
    typedef ThreadUnsafeListenerContainer<
        ViewEventHandlerWeakPtrWrapper,
        std::vector<ViewEventHandlerWeakPtrWrapper> >     ImplViewHandlers;
    typedef ThreadUnsafeListenerContainer<
        ViewRepaintHandlerSharedPtr,
        std::vector<ViewRepaintHandlerSharedPtr> >        ImplRepaintHandlers;
    typedef ThreadUnsafeListenerContainer<
        ShapeListenerEventHandlerSharedPtr,
        std::vector<ShapeListenerEventHandlerSharedPtr> > ImplShapeListenerHandlers;
    typedef ThreadUnsafeListenerContainer<
        UserPaintEventHandlerSharedPtr,
        std::vector<UserPaintEventHandlerSharedPtr> >     ImplUserPaintEventHandlers;
    typedef ThreadUnsafeListenerContainer<
        PrioritizedHandlerEntry<HyperlinkHandler>,
        std::vector<PrioritizedHandlerEntry<HyperlinkHandler> > > ImplHyperLinkHandlers;

    template <typename XSlideShowViewFunc>
    void forEachView( XSlideShowViewFunc pViewMethod );

    UnoViewSharedPtr findUnoView(const uno::Reference<
                                   presentation::XSlideShowView>& xView) const;

    template< typename RegisterFunction >
    void addMouseHandler( ImplMouseHandlers&                rHandlerContainer,
                          const MouseEventHandlerSharedPtr& rHandler,
                          double                            nPriority,
                          RegisterFunction                  pRegisterListener );

    static bool notifyAllAnimationHandlers( ImplAnimationHandlers const& rContainer,
                                     AnimationNodeSharedPtr const& rNode );

    bool notifyMouseHandlers(
        const ImplMouseHandlers& rQueue,
        bool (MouseEventHandler::*pHandlerMethod)(
            const awt::MouseEvent& ),
        const awt::MouseEvent& e );

    bool notifyNextEffect();

    /// Called for automatic nextEffect
    void tick();

    /// Schedules a tick event
    void scheduleTick();

    /// Schedules tick events, if mbIsAutoMode is true
    void handleTicks();

    basegfx::B2DPoint toMatrixPoint(const uno::Reference<presentation::XSlideShowView>& xView,
                                    const basegfx::B2DPoint& pnt);
    basegfx::B2DPoint toNormalPoint(const uno::Reference<presentation::XSlideShowView>& xView,
                                    const basegfx::B2DPoint& pnt);

    EventQueue&                         mrEventQueue;
    UnoViewContainer const&             mrViewContainer;
    ::rtl::Reference<
        EventMultiplexerListener>       mxListener;

    ImplNextEffectHandlers              maNextEffectHandlers;
    ImplEventHandlers                   maSlideStartHandlers;
    ImplEventHandlers                   maSlideEndHandlers;
    ImplAnimationHandlers               maAnimationStartHandlers;
    ImplAnimationHandlers               maAnimationEndHandlers;
    ImplEventHandlers                   maSlideAnimationsEndHandlers;
    ImplAnimationHandlers               maAudioStoppedHandlers;
    ImplAnimationHandlers               maCommandStopAudioHandlers;
    ImplPauseHandlers                   maPauseHandlers;
    ImplViewHandlers                    maViewHandlers;
    ImplRepaintHandlers                 maViewRepaintHandlers;
    ImplShapeListenerHandlers           maShapeListenerHandlers;
    ImplUserPaintEventHandlers          maUserPaintEventHandlers;
    ImplMouseHandlers                   maMouseClickHandlers;
    ImplMouseHandlers                   maMouseDoubleClickHandlers;
    ImplMouseHandlers                   maMouseMoveHandlers;
    ImplHyperLinkHandlers               maHyperlinkHandlers;

    /// automatic next effect mode timeout
    double                        mnTimeout;

    /** Holds ptr to optional tick event weakly

        When event queue is cleansed, the next
        setAutomaticMode(true) call is then able to
        regenerate the event.
    */
    ::std::weak_ptr< Event >    mpTickEvent;
    bool                          mbIsAutoMode;
};


void EventMultiplexerListener::disposing(std::unique_lock<std::mutex>& /*rGuard*/)
{
    mpEventQueue = nullptr;
    mpEventMultiplexer = nullptr;
}

void SAL_CALL EventMultiplexerListener::disposing(
    const lang::EventObject& /*rSource*/ )
{
    // there's no real point in acting on this message - after all,
    // the event sources are the XSlideShowViews, which must be
    // explicitly removed from the slideshow via
    // XSlideShow::removeView(). thus, if a XSlideShowView has
    // properly removed itself from the slideshow, it will not be
    // found here. and if it hasn't, there'll be other references at
    // other places within the slideshow, anyway...
}

void SAL_CALL EventMultiplexerListener::mousePressed(
    const awt::MouseEvent& e )
{
    std::unique_lock const guard( m_aMutex );

    // notify mouse press. Don't call handlers directly, this
    // might not be the main thread!
    if( mpEventQueue )
        mpEventQueue->addEvent(
            makeEvent( std::bind( &EventMultiplexerImpl::mousePressed,
                                    mpEventMultiplexer,
                                    e ),
                       "EventMultiplexerImpl::mousePressed") );
}

void SAL_CALL EventMultiplexerListener::mouseReleased(
    const awt::MouseEvent& e )
{
    std::unique_lock const guard( m_aMutex );

    // notify mouse release. Don't call handlers directly,
    // this might not be the main thread!
    if( mpEventQueue )
        mpEventQueue->addEvent(
            makeEvent( std::bind( &EventMultiplexerImpl::mouseReleased,
                                    mpEventMultiplexer,
                                    e ),
                       "EventMultiplexerImpl::mouseReleased") );
}

void SAL_CALL EventMultiplexerListener::mouseEntered(
    const awt::MouseEvent& /*e*/ )
{
    // not used here
}

void SAL_CALL EventMultiplexerListener::mouseExited(
    const awt::MouseEvent& /*e*/ )
{
    // not used here
}

// XMouseMotionListener implementation
void SAL_CALL EventMultiplexerListener::mouseDragged(
    const awt::MouseEvent& e )
{
    std::unique_lock const guard( m_aMutex );

    // notify mouse drag. Don't call handlers directly, this
    // might not be the main thread!
    if( mpEventQueue )
        mpEventQueue->addEvent(
            makeEvent( std::bind( &EventMultiplexerImpl::mouseDragged,
                                    mpEventMultiplexer,
                                    e ),
                       "EventMultiplexerImpl::mouseDragged") );
}

void SAL_CALL EventMultiplexerListener::mouseMoved(
    const awt::MouseEvent& e )
{
    std::unique_lock const guard( m_aMutex );

    // notify mouse move. Don't call handlers directly, this
    // might not be the main thread!
    if( mpEventQueue )
        mpEventQueue->addEvent(
            makeEvent( std::bind( &EventMultiplexerImpl::mouseMoved,
                                    mpEventMultiplexer,
                                    e ),
                       "EventMultiplexerImpl::mouseMoved") );
}


bool EventMultiplexerImpl::notifyAllAnimationHandlers( ImplAnimationHandlers const& rContainer,
                                                       AnimationNodeSharedPtr const& rNode )
{
    return rContainer.applyAll(
        [&rNode]( const AnimationEventHandlerSharedPtr& pEventHandler )
        { return pEventHandler->handleAnimationEvent( rNode ); } );
}

template <typename XSlideShowViewFunc>
void EventMultiplexerImpl::forEachView( XSlideShowViewFunc pViewMethod )
{
    if( !pViewMethod )
        return;

    // (un)register mouse listener on all views
    for( UnoViewVector::const_iterator aIter( mrViewContainer.begin() ),
             aEnd( mrViewContainer.end() ); aIter != aEnd; ++aIter )
    {
        uno::Reference<presentation::XSlideShowView> xView ((*aIter)->getUnoView());
        if (xView.is())
        {
            (xView.get()->*pViewMethod)( mxListener.get() );
        }
        else
        {
            OSL_ASSERT(xView.is());
        }
    }
}

UnoViewSharedPtr EventMultiplexerImpl::findUnoView(
    const uno::Reference<presentation::XSlideShowView>& xView) const
{
    // find view from which the change originated
    UnoViewVector::const_iterator       aIter;
    const UnoViewVector::const_iterator aEnd ( mrViewContainer.end() );
    if( (aIter=std::find_if( mrViewContainer.begin(),
                             aEnd,
                             [&xView]( const UnoViewSharedPtr& pView )
                             { return xView == pView->getUnoView(); } )) == aEnd )
    {
        OSL_FAIL("EventMultiplexer::findUnoView(): unexpected message source" );
        return UnoViewSharedPtr();
    }

    return *aIter;
}

template< typename RegisterFunction >
void EventMultiplexerImpl::addMouseHandler(
    ImplMouseHandlers&                rHandlerContainer,
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority,
    RegisterFunction                  pRegisterListener )
{
    ENSURE_OR_THROW(
        rHandler,
        "EventMultiplexer::addMouseHandler(): Invalid handler" );

    // register mouse listener on all views
    forEachView( pRegisterListener );

    // add into sorted container:
    rHandlerContainer.addSorted(
        typename ImplMouseHandlers::container_type::value_type(
            rHandler,
            nPriority ));
}

bool EventMultiplexerImpl::isMouseListenerRegistered() const
{
    return !(maMouseClickHandlers.isEmpty() &&
             maMouseDoubleClickHandlers.isEmpty());
}

void EventMultiplexerImpl::tick()
{
    if( !mbIsAutoMode )
        return; // this event is just a left-over, ignore

    notifyNextEffect();

    if( !maNextEffectHandlers.isEmpty() )
    {
        // still handlers left, schedule next timeout
        // event. Will also set mbIsTickEventOn back to true
        scheduleTick();
    }
}

void EventMultiplexerImpl::scheduleTick()
{
    EventSharedPtr pEvent(
        makeDelay( [this] () { this->tick(); },
                   mnTimeout,
                   "EventMultiplexerImpl::tick with delay"));

    // store weak reference to generated event, to notice when
    // the event queue gets cleansed (we then have to
    // regenerate the tick event!)
    mpTickEvent = pEvent;

    // enabled auto mode: simply schedule a timeout event,
    // which will eventually call our tick() method
    mrEventQueue.addEventForNextRound( pEvent );
}

void EventMultiplexerImpl::handleTicks()
{
    if( !mbIsAutoMode )
        return; // nothing to do, don't need no ticks

    EventSharedPtr pTickEvent( mpTickEvent.lock() );
    if( pTickEvent )
        return; // nothing to do, there's already a tick
                // pending

    // schedule initial tick (which reschedules itself
    // after that, all by itself)
    scheduleTick();
}

basegfx::B2DPoint
EventMultiplexerImpl::toNormalPoint(const uno::Reference<presentation::XSlideShowView>& xView,
                                    const basegfx::B2DPoint& pnt)
{
    UnoViewVector::const_iterator aIter;
    const UnoViewVector::const_iterator aEnd(mrViewContainer.end());
    if ((aIter = std::find_if(
             mrViewContainer.begin(), aEnd,
             [&xView](const UnoViewSharedPtr& pView) { return xView == pView->getUnoView(); }))
        == aEnd)
    {
        return pnt;
    }

    basegfx::B2DPoint aPosition(pnt.getX(), pnt.getY());
    basegfx::B2DHomMatrix aMatrix((*aIter)->getTransformation());
    aPosition *= aMatrix;

    aPosition.setX(std::round(aPosition.getX()));
    aPosition.setY(std::round(aPosition.getY()));
    return aPosition;
}

basegfx::B2DPoint
EventMultiplexerImpl::toMatrixPoint(const uno::Reference<presentation::XSlideShowView>& xView,
                                    const basegfx::B2DPoint& pnt)
{
    UnoViewVector::const_iterator aIter;
    const UnoViewVector::const_iterator aEnd(mrViewContainer.end());
    if ((aIter = std::find_if(
             mrViewContainer.begin(), aEnd,
             [&xView](const UnoViewSharedPtr& pView) { return xView == pView->getUnoView(); }))
        == aEnd)
    {
        return pnt;
    }

    basegfx::B2DPoint aPosition(pnt.getX(), pnt.getY());
    basegfx::B2DHomMatrix aMatrix((*aIter)->getTransformation());
    if (!aMatrix.invert())
        ENSURE_OR_THROW(false, "EventMultiplexer::notifyHandlers():"
                               " view matrix singular");
    aPosition *= aMatrix;

    aPosition.setX(std::round(aPosition.getX()));
    aPosition.setY(std::round(aPosition.getY()));
    return aPosition;
}

void EventMultiplexerImpl::clear()
{
    // deregister from all views.
    if( isMouseListenerRegistered() )
    {
        for( UnoViewVector::const_iterator aIter=mrViewContainer.begin(),
                 aEnd=mrViewContainer.end();
             aIter!=aEnd;
             ++aIter )
        {
            if( (*aIter)->getUnoView().is() )
                (*aIter)->getUnoView()->removeMouseListener( mxListener );
        }
    }

    if( !maMouseMoveHandlers.isEmpty() )
    {
        for( UnoViewVector::const_iterator aIter=mrViewContainer.begin(),
                 aEnd=mrViewContainer.end();
             aIter!=aEnd;
             ++aIter )
        {
            if( (*aIter)->getUnoView().is() )
                (*aIter)->getUnoView()->removeMouseMotionListener( mxListener );
        }
    }

    // clear all handlers (releases all references)
    maNextEffectHandlers.clear();
    maSlideStartHandlers.clear();
    maSlideEndHandlers.clear();
    maAnimationStartHandlers.clear();
    maAnimationEndHandlers.clear();
    maSlideAnimationsEndHandlers.clear();
    maAudioStoppedHandlers.clear();
    maCommandStopAudioHandlers.clear();
    maPauseHandlers.clear();
    maViewHandlers.clear();
    maViewRepaintHandlers.clear();
    maMouseClickHandlers.clear();
    maMouseDoubleClickHandlers.clear();
    maMouseMoveHandlers.clear();
    maHyperlinkHandlers.clear();
    mpTickEvent.reset();
}

// XMouseListener implementation
bool EventMultiplexerImpl::notifyMouseHandlers(
    const ImplMouseHandlers& rQueue,
    bool (MouseEventHandler::*pHandlerMethod)( const awt::MouseEvent& ),
    const awt::MouseEvent& e )
{
    uno::Reference<presentation::XSlideShowView> xView(
        e.Source, uno::UNO_QUERY );

    ENSURE_OR_RETURN_FALSE( xView.is(), "EventMultiplexer::notifyHandlers(): "
                       "event source is not an XSlideShowView" );

    // find corresponding view (to map mouse position into user
    // coordinate space)
    UnoViewVector::const_iterator       aIter;
    const UnoViewVector::const_iterator aEnd  ( mrViewContainer.end() );
    if( (aIter=::std::find_if(
             mrViewContainer.begin(),
             aEnd,
             [&xView]( const UnoViewSharedPtr& pView )
             { return xView == pView->getUnoView(); } )) == aEnd )
    {
        ENSURE_OR_RETURN_FALSE(
            false, "EventMultiplexer::notifyHandlers(): "
            "event source not found under registered views" );
    }

    // convert mouse position to user coordinate space
    ::basegfx::B2DPoint     aPosition( e.X, e.Y );
    ::basegfx::B2DHomMatrix aMatrix( (*aIter)->getTransformation() );
    if( !aMatrix.invert() )
        ENSURE_OR_THROW( false, "EventMultiplexer::notifyHandlers():"
                          " view matrix singular" );
    aPosition *= aMatrix;

    awt::MouseEvent aEvent( e );
    aEvent.X = ::basegfx::fround( aPosition.getX() );
    aEvent.Y = ::basegfx::fround( aPosition.getY() );

    // fire event on handlers, try in order of precedence. If
    // one high-priority handler rejects the event
    // (i.e. returns false), try next handler.
    return rQueue.apply(
        [&pHandlerMethod, &aEvent]( const ImplMouseHandlerEntry& rMouseHandler )
        { return ( ( *rMouseHandler.getHandler() ).*pHandlerMethod )( aEvent ); } );
}

void EventMultiplexerImpl::mousePressed( const awt::MouseEvent& e )
{
    // fire double-click events for every second click
    sal_Int32 nCurrClickCount = e.ClickCount;
    while( nCurrClickCount > 1 &&
           notifyMouseHandlers( maMouseDoubleClickHandlers,
                                &MouseEventHandler::handleMousePressed,
                                e ))
    {
        nCurrClickCount -= 2;
    }

    // fire single-click events for all remaining clicks
    while( nCurrClickCount > 0 &&
           notifyMouseHandlers( maMouseClickHandlers,
                                &MouseEventHandler::handleMousePressed,
                                e ))
    {
        --nCurrClickCount;
    }
}

void EventMultiplexerImpl::mouseReleased( const awt::MouseEvent& e )
{
    // fire double-click events for every second click
    sal_Int32 nCurrClickCount = e.ClickCount;
    while( nCurrClickCount > 1 &&
           notifyMouseHandlers( maMouseDoubleClickHandlers,
                                &MouseEventHandler::handleMouseReleased,
                                e ))
    {
        nCurrClickCount -= 2;
    }

    // fire single-click events for all remaining clicks
    while( nCurrClickCount > 0 &&
           notifyMouseHandlers( maMouseClickHandlers,
                                &MouseEventHandler::handleMouseReleased,
                                e ))
    {
        --nCurrClickCount;
    }
}

void EventMultiplexerImpl::mouseDragged( const awt::MouseEvent& e )
{
    notifyMouseHandlers( maMouseMoveHandlers,
                         &MouseEventHandler::handleMouseDragged,
                         e );
}

void EventMultiplexerImpl::mouseMoved( const awt::MouseEvent& e )
{
    notifyMouseHandlers( maMouseMoveHandlers,
                         &MouseEventHandler::handleMouseMoved,
                         e );
}

bool EventMultiplexerImpl::notifyNextEffect()
{
    // fire event on handlers, try in order of precedence. If one
    // high-priority handler rejects the event (i.e. returns false),
    // try next handler.
    return maNextEffectHandlers.apply(
        []( const PrioritizedHandlerEntry< EventHandler >& pHandler )
        { return pHandler.getHandler()->handleEvent(); } );
}


EventMultiplexer::EventMultiplexer( EventQueue&             rEventQueue,
                                    UnoViewContainer const& rViewContainer ) :
    mpImpl( new EventMultiplexerImpl(rEventQueue, rViewContainer) )
{
}

EventMultiplexer::~EventMultiplexer()
{
    // outline because of EventMultiplexerImpl's incomplete type
}

void EventMultiplexer::clear()
{
    mpImpl->clear();
}

void EventMultiplexer::setAutomaticMode( bool bIsAuto )
{
    if( bIsAuto == mpImpl->mbIsAutoMode )
        return; // no change, nothing to do

    mpImpl->mbIsAutoMode = bIsAuto;

    mpImpl->handleTicks();
}

bool EventMultiplexer::getAutomaticMode() const
{
    return mpImpl->mbIsAutoMode;
}

void EventMultiplexer::setAutomaticTimeout( double nTimeout )
{
    mpImpl->mnTimeout = nTimeout;
}

double EventMultiplexer::getAutomaticTimeout() const
{
    return mpImpl->mnTimeout;
}

void EventMultiplexer::addNextEffectHandler(
    EventHandlerSharedPtr const& rHandler,
    double                       nPriority )
{
    mpImpl->maNextEffectHandlers.addSorted(
        EventMultiplexerImpl::ImplNextEffectHandlers::container_type::value_type(
            rHandler,
            nPriority) );

    // Enable tick events, if not done already
    mpImpl->handleTicks();
}

void EventMultiplexer::removeNextEffectHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maNextEffectHandlers.remove(
        EventMultiplexerImpl::ImplNextEffectHandlers::container_type::value_type(
            rHandler,
            0.0) );
}

void EventMultiplexer::addSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideStartHandlers.add( rHandler );
}

void EventMultiplexer::removeSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideStartHandlers.remove( rHandler );
}

void EventMultiplexer::addSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideEndHandlers.add( rHandler );
}

void EventMultiplexer::removeSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideEndHandlers.remove( rHandler );
}

void EventMultiplexer::addAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAnimationStartHandlers.add( rHandler );
}

void EventMultiplexer::removeAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAnimationStartHandlers.remove( rHandler );
}

void EventMultiplexer::addAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAnimationEndHandlers.add( rHandler );
}

void EventMultiplexer::removeAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAnimationEndHandlers.remove( rHandler );
}

void EventMultiplexer::addSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideAnimationsEndHandlers.add( rHandler );
}

void EventMultiplexer::removeSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    mpImpl->maSlideAnimationsEndHandlers.remove( rHandler );
}

void EventMultiplexer::addAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAudioStoppedHandlers.add( rHandler );
}

void EventMultiplexer::removeAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maAudioStoppedHandlers.remove( rHandler );
}

void EventMultiplexer::addCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maCommandStopAudioHandlers.add( rHandler );
}

void EventMultiplexer::removeCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    mpImpl->maCommandStopAudioHandlers.remove( rHandler );
}

void EventMultiplexer::addPauseHandler(
    const PauseEventHandlerSharedPtr& rHandler )
{
    mpImpl->maPauseHandlers.add( rHandler );
}

void EventMultiplexer::removePauseHandler(
    const PauseEventHandlerSharedPtr&  rHandler )
{
    mpImpl->maPauseHandlers.remove( rHandler );
}

void EventMultiplexer::addViewHandler(
    const ViewEventHandlerWeakPtr& rHandler )
{
    mpImpl->maViewHandlers.add( rHandler );
}

void EventMultiplexer::removeViewHandler( const ViewEventHandlerWeakPtr& rHandler )
{
    mpImpl->maViewHandlers.remove( rHandler );
}

void EventMultiplexer::addViewRepaintHandler( const ViewRepaintHandlerSharedPtr& rHandler )
{
    mpImpl->maViewRepaintHandlers.add( rHandler );
}

void EventMultiplexer::removeViewRepaintHandler( const ViewRepaintHandlerSharedPtr& rHandler )
{
    mpImpl->maViewRepaintHandlers.remove( rHandler );
}

void EventMultiplexer::addShapeListenerHandler( const ShapeListenerEventHandlerSharedPtr& rHandler )
{
    mpImpl->maShapeListenerHandlers.add( rHandler );
}

void EventMultiplexer::removeShapeListenerHandler( const ShapeListenerEventHandlerSharedPtr& rHandler )
{
    mpImpl->maShapeListenerHandlers.remove( rHandler );
}

void EventMultiplexer::addUserPaintHandler( const UserPaintEventHandlerSharedPtr& rHandler )
{
    mpImpl->maUserPaintEventHandlers.add( rHandler );
}

void EventMultiplexer::addClickHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    mpImpl->addMouseHandler(
        mpImpl->maMouseClickHandlers,
        rHandler,
        nPriority,
        mpImpl->isMouseListenerRegistered()
        ? nullptr
        : &presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeClickHandler(
    const MouseEventHandlerSharedPtr&  rHandler )
{
    mpImpl->maMouseClickHandlers.remove(
        EventMultiplexerImpl::ImplMouseHandlers::container_type::value_type(
            rHandler,
            0.0) );

    if( !mpImpl->isMouseListenerRegistered() )
        mpImpl->forEachView( &presentation::XSlideShowView::removeMouseListener );
}

void EventMultiplexer::addDoubleClickHandler(
    const MouseEventHandlerSharedPtr&   rHandler,
    double                              nPriority )
{
    mpImpl->addMouseHandler(
        mpImpl->maMouseDoubleClickHandlers,
        rHandler,
        nPriority,
        mpImpl->isMouseListenerRegistered()
        ? nullptr
        : &presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeDoubleClickHandler(
    const MouseEventHandlerSharedPtr&    rHandler )
{
    mpImpl->maMouseDoubleClickHandlers.remove(
        EventMultiplexerImpl::ImplMouseHandlers::container_type::value_type(
            rHandler,
            0.0) );

    if( !mpImpl->isMouseListenerRegistered() )
        mpImpl->forEachView( &presentation::XSlideShowView::removeMouseListener );
}

void EventMultiplexer::addMouseMoveHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    mpImpl->addMouseHandler(
        mpImpl->maMouseMoveHandlers,
        rHandler,
        nPriority,
        mpImpl->maMouseMoveHandlers.isEmpty()
        ? &presentation::XSlideShowView::addMouseMotionListener
        : nullptr );
}

void EventMultiplexer::removeMouseMoveHandler(
    const MouseEventHandlerSharedPtr&  rHandler )
{
    mpImpl->maMouseMoveHandlers.remove(
        EventMultiplexerImpl::ImplMouseHandlers::container_type::value_type(
            rHandler,
            0.0) );

    if( mpImpl->maMouseMoveHandlers.isEmpty() )
        mpImpl->forEachView(
            &presentation::XSlideShowView::removeMouseMotionListener );
}

void EventMultiplexer::addHyperlinkHandler( const HyperlinkHandlerSharedPtr& rHandler,
                                            double                           nPriority )
{
    mpImpl->maHyperlinkHandlers.addSorted(
        EventMultiplexerImpl::ImplHyperLinkHandlers::container_type::value_type(
            rHandler,
            nPriority) );
}

void EventMultiplexer::removeHyperlinkHandler( const HyperlinkHandlerSharedPtr& rHandler )
{
    mpImpl->maHyperlinkHandlers.remove(
        EventMultiplexerImpl::ImplHyperLinkHandlers::container_type::value_type(
            rHandler,
            0.0) );
}

void EventMultiplexer::notifyShapeListenerAdded(
    const uno::Reference<drawing::XShape>&                   xShape )
{
    mpImpl->maShapeListenerHandlers.applyAll(
        [&xShape]( const ShapeListenerEventHandlerSharedPtr& pHandler )
        { return pHandler->listenerAdded( xShape ); } );
}

void EventMultiplexer::notifyShapeListenerRemoved(
    const uno::Reference<drawing::XShape>&                   xShape )
{
    mpImpl->maShapeListenerHandlers.applyAll(
        [&xShape]( const ShapeListenerEventHandlerSharedPtr& pHandler )
        { return pHandler->listenerRemoved( xShape ); } );
}

void EventMultiplexer::notifyUserPaintColor( RGBColor const& rUserColor )
{
    mpImpl->maUserPaintEventHandlers.applyAll(
        [&rUserColor]( const UserPaintEventHandlerSharedPtr& pHandler )
        { return pHandler->colorChanged( rUserColor ); } );
}

void EventMultiplexer::notifyUserPaintStrokeWidth( double rUserStrokeWidth )
{
    mpImpl->maUserPaintEventHandlers.applyAll(
        [&rUserStrokeWidth]( const UserPaintEventHandlerSharedPtr& pHandler )
        { return pHandler->widthChanged( rUserStrokeWidth ); } );
}

void EventMultiplexer::notifyUserPaintDisabled()
{
    mpImpl->maUserPaintEventHandlers.applyAll(
        std::mem_fn(&UserPaintEventHandler::disable));
}

void EventMultiplexer::notifySwitchPenMode(){
    mpImpl->maUserPaintEventHandlers.applyAll(
        std::mem_fn(&UserPaintEventHandler::switchPenMode));
}

void EventMultiplexer::notifySwitchEraserMode(){
    mpImpl->maUserPaintEventHandlers.applyAll(
        std::mem_fn(&UserPaintEventHandler::switchEraserMode));
}

//adding erasing all ink features with UserPaintOverlay
void EventMultiplexer::notifyEraseAllInk( bool bEraseAllInk )
{
    mpImpl->maUserPaintEventHandlers.applyAll(
        [&bEraseAllInk]( const UserPaintEventHandlerSharedPtr& pHandler )
        { return pHandler->eraseAllInkChanged( bEraseAllInk ); } );
}

//adding erasing features with UserPaintOverlay
void EventMultiplexer::notifyEraseInkWidth( sal_Int32 rEraseInkSize )
{
    mpImpl->maUserPaintEventHandlers.applyAll(
        [&rEraseInkSize]( const UserPaintEventHandlerSharedPtr& pHandler )
        { return pHandler->eraseInkWidthChanged( rEraseInkSize ); } );
}

bool EventMultiplexer::notifyNextEffect()
{
    return mpImpl->notifyNextEffect();
}

void EventMultiplexer::notifySlideStartEvent()
{
    mpImpl->maSlideStartHandlers.applyAll(
        std::mem_fn(&EventHandler::handleEvent) );
}

bool EventMultiplexer::notifySlideEndEvent()
{
    return mpImpl->maSlideEndHandlers.applyAll(
        std::mem_fn(&EventHandler::handleEvent) );
}

bool EventMultiplexer::notifyAnimationStart(
    const AnimationNodeSharedPtr& rNode )
{
    return EventMultiplexerImpl::notifyAllAnimationHandlers( mpImpl->maAnimationStartHandlers,
                                               rNode );
}

bool EventMultiplexer::notifyAnimationEnd(
    const AnimationNodeSharedPtr& rNode )
{
    return EventMultiplexerImpl::notifyAllAnimationHandlers( mpImpl->maAnimationEndHandlers,
                                               rNode );
}

bool EventMultiplexer::notifySlideAnimationsEnd()
{
    return mpImpl->maSlideAnimationsEndHandlers.applyAll(
        std::mem_fn(&EventHandler::handleEvent));
}

bool EventMultiplexer::notifyAudioStopped(
    const AnimationNodeSharedPtr& rNode )
{
    return EventMultiplexerImpl::notifyAllAnimationHandlers(
        mpImpl->maAudioStoppedHandlers,
        rNode );
}

bool EventMultiplexer::notifyCommandStopAudio(
    const AnimationNodeSharedPtr& rNode )
{
    return EventMultiplexerImpl::notifyAllAnimationHandlers(
        mpImpl->maCommandStopAudioHandlers,
        rNode );
}

void EventMultiplexer::notifyPauseMode( bool bPauseShow )
{
    mpImpl->maPauseHandlers.applyAll(
        [&bPauseShow]( const PauseEventHandlerSharedPtr& pHandler )
        { return pHandler->handlePause( bPauseShow ); } );
}

void EventMultiplexer::notifyViewAdded( const UnoViewSharedPtr& rView )
{
    ENSURE_OR_THROW( rView, "EventMultiplexer::notifyViewAdded(): Invalid view");

    // register event listener
    uno::Reference<presentation::XSlideShowView> const rUnoView(
        rView->getUnoView() );

    if( mpImpl->isMouseListenerRegistered() )
        rUnoView->addMouseListener(
            mpImpl->mxListener );

    if( !mpImpl->maMouseMoveHandlers.isEmpty() )
        rUnoView->addMouseMotionListener(
            mpImpl->mxListener );

    mpImpl->maViewHandlers.applyAll(
        [&rView]( const ViewEventHandlerWeakPtr& pHandler )
        { return pHandler.lock()->viewAdded( rView ); } );
}

void EventMultiplexer::notifyViewRemoved( const UnoViewSharedPtr& rView )
{
    ENSURE_OR_THROW( rView,
                      "EventMultiplexer::removeView(): Invalid view" );

    // revoke event listeners
    uno::Reference<presentation::XSlideShowView> const rUnoView(
        rView->getUnoView() );

    if( mpImpl->isMouseListenerRegistered() )
        rUnoView->removeMouseListener(
            mpImpl->mxListener );

    if( !mpImpl->maMouseMoveHandlers.isEmpty() )
        rUnoView->removeMouseMotionListener(
            mpImpl->mxListener );

    mpImpl->maViewHandlers.applyAll(
        [&rView]( const ViewEventHandlerWeakPtr& pHandler )
        { return pHandler.lock()->viewRemoved( rView ); } );
}

void EventMultiplexer::notifyViewChanged( const UnoViewSharedPtr& rView )
{
    mpImpl->maViewHandlers.applyAll(
        [&rView]( const ViewEventHandlerWeakPtr& pHandler )
        { return pHandler.lock()->viewChanged( rView ); } );
}

void EventMultiplexer::notifyViewChanged( const uno::Reference<presentation::XSlideShowView>& xView )
{
    UnoViewSharedPtr pView( mpImpl->findUnoView(xView) );

    if( !pView )
        return; // view not registered here

    notifyViewChanged( pView );
}

void EventMultiplexer::notifyViewsChanged()
{
    mpImpl->maViewHandlers.applyAll(
        std::mem_fn( &ViewEventHandler::viewsChanged ));
}

void EventMultiplexer::notifyViewClobbered(
    const uno::Reference<presentation::XSlideShowView>& xView )
{
    UnoViewSharedPtr pView( mpImpl->findUnoView(xView) );

    if( !pView )
        return; // view not registered here

    mpImpl->maViewRepaintHandlers.applyAll(
        [&pView]( const ViewRepaintHandlerSharedPtr& pHandler )
        { return pHandler->viewClobbered( pView ); } );
}

void EventMultiplexer::notifyHyperlinkClicked(
    OUString const& hyperLink )
{
    mpImpl->maHyperlinkHandlers.apply(
        [&hyperLink]( const PrioritizedHandlerEntry< HyperlinkHandler >& pHandler )
        { return pHandler.getHandler()->handleHyperlink( hyperLink ); } );
}

basegfx::B2DPoint EventMultiplexer::toMatrixPoint(const uno::Reference<uno::XInterface>& xInterface,
                                                  const basegfx::B2DPoint& pnt)
{
    uno::Reference<presentation::XSlideShowView> xView(xInterface, uno::UNO_QUERY_THROW);
    return mpImpl->toMatrixPoint(xView, pnt);
}

basegfx::B2DPoint EventMultiplexer::toNormalPoint(const uno::Reference<uno::XInterface>& xInterface,
                                                  const basegfx::B2DPoint& pnt)
{
    uno::Reference<presentation::XSlideShowView> xView(xInterface, uno::UNO_QUERY_THROW);
    return mpImpl->toNormalPoint(xView, pnt);
}

} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
