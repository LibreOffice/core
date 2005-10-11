/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventmultiplexer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:32:19 $
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

// must be first
#include "canvas/debug.hxx"
#include "tools.hxx"
#include "eventmultiplexer.hxx"
#include "com/sun/star/awt/SystemPointer.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/MouseButton.hpp"
#include "com/sun/star/presentation/XSlideShowView.hpp"
#include "basegfx/numeric/ftools.hxx"
#include "delayevent.hxx"
#include "boost/bind.hpp"
#include <vector>
#include <hash_map>
#include <algorithm>

namespace css = ::com::sun::star;
using namespace css;

/* Implementation of EventMultiplexer class */

namespace presentation {
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

template <typename ContainerT, typename FuncT>
bool EventMultiplexer::notifyAllHandlers( ContainerT const& rContainer,
                                          FuncT const& func,
                                          bool bOperateOnCopy )
{
    if (bOperateOnCopy) {
        osl::ClearableMutexGuard guard( m_aMutex );
        // generate a local copy of all handlers, since we have to
        // release the object mutex before firing.
        ContainerT const local( rContainer );
        guard.clear();
        // true: at least one handler returned true
        // false: not a single handler returned true
        return (std::count_if( local.begin(), local.end(), func ) > 0);
    }
    else {
        // true: at least one handler returned true
        // false: not a single handler returned true
        return (std::count_if( rContainer.begin(), rContainer.end(), func ) >0);
    }
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
        pHandler.get() != 0,
        "EventMultiplexer::addHandler(): Invalid handler" );

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
        pHandler.get() != 0,
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
        rHandler.get(),
        "EventMultiplexer::removeHandler(): Invalid handler" );

    const typename Container::iterator aEnd( rContainer.end() );
    rContainer.erase( ::std::remove(rContainer.begin(), aEnd, rHandler), aEnd );
}

template <typename XSlideShowViewFunc>
void EventMultiplexer::forEachView( XSlideShowViewFunc pViewMethod )
{
    if (pViewMethod) {
        // (un)register mouse listener on all views
        for( UnoViewVector::const_iterator aIter( maViews.begin() ),
                 aEnd( maViews.end() ); aIter != aEnd; ++aIter ) {
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
        rHandler.get(),
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
    mrEventQueue.addEvent( pEvent );
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
    std::for_each( maViews.begin(), maViews.end(),
                   boost::bind( &View::setMouseCursor, _1, nCursor ) );
}

bool EventMultiplexer::addView( const UnoViewSharedPtr& rView )
{
    ENSURE_AND_THROW( rView.get(), "EventMultiplexer::addView(): Invalid view");

    ::osl::MutexGuard aGuard( m_aMutex );

    // check whether same view is already added
    const UnoViewVector::iterator aEnd( maViews.end() );

    // already added?
    if( ::std::find( maViews.begin(),
                     aEnd,
                     rView ) != aEnd )
    {
        // yes, nothing to do
        return false;
    }

    maViews.push_back( rView );

    return true;
}

bool EventMultiplexer::removeView( const UnoViewSharedPtr& rView )
{
    ENSURE_AND_THROW( rView.get(),
                      "EventMultiplexer::removeView(): Invalid view" );

    ::osl::MutexGuard aGuard( m_aMutex );

    // check whether same view is already added
    const UnoViewVector::iterator aEnd( maViews.end() );
    UnoViewVector::iterator aIter;

    // view added?
    if( (aIter=::std::find( maViews.begin(),
                            aEnd,
                            rView )) == aEnd )
    {
        // nope, nothing to do
        return false;
    }

    // revoke event listeners
    uno::Reference<css::presentation::XSlideShowView> const rUnoView(
        rView->getUnoView() );

    if( isMouseListenerRegistered() )
        rUnoView->removeMouseListener( this );

    if( !maMouseMoveHandlers.empty() )
        rUnoView->removeMouseMotionListener( this );

    // actually erase from container
    maViews.erase( aIter );

    return true;
}

void EventMultiplexer::clear()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // deregister from all views.
    if( isMouseListenerRegistered() )
    {
        for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
             aIter!=aEnd;
             ++aIter )
        {
            (*aIter)->getUnoView()->removeMouseListener( this );
        }
    }

    if( !maMouseMoveHandlers.empty() )
    {
        for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
             aIter!=aEnd;
             ++aIter )
        {
            (*aIter)->getUnoView()->removeMouseMotionListener( this );
        }
    }

    // release all references
    maViews.clear();

    maNextEffectHandlers.clear();
    maSlideStartHandlers.clear();
    maSlideEndHandlers.clear();
    maAnimationStartHandlers.clear();
    maAnimationEndHandlers.clear();
    maSlideAnimationsEndHandlers.clear();
    maAudioStoppedHandlers.clear();
    maCommandStopAudioHandlers.clear();
    maPauseHandlers.clear();
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

