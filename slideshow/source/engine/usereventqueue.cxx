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


class MouseEventHandler_ : public MouseEventHandler
{
public:
    virtual bool handleMousePressed( awt::MouseEvent const& /*e*/ ) { return false;}
    virtual bool handleMouseReleased( awt::MouseEvent const& /*e*/) { return false;}
    virtual bool handleMouseEntered( awt::MouseEvent const& /*e*/ ) { return false;}
    virtual bool handleMouseExited( awt::MouseEvent const& /*e*/ ) { return false; }
    virtual bool handleMouseDragged( awt::MouseEvent const& /*e*/ ) { return false;}
    virtual bool handleMouseMoved( awt::MouseEvent const& /*e*/ ) { return false; }
};

/** @return one event has been posted
 */
template <typename ContainerT>
bool fireSingleEvent( ContainerT & rQueue, EventQueue & rEventQueue )
{
    
    while (! rQueue.empty())
    {
        EventSharedPtr const pEvent(rQueue.front());
        rQueue.pop();

        
        
        
        
        
        if(pEvent->isCharged())
            return rEventQueue.addEvent( pEvent );
    }
    return false; 
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

    void clearContainer()
    {
        maEvents = ImpEventQueue();
    }

    void addEvent( const EventSharedPtr& rEvent )
    {
        maEvents.push( rEvent );
    }

protected:
    ImpEventQueue maEvents;
};

} 

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
        return fireAllEvents( maEvents, mrEventQueue );
    }

private:
    EventQueue & mrEventQueue;
};

class AllAnimationEventHandler : public AnimationEventHandler
{
public:
    AllAnimationEventHandler( EventQueue& rEventQueue ) :
        mrEventQueue( rEventQueue ),
        maAnimationEventMap()
    {}

    virtual void dispose()
    {
        maAnimationEventMap.clear();
    }

