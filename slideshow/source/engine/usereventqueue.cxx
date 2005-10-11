/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: usereventqueue.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-10-11 08:36:36 $
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
#include "delayevent.hxx"
#include "usereventqueue.hxx"
#include "slideshowexceptions.hxx"
#include "osl/mutex.hxx"
#include "com/sun/star/awt/SystemPointer.hpp"
#include "com/sun/star/awt/MouseButton.hpp"
#include "com/sun/star/awt/MouseEvent.hpp"
#include "boost/bind.hpp"
#include <vector>
#include <queue>
#include <map>
#include <functional>
#include <algorithm>


using namespace com::sun::star;

/* Implementation of UserEventQueue class */

namespace presentation {
namespace internal {

namespace {

typedef std::vector<EventSharedPtr> ImpEventVector;
typedef std::queue<EventSharedPtr> ImpEventQueue;
typedef std::map<uno::Reference<animations::XAnimationNode>,
                 ImpEventVector> ImpAnimationEventMap;
typedef std::map<ShapeSharedPtr, ImpEventQueue,
                 Shape::lessThanShape> ImpShapeEventMap;

// MouseEventHandler base class, not consuming any event:
class MouseEventHandler_ : public MouseEventHandler
{
public:
    virtual bool handleMousePressed( awt::MouseEvent const& e ) { return false;}
    virtual bool handleMouseReleased( awt::MouseEvent const& e) { return false;}
    virtual bool handleMouseEntered( awt::MouseEvent const& e ) { return false;}
    virtual bool handleMouseExited( awt::MouseEvent const& e ) { return false; }
    virtual bool handleMouseDragged( awt::MouseEvent const& e ) { return false;}
    virtual bool handleMouseMoved( awt::MouseEvent const& e ) { return false; }
};

/** @return one event has been posted
 */
template <typename ContainerT>
bool fireSingleEvent( ContainerT & rQueue, EventQueue & rEventQueue )
{
    // post next event in given queue:
    while (! rQueue.empty())
    {
        EventSharedPtr const pEvent(rQueue.front());
        rQueue.pop();

        // skip all inactive events (as the purpose of
        // nextEventFromQueue() is to activate the next
        // event, and events which return false on
        // isCharged() will never be activated by the
        // EventQueue)
        if (pEvent->isCharged())
            return rEventQueue.addEvent( pEvent );
    }
    return false; // no more (active) events in queue
}

/** @return at least one event has been posted
 */
template <typename ContainerT>
bool fireAllEvents( ContainerT & rQueue, EventQueue & rEventQueue )
{
    bool bFiredAny = false;
    while (fireSingleEvent( rQueue, rEventQueue ))
        bFiredAny = true;
    return bFiredAny;
}

class EventContainer
{
public:
    EventContainer() : maMutex(), maEvents() {}

    void clearContainer()
    {
        osl::MutexGuard const guard( maMutex );
        maEvents = ImpEventQueue();
    }

    void addEvent( const EventSharedPtr& rEvent )
    {
        osl::MutexGuard const guard( maMutex );
        maEvents.push( rEvent );
    }

    bool isEmpty()
    {
        osl::MutexGuard const guard( maMutex );
        return maEvents.empty();
    }

protected:
    osl::Mutex    maMutex;
    ImpEventQueue maEvents;
};

} // anon namespace

class PlainEventHandler : public EventHandler,
                          public EventContainer
{
public:
    PlainEventHandler( EventQueue & rEventQueue )
        : EventContainer(), mrEventQueue(rEventQueue) {}

    virtual void dispose()
    {
        clearContainer();
    }

    virtual bool handleEvent()
    {
        osl::MutexGuard const guard( maMutex );
        return fireAllEvents( maEvents, mrEventQueue );
    }

private:
    EventQueue & mrEventQueue;
};

class AllAnimationEventHandler : public AnimationEventHandler
{
public:
    AllAnimationEventHandler( EventQueue& rEventQueue )
        : maMutex(),
          mrEventQueue( rEventQueue ),
          maAnimationEventMap() {}

