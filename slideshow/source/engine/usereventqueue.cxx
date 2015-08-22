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

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>

#include <boost/bind.hpp>

#include "delayevent.hxx"
#include "usereventqueue.hxx"
#include "cursormanager.hxx"
#include "slideshowexceptions.hxx"

#include <vector>
#include <queue>
#include <map>
#include <functional>
#include <algorithm>


using namespace com::sun::star;

/* Implementation of UserEventQueue class */

namespace slideshow {
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
    virtual bool handleMousePressed( awt::MouseEvent const& /*e*/ ) SAL_OVERRIDE { return false;}
    virtual bool handleMouseReleased( awt::MouseEvent const& /*e*/) SAL_OVERRIDE { return false;}
    virtual bool handleMouseDragged( awt::MouseEvent const& /*e*/ ) SAL_OVERRIDE { return false;}
    virtual bool handleMouseMoved( awt::MouseEvent const& /*e*/ ) SAL_OVERRIDE { return false; }
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
        if(pEvent->isCharged())
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
    EventContainer() :
        maEvents()
    {}

    void addEvent( const EventSharedPtr& rEvent )
    {
        maEvents.push( rEvent );
    }

protected:
    ImpEventQueue maEvents;
};

} // anon namespace

class AllAnimationEventHandler : public AnimationEventHandler
{
public:
    explicit AllAnimationEventHandler( EventQueue& rEventQueue ) :
        mrEventQueue( rEventQueue ),
        maAnimationEventMap()
    {}

    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode ) SAL_OVERRIDE
    {
        ENSURE_OR_RETURN_FALSE(
            rNode,
            "AllAnimationEventHandler::handleAnimationEvent(): Invalid node" );

        bool bRet( false );

        ImpAnimationEventMap::iterator aIter;
        if( (aIter=maAnimationEventMap.find(
                 rNode->getXAnimationNode() )) != maAnimationEventMap.end() )
        {
            ImpEventVector& rVec( aIter->second );

            bRet = !rVec.empty();

            // registered node found -> fire all events in the vector
            for( const auto& pEvent : rVec )
                mrEventQueue.addEvent( pEvent );

            rVec.clear();
        }

        return bRet;
    }

    void addEvent( const EventSharedPtr&                                rEvent,
                   const uno::Reference< animations::XAnimationNode >&  xNode )
    {
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

private:
    EventQueue&             mrEventQueue;
    ImpAnimationEventMap    maAnimationEventMap;
};

