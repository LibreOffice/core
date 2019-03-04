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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_EVENTMULTIPLEXER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_EVENTMULTIPLEXER_HXX

#include "eventhandler.hxx"
#include "mouseeventhandler.hxx"
#include "animationeventhandler.hxx"
#include "pauseeventhandler.hxx"
#include "shapelistenereventhandler.hxx"
#include "vieweventhandler.hxx"

#include <memory>
#include <com/sun/star/uno/Reference.hxx>

#include "unoview.hxx"

namespace com { namespace sun { namespace star { namespace drawing
{
    class XShape;
} } } }

namespace slideshow {
namespace internal {

class EventQueue;
class UnoViewContainer;
class AnimationNode;

struct EventMultiplexerImpl;

class RGBColor;

/** Interface for handling view repaint events.

    Classes implementing this interface can be added to an
    EventMultiplexer object, and are called from there to
    handle view repaint events.
*/
class ViewRepaintHandler
{
public:
    virtual ~ViewRepaintHandler() {}

    /** Notify clobbered view.

      Reasons for a viewChanged notification can be
      different view size, transformation, or other device
      properties (color resolution or profile, etc.)

      @param rView
      The changed view
     */
    virtual void viewClobbered( const UnoViewSharedPtr& rView ) = 0;
};

typedef ::std::shared_ptr< ViewRepaintHandler > ViewRepaintHandlerSharedPtr;

/** Interface for handling hyperlink clicks.

  Classes implementing this interface can be added to an
  EventMultiplexer object, and are called from there to
  handle hyperlink events.
 */
class HyperlinkHandler
{
public:
    /** Handle the event.

        @param rLink
        The actual hyperlink URI

        @return true, if this handler has successfully
        processed the event. When this method returns false,
        possibly other, less prioritized handlers are called,
        too.
    */
    virtual bool handleHyperlink( OUString const& rLink ) = 0;

protected:
    ~HyperlinkHandler() {}
};

typedef ::std::shared_ptr< HyperlinkHandler > HyperlinkHandlerSharedPtr;

/** Interface for handling user paint state changes.

    Classes implementing this interface can be added to an
    EventMultiplexer object, and are called from there to
    handle user paint events.
*/
class UserPaintEventHandler
{
public:
    virtual ~UserPaintEventHandler() {}
    virtual bool colorChanged( RGBColor const& rUserColor ) = 0;
    virtual bool widthChanged( double nUserStrokeWidth ) = 0;
    virtual bool eraseAllInkChanged(bool bEraseAllInk) =0;
    virtual bool eraseInkWidthChanged(sal_Int32 rEraseInkSize) =0;
    virtual bool switchEraserMode() = 0;
    virtual bool switchPenMode() = 0;
    virtual bool disable() = 0;
};

typedef ::std::shared_ptr< UserPaintEventHandler > UserPaintEventHandlerSharedPtr;

/** This class multiplexes user-activated and
    slide-show global events.

    This class listens at the XSlideShowView and fires events
    registered for certain user actions. Furthermore, global
    slide show state changes (such as start or end of a slide)
    are handled as well. Note that registered events which
    have a non-zero timeout (i.e. events that return non-zero
    from getActivationTime()) will not be fired immediately
    after the user action occurred, but only after the given
    timeout. Which is actually a feature.
*/
class EventMultiplexer
{
public:
    /** Create an event multiplexer

        @param rEventQueue
        Reference to the main event queue. Since we hold this
        object by plain reference, it must live longer than we
        do. On the other hand, that queue must not fire events
        after this object is destroyed, since we might
        schedule events there which itself contain plain
        references to this object. Basically, EventQueue and
        EventMultiplexer should have the same lifetime, and since
        this is not possible, both must be destructed in a
        phased mode: first clear both of any remaining events,
        then destruct them.

        @param rViewContainer
        Globally managed list of all registered views. Used to
        determine event sources, and for registering view listeners
        at.
    */
    EventMultiplexer( EventQueue&             rEventQueue,
                      UnoViewContainer const& rViewContainer );
    ~EventMultiplexer();
    EventMultiplexer(const EventMultiplexer&) = delete;
    EventMultiplexer& operator=(const EventMultiplexer&) = delete;

    // Management methods


    /** Clear all registered handlers.
     */
    void clear();