    virtual void dispose()
    {
        osl::MutexGuard aGuard( maMutex );
        maAnimationEventMap.clear();
    }

    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode )
    {
        ENSURE_AND_RETURN(
            rNode.get(),
            "AllAnimationEventHandler::handleAnimationEvent(): Invalid node" );

        osl::MutexGuard aGuard( maMutex );

        bool bRet( false );

        ImpAnimationEventMap::iterator aIter;
        if( (aIter=maAnimationEventMap.find(
                 rNode->getXAnimationNode() )) != maAnimationEventMap.end() )
        {
            ImpEventVector& rVec( aIter->second );

            bRet = !rVec.empty();

            // registered node found -> fire all events in the vector
            std::for_each( rVec.begin(), rVec.end(),
                           boost::bind( &EventQueue::addEvent,
                                        boost::ref( mrEventQueue ), _1 ) );

            rVec.clear();
        }

        return bRet;
    }

    void addEvent( const EventSharedPtr&                                rEvent,
                   const uno::Reference< animations::XAnimationNode >&  xNode )
    {
        osl::MutexGuard aGuard( maMutex );

        ImpAnimationEventMap::iterator aIter;
        if( (aIter=maAnimationEventMap.find( xNode )) ==
            maAnimationEventMap.end() )
        {
            // no entry for this animation -> create one
            aIter = maAnimationEventMap.insert(
                ImpAnimationEventMap::value_type( xNode,
                                                  ImpEventVector() ) ).first;
        }

        // add new event to queue
        aIter->second.push_back( rEvent );
    }

    bool isEmpty()
    {
        osl::MutexGuard aGuard( maMutex );

        // find at least one animation with a non-empty vector
        ImpAnimationEventMap::iterator aCurr( maAnimationEventMap.begin() );
        ImpAnimationEventMap::iterator aEnd( maAnimationEventMap.end() );
        while( aCurr != aEnd )
        {
            if( !aCurr->second.empty() )
                return false; // at least one non-empty entry found

            ++aCurr;
        }

        return true; // not a single non-empty entry found
    }

private:
    /// Object mutex
    osl::Mutex              maMutex;
    EventQueue&             mrEventQueue;
    ImpAnimationEventMap    maAnimationEventMap;
};

class ClickEventHandler : public MouseEventHandler_,
                          public EventHandler,
                          public EventContainer
{
public:
    ClickEventHandler( EventQueue& rEventQueue )
        : EventContainer(),
          mrEventQueue( rEventQueue ),
          mbAdvanceOnClick( true ) {}

    void setAdvanceOnClick( bool bAdvanceOnClick )
    {
        osl::MutexGuard aGuard( maMutex );
        mbAdvanceOnClick = bAdvanceOnClick;
    }

private:
    virtual void dispose()
    {
        clearContainer();
    }

    // triggered by API calls, e.g. space bar
    virtual bool handleEvent()
    {
        osl::MutexGuard const guard( maMutex );
        return handleEvent_impl();
    }

    // triggered by mouse release:
    virtual bool handleMouseReleased( const awt::MouseEvent& evt )
    {
        if (evt.Buttons != awt::MouseButton::LEFT)
            return false;

        osl::MutexGuard const guard( maMutex );

        if( mbAdvanceOnClick ) {
            // fire next event
            return handleEvent_impl();
        }
        else {
            return false; // advance-on-click disabled
        }
    }

    // triggered by both:
    virtual bool handleEvent_impl()
    {
        // fire next event:
        return fireSingleEvent( maEvents, mrEventQueue );
    }

private:
    /// Object mutex
    EventQueue&         mrEventQueue;
    bool                mbAdvanceOnClick;
};

class SkipEffectEventHandler : public ClickEventHandler
{
public:
    SkipEffectEventHandler( EventQueue & rEventQueue,
                            EventMultiplexer & rEventMultiplexer )
        : ClickEventHandler(rEventQueue),
          mrEventQueue(rEventQueue),
          mrEventMultiplexer(rEventMultiplexer) {}

private:
    virtual bool handleEvent_impl()
    {
        // fire all events, so animation nodes can register their
        // next effect listeners:
        if (fireAllEvents( maEvents, mrEventQueue )) {
            // then simulate a next effect event:
            // this skip effect handler is triggered upon next effect
            // events (multiplexer prio=-1)!
            // Posting a notifyNextEffect() here is only safe
            // (we don't run into busy loop), because we assume that
            // someone has registerered above for next effects
            // (multiplexer prio=0) at the user event queue.
            return mrEventQueue.addEvent(
                makeEvent( boost::bind(
                               &EventMultiplexer::notifyNextEffect,
                               boost::ref(mrEventMultiplexer) ) ) );
        }
        return false;
    }

private:
    EventQueue & mrEventQueue;
    EventMultiplexer & mrEventMultiplexer;
};

class RewindEffectEventHandler : public MouseEventHandler_,
                                 public EventContainer
{
public:
    RewindEffectEventHandler( EventQueue & rEventQueue )
        : EventContainer(), mrEventQueue(rEventQueue) {}

private:
    virtual void dispose()
    {
        clearContainer();
    }

    virtual bool handleMouseReleased( awt::MouseEvent const& evt )
    {
        if (evt.Buttons != awt::MouseButton::RIGHT)
            return false;

        osl::MutexGuard const guard( maMutex );
        return fireAllEvents( maEvents, mrEventQueue );
    }

private:
    EventQueue & mrEventQueue;
};

/** Base class to share some common code between
    ShapeClickEventHandler and MouseMoveHandler

    @derive override necessary MouseEventHandler interface methods,
    call sendEvent() method to actually process the event.
*/
class MouseHandlerBase : public MouseEventHandler_
{
public:
    MouseHandlerBase( EventQueue& rEventQueue )
        : maMutex(),
          mrEventQueue( rEventQueue ),
          maShapeEventMap() {}