    virtual bool handleAnimationEvent( const AnimationNodeSharedPtr& rNode )
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
        ImpAnimationEventMap::iterator aIter;
        if( (aIter=maAnimationEventMap.find( xNode )) ==
            maAnimationEventMap.end() )
        {
            
            aIter = maAnimationEventMap.insert(
                ImpAnimationEventMap::value_type( xNode,
                                                  ImpEventVector() ) ).first;
        }

        
        aIter->second.push_back( rEvent );
    }

    bool isEmpty()
    {
        
        ImpAnimationEventMap::const_iterator aCurr( maAnimationEventMap.begin() );
        const ImpAnimationEventMap::const_iterator aEnd( maAnimationEventMap.end() );
        while( aCurr != aEnd )
        {
            if( !aCurr->second.empty() )
                return false; 

            ++aCurr;
        }

        return true; 
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
    ClickEventHandler( EventQueue& rEventQueue ) :
        EventContainer(),
        mrEventQueue( rEventQueue ),
        mbAdvanceOnClick( true )
    {}

    void setAdvanceOnClick( bool bAdvanceOnClick )
    {
        mbAdvanceOnClick = bAdvanceOnClick;
    }

private:
    virtual void dispose()
    {
        clearContainer();
    }

    
    virtual bool handleEvent()
    {
        return handleEvent_impl();
    }

    
    virtual bool handleMouseReleased( const awt::MouseEvent& evt )
    {
        if(evt.Buttons != awt::MouseButton::LEFT)
            return false;

        if( mbAdvanceOnClick ) {
            
            return handleEvent_impl();
        }
        else {
            return false; 
        }
    }

    
    virtual bool handleEvent_impl()
    {
        
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
        current effect is skiped.
    */
    void setSkipTriggersNextEffect (const bool bSkipTriggersNextEffect)
    { mbSkipTriggersNextEffect = bSkipTriggersNextEffect; }

    
    void skipEffect (void) { handleEvent_impl(false); }

private:
    virtual bool handleEvent_impl()
    {
        return handleEvent_impl(true);
    }

    bool handleEvent_impl (bool bNotifyNextEffect)
    {
        
        
        if(fireAllEvents( maEvents, mrEventQueue ))
        {
            if (mbSkipTriggersNextEffect && bNotifyNextEffect)
            {
                
                
                
                
                
                
                return mrEventQueue.addEventWhenQueueIsEmpty(
                    makeEvent( boost::bind( &EventMultiplexer::notifyNextEffect,
                                            boost::ref(mrEventMultiplexer) ),
                               "EventMultiplexer::notifyNextEffect") );
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
        if(evt.Buttons != awt::MouseButton::RIGHT)
            return false;

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
    MouseHandlerBase( EventQueue& rEventQueue ) :
        mrEventQueue( rEventQueue ),
        maShapeEventMap()
    {}

    virtual void dispose()
    {
        
        
        maShapeEventMap = ImpShapeEventMap();
    }

    void addEvent( const EventSharedPtr& rEvent,
                   const ShapeSharedPtr& rShape )
    {
        ImpShapeEventMap::iterator aIter;
        if( (aIter=maShapeEventMap.find( rShape )) == maShapeEventMap.end() )
        {
            
            aIter = maShapeEventMap.insert(
                ImpShapeEventMap::value_type( rShape,
                                              ImpEventQueue() ) ).first;
        }

        
        aIter->second.push( rEvent );
    }

    bool isEmpty()
    {
        
        ImpShapeEventMap::reverse_iterator aCurrShape( maShapeEventMap.begin());
        ImpShapeEventMap::reverse_iterator aEndShape( maShapeEventMap.end() );
        while( aCurrShape != aEndShape )
        {
            if( !aCurrShape->second.empty() )
                return false; 

            ++aCurrShape;
        }

        return true; 
    }

protected:
    bool hitTest( const awt::MouseEvent&                e,
                  ImpShapeEventMap::reverse_iterator&   o_rHitShape )
    {
        
        const basegfx::B2DPoint aPosition( e.X, e.Y );

        
        
        ImpShapeEventMap::reverse_iterator       aCurrShape(maShapeEventMap.rbegin());
        const ImpShapeEventMap::reverse_iterator aEndShape( maShapeEventMap.rend() );
        while( aCurrShape != aEndShape )
        {
            
            
            
            if( aCurrShape->first->getBounds().isInside( aPosition ) &&
                aCurrShape->first->isVisible() )
            {
                
                
                o_rHitShape = aCurrShape;
                return true;
            }

            ++aCurrShape;
        }

        return false; 
    }

    bool sendEvent( ImpShapeEventMap::reverse_iterator& io_rHitShape )
    {
        
        const bool bRet( fireSingleEvent( io_rHitShape->second,
                                          mrEventQueue ) );

        
        
        
        
        
        if( io_rHitShape->second.empty() )
        {
            
            
            
            
            
            
            maShapeEventMap.erase( io_rHitShape->first );
        }

        return bRet;
    }

    bool processEvent( const awt::MouseEvent& e )
    {
        ImpShapeEventMap::reverse_iterator aCurrShape;

        if( hitTest( e, aCurrShape ) )
            return sendEvent( aCurrShape );

        return false; 
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

    virtual bool handleMouseReleased( const awt::MouseEvent& e )
    {
        if(e.Buttons != awt::MouseButton::LEFT)
            return false;
        return processEvent( e );
    }

    virtual bool handleMouseMoved( const awt::MouseEvent& e )
    {
        

        
        
        ImpShapeEventMap::reverse_iterator aDummy;
        if( hitTest( e, aDummy ) )
            mrCursorManager.requestCursor( awt::SystemPointer::REFHAND );

        return false; 
        
    }

private:
    CursorManager& mrCursorManager;
};

class MouseEnterHandler : public MouseHandlerBase
{
public:
    MouseEnterHandler( EventQueue& rEventQueue )
        : MouseHandlerBase( rEventQueue ),
          mpLastShape() {}

    virtual bool handleMouseMoved( const awt::MouseEvent& e )
    {
        
        

        ImpShapeEventMap::reverse_iterator aCurr;
        if( hitTest( e, aCurr ) )
        {
            if( aCurr->first != mpLastShape )
            {
                
                
                
                sendEvent( aCurr );
                mpLastShape = aCurr->first;
            }
        }
        else
        {
            
            mpLastShape.reset();
        }

        return false; 
        
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
        
        

        ImpShapeEventMap::reverse_iterator aCurr;
        if( hitTest( e, aCurr ) )
        {
            maLastIter = aCurr;
        }
        else
        {
            if( maLastIter->first )
            {
                
                
                
                sendEvent( maLastIter );
            }

            
            
            maLastIter = ImpShapeEventMap::reverse_iterator();
        }

        return false; 
        
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
        
        rHandler.reset( new Handler( mrEventQueue ) );
        
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
        
        rHandler.reset( new Handler( mrEventQueue ) );

        
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
    try
    {
        
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
    
    if( mpStartEventHandler ) {
        mrMultiplexer.removeSlideStartHandler( mpStartEventHandler );
        mpStartEventHandler.reset();
    }
    if( mpEndEventHandler ) {
        mrMultiplexer.removeSlideEndHandler( mpEndEventHandler );
        mpEndEventHandler.reset();
    }
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
    if(mpRewindEffectEventHandler) {
        mrMultiplexer.removeClickHandler( mpRewindEffectEventHandler );
        mpRewindEffectEventHandler.reset();
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
        
        mpShapeClickEventHandler.reset(
            new ShapeClickEventHandler(mrCursorManager,
                                       mrEventQueue) );

        
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
        
        
        mrMultiplexer.addClickHandler( rHandler, mnPrio );
        mrMultiplexer.addNextEffectHandler( rHandler, mnPrio );

        
        
        
        rHandler->setAdvanceOnClick( mbAdvanceOnClick );
    }

private:
    EventMultiplexer&   mrMultiplexer;
    double const        mnPrio;
    bool const          mbAdvanceOnClick;
};
} 

void UserEventQueue::registerNextEffectEvent( const EventSharedPtr& rEvent )
{
    
    
    
    
    
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
        
        
        mrMultiplexer.addClickHandler( mpSkipEffectEventHandler,
                                       -1.0 /* prio below default */ );
        mrMultiplexer.addNextEffectHandler( mpSkipEffectEventHandler,
                                            -1.0 /* prio below default */ );
        
        
        
        mpSkipEffectEventHandler->setAdvanceOnClick( mbAdvanceOnClick );
    }
    mpSkipEffectEventHandler->setSkipTriggersNextEffect(bSkipTriggersNextEffect);
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
    ENSURE_OR_THROW(
        rEvent,
        "UserEventQueue::registerShapeDoubleClickEvent(): Invalid event" );

    if( !mpShapeDoubleClickEventHandler )
    {
        
        mpShapeDoubleClickEventHandler.reset(
            new ShapeClickEventHandler(mrCursorManager,
                                       mrEventQueue) );

        
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

void UserEventQueue::callSkipEffectEventHandler (void)
{
    ::boost::shared_ptr<SkipEffectEventHandler> pHandler (
        ::boost::dynamic_pointer_cast<SkipEffectEventHandler>(mpSkipEffectEventHandler));
    if (pHandler)
        pHandler->skipEffect();
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
