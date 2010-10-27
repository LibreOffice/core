/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_USEREVENTQUEUE_HXX
#define INCLUDED_SLIDESHOW_USEREVENTQUEUE_HXX

#include <com/sun/star/animations/XAnimationNode.hpp>

#include "eventmultiplexer.hxx"
#include "eventqueue.hxx"
#include "shape.hxx"

#include <boost/noncopyable.hpp>

/* Definition of UserEventQueue class */

namespace slideshow {
namespace internal {

class PlainEventHandler;
class AllAnimationEventHandler;
class ShapeClickEventHandler;
class ClickEventHandler;
class CursorManager;
class SkipEffectEventHandler;
class RewindEffectEventHandler;
class MouseEnterHandler;
class MouseLeaveHandler;

/** This class schedules user-activated events.

    This class registeres at the EventMultiplexer and fires
    events registered for certain user actions. Note that all
    events will not be fired immediately after the user action
    occured, but always added to the EventQueue (and fired the
    next time that queue is processed). Which is actually a
    feature.

    Conceptually, an event is an object that typically is
    fired only once. After that, the event is exhausted, and
    should be discarded. Therefore, all events registered on
    this object are fired and then all references to them are
    removed.
*/
class UserEventQueue : private ::boost::noncopyable
{
public:
    /** Create a user event queue

        @param rEventMultiplexer
        The slideshow-global event source, where this class
        registeres its event handlers.

        @param rEventQueue
        Reference to the main event queue. Since we hold this
        object by plain reference, it must live longer than we
        do. On the other hand, that queue must not fire events
        after this object is destroyed, since we might
        schedule events there which itself contain plain
        references to this object. Basically, EventQueue and
        UserEventQueue should have the same lifetime, and since
        this is not possible, both must be destructed in a
        phased mode: first clear both of any remaining events,
        then destruct them.
    */
    UserEventQueue( EventMultiplexer&   rMultiplexer,
                    EventQueue&         rEventQueue,
                    CursorManager&      rCursorManager );
    ~UserEventQueue();

    /** Query whether there are any events still pending.
     */
    bool isEmpty() const;

    /** Clear all registered events.

        This method clears all registered, but
        not-yet-executed events. This comes in handy when
        force-ending a slide, to avoid interference with the
        next slide's event registration.
    */
    void clear();

    /** Set advance on click behaviour.

        @param bAdvanceOnClick
        When true, a click somewhere on the slide will also
        generate next effect event.  In this case, it is
        irrelevant where on the slide the mouse is clicked,
        i.e. the shape need not be hit by the mouse.
    */
    void setAdvanceOnClick( bool bAdvanceOnClick );

    /** Register an event that will be fired when the slide is
        just shown.

        Note that <em>all</em> registered events will be fired
        when the slide start occurs. This is in contrast to
        the mouse events below.
    */
    void registerSlideStartEvent( const EventSharedPtr& rEvent );

    /** Register an event that will be fired when the slide is
        about to vanish.

        Note that <em>all</em> registered events will be fired
        when the slide end occurs. This is in contrast to
        the mouse events below.
    */
    void registerSlideEndEvent( const EventSharedPtr& rEvent );

    /** Register an event that will be fired when the given
        animation node starts.

        Note that <em>all</em> registered events will be fired
        when the animation start occurs. This is in contrast to
        the mouse events below.
    */
    void registerAnimationStartEvent(
        const EventSharedPtr&                             rEvent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode>&    xNode );

    /** Register an event that will be fired when the given
        animation node ends its active duration.

        Note that <em>all</em> registered events will be fired
        when the animation end occurs. This is in contrast to
        the mouse events below.
    */
    void registerAnimationEndEvent(
        const EventSharedPtr&                               rEvent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode>&      xNode );

    /** Register an event that will be fired when audio output
        stopped for the given animation node.

        Note that <em>all</em> registered events will be fired
        when the audio stopping occurs. This is in contrast to
        the mouse events below.
    */
    void registerAudioStoppedEvent(
        const EventSharedPtr&                               rEvent,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::animations::XAnimationNode>&      xNode );

    /** Register an event that is fired when a shape is clicked

        For every mouse click, only one of the events
        registered here is fired. The order of fired events is
        the order of registration, i.e. the first event
        registered will be the one fired for the first mouse
        click on the given shape.
    */
    void registerShapeClickEvent( const EventSharedPtr& rEvent,
                                  const ShapeSharedPtr& rShape );

    /** Registes an event that is fired when the current effects(s)
        are skipped, .e.g. when the left mouse button is pressed.
        Then, all registered events are fired and removed from this
        queue.  After firing, a next effect event is issued to this
        queue to start the next effect.
        @param pEvent
            The event to execute when skipping the current effect.
        @param bSkipTriggersNextEffect
            When <TRUE/> then after skipping the current effect the next
            effect is triggered.  When <FALSE/> then the next effect is not
            triggered.
    */
    void registerSkipEffectEvent(
        EventSharedPtr const& pEvent,
        const bool bSkipTriggersNextEffect);