    virtual void dispose()
    {
        osl::MutexGuard aGuard( maMutex );

        // TODO(Q1): Check whether plain vector with swap idiom is
        // okay here
        maShapeEventMap = ImpShapeEventMap();
    }

    void addEvent( const EventSharedPtr& rEvent,
                   const ShapeSharedPtr& rShape )
    {
        osl::MutexGuard aGuard( maMutex );

        ImpShapeEventMap::iterator aIter;
        if( (aIter=maShapeEventMap.find( rShape )) == maShapeEventMap.end() )
        {
            // no entry for this shape -> create one
            aIter = maShapeEventMap.insert(
                ImpShapeEventMap::value_type( rShape,
                                              ImpEventQueue() ) ).first;
        }

        // add new event to queue
        aIter->second.push( rEvent );
    }

    bool isEmpty()
    {
        osl::MutexGuard aGuard( maMutex );

        // find at least one shape with a non-empty queue
        ImpShapeEventMap::reverse_iterator aCurrShape( maShapeEventMap.begin());
        ImpShapeEventMap::reverse_iterator aEndShape( maShapeEventMap.end() );
        while( aCurrShape != aEndShape )
        {
            if( !aCurrShape->second.empty() )
                return false; // at least one non-empty entry found

            ++aCurrShape;
        }

        return true; // not a single non-empty entry found
    }

protected:
    bool hitTest( const awt::MouseEvent&                e,
                  ImpShapeEventMap::reverse_iterator&   o_rHitShape )
    {
        // find hit shape in map
        const basegfx::B2DPoint aPosition( e.X, e.Y );

        // find matching shape (scan reversely, to coarsely match
        // paint order)
        ImpShapeEventMap::reverse_iterator aCurrShape(maShapeEventMap.rbegin());
        ImpShapeEventMap::reverse_iterator aEndShape( maShapeEventMap.rend() );
        while( aCurrShape != aEndShape )
        {
            // TODO(F2): Get proper geometry polygon from the
            // shape, to avoid having areas outside the shape
            // react on the mouse
            if( aCurrShape->first->getPosSize().isInside( aPosition ) &&
                aCurrShape->first->isVisible() )
            {
                // shape hit, and shape is visible - report a
                // hit
                o_rHitShape = aCurrShape;
                return true;
            }

            ++aCurrShape;
        }

        return false; // nothing hit
    }