void EventMultiplexer::updateScreenContent( bool bForceUpdate )
{
    // perform screen update (either if we're forced to do it,
    // or if the layer manager signals that it needs one).
    if( bForceUpdate ||
        (mpLayerManager.get() &&
         mpLayerManager->isUpdatePending() ) )
    {
        // call update() on the registered
        // LayerManager. This will only update the
        // backbuffer, not flush anything to screen
        if( mpLayerManager.get() )
            mpLayerManager->update();

        // call updateScreen() on all registered views (which
        // will copy the backbuffers to the front). Do NOT use
        // LayerManager::updateScreen(), we might need screen
        // updates independent from a valid LayerManager!
        ::std::for_each( maViews.begin(),
                         maViews.end(),
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
    addPrioritizedHandler( maNextEffectHandlers, rHandler, nPriority );

    // Enable tick events, if not done already
    handleTicks();
}

void EventMultiplexer::removeNextEffectHandler(
    const EventHandlerSharedPtr& rHandler )
{
    removeHandler( maNextEffectHandlers, rHandler );
}

void EventMultiplexer::addSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    addHandler( maSlideStartHandlers, rHandler );
}

void EventMultiplexer::removeSlideStartHandler(
    const EventHandlerSharedPtr& rHandler )
{
    removeHandler( maSlideStartHandlers, rHandler );
}

void EventMultiplexer::addSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    addHandler( maSlideEndHandlers, rHandler );
}

void EventMultiplexer::removeSlideEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    removeHandler( maSlideEndHandlers, rHandler );
}

void EventMultiplexer::addAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    addHandler( maAnimationStartHandlers, rHandler );
}

void EventMultiplexer::removeAnimationStartHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    removeHandler( maAnimationStartHandlers, rHandler );
}

void EventMultiplexer::addAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    addHandler( maAnimationEndHandlers, rHandler );
}

void EventMultiplexer::removeAnimationEndHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    removeHandler( maAnimationEndHandlers, rHandler );
}

void EventMultiplexer::addSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    addHandler( maSlideAnimationsEndHandlers, rHandler );
}

void EventMultiplexer::removeSlideAnimationsEndHandler(
    const EventHandlerSharedPtr& rHandler )
{
    removeHandler( maSlideAnimationsEndHandlers, rHandler );
}

void EventMultiplexer::addAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    addHandler( maAudioStoppedHandlers, rHandler );
}

void EventMultiplexer::removeAudioStoppedHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    removeHandler( maAudioStoppedHandlers, rHandler );
}

void EventMultiplexer::addCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    addHandler( maCommandStopAudioHandlers, rHandler );
}

void EventMultiplexer::removeCommandStopAudioHandler(
    const AnimationEventHandlerSharedPtr& rHandler )
{
    removeHandler( maCommandStopAudioHandlers, rHandler );
}

void EventMultiplexer::addPauseHandler(
    const PauseEventHandlerSharedPtr& rHandler )
{
    addHandler( maPauseHandlers, rHandler );
}