    /** Registes an event that is fired when the current effects(s)
        are rewound, .e.g. when the right mouse button is pressed.
        Then, all registered events are fired and removed from this
        queue.
    */
    void registerRewindEffectEvent( EventSharedPtr const& rEvent );

    /** Register an event that is fired to show the next event

        For every next effect event, only one of the events
        registered here is fired. The order of fired events is
        the order of registration, i.e. the first event
        registered will be the one fired for the first mouse
        click. When advance-on-click (see method
        setAdvanceOnClick()) is enabled, a mouse click
        somewhere on the slide will also generate a next
        effect event. In this case, it is irrelevant where on
        the slide the mouse is clicked, i.e. the shape need
        not be hit by the mouse.
    */
    void registerNextEffectEvent( const EventSharedPtr& rEvent );

    /** Register an event that is fired on a double mouse
        click on a shape

        For every mouse double click, only one of the events
        registered here is fired. The order of fired events is
        the order of registration, i.e. the first event
        registered will be the one fired for the first mouse
        double click. It is irrelevant where on the slide the
        mouse is clicked, i.e. the shape need not be hit by
        the mouse.
    */
    void registerShapeDoubleClickEvent( const EventSharedPtr& rEvent,
                                        const ShapeSharedPtr& rShape );

    /** Register an event that is fired on a double mouse click

        For every mouse double click, only one of the events
        registered here is fired. The order of fired events is
        the order of registration, i.e. the first event
        registered will be the one fired for the first mouse
        double click. It is irrelevant where on the slide the
        mouse is clicked, i.e. the shape need not be hit by
        the mouse.
    */
    void registerDoubleClickEvent( const EventSharedPtr& rEvent );

    /** Register an event that is fired when the mouse enters
        the area of the given shape

        For every enter, only one of the events registered
        here is fired. The order of fired events is the order
        of registration, i.e. the first event registered will
        be the one fired for the first time the mouse enters
        the given shape.
    */
    void registerMouseEnterEvent( const EventSharedPtr& rEvent,
                                  const ShapeSharedPtr& rShape );

    /** Register an event that is fired when the mouse leaves
        the area of the given shape

        For every leave, only one of the events registered
        here is fired. The order of fired events is the order
        of registration, i.e. the first event registered will
        be the one fired for the first time the mouse leaves
        the given shape area.
    */
    void registerMouseLeaveEvent( const EventSharedPtr& rEvent,
                                  const ShapeSharedPtr& rShape );

    /** Typically skipping the current effect is triggered by mouse clicks
        or key presses that trigger the next effect.  This method allows the
        skipping of effects to be triggered programatically.
    */
    void callSkipEffectEventHandler (void);

private:
    /** Generically register an event on one of the handlers.

        If the handler is not yet created, do that and
        register it via the Functor
    */
    template< typename Handler, typename Functor >
    void registerEvent( ::boost::shared_ptr< Handler >& rHandler,
                        const EventSharedPtr&           rEvent,
                        const Functor&                  rRegistrationFunctor );

    /** Generically register an event on one of the handlers.

        If the handler is not yet created, do that and
        register it via the Functor. This version of the
        registerEvent method takes an additional parameter
        rArg, which is passed as the second argument to
        rHandler's addEvent() method.
    */
    template< typename Handler, typename Arg, typename Functor >
    void registerEvent( ::boost::shared_ptr< Handler >& rHandler,
                        const EventSharedPtr&           rEvent,
                        const Arg&                      rArg,
                        const Functor&                  rRegistrationFunctor );

    EventMultiplexer&                               mrMultiplexer;
    EventQueue&                                     mrEventQueue;
    CursorManager&                                  mrCursorManager;

    ::boost::shared_ptr<PlainEventHandler>          mpStartEventHandler;
    ::boost::shared_ptr<PlainEventHandler>          mpEndEventHandler;
    ::boost::shared_ptr<AllAnimationEventHandler>   mpAnimationStartEventHandler;
    ::boost::shared_ptr<AllAnimationEventHandler>   mpAnimationEndEventHandler;
    ::boost::shared_ptr<AllAnimationEventHandler>   mpAudioStoppedEventHandler;
    ::boost::shared_ptr<ShapeClickEventHandler>     mpShapeClickEventHandler;
    ::boost::shared_ptr<ClickEventHandler>          mpClickEventHandler;
    ::boost::shared_ptr<SkipEffectEventHandler>     mpSkipEffectEventHandler;
    ::boost::shared_ptr<RewindEffectEventHandler>   mpRewindEffectEventHandler;
    ::boost::shared_ptr<ShapeClickEventHandler>     mpShapeDoubleClickEventHandler;
    ::boost::shared_ptr<ClickEventHandler>          mpDoubleClickEventHandler;
    ::boost::shared_ptr<MouseEnterHandler>          mpMouseEnterHandler;
    ::boost::shared_ptr<MouseLeaveHandler>          mpMouseLeaveHandler;

    bool                                            mbAdvanceOnClick;
};

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_USEREVENTQUEUE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
