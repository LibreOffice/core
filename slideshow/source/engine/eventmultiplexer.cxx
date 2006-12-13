/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventmultiplexer.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:56:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <basegfx/numeric/ftools.hxx>

#include "tools.hxx"
#include "eventmultiplexer.hxx"
#include "delayevent.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <hash_map>
#include <algorithm>

using namespace ::com::sun::star;

/* Implementation of EventMultiplexer class */

namespace slideshow {
namespace internal {

template <typename HandlerT>
class EventMultiplexer::PrioritizedHandlerEntry
{
    typedef PrioritizedHandlerEntry<HandlerT> thisT;
    typedef boost::shared_ptr<HandlerT> HandlerSharedPtrT;
    HandlerSharedPtrT m_pHandler;
    double m_nPrio;

public:
    PrioritizedHandlerEntry( HandlerSharedPtrT const& pHandler, double nPrio )
        : m_pHandler(pHandler), m_nPrio(nPrio) {}

    HandlerSharedPtrT const& getHandler() const { return m_pHandler; }

    /// To sort according to priority
    bool operator<( thisT const& rRHS ) const {
        // reversed order - high prioritized entries
        // should be at the beginning of the queue
        return m_nPrio > rRHS.m_nPrio;
    }

    /// To permit ::std::remove in removeHandler template
    bool operator==( HandlerSharedPtrT const& rRHS ) const {
        return m_pHandler == rRHS;
    }
};

/** Notify handlers

    @return true, if at least one handler was called.
 */
template <typename T, typename FuncT>
bool EventMultiplexer::notifyAllHandlers( std::vector< boost::shared_ptr<T> > const& rContainer,
                                          FuncT const& func )
{
    osl::ClearableMutexGuard guard( m_aMutex );
    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    std::vector< boost::shared_ptr<T> >  const local( rContainer );
    guard.clear();
    // true: at least one handler returned true
    // false: not a single handler returned true
    return (std::count_if( local.begin(), local.end(), func ) > 0);
}

/** Notify weakly held handlers

    @return true, if at least one handler was called.
 */
template <typename FuncT>
bool EventMultiplexer::notifyAllViewHandlers( FuncT const& func )
{
    osl::ResettableMutexGuard guard( m_aMutex );

    if( maViewHandlers.empty() )
        return false;

    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    ImplViewHandlers local( maViewHandlers );
    guard.clear();

    // call handler for each alife container entry
    unsigned int nStalePtrs(0);
    ImplViewHandlers::const_iterator       aCurr( local.begin() );
    ImplViewHandlers::const_iterator const aEnd( local.end() );
    while( aCurr != aEnd )
    {
        ViewEventHandlerSharedPtr pCurrHandler( aCurr->lock() );

        if( pCurrHandler )
            func( pCurrHandler );
        else
            ++nStalePtrs;

        ++aCurr;
    }

    // needs to be two-step, because the notification above happens
    // with unlocked mutex.
    if( nStalePtrs )
    {
        // re-acquire lock.
        guard.reset();

        // prune handlers from stale ones
        ImplViewHandlers aAliveHandlers;

        ImplViewHandlers::const_iterator       aCurrHandler( maViewHandlers.begin() );
        ImplViewHandlers::const_iterator const aEndHandler( maViewHandlers.end() );
        while( aCurrHandler != aEndHandler )
        {
            ViewEventHandlerSharedPtr pCurrHandler( aCurrHandler->lock() );

            if( pCurrHandler )
                aAliveHandlers.push_back( *aCurrHandler );

            ++aCurrHandler;
        }

        const bool bRet( maViewHandlers.size() > nStalePtrs );
        std::swap(aAliveHandlers, maViewHandlers);
        return bRet;
    }

    // at least one handler was called.
    return true;
}

template <typename ContainerT, typename FuncT>
bool EventMultiplexer::notifySingleHandler( ContainerT const& rContainer,
                                            FuncT const& func,
                                            bool bOperateOnCopy )
{
    // fire event on handlers, try in order of precedence. If
    // one high-priority handler rejects the event
    // (i.e. returns false), try next handler.

    if (bOperateOnCopy) {
        osl::ClearableMutexGuard guard( m_aMutex );
        // generate a local copy of all handlers, since we have to
        // release the object mutex before firing.
        ContainerT const localHandlers( rContainer );
        guard.clear();
        typename ContainerT::const_iterator const iEnd( localHandlers.end() );
        // true: a handler in this queue processed the event
        // false: no handler in this queue finally processed the event
        return (std::find_if( localHandlers.begin(), iEnd, func ) != iEnd);
    }
    else
    {
        typename ContainerT::const_iterator const iEnd( rContainer.end() );
        // true: a handler in this queue processed the event
        // false: no handler in this queue finally processed the event
        return (std::find_if( rContainer.begin(), iEnd, func ) != iEnd);
    }
}

template <typename ContainerT, typename HandlerT>
void EventMultiplexer::addHandler( ContainerT & rContainer,
                                   boost::shared_ptr<HandlerT> const& pHandler )
{
    ENSURE_AND_THROW(
        pHandler,
        "EventMultiplexer::addHandler(): Invalid handler" );

    osl::MutexGuard const guard( m_aMutex );
    rContainer.push_back( pHandler );
}

template <typename ContainerT, typename HandlerT>
void EventMultiplexer::addHandler( ContainerT & rContainer,
                                   HandlerT const& pHandler )
{
    osl::MutexGuard const guard( m_aMutex );
    rContainer.push_back( pHandler );
}

template <typename ContainerT, typename HandlerT>
void EventMultiplexer::addPrioritizedHandler(
    ContainerT & rContainer,
    boost::shared_ptr<HandlerT> const& pHandler,
    double nPriority )
{
    ENSURE_AND_THROW(
        pHandler,
        "EventMultiplexer::addHandler(): Invalid handler" );

    osl::MutexGuard const guard( m_aMutex );
    // insert into queue (not sorted yet)
    rContainer.push_back( typename ContainerT::value_type( pHandler,
                                                           nPriority ) );
    if (rContainer.size() > 1) {
        // element was inserted, but now we have to keep the
        // entries sorted
        std::inplace_merge(
            rContainer.begin(), rContainer.end() - 1, rContainer.end() );
    }
    // no need to call inplace_merge otherwise, added
    // entry is the only one on the heap.
}

template< typename Container, typename Handler >
void EventMultiplexer::removeHandler(
    Container & rContainer,
    const ::boost::shared_ptr<Handler>& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ENSURE_AND_THROW(
        rHandler,
        "EventMultiplexer::removeHandler(): Invalid handler" );

    const typename Container::iterator aEnd( rContainer.end() );
    rContainer.erase( ::std::remove(rContainer.begin(), aEnd, rHandler), aEnd );
}

template< typename Container, typename Handler >
void EventMultiplexer::removeHandler(
    Container &    rContainer,
    const Handler& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    const typename Container::iterator aEnd( rContainer.end() );
    rContainer.erase( ::std::remove(rContainer.begin(), aEnd, rHandler), aEnd );
}

template <typename XSlideShowViewFunc>
void EventMultiplexer::forEachView( XSlideShowViewFunc pViewMethod )
{
    if (pViewMethod) {
        // (un)register mouse listener on all views
        for( UnoViewVector::const_iterator aIter( maViewContainer.begin() ),
                 aEnd( maViewContainer.end() ); aIter != aEnd; ++aIter ) {
            ((*aIter)->getUnoView().get()->*pViewMethod)( this );
        }
    }
}

template< typename RegisterFunction >
void EventMultiplexer::addMouseHandler(
    ImplMouseHandlers&            rHandlerContainer,
    const MouseEventHandlerSharedPtr& rHandler,
    double                        nPriority,
    RegisterFunction              pRegisterListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ENSURE_AND_THROW(
        rHandler,
        "EventMultiplexer::addMouseHandler(): Invalid handler" );

    // register mouse listener on all views
    forEachView( pRegisterListener );

    // sort into container:
    addPrioritizedHandler( rHandlerContainer, rHandler, nPriority );
}

bool EventMultiplexer::isMouseListenerRegistered() const
{
    return !(maMouseClickHandlers.empty() &&
             maMouseDoubleClickHandlers.empty());
}

void EventMultiplexer::tick()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( !mbIsAutoMode )
        return; // this event is just a left-over, ignore