class ClickEventHandler : public MouseEventHandler_,
                          public EventHandler,
                          public EventContainer
{
public:
    explicit ClickEventHandler( EventQueue& rEventQueue ) :
        EventContainer(),
        mrEventQueue( rEventQueue ),
        mbAdvanceOnClick( true )
    {}

    void setAdvanceOnClick( bool bAdvanceOnClick )
    {
        mbAdvanceOnClick = bAdvanceOnClick;
    }

private:

    // triggered by API calls, e.g. space bar
    virtual bool handleEvent() SAL_OVERRIDE
    {
        return handleEvent_impl();
    }

    // triggered by mouse release:
    virtual bool handleMouseReleased( const awt::MouseEvent& evt ) SAL_OVERRIDE
    {
        if(evt.Buttons != awt::MouseButton::LEFT)
            return false;

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
    EventQueue& mrEventQueue;
    bool        mbAdvanceOnClick;
};

class SkipEffectEventHandler : public ClickEventHandler
{
public:
    SkipEffectEventHandler( EventQueue & rEventQueue,
                            EventMultiplexer & rEventMultiplexer )
        : ClickEventHandler(rEventQueue),
          mrEventQueue(rEventQueue),
          mrEventMultiplexer(rEventMultiplexer),
          mbSkipTriggersNextEffect(true) {}

    /** Remember to trigger (or not to trigger) the next effect after the
        current effect is skipped.
    */
    void setSkipTriggersNextEffect (const bool bSkipTriggersNextEffect)
    { mbSkipTriggersNextEffect = bSkipTriggersNextEffect; }

    ///  Skip the current effect but do not trigger the next effect.
    void skipEffect() { handleEvent_impl(false); }

private:
    virtual bool handleEvent_impl() SAL_OVERRIDE
    {
        return handleEvent_impl(true);
    }

    bool handleEvent_impl (bool bNotifyNextEffect)
    {
        // fire all events, so animation nodes can register their
        // next effect listeners:
        if(fireAllEvents( maEvents, mrEventQueue ))
        {
            if (mbSkipTriggersNextEffect && bNotifyNextEffect)
            {
                // then simulate a next effect event: this skip effect
                // handler is triggered upon next effect events (multiplexer
                // prio=-1)!  Posting a notifyNextEffect() here is only safe
                // (we don't run into busy loop), because we assume that
                // someone has registerered above for next effects
                // (multiplexer prio=0) at the user event queue.
                return mrEventQueue.addEventWhenQueueIsEmpty(
                    makeEvent( [this] () {
                            this->mrEventMultiplexer.notifyNextEffect();
                        }, "EventMultiplexer::notifyNextEffect") );
            }
            else
                return true;
        }
        return false;
    }

private:
    EventQueue & mrEventQueue;
    EventMultiplexer & mrEventMultiplexer;
    bool mbSkipTriggersNextEffect;
};

/** Base class to share some common code between
    ShapeClickEventHandler and MouseMoveHandler

    @derive override necessary MouseEventHandler interface methods,
    call sendEvent() method to actually process the event.
*/
class MouseHandlerBase : public MouseEventHandler_
{
public:
    explicit MouseHandlerBase( EventQueue& rEventQueue ) :
        mrEventQueue( rEventQueue ),
        maShapeEventMap()
    {}

    void addEvent( const EventSharedPtr& rEvent,
                   const ShapeSharedPtr& rShape )
    {
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

protected:
    bool hitTest( const awt::MouseEvent&                e,
                  ImpShapeEventMap::reverse_iterator&   o_rHitShape )
    {
        // find hit shape in map
        const basegfx::B2DPoint aPosition( e.X, e.Y );

        // find matching shape (scan reversely, to coarsely match
        // paint order)
        ImpShapeEventMap::reverse_iterator       aCurrShape(maShapeEventMap.rbegin());
        const ImpShapeEventMap::reverse_iterator aEndShape( maShapeEventMap.rend() );
        while( aCurrShape != aEndShape )
        {
            // TODO(F2): Get proper geometry polygon from the
            // shape, to avoid having areas outside the shape
            // react on the mouse
            if( aCurrShape->first->getBounds().isInside( aPosition ) &&
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
            // method. Unfortunately, C++ does not
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

private:
    EventQueue&         mrEventQueue;
    ImpShapeEventMap    maShapeEventMap;
};

class ShapeClickEventHandler : public MouseHandlerBase
{
public:
    ShapeClickEventHandler( CursorManager& rCursorManager,
                            EventQueue&    rEventQueue ) :
        MouseHandlerBase( rEventQueue ),
        mrCursorManager( rCursorManager )
    {}

    virtual bool handleMouseReleased( const awt::MouseEvent& e ) SAL_OVERRIDE
    {
        if(e.Buttons != awt::MouseButton::LEFT)
            return false;
        return processEvent( e );
    }

    virtual bool handleMouseMoved( const awt::MouseEvent& e ) SAL_OVERRIDE
    {
        // TODO(P2): Maybe buffer last shape touched

        // if we have a shape click event, and the mouse
        // hovers over this shape, change cursor to hand
        ImpShapeEventMap::reverse_iterator aDummy;
        if( hitTest( e, aDummy ) )
            mrCursorManager.requestCursor( awt::SystemPointer::REFHAND );

        return false; // we don't /eat/ this event. Lower prio
        // handler should see it, too.
    }

private:
    CursorManager& mrCursorManager;
};

class MouseEnterHandler : public MouseHandlerBase
{
public:
    explicit MouseEnterHandler( EventQueue& rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          mpLastShape() {}

    virtual bool handleMouseMoved( const awt::MouseEvent& e ) SAL_OVERRIDE
    {
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
    explicit MouseLeaveHandler( EventQueue& rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          maLastIter() {}

    virtual bool handleMouseMoved( const awt::MouseEvent& e ) SAL_OVERRIDE
    {
        // TODO(P2): Maybe buffer last shape touched, and
        // check against that _first_

        ImpShapeEventMap::reverse_iterator aCurr;
        if( hitTest( e, aCurr ) )
        {
            maLastIter = aCurr;
        }
        else
        {
            if( maLastIter->first )
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
    ENSURE_OR_THROW( rEvent,
                      "UserEventQueue::registerEvent(): Invalid event" );

    if( !rHandler ) {
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
    ENSURE_OR_THROW( rEvent,
                      "UserEventQueue::registerEvent(): Invalid event" );

    if( !rHandler ) {
        // create handler
        rHandler.reset( new Handler( mrEventQueue ) );

        // register handler on EventMultiplexer
        rRegistrationFunctor( rHandler );
    }

    rHandler->addEvent( rEvent, rArg );
}



UserEventQueue::UserEventQueue( EventMultiplexer&   rMultiplexer,
                                EventQueue&         rEventQueue,
                                CursorManager&      rCursorManager )
    : mrMultiplexer( rMultiplexer ),
      mrEventQueue( rEventQueue ),
      mrCursorManager( rCursorManager ),
      mpAnimationStartEventHandler(),
      mpAnimationEndEventHandler(),
      mpAudioStoppedEventHandler(),
      mpClickEventHandler(),
      mpSkipEffectEventHandler(),
      mpDoubleClickEventHandler(),
      mpMouseEnterHandler(),
      mpMouseLeaveHandler(),
      mbAdvanceOnClick( true )
{
}

UserEventQueue::~UserEventQueue()
{
    try
    {
        // unregister all handlers
        clear();
    }
    catch (uno::Exception &) {
        OSL_FAIL( OUStringToOString(
                        comphelper::anyToString(
                            cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

void UserEventQueue::clear()
{
    // unregister and delete all handlers
    if( mpAnimationStartEventHandler ) {
        mrMultiplexer.removeAnimationStartHandler(
            mpAnimationStartEventHandler );
        mpAnimationStartEventHandler.reset();
    }
    if( mpAnimationEndEventHandler ) {
        mrMultiplexer.removeAnimationEndHandler( mpAnimationEndEventHandler );
        mpAnimationEndEventHandler.reset();
    }
    if( mpAudioStoppedEventHandler ) {
        mrMultiplexer.removeAudioStoppedHandler( mpAudioStoppedEventHandler );
        mpAudioStoppedEventHandler.reset();
    }
    if( mpShapeClickEventHandler ) {
        mrMultiplexer.removeClickHandler( mpShapeClickEventHandler );
        mrMultiplexer.removeMouseMoveHandler( mpShapeClickEventHandler );
        mpShapeClickEventHandler.reset();
    }
    if( mpClickEventHandler ) {
        mrMultiplexer.removeClickHandler( mpClickEventHandler );
        mrMultiplexer.removeNextEffectHandler( mpClickEventHandler );
        mpClickEventHandler.reset();
    }
    if(mpSkipEffectEventHandler) {
        mrMultiplexer.removeClickHandler( mpSkipEffectEventHandler );
        mrMultiplexer.removeNextEffectHandler( mpSkipEffectEventHandler );
        mpSkipEffectEventHandler.reset();
    }
    if( mpShapeDoubleClickEventHandler ) {
        mrMultiplexer.removeDoubleClickHandler( mpShapeDoubleClickEventHandler );
        mrMultiplexer.removeMouseMoveHandler( mpShapeDoubleClickEventHandler );
        mpShapeDoubleClickEventHandler.reset();
    }
    if( mpDoubleClickEventHandler ) {
        mrMultiplexer.removeDoubleClickHandler( mpDoubleClickEventHandler );
        mpDoubleClickEventHandler.reset();
    }
    if( mpMouseEnterHandler ) {
        mrMultiplexer.removeMouseMoveHandler( mpMouseEnterHandler );
        mpMouseEnterHandler.reset();
    }
    if( mpMouseLeaveHandler ) {
        mrMultiplexer.removeMouseMoveHandler( mpMouseLeaveHandler );
        mpMouseLeaveHandler.reset();
    }
}

void UserEventQueue::setAdvanceOnClick( bool bAdvanceOnClick )
{
    mbAdvanceOnClick = bAdvanceOnClick;

    // forward to handler, if existing. Otherwise, the handler
    // creation will do the forwarding.
    if( mpClickEventHandler )
        mpClickEventHandler->setAdvanceOnClick( bAdvanceOnClick );
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
    ENSURE_OR_THROW(
        rEvent,
        "UserEventQueue::registerShapeClickEvent(): Invalid event" );

    if( !mpShapeClickEventHandler )
    {
        // create handler
        mpShapeClickEventHandler.reset(
            new ShapeClickEventHandler(mrCursorManager,
                                       mrEventQueue) );

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

void UserEventQueue::registerSkipEffectEvent(
    EventSharedPtr const & pEvent,
    const bool bSkipTriggersNextEffect)
{
    if(!mpSkipEffectEventHandler)
    {
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
    mpSkipEffectEventHandler->setSkipTriggersNextEffect(bSkipTriggersNextEffect);
    mpSkipEffectEventHandler->addEvent( pEvent );
}

void UserEventQueue::registerShapeDoubleClickEvent(
    const EventSharedPtr& rEvent,
    const ShapeSharedPtr& rShape )
{
    ENSURE_OR_THROW(
        rEvent,
        "UserEventQueue::registerShapeDoubleClickEvent(): Invalid event" );

    if( !mpShapeDoubleClickEventHandler )
    {
        // create handler
        mpShapeDoubleClickEventHandler.reset(
            new ShapeClickEventHandler(mrCursorManager,
                                       mrEventQueue) );

        // register handler on EventMultiplexer
        mrMultiplexer.addDoubleClickHandler( mpShapeDoubleClickEventHandler,
                                             1.0 );
        mrMultiplexer.addMouseMoveHandler( mpShapeDoubleClickEventHandler,
                                           1.0 );
    }

    mpShapeDoubleClickEventHandler->addEvent( rEvent, rShape );
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

void UserEventQueue::callSkipEffectEventHandler()
{
    ::boost::shared_ptr<SkipEffectEventHandler> pHandler (
        ::boost::dynamic_pointer_cast<SkipEffectEventHandler>(mpSkipEffectEventHandler));
    if (pHandler)
        pHandler->skipEffect();
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