    bool sendEvent( ImpShapeEventMap::reverse_iterator& io_rHitShape )
    {
        // take next event from queue
        const bool bRet( fireSingleEvent( io_rHitShape->second,
                                          mrEventQueue ) );

        // clear shape entry, if its queue is
        // empty. This is important, since the shapes
        // are held by shared ptr, and might otherwise
        // not get released, even after their owning
        // slide is long gone.
        if( io_rHitShape->second.empty() )
        {
            // this looks funny, since ::std::map does
            // provide an erase( iterator )
            // method. Unfortunately, stlport does not
            // declare the obvious erase(
            // reverse_iterator ) needed here (missing
            // orthogonality, eh?)
            maShapeEventMap.erase( io_rHitShape->first );
        }

        return bRet;
    }

    bool processEvent( const awt::MouseEvent& e )
    {
        ImpShapeEventMap::reverse_iterator aCurrShape;

        if( hitTest( e, aCurrShape ) )
            return sendEvent( aCurrShape );

        return false; // did not handle the event
    }

protected:
    /// Object mutex
    osl::Mutex          maMutex;

private:
    EventQueue&         mrEventQueue;
    ImpShapeEventMap    maShapeEventMap;
};

class ShapeClickEventHandler : public MouseHandlerBase
{
public:
    ShapeClickEventHandler( EventMultiplexer&   rMultiplexer,
                            EventQueue&         rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          mrMultiplexer( rMultiplexer ) {}

    virtual bool handleMouseReleased( const awt::MouseEvent& e )
    {
        if (e.Buttons != awt::MouseButton::LEFT)
            return false;
        osl::MutexGuard aGuard( maMutex );
        return processEvent( e );
    }

    virtual bool handleMouseMoved( const awt::MouseEvent& e )
    {
        osl::MutexGuard aGuard( maMutex );

        // TODO(P2): Maybe buffer last shape touched

        // if we have a shape click event, and the mouse
        // hovers over this shape, change cursor to hand
        ImpShapeEventMap::reverse_iterator aDummy;
        if( hitTest( e, aDummy ) )
            mrMultiplexer.setVolatileMouseCursor( awt::SystemPointer::REFHAND );

        return false; // we don't /eat/ this event. Lower prio
        // handler should see it, too.
    }

private:
    EventMultiplexer& mrMultiplexer;
};

class MouseEnterHandler : public MouseHandlerBase
{
public:
    MouseEnterHandler( EventQueue& rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          mpLastShape() {}

    virtual bool handleMouseMoved( const awt::MouseEvent& e )
    {
        osl::MutexGuard aGuard( maMutex );

        // TODO(P2): Maybe buffer last shape touched, and
        // check against that _first_

        ImpShapeEventMap::reverse_iterator aCurr;
        if( hitTest( e, aCurr ) )
        {
            if( aCurr->first != mpLastShape )
            {
                // we actually hit a shape, and it's different
                // from the previous one - thus we just
                // entered it, raise event
                sendEvent( aCurr );
                mpLastShape = aCurr->first;
            }
        }
        else
        {
            // don't hit no shape - thus, last shape is NULL
            mpLastShape.reset();
        }

        return false; // we don't /eat/ this event. Lower prio
        // handler should see it, too.
    }

private:
    ShapeSharedPtr mpLastShape;
};

class MouseLeaveHandler : public MouseHandlerBase
{
public:
    MouseLeaveHandler( EventQueue& rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          maLastIter() {}

    virtual bool handleMouseMoved( const awt::MouseEvent& e )
    {
        osl::MutexGuard aGuard( maMutex );

        // TODO(P2): Maybe buffer last shape touched, and
        // check against that _first_

        ImpShapeEventMap::reverse_iterator aCurr;
        if( hitTest( e, aCurr ) )
        {
            maLastIter = aCurr;
        }
        else
        {
            if( maLastIter->first.get() != NULL )
            {
                // last time, we were over a shape, now we're
                // not - we thus just left that shape, raise
                // event
                sendEvent( maLastIter );
            }

            // in any case, when we hit this else-branch: no
            // shape hit, thus have to clear maLastIter
            maLastIter = ImpShapeEventMap::reverse_iterator();
        }

        return false; // we don't /eat/ this event. Lower prio
        // handler should see it, too.
    }

private:
    ImpShapeEventMap::reverse_iterator maLastIter;
};

template< typename Handler, typename Functor >
void UserEventQueue::registerEvent(
    boost::shared_ptr< Handler >& rHandler,
    const EventSharedPtr&         rEvent,
    const Functor&                rRegistrationFunctor )
{
    ENSURE_AND_THROW( rEvent.get(),
                      "UserEventQueue::registerEvent(): Invalid event" );

    if( !rHandler.get() ) {
        // create handler
        rHandler.reset( new Handler( mrEventQueue ) );
        // register handler on EventMultiplexer
        rRegistrationFunctor( rHandler );
    }

    rHandler->addEvent( rEvent );
}

template< typename Handler, typename Arg, typename Functor >
void UserEventQueue::registerEvent(
    boost::shared_ptr< Handler >& rHandler,
    const EventSharedPtr&         rEvent,
    const Arg&                    rArg,
    const Functor&                rRegistrationFunctor )
{
    ENSURE_AND_THROW( rEvent.get(),
                      "UserEventQueue::registerEvent(): Invalid event" );

    if( !rHandler.get() ) {
        // create handler
        rHandler.reset( new Handler( mrEventQueue ) );

        // register handler on EventMultiplexer
        rRegistrationFunctor( rHandler );
    }

    rHandler->addEvent( rEvent, rArg );
}


// Public methods
// =====================================================

UserEventQueue::UserEventQueue( EventMultiplexer&   rMultiplexer,
                                EventQueue&         rEventQueue )
    : mrMultiplexer( rMultiplexer ),
      mrEventQueue( rEventQueue ),
      mpStartEventHandler(),
      mpEndEventHandler(),
      mpAnimationStartEventHandler(),
      mpAnimationEndEventHandler(),
      mpAudioStoppedEventHandler(),
      mpClickEventHandler(),
      mpSkipEffectEventHandler(),
      mpRewindEffectEventHandler(),
      mpDoubleClickEventHandler(),
      mpMouseEnterHandler(),
      mpMouseLeaveHandler(),
      mbAdvanceOnClick( true )
{
}

UserEventQueue::~UserEventQueue()
{
    // unregister all handlers
    clear();
}

bool UserEventQueue::isEmpty() const
{
    // TODO(T2): This is not thread safe, the handlers are all
    // only separately synchronized. This poses the danger of
    // generating false empty status on XSlideShow::update(), such
    // that the last events of a slide are not triggered.

    // we're empty iff all handler queues are empty
    return
        (mpStartEventHandler.get() ? mpStartEventHandler->isEmpty() : true) &&
        (mpEndEventHandler.get() ? mpEndEventHandler->isEmpty() : true) &&
        (mpAnimationStartEventHandler.get() ? mpAnimationStartEventHandler->isEmpty() : true) &&
        (mpAnimationEndEventHandler.get() ? mpAnimationEndEventHandler->isEmpty() : true) &&
        (mpAudioStoppedEventHandler.get() ? mpAudioStoppedEventHandler->isEmpty() : true) &&
        (mpShapeClickEventHandler.get() ? mpShapeClickEventHandler->isEmpty() : true) &&
        (mpClickEventHandler.get() ? mpClickEventHandler->isEmpty() : true) &&
        (mpSkipEffectEventHandler.get() != 0 ? mpSkipEffectEventHandler->isEmpty() : true) &&
        (mpRewindEffectEventHandler.get() != 0 ? mpRewindEffectEventHandler->isEmpty() : true) &&
        (mpShapeDoubleClickEventHandler.get() ? mpShapeDoubleClickEventHandler->isEmpty() : true) &&
        (mpDoubleClickEventHandler.get() ? mpDoubleClickEventHandler->isEmpty() : true) &&
        (mpMouseEnterHandler.get() ? mpMouseEnterHandler->isEmpty() : true) &&
        (mpMouseLeaveHandler.get() ? mpMouseLeaveHandler->isEmpty() : true);
}

void UserEventQueue::clear()
{
    // unregister and delete all handlers
    if( mpStartEventHandler.get() ) {
        mrMultiplexer.removeSlideStartHandler( mpStartEventHandler );
        mpStartEventHandler.reset();
    }
    if( mpEndEventHandler.get() ) {
        mrMultiplexer.removeSlideEndHandler( mpEndEventHandler );
        mpEndEventHandler.reset();
    }
    if( mpAnimationStartEventHandler.get() ) {
        mrMultiplexer.removeAnimationStartHandler(
            mpAnimationStartEventHandler );
        mpAnimationStartEventHandler.reset();
    }
    if( mpAnimationEndEventHandler.get() ) {
        mrMultiplexer.removeAnimationEndHandler( mpAnimationEndEventHandler );
        mpAnimationEndEventHandler.reset();
    }
    if( mpAudioStoppedEventHandler.get() ) {
        mrMultiplexer.removeAudioStoppedHandler( mpAudioStoppedEventHandler );
        mpAudioStoppedEventHandler.reset();
    }
    if( mpShapeClickEventHandler.get() ) {
        mrMultiplexer.removeClickHandler( mpShapeClickEventHandler );
        mrMultiplexer.removeMouseMoveHandler( mpShapeClickEventHandler );
        mpShapeClickEventHandler.reset();
    }
    if( mpClickEventHandler.get() ) {
        mrMultiplexer.removeClickHandler( mpClickEventHandler );
        mrMultiplexer.removeNextEffectHandler( mpClickEventHandler );
        mpClickEventHandler.reset();
    }
    if (mpSkipEffectEventHandler.get() != 0) {
        mrMultiplexer.removeClickHandler( mpSkipEffectEventHandler );
        mrMultiplexer.removeNextEffectHandler( mpSkipEffectEventHandler );
        mpSkipEffectEventHandler.reset();
    }
    if (mpRewindEffectEventHandler.get() != 0) {
        mrMultiplexer.removeClickHandler( mpRewindEffectEventHandler );
        mpRewindEffectEventHandler.reset();
    }
    if( mpShapeDoubleClickEventHandler.get() ) {
        mrMultiplexer.removeDoubleClickHandler( mpShapeDoubleClickEventHandler );
        mrMultiplexer.removeMouseMoveHandler( mpShapeDoubleClickEventHandler );
        mpShapeDoubleClickEventHandler.reset();
    }
    if( mpDoubleClickEventHandler.get() ) {
        mrMultiplexer.removeDoubleClickHandler( mpDoubleClickEventHandler );
        mpDoubleClickEventHandler.reset();
    }
    if( mpMouseEnterHandler.get() ) {
        mrMultiplexer.removeMouseMoveHandler( mpMouseEnterHandler );
        mpMouseEnterHandler.reset();
    }
    if( mpMouseLeaveHandler.get() ) {
        mrMultiplexer.removeMouseMoveHandler( mpMouseLeaveHandler );
        mpMouseLeaveHandler.reset();
    }
}

void UserEventQueue::setAdvanceOnClick( bool bAdvanceOnClick )
{
    mbAdvanceOnClick = bAdvanceOnClick;

    // forward to handler, if existing. Otherwise, the handler
    // creation will do the forwarding.
    if( mpClickEventHandler.get() )
        mpClickEventHandler->setAdvanceOnClick( bAdvanceOnClick );
}

void UserEventQueue::registerSlideStartEvent( const EventSharedPtr& rEvent )
{
    registerEvent( mpStartEventHandler,
                   rEvent,
                   boost::bind( &EventMultiplexer::addSlideStartHandler,
                                boost::ref( mrMultiplexer ), _1 ) );
}

void UserEventQueue::registerSlideEndEvent( const EventSharedPtr& rEvent )
{
    registerEvent( mpEndEventHandler,
                   rEvent,
                   boost::bind( &EventMultiplexer::addSlideEndHandler,
                                boost::ref( mrMultiplexer ), _1 ) );
}

void UserEventQueue::registerAnimationStartEvent(
    const EventSharedPtr&                                 rEvent,
    const uno::Reference< animations::XAnimationNode>&    xNode )
{
    registerEvent( mpAnimationStartEventHandler,
                   rEvent,
                   xNode,
                   boost::bind( &EventMultiplexer::addAnimationStartHandler,
                                boost::ref( mrMultiplexer ), _1 ) );
}

void UserEventQueue::registerAnimationEndEvent(
    const EventSharedPtr&                               rEvent,
    const uno::Reference<animations::XAnimationNode>&   xNode )
{
    registerEvent( mpAnimationEndEventHandler,
                   rEvent,
                   xNode,
                   boost::bind( &EventMultiplexer::addAnimationEndHandler,
                                boost::ref( mrMultiplexer ), _1 ) );
}

void UserEventQueue::registerAudioStoppedEvent(
    const EventSharedPtr&                               rEvent,
    const uno::Reference<animations::XAnimationNode>&   xNode )
{
    registerEvent( mpAudioStoppedEventHandler,
                   rEvent,
                   xNode,
                   boost::bind( &EventMultiplexer::addAudioStoppedHandler,
                                boost::ref( mrMultiplexer ), _1 ) );
}

void UserEventQueue::registerShapeClickEvent( const EventSharedPtr& rEvent,
                                              const ShapeSharedPtr& rShape )
{
    ENSURE_AND_THROW(
        rEvent.get(),
        "UserEventQueue::registerShapeClickEvent(): Invalid event" );

    if( !mpShapeClickEventHandler.get() )
    {
        // create handler
        mpShapeClickEventHandler.reset(
            new ShapeClickEventHandler( mrMultiplexer,
                                        mrEventQueue ) );

        // register handler on EventMultiplexer
        mrMultiplexer.addClickHandler( mpShapeClickEventHandler, 1.0 );
        mrMultiplexer.addMouseMoveHandler( mpShapeClickEventHandler, 1.0 );
    }

    mpShapeClickEventHandler->addEvent( rEvent, rShape );
}

namespace {
class ClickEventRegistrationFunctor
{
public:
    ClickEventRegistrationFunctor( EventMultiplexer& rMultiplexer,
                                   double            nPrio,
                                   bool              bAdvanceOnClick )
        : mrMultiplexer( rMultiplexer ),
          mnPrio(nPrio),
          mbAdvanceOnClick( bAdvanceOnClick ) {}