    notifyNextEffect();

    if( !maNextEffectHandlers.empty() )
    {
        // still handlers left, schedule next timeout
        // event. Will also set mbIsTickEventOn back to true
        scheduleTick();
    }
}

void EventMultiplexer::scheduleTick()
{
    // TODO(Q3): make robust (no boost::ref()) when
    //           get_pointer(uno::Ref...) is available
    EventSharedPtr pEvent(
        makeDelay( boost::bind( &EventMultiplexer::tick, this ),
                   mnTimeout ) );

    // store weak reference to generated event, to notice when
    // the event queue gets cleansed (we then have to
    // regenerate the tick event!)
    mpTickEvent = pEvent;

    // enabled auto mode: simply schedule a timeout event,
    // which will eventually call our tick() method
    mrEventQueue.addEventForNextRound( pEvent );
}

void EventMultiplexer::handleTicks()
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

void EventMultiplexer::implSetMouseCursor( sal_Int16 nCursor ) const
{
    // change all views to the requested cursor ID
    std::for_each( maViewContainer.begin(), maViewContainer.end(),
                   boost::bind( &View::setMouseCursor, _1, nCursor ) );
}

void EventMultiplexer::clear()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // deregister from all views.
    if( isMouseListenerRegistered() )
    {
        for( UnoViewVector::const_iterator aIter=maViewContainer.begin(), aEnd=maViewContainer.end();
             aIter!=aEnd;
             ++aIter )
        {
            (*aIter)->getUnoView()->removeMouseListener( this );
        }
    }