void EventMultiplexer::removePauseHandler(
    const PauseEventHandlerSharedPtr&  rHandler )
{
    removeHandler( maPauseHandlers, rHandler );
}

void EventMultiplexer::addClickHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    addMouseHandler(
        maMouseClickHandlers,
        rHandler,
        nPriority,
        isMouseListenerRegistered()
        ? NULL
        : &css::presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeClickHandler(
    const MouseEventHandlerSharedPtr&  rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maMouseClickHandlers,
                   rHandler );

    if( !isMouseListenerRegistered() ) {
        forEachView( &css::presentation::XSlideShowView::removeMouseListener );
    }
}

void EventMultiplexer::addDoubleClickHandler(
    const MouseEventHandlerSharedPtr&   rHandler,
    double                              nPriority )
{
    addMouseHandler(
        maMouseDoubleClickHandlers,
        rHandler,
        nPriority,
        isMouseListenerRegistered()
        ? NULL
        : &css::presentation::XSlideShowView::addMouseListener );
}

void EventMultiplexer::removeDoubleClickHandler(
    const MouseEventHandlerSharedPtr&    rHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    removeHandler( maMouseDoubleClickHandlers,
                   rHandler );

    if( !isMouseListenerRegistered() ) {
        forEachView( &css::presentation::XSlideShowView::removeMouseListener );
    }
}

void EventMultiplexer::addMouseMoveHandler(
    const MouseEventHandlerSharedPtr& rHandler,
    double                            nPriority )
{
    addMouseHandler(
        maMouseMoveHandlers,
        rHandler,
        nPriority,
        maMouseMoveHandlers.empty()
        ? &css::presentation::XSlideShowView::addMouseMotionListener
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
            &css::presentation::XSlideShowView::removeMouseMotionListener );
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
                                           _1, bPauseShow ) );
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
    const lang::EventObject& rSource ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // remove given event source
    for( UnoViewVector::iterator aIter=maViews.begin(), aEnd=maViews.end();
         aIter!=aEnd;
         ++aIter )
    {
        if( (*aIter)->getUnoView() == rSource.Source )
        {
            // one instance found, remove
            aIter = maViews.erase( aIter );
            aEnd = maViews.end(); // erase might invalidate all iterators!
        }
    }
}

bool EventMultiplexer::notifyMouseHandlers(
    const ImplMouseHandlers& rQueue,
    bool (MouseEventHandler::*pHandlerMethod)( const awt::MouseEvent& ),
    const awt::MouseEvent& e )
{
    uno::Reference<css::presentation::XSlideShowView> xView(
        e.Source, uno::UNO_QUERY );

    ENSURE_AND_RETURN( xView.is(), "EventMultiplexer::notifyHandlers(): "
                       "event source is not an XSlideShowView" );

    // find corresponding view (to mouse position into user coordinate space)
    UnoViewVector::iterator         aIter;
    const UnoViewVector::iterator   aEnd( maViews.end() );
    if( (aIter=::std::find_if(
             maViews.begin(), aEnd,
             boost::bind( std::equal_to< uno::Reference<
                          css::presentation::XSlideShowView > >(),
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
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    // not used here
}

void SAL_CALL EventMultiplexer::mouseExited(
    const awt::MouseEvent& e ) throw (uno::RuntimeException)
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

EventMultiplexer::EventMultiplexer( EventQueue & rEventQueue )
    : Listener_UnoBase( m_aMutex ),
      mrEventQueue( rEventQueue ),
      maViews(),
      maNextEffectHandlers(),
      maSlideStartHandlers(),
      maSlideEndHandlers(),
      maAnimationStartHandlers(),
      maAnimationEndHandlers(),
      maSlideAnimationsEndHandlers(),
      maAudioStoppedHandlers(),
      maCommandStopAudioHandlers(),
      maPauseHandlers(),
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
    EventQueue & rEventQueue )
{
    return new EventMultiplexer(rEventQueue);
}

} // namespace internal
} // namespace presentation