    // Automatic mode methods


    /** Change automatic mode.

        @param bIsAuto
        When true, events will be fired automatically, not
        only triggered by UI events. When false, auto events
        will quit.
    */
    void setAutomaticMode( bool bIsAuto );

    /** Get automatic mode setting.
     */
    bool getAutomaticMode() const;

    /** Set the timeout for automatic mode.

        @param nTimeout
        Timeout, between end of effect until start of next
        effect.
    */
    void setAutomaticTimeout( double nTimeout );

    /** Get automatic mode timeout value.
     */
    double getAutomaticTimeout() const;

    // Handler registration methods


    /** Register an event handler that will be called when views are
        changed.

        For each view added, viewAdded() will be called on the
        handler. For each view removed, viewRemoved() will be
        called. Each modified view will cause a viewChanged() call on
        each handler.

        You don't need to deregister the handler, it will be
        automatically removed, once the pointee becomes stale.

        @param rHandler
        Handler to call.
    */
    void addViewHandler( const ViewEventHandlerWeakPtr& rHandler );
    void removeViewHandler( const ViewEventHandlerWeakPtr& rHandler );

    /** Register an event handler that will be called when a view gets
        clobbered.

        Note that <em>all</em> registered handlers will be called when
        the event. This is in contrast to the mouse events below.

        @param rHandler
        Handler to call when a view needs a repaint
    */
    void addViewRepaintHandler( const ViewRepaintHandlerSharedPtr& rHandler );
    void removeViewRepaintHandler( const ViewRepaintHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when
        XShapeListeners are changed.

        @param rHandler
        Handler to call when a shape listener changes
    */
    void addShapeListenerHandler( const ShapeListenerEventHandlerSharedPtr& rHandler );
    void removeShapeListenerHandler( const ShapeListenerEventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when
        user paint parameters change.

        @param rHandler
        Handler to call when a shape listener changes
    */
    void addUserPaintHandler( const UserPaintEventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when the
        user requests the next effect.

        For every nextEffect event, only one of the handlers
        registered here is called. The handlers are considered
        with decreasing priority, i.e. the handler with the
        currently highest priority will be called.

        @param rHandler
        Handler to call when the next effect should start

        @param nPriority
        Priority with which the handlers are called. The
        higher the priority, the earlier this handler will be
        tried.
    */
    void addNextEffectHandler( const EventHandlerSharedPtr& rHandler,
                               double                       nPriority );
    void removeNextEffectHandler( const EventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when the
        slide is just shown.

        Note that <em>all</em> registered handlers will be called
        when the slide start occurs. This is in contrast to
        the mouse events below.

        @param rHandler
        Handler to call when the next slide starts
    */
    void addSlideStartHandler( const EventHandlerSharedPtr& rHandler );
    void removeSlideStartHandler( const EventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when the
        slide is about to vanish.

        Note that <em>all</em> registered handlers will be
        called when the slide end occurs. This is in contrast
        to the mouse events below.

        @param rHandler
        Handler to call when the current slide ends
    */
    void addSlideEndHandler( const EventHandlerSharedPtr& rHandler );
    void removeSlideEndHandler( const EventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when an
        XAnimationNode starts its active duration.

        Note that <em>all</em> registered handlers will be called
        when the animation start occurs. This is in contrast to
        the mouse events below.

        @param rHandler
        Handler to call when the animation start
    */
    void addAnimationStartHandler(
        const AnimationEventHandlerSharedPtr& rHandler );
    void removeAnimationStartHandler(
        const AnimationEventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when an
        XAnimationNode ends its active duration.

        Note that <em>all</em> registered handlers will be called
        when the animation end occurs. This is in contrast to
        the mouse events below.

        @param rHandler
        Handler to call when the animation ends
    */
    void addAnimationEndHandler(
        const AnimationEventHandlerSharedPtr& rHandler );
    void removeAnimationEndHandler(
        const AnimationEventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when the
        main animation sequence of a slide ends its active
        duration.

        Note that <em>all</em> registered handlers will be
        called when the animation end occurs. This is in
        contrast to the mouse events below.

        @param rHandler
        Handler to call when the animation ends
    */
    void addSlideAnimationsEndHandler(
        const EventHandlerSharedPtr& rHandler );
    void removeSlideAnimationsEndHandler(
        const EventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when an
        XAudio node's sound stops playing.

        Note that <em>all</em> registered handlers will be
        called when the audio stops. This is in contrast to
        the mouse events below.

        @param rHandler
        Handler to call when the audio stops
    */
    void addAudioStoppedHandler(
        const AnimationEventHandlerSharedPtr& rHandler );
    void removeAudioStoppedHandler(
        const AnimationEventHandlerSharedPtr& rHandler );

    /** Register an event handler that will be called when an
        XCommand node's with the command STOPAUDIO is activated.

        Note that <em>all</em> registered handlers will be
        called when the audio stops. This is in contrast to
        the mouse events below.

        @param rHandler
        Handler to call when command is activated
    */
    void addCommandStopAudioHandler(
        const AnimationEventHandlerSharedPtr& rHandler );
    void removeCommandStopAudioHandler(
        const AnimationEventHandlerSharedPtr& rHandler );

    /** Register a handler that is called when the show enters
        or exits pause mode.
    */
    void addPauseHandler( const PauseEventHandlerSharedPtr& rHandler );
    void removePauseHandler( const PauseEventHandlerSharedPtr& rHandler );

    /** Register a mouse handler that is called on mouse click

        For every mouse click, only one of the handlers
        registered here is called. The handlers are considered
        with decreasing priority, i.e. the handler with the
        currently highest priority will be called.

        Since the handlers can reject down and up events
        individually, handlers should expect to be called with
        non-matching down and up-press counts. If your handler
        cannot cope with that, it must have the highest
        priority of all added handlers.
    */
    void addClickHandler( const MouseEventHandlerSharedPtr& rHandler,
                          double                            nPriority );
    void removeClickHandler( const MouseEventHandlerSharedPtr& rHandler );

    /** Register a mouse handler that is called on a double
        mouse click

        For every mouse double click, only one of the handlers
        registered here is called. The handlers are considered
        with decreasing priority, i.e. the handler with the
        currently highest priority will be called.

        Since the handlers can reject down and up events
        individually, handlers should expect to be called with
        non-matching down and up-press counts. If your handler
        cannot cope with that, it must have the highest
        priority of all added handlers.
    */
    void addDoubleClickHandler( const MouseEventHandlerSharedPtr&   rHandler,
                                double                              nPriority );
    void removeDoubleClickHandler( const MouseEventHandlerSharedPtr& rHandler );

    /** Register a mouse handler that is called for mouse moves.

        For every mouse move, only one of the handlers
        registered here is called. The handlers are considered
        with decreasing priority, i.e. the handler with the
        currently highest priority will be called.
    */
    void addMouseMoveHandler( const MouseEventHandlerSharedPtr& rHandler,
                              double                            nPriority );
    void removeMouseMoveHandler( const MouseEventHandlerSharedPtr& rHandler );


    /** Registers a hyperlink click handler.

        For every hyperlink click, only one of the handlers registered
        here is called. The handlers are considered with decreasing
        priority, i.e. the handler with the currently highest priority
        will be called.

        @param rHandler
        @param nPriority
    */
    void addHyperlinkHandler( const HyperlinkHandlerSharedPtr& rHandler,
                              double                           nPriority );
    void removeHyperlinkHandler( const HyperlinkHandlerSharedPtr& rHandler );


    // External event notifications


    /** View added.

        This method adds another view, which the show is
        displayed on. On every added view, the EventMultiplexer
        registers mouse and motion event listeners.
    */
    void notifyViewAdded( const UnoViewSharedPtr& rView );

    /** View removed

        This method removes a view. Registered mouse and
        motion event listeners are revoked.
    */
    void notifyViewRemoved( const UnoViewSharedPtr& rView );

    /** View changed

        This method announces a changed view to all view
        listeners. View changes include size and transformation.

        @param rView
        View that has changed
    */
    void notifyViewChanged( const UnoViewSharedPtr& rView );

    /** View changed

        This method announces a changed view to all view
        listeners. View changes include size and transformation.

        @param xView
        View that has changed
    */
    void notifyViewChanged( const css::uno::Reference<css::presentation::XSlideShowView>& xView );

    /** All Views changed

        This method announces to all view listeners that
        <em>every</em> known view has changed. View changes include
        size and transformation.
    */
    void notifyViewsChanged();

    /** View clobbered

        This method announces that the given view has been clobbered
        by something external to the slideshow, and needs an update.

        @param xView
        View that has been clobbered
    */
    void notifyViewClobbered( const css::uno::Reference<css::presentation::XSlideShowView>& xView );

    /** New shape event listener added

        This method announces that the given listener was added for
        the specified shape.
     */
    void notifyShapeListenerAdded( const css::uno::Reference<css::presentation::XShapeEventListener>& xListener,
                                   const css::uno::Reference<css::drawing::XShape>&                   xShape );

    /** A shape event listener was removed

        This method announces that the given listener was removed for
        the specified shape.
     */
    void notifyShapeListenerRemoved( const css::uno::Reference<css::presentation::XShapeEventListener>& xListener,
                                     const css::uno::Reference<css::drawing::XShape>&                   xShape );

    /** Notify a new user paint color

        Sending this notification also implies that user paint is
        enabled. User paint denotes the feature to draw colored lines
        on top of the slide content.
    */
    void notifyUserPaintColor( RGBColor const& rUserColor );

    /** Notify a new user paint width

         Sending this notification also implies that user paint is
         enabled. .
    */
    void notifyUserPaintStrokeWidth( double rUserStrokeWidth );


    /** Notify a new user paint erase all ink mode

     Sending this notification also implies that user paint is
     enabled. User paint denotes the feature to draw colored lines
     on top of the slide content.
     */
    void notifyEraseAllInk( bool bEraseAllInk );
    void notifySwitchPenMode();
    void notifySwitchEraserMode();
    void notifyEraseInkWidth( sal_Int32 rEraseInkSize );

    /** Notify that user paint is disabled

        User paint denotes the feature to draw colored lines on top of
        the slide content.
    */
    void notifyUserPaintDisabled();

    /** Notify that the user requested the next effect.

        This requests the slideshow to display the next
        effect, or move to the next slide, if none are left.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifyNextEffect();

    /** Notify that a new slide has started

        This method is to be used from the Presentation object
        to signal that a new slide is starting now. This will
        invoke all registered slide start handlers.
    */
    void notifySlideStartEvent();

    /** Notify that a slide has ended

        This method is to be used from the Presentation object
        to signal that a slide is ending now. This will invoke
        all registered slide end handlers.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifySlideEndEvent();

    /** Notify that the given node enters its active duration.

        This method is to be used from the AnimationNode
        objects to signal that the active duration
        begins. This will invoke all registered animation
        start handlers.

        @param rNode
        Node which enters active duration.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifyAnimationStart( const AnimationNodeSharedPtr& rNode );

    /** Notify that the given node leaves its active duration.

        This method is to be used from the AnimationNode
        objects to signal that the active duration
        ends now. This will invoke all registered animation
        end handlers.

        @param rNode
        Node which leaves active duration.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifyAnimationEnd( const AnimationNodeSharedPtr& rNode );

    /** Notify that the slide animations sequence leaves its
        active duration.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifySlideAnimationsEnd();

    /** Notify that for the given node, audio output has stopped.

        This method is to be used from the AnimationNode
        objects to signal that audio playback has just
        stopped. This will invoke all registered audio
        stopped handlers.

        @param rNode
        Node for which audio has stopped.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifyAudioStopped( const AnimationNodeSharedPtr& rNode );

    /** Notify that the show has entered or exited pause mode

        This method is to be used from the Presentation object
        to signal that a slide is entering (bPauseShow=true)
        or exiting (bPauseShow=false) pause mode. This will
        invoke all registered slide end handlers.
    */
    void notifyPauseMode( bool bPauseShow );

    /** Notify that all audio has to be stopped.

        This method is used by XCommand nodes and all sound
        playing nodes should listen for this command and
        stop theire sounds when its fired.

        @return true, if this event was processed by
        anybody. If false is returned, no handler processed
        this event (and probably, nothing will happen at all)
    */
    bool notifyCommandStopAudio( const AnimationNodeSharedPtr& rNode );

    /** Notifies that a hyperlink has been clicked.
    */
    void notifyHyperlinkClicked( OUString const& hyperLink );

private:
    std::unique_ptr<EventMultiplexerImpl> mpImpl;
};

} // namespace internal
} // namespace Presentation

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_EVENTMULTIPLEXER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