    if( !maMouseMoveHandlers.empty() )
    {
        for( UnoViewVector::const_iterator aIter=maViewContainer.begin(), aEnd=maViewContainer.end();
             aIter!=aEnd;
             ++aIter )
        {
            (*aIter)->getUnoView()->removeMouseMotionListener( this );
        }
    }

    // release all references
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
    maMouseClickHandlers.clear();
    maMouseDoubleClickHandlers.clear();
    maMouseMoveHandlers.clear();
    maHyperlinkHandlers.clear();
    mpLayerManager.reset();
}

void EventMultiplexer::setMouseCursor( sal_Int16 nCursor )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mnMouseCursor = nCursor;

    implSetMouseCursor( mnMouseCursor );
}

void EventMultiplexer::setVolatileMouseCursor( sal_Int16 nCursor )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // only change, if volatile cursor is not already
    // set. This effectively prioritizes handlers with the
    // highest priority (otherwise, the handler with the
    // lowest prio would determine the mouse cursor -
    // unwanted).
    if( mnVolatileMouseCursor == -1 )
        mnVolatileMouseCursor = nCursor;
}

void EventMultiplexer::setLayerManager(
    const LayerManagerSharedPtr& rMgr )
{
    mpLayerManager = rMgr;
}

void EventMultiplexer::updateScreenContent( const UnoViewSharedPtr& rView,
                                            bool                    bForceUpdate )
{
    const bool bLayerUpdate( mpLayerManager &&
                             mpLayerManager->isUpdatePending() );

    // perform screen update (either if we're forced to do it,
    // or if the layer manager signals that it needs one).
    if( bForceUpdate || bLayerUpdate )
    {
        // call update() on the registered
        // LayerManager. This will only update the
        // backbuffer, not flush anything to screen
        if( bLayerUpdate )
        {
            mpLayerManager->update();

            // call updateScreen() on all registered views (which will
            // copy the backbuffers to the front). This is because
            // LayerManager::update() updated all views.
            std::for_each( maViewContainer.begin(),
                           maViewContainer.end(),
                           ::boost::mem_fn( &View::updateScreen ) );
        }
        else
        {
            // selectively only update affected view
            rView->updateScreen();
        }
    }
}