    void operator()( const boost::shared_ptr<ClickEventHandler>& rHandler )const
    {
        // register the handler on _two_ sources: we want the
        // nextEffect events, e.g. space bar, to trigger clicks, as well!
        mrMultiplexer.addClickHandler( rHandler, mnPrio );
        mrMultiplexer.addNextEffectHandler( rHandler, mnPrio );

        // forward advance-on-click state to newly
        // generated handler (that's the only reason why
        // we're called here)
        rHandler->setAdvanceOnClick( mbAdvanceOnClick );
    }

private:
    EventMultiplexer&   mrMultiplexer;
    double const        mnPrio;
    bool const          mbAdvanceOnClick;
};
} // anon namespace

void UserEventQueue::registerNextEffectEvent( const EventSharedPtr& rEvent )
{
    // TODO: better name may be mpNextEffectEventHandler?  then we have
    //       next effect (=> waiting to be started)
    //       skip effect (skipping the currently running one)
    //       rewind effect (rewinding back running one and waiting (again)
    //                      to be started)
    registerEvent( mpClickEventHandler,
                   rEvent,
                   ClickEventRegistrationFunctor( mrMultiplexer,
                                                  0.0 /* default prio */,
                                                  mbAdvanceOnClick ) );
}

void UserEventQueue::registerSkipEffectEvent( EventSharedPtr const & pEvent )
{
    if (mpSkipEffectEventHandler.get() == 0) {
        mpSkipEffectEventHandler.reset(
            new SkipEffectEventHandler( mrEventQueue, mrMultiplexer ) );
        // register the handler on _two_ sources: we want the
        // nextEffect events, e.g. space bar, to trigger clicks, as well!
        mrMultiplexer.addClickHandler( mpSkipEffectEventHandler,
                                       -1.0 /* prio below default */ );
        mrMultiplexer.addNextEffectHandler( mpSkipEffectEventHandler,
                                            -1.0 /* prio below default */ );
        // forward advance-on-click state to newly
        // generated handler (that's the only reason why
        // we're called here)
        mpSkipEffectEventHandler->setAdvanceOnClick( mbAdvanceOnClick );
    }
    mpSkipEffectEventHandler->addEvent( pEvent );
}

void UserEventQueue::registerRewindEffectEvent( EventSharedPtr const& pEvent )
{
    registerEvent( mpRewindEffectEventHandler,
                   pEvent,
                   boost::bind( &EventMultiplexer::addClickHandler,
                                boost::ref(mrMultiplexer), _1,
                                -1.0 /* prio below default */ ) );
}

void UserEventQueue::registerShapeDoubleClickEvent(
    const EventSharedPtr& rEvent,
    const ShapeSharedPtr& rShape )
{
    ENSURE_AND_THROW(
        rEvent.get(),
        "UserEventQueue::registerShapeDoubleClickEvent(): Invalid event" );

    if( !mpShapeDoubleClickEventHandler.get() )
    {
        // create handler
        mpShapeDoubleClickEventHandler.reset(
            new ShapeClickEventHandler( mrMultiplexer,
                                        mrEventQueue ) );

        // register handler on EventMultiplexer
        mrMultiplexer.addDoubleClickHandler( mpShapeDoubleClickEventHandler,
                                             1.0 );
        mrMultiplexer.addMouseMoveHandler( mpShapeDoubleClickEventHandler,
                                           1.0 );
    }

    mpShapeDoubleClickEventHandler->addEvent( rEvent, rShape );
}

void UserEventQueue::registerDoubleClickEvent( const EventSharedPtr& rEvent )
{
    registerEvent( mpDoubleClickEventHandler,
                   rEvent,
                   boost::bind( &EventMultiplexer::addDoubleClickHandler,
                                boost::ref( mrMultiplexer ), _1,
                                0.0 /* default prio */ ) );
}

void UserEventQueue::registerMouseEnterEvent( const EventSharedPtr& rEvent,
                                              const ShapeSharedPtr& rShape )
{
    registerEvent( mpMouseEnterHandler,
                   rEvent,
                   rShape,
                   boost::bind( &EventMultiplexer::addMouseMoveHandler,
                                boost::ref( mrMultiplexer ), _1,
                                0.0 /* default prio */ ) );
}

void UserEventQueue::registerMouseLeaveEvent( const EventSharedPtr& rEvent,
                                              const ShapeSharedPtr& rShape )
{
    registerEvent( mpMouseLeaveHandler,
                   rEvent,
                   rShape,
                   boost::bind( &EventMultiplexer::addMouseMoveHandler,
                                boost::ref( mrMultiplexer ), _1,
                                0.0 /* default prio */ ) );
}

} // namespace internal
} // namespace presentation