void EventMultiplexer::updateScreenContent( bool bForceUpdate )
{
    const bool bLayerUpdate( mpLayerManager &&
                             mpLayerManager->isUpdatePending() );

    // perform screen update (either if we're forced to do it,
    // or if the layer manager signals that it needs one).
    if( bForceUpdate || bLayerUpdate )
    {
        // call update() on the registered
        // LayerManager. This will only update the
        // backbuffer, not flush anything to screen
        if( bLayerUpdate )
            mpLayerManager->update();

        // call updateScreen() on all registered views (which
        // will copy the backbuffers to the front). Do NOT use
        // LayerManager::updateScreen(), we might need screen
        // updates independent from a valid LayerManager!
        ::std::for_each( maViewContainer.begin(),
                         maViewContainer.end(),
                         ::boost::mem_fn( &View::updateScreen ) );
    }
}

void EventMultiplexer::setAutomaticMode( bool bIsAuto )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if( bIsAuto == mbIsAutoMode )
        return; // no change, nothing to do

    mbIsAutoMode = bIsAuto;

    handleTicks();
}

bool EventMultiplexer::getAutomaticMode() const
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return mbIsAutoMode;
}

void EventMultiplexer::setAutomaticTimeout( double nTimeout )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mnTimeout = nTimeout;
}

double EventMultiplexer::getAutomaticTimeout() const
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return mnTimeout;
}

void EventMultiplexer::addNextEffectHandler(
    EventHandlerSharedPtr const& rHandler,
    double                       nPriority )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addPrioritizedHandler( maNextEffectHandlers, rHandler, nPriority );

    // Enable tick events, if not done already
    handleTicks();
}

void EventMultiplexer::removeNextEffectHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maNextEffectHandlers, rHandler );
}

void EventMultiplexer::addSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maSlideStartHandlers, rHandler );
}

void EventMultiplexer::removeSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maSlideStartHandlers, rHandler );
}

void EventMultiplexer::addSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maSlideEndHandlers, rHandler );
}

void EventMultiplexer::removeSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maSlideEndHandlers, rHandler );
}

void EventMultiplexer::addAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maAnimationStartHandlers, rHandler );
}

void EventMultiplexer::removeAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maAnimationStartHandlers, rHandler );
}

void EventMultiplexer::addAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maAnimationEndHandlers, rHandler );
}

void EventMultiplexer::removeAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maAnimationEndHandlers, rHandler );
}

void EventMultiplexer::addSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maSlideAnimationsEndHandlers, rHandler );
}

void EventMultiplexer::removeSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maSlideAnimationsEndHandlers, rHandler );
}

void EventMultiplexer::addAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maAudioStoppedHandlers, rHandler );
}

void EventMultiplexer::removeAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maAudioStoppedHandlers, rHandler );
}

void EventMultiplexer::addCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maCommandStopAudioHandlers, rHandler );
}

void EventMultiplexer::removeCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maCommandStopAudioHandlers, rHandler );
}

void EventMultiplexer::addPauseHandler(
    const PauseEventHandlerSharedPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addHandler( maPauseHandlers, rHandler );
}

void EventMultiplexer::removePauseHandler(
    const PauseEventHandlerSharedPtr&  rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maPauseHandlers, rHandler );
}

void EventMultiplexer::addViewHandler(
    const ViewEventHandlerWeakPtr& rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // TODO(Q1): Maybe prune vector even here, some times...
    addHandler( maViewHandlers, rHandler );
}

void EventMultiplexer::addClickHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addMouseHandler(
        maMouseClickHandlers,
        rHandler,
        nPriority,
        isMouseListenerRegistered()
        ? NULL
        : &presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeClickHandler(
    const MouseEventHandlerSharedPtr&  rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maMouseClickHandlers,
                   rHandler );

    if( !isMouseListenerRegistered() ) {
        forEachView( &presentation::XSlideShowView::removeMouseListener );
    }
}

void EventMultiplexer::addDoubleClickHandler(
    const MouseEventHandlerSharedPtr&   rHandler,
    double                              nPriority )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addMouseHandler(
        maMouseDoubleClickHandlers,
        rHandler,
        nPriority,
        isMouseListenerRegistered()
        ? NULL
        : &presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeDoubleClickHandler(
    const MouseEventHandlerSharedPtr&    rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maMouseDoubleClickHandlers,
                   rHandler );

    if( !isMouseListenerRegistered() ) {
        forEachView( &presentation::XSlideShowView::removeMouseListener );
    }
}

void EventMultiplexer::addMouseMoveHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    addMouseHandler(
        maMouseMoveHandlers,
        rHandler,
        nPriority,
        maMouseMoveHandlers.empty()
        ? &presentation::XSlideShowView::addMouseMotionListener
        : NULL );
}

void EventMultiplexer::removeMouseMoveHandler(
    const MouseEventHandlerSharedPtr&  rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maMouseMoveHandlers,
                   rHandler );

    if( maMouseMoveHandlers.empty() ) {
        forEachView(
            &presentation::XSlideShowView::removeMouseMotionListener );
    }
}

void EventMultiplexer::addHyperlinkHandler(
    HyperLinkHandlerFunc const& func, HandlerId id )
{
    osl::MutexGuard const guard(m_aMutex);
    maHyperlinkHandlers[id] = func; // second add will overwrite previous func
}

void EventMultiplexer::removeHyperlinkHandler( HandlerId id )
{
    osl::MutexGuard const guard(m_aMutex);
    maHyperlinkHandlers.erase(id);
}

bool EventMultiplexer::notifyNextEffect()
{
    // fire event on handlers, try in order of precedence. If
    // one high-priority handler rejects the event
    // (i.e. returns false), try next handler.
    return notifySingleHandler(
        maNextEffectHandlers,
        boost::bind(
            &EventHandler::handleEvent,
            boost::bind( &ImplNextEffectHandlers::value_type::getHandler,
                         _1 ) ) );
}

bool EventMultiplexer::notifySlideStartEvent()
{
    return notifyAllEventHandlers( maSlideStartHandlers );
}

bool EventMultiplexer::notifySlideEndEvent()
{
    return notifyAllEventHandlers( maSlideEndHandlers );
}

bool EventMultiplexer::notifyAnimationStart(
    const AnimationNodeSharedPtr& rNode )
{
    return notifyAllAnimationHandlers( maAnimationStartHandlers, rNode );
}

bool EventMultiplexer::notifyAnimationEnd(
    const AnimationNodeSharedPtr& rNode )
{
    return notifyAllAnimationHandlers( maAnimationEndHandlers, rNode );
}

bool EventMultiplexer::notifySlideAnimationsEnd()
{
    return notifyAllEventHandlers( maSlideAnimationsEndHandlers );
}

bool EventMultiplexer::notifyAudioStopped(
    const AnimationNodeSharedPtr& rNode )
{
    return notifyAllAnimationHandlers( maAudioStoppedHandlers, rNode );
}

bool EventMultiplexer::notifyCommandStopAudio(
    const AnimationNodeSharedPtr& rNode )
{
    return notifyAllAnimationHandlers( maCommandStopAudioHandlers, rNode );
}

bool EventMultiplexer::notifyPauseMode( bool bPauseShow )
{
    return notifyAllHandlers( maPauseHandlers,
                              boost::bind( &PauseEventHandler::handlePause,
                                           _1, bPauseShow ));
}

bool EventMultiplexer::notifyViewAdded( const UnoViewSharedPtr& rView )
{
    ENSURE_AND_THROW( rView, "EventMultiplexer::notifyViewAdded(): Invalid view");

    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    maViewContainer.push_back( rView );

    // register event listeners
    uno::Reference<presentation::XSlideShowView> const rUnoView(
        rView->getUnoView() );

    if( isMouseListenerRegistered() )
        rUnoView->addMouseListener( this );

    if( !maMouseMoveHandlers.empty() )
        rUnoView->addMouseMotionListener( this );

    aGuard.clear();

    return notifyAllViewHandlers( boost::bind( &ViewEventHandler::viewAdded,
                                               _1,
                                               rView ));
}

bool EventMultiplexer::notifyViewRemoved( const UnoViewSharedPtr& rView )
{
    ENSURE_AND_THROW( rView,
                      "EventMultiplexer::removeView(): Invalid view" );

    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    // revoke event listeners
    uno::Reference<presentation::XSlideShowView> const rUnoView(
        rView->getUnoView() );

    if( isMouseListenerRegistered() )
        rUnoView->removeMouseListener( this );

    if( !maMouseMoveHandlers.empty() )
        rUnoView->removeMouseMotionListener( this );

    UnoViewVector::iterator aEnd( maViewContainer.end() );
    maViewContainer.erase( ::std::remove(maViewContainer.begin(), aEnd, rView), aEnd );

    aGuard.clear();

    return notifyAllViewHandlers( boost::bind( &ViewEventHandler::viewRemoved,
                                               _1,
                                               rView ));
}

bool EventMultiplexer::notifyViewChanged( const UnoViewSharedPtr& rView )
{
    return notifyAllViewHandlers( boost::bind( &ViewEventHandler::viewChanged,
                                               _1,
                                               rView ));
}

struct EventMultiplexer::AllViewNotifier
{
    AllViewNotifier( const UnoViewVector& rViews ) : mrViews( rViews ) {}

    void operator()( const ViewEventHandlerWeakPtr& rHandler ) const
    {
        ViewEventHandlerSharedPtr pHandler( rHandler.lock() );

        if( pHandler )
        {
            std::for_each( mrViews.begin(),
                           mrViews.end(),
                           boost::bind( &ViewEventHandler::viewChanged,
                                        boost::cref(pHandler),
                                        _1 ));
        }
    }

    const UnoViewVector& mrViews;
};

bool EventMultiplexer::notifyViewsChanged()
{
    return notifyAllViewHandlers( AllViewNotifier( maViewContainer ));
}

struct EventMultiplexer::HyperLinkNotifier {
    rtl::OUString const& m_rLink;
    HyperLinkNotifier( rtl::OUString const& link )
        : m_rLink(link) {}
    bool operator()( ImplHyperLinkHandlers::value_type const& v ) const {
        return v.second( m_rLink );
    }
};

bool EventMultiplexer::notifyHyperlinkClicked(
    rtl::OUString const& hyperLink )
{
    return notifySingleHandler( maHyperlinkHandlers,
                                HyperLinkNotifier( hyperLink ) );
}

void EventMultiplexer::disposing()
{
    // release all references
    clear();
    WeakComponentImplHelperBase::disposing();
}

// XMouseListener implementation
void SAL_CALL EventMultiplexer::disposing(
    const lang::EventObject& /*rSource*/ ) throw (uno::RuntimeException)
{
    // there's no real point in acting on this message - after all,
    // the event sources are the XSlideShowViews, which must be
    // explicitely removed from the slideshow via
    // XSlideShow::removeView(). thus, if a XSlideShowView has
    // properly removed itself from the slideshow, it will not be
    // found here. and if it hasn't, there'll be other references at
    // other places within the slideshow.
}

bool EventMultiplexer::notifyMouseHandlers(
    const ImplMouseHandlers& rQueue,
    bool (MouseEventHandler::*pHandlerMethod)( const awt::MouseEvent& ),
    const awt::MouseEvent& e )
{
    uno::Reference<presentation::XSlideShowView> xView(
        e.Source, uno::UNO_QUERY );

    ENSURE_AND_RETURN( xView.is(), "EventMultiplexer::notifyHandlers(): "
                       "event source is not an XSlideShowView" );

    // find corresponding view (to mouse position into user coordinate space)
    UnoViewVector::const_iterator       aIter;
    const UnoViewVector::const_iterator aBegin( maViewContainer.begin() );
    const UnoViewVector::const_iterator aEnd  ( maViewContainer.end() );
    if( (aIter=::std::find_if(
             aBegin, aEnd,
             boost::bind( std::equal_to< uno::Reference<
                          presentation::XSlideShowView > >(),
                          boost::cref( xView ),
                          boost::bind( &UnoView::getUnoView, _1 ) ) ) ) == aEnd)
    {
        ENSURE_AND_RETURN(
            false, "EventMultiplexer::notifyHandlers(): "
            "event source not found under registered views" );
    }

    // convert mouse position to user coordinate space
    ::basegfx::B2DPoint     aPosition( e.X, e.Y );
    ::basegfx::B2DHomMatrix aMatrix( (*aIter)->getTransformation() );
    if( !aMatrix.invert() )
        ENSURE_AND_THROW( false, "EventMultiplexer::notifyHandlers():"
                          " view matrix singular" );
    aPosition *= aMatrix;

    awt::MouseEvent aEvent( e );
    aEvent.X = ::basegfx::fround( aPosition.getX() );
    aEvent.Y = ::basegfx::fround( aPosition.getY() );

    // fire event on handlers, try in order of precedence. If
    // one high-priority handler rejects the event
    // (i.e. returns false), try next handler.
    return notifySingleHandler(
        rQueue, boost::bind(
            pHandlerMethod,
            boost::bind( &ImplMouseHandlers::value_type::getHandler, _1 ),
            aEvent ),
        false /* no separate copy: already operating on copy made in
                 implMouseXXX() */ );
}

void EventMultiplexer::implMousePressed( const awt::MouseEvent& e )
{
    osl::ClearableMutexGuard guard( m_aMutex );

    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    ImplMouseHandlers const localDoubleClickHandlers(
        maMouseDoubleClickHandlers );
    ImplMouseHandlers const localClickHandlers(
        maMouseClickHandlers );

    guard.clear();

    // fire double-click events for every second click
    sal_Int32 nCurrClickCount = e.ClickCount;
    while( nCurrClickCount > 1 &&
           notifyMouseHandlers( localDoubleClickHandlers,
                                &MouseEventHandler::handleMousePressed,
                                e ) )
    {
        nCurrClickCount -= 2;
    }

    // fire single-click events for all remaining clicks
    while( nCurrClickCount > 0 &&
           notifyMouseHandlers( localClickHandlers,
                                &MouseEventHandler::handleMousePressed,
                                e ) )
    {
        --nCurrClickCount;
    }
}

void EventMultiplexer::implMouseReleased( const awt::MouseEvent& e )
{
    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    osl::ClearableMutexGuard guard( m_aMutex );
    ImplMouseHandlers const localDoubleClickHandlers(
        maMouseDoubleClickHandlers );
    ImplMouseHandlers const localClickHandlers(
        maMouseClickHandlers );
    guard.clear();

    // fire double-click events for every second click
    sal_Int32 nCurrClickCount = e.ClickCount;
    while( nCurrClickCount > 1 &&
           notifyMouseHandlers( localDoubleClickHandlers,
                                &MouseEventHandler::handleMouseReleased,
                                e ) )
    {
        nCurrClickCount -= 2;
    }

    // fire single-click events for all remaining clicks
    while( nCurrClickCount > 0 &&
           notifyMouseHandlers( localClickHandlers,
                                &MouseEventHandler::handleMouseReleased,
                                e ) )
    {
        --nCurrClickCount;
    }
}

void EventMultiplexer::implMouseDragged( const awt::MouseEvent& e )
{
    osl::ResettableMutexGuard guard( m_aMutex );

    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    ImplMouseHandlers const localHandlers( maMouseMoveHandlers );

    guard.clear();

    notifyMouseHandlers(
        localHandlers, &MouseEventHandler::handleMouseDragged, e );

    // re-acquire mutex
    guard.reset();

    if( mnVolatileMouseCursor != -1 )
    {
        // handlers requested a volatile mouse cursor - set
        // one
        mnLastVolatileMouseCursor = mnVolatileMouseCursor;
        implSetMouseCursor( mnVolatileMouseCursor );

        mnVolatileMouseCursor = -1;
    }
    else if( mnMouseCursor != mnLastVolatileMouseCursor )
    {
        // handlers did not request a volatile mouse cursor -
        // clear to default, if necessary
        implSetMouseCursor( mnMouseCursor );

        mnLastVolatileMouseCursor = mnMouseCursor;
    }
}

void EventMultiplexer::implMouseMoved( const awt::MouseEvent& e )
{
    osl::ResettableMutexGuard guard( m_aMutex );
    // generate a local copy of all handlers, since we have to
    // release the object mutex before firing.
    ImplMouseHandlers const localHandlers( maMouseMoveHandlers );

    guard.clear();

    notifyMouseHandlers(
        localHandlers, &MouseEventHandler::handleMouseMoved, e );

    // re-acquire mutex
    guard.reset();

    if( mnVolatileMouseCursor != -1 )
    {
        // handlers requested a volatile mouse cursor - set
        // one
        mnLastVolatileMouseCursor = mnVolatileMouseCursor;
        implSetMouseCursor( mnVolatileMouseCursor );

        mnVolatileMouseCursor = -1;
    }
    else if( mnMouseCursor != mnLastVolatileMouseCursor )
    {
        // handlers did not request a volatile mouse cursor -
        // clear to default, if necessary
        implSetMouseCursor( mnMouseCursor );

        mnLastVolatileMouseCursor = mnMouseCursor;
    }
}

void SAL_CALL EventMultiplexer::mousePressed(
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    // notify mouse press. Don't call handlers directly, this
    // might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::implMousePressed,
                                this, e ) ) );
}

void SAL_CALL EventMultiplexer::mouseReleased(
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    // notify mouse release. Don't call handlers directly,
    // this might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::implMouseReleased,
                                this, e ) ) );
}

void SAL_CALL EventMultiplexer::mouseEntered(
    const awt::MouseEvent& /*e*/ ) throw (uno::RuntimeException)
{
    // not used here
}

void SAL_CALL EventMultiplexer::mouseExited(
    const awt::MouseEvent& /*e*/ ) throw (uno::RuntimeException)
{
    // not used here
}

// XMouseMotionListener implementation
void SAL_CALL EventMultiplexer::mouseDragged(
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    // notify mouse drag. Don't call handlers directly, this
    // might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::implMouseDragged,
                                this, e ) ) );
}

void SAL_CALL EventMultiplexer::mouseMoved(
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    osl::MutexGuard const guard( m_aMutex );

    // notify mouse move. Don't call handlers directly, this
    // might not be the main thread!
    mrEventQueue.addEvent(
        makeEvent( boost::bind( &EventMultiplexer::implMouseMoved,
                                this, e ) ) );
}

EventMultiplexer::~EventMultiplexer()
{
    // WeakComponentImplHelperBase calls disposing() upon destruction
    // if object has not been disposed yet.
}

EventMultiplexer::EventMultiplexer( EventQueue&             rEventQueue,
                                    const UnoViewContainer& rViewContainer ) :
    Listener_UnoBase( m_aMutex ),
    mrEventQueue( rEventQueue ),
    maViewContainer(rViewContainer.begin(),
                    rViewContainer.end()),
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
    maMouseClickHandlers(),
    maMouseDoubleClickHandlers(),
    maMouseMoveHandlers(),
    maHyperlinkHandlers(),
    mnTimeout( 0.0 ),
    mpTickEvent(),
    mnMouseCursor( awt::SystemPointer::ARROW ),
    mnVolatileMouseCursor( -1 ),
    mnLastVolatileMouseCursor( mnMouseCursor ),
    mbIsAutoMode( false )
{
}

rtl::Reference<EventMultiplexer> EventMultiplexer::create(
    EventQueue&             rEventQueue,
    const UnoViewContainer& rViews     )
{
    return new EventMultiplexer(rEventQueue,
                                rViews);
}

} // namespace internal
} // namespace presentation

