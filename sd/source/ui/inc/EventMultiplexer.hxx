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

#ifndef INCLUDED_SD_SOURCE_UI_INC_EVENTMULTIPLEXER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_EVENTMULTIPLEXER_HXX

#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <set>
#include <memory>

namespace sd {
class ViewShellBase;
}

namespace sd { namespace tools {

class EventMultiplexerEvent
{
public:
    typedef sal_uInt32 EventId;
    /** The EventMultiplexer itself is being disposed.  Called for a live
        EventMultiplexer.  Removing a listener as response is not necessary,
        though.
    */
    static const EventId EID_DISPOSING              = 0x00000001;

    /** The selection in the center pane has changed.
    */
    static const EventId EID_EDIT_VIEW_SELECTION    = 0x00000002;

    /** The selection in the slide sorter has changed, regardless of whether
        the slide sorter is displayed in the left pane or the center pane.
    */
    static const EventId EID_SLIDE_SORTER_SELECTION = 0x00000004;

    /** The current page has changed.
    */
    static const EventId EID_CURRENT_PAGE           = 0x00000008;

    /** The current MainViewShell (the ViewShell displayed in the center
        pane) has been removed.
    */
    static const EventId EID_MAIN_VIEW_REMOVED      = 0x00000010;

    /** A new ViewShell has been made the MainViewShell.
    */
    static const EventId EID_MAIN_VIEW_ADDED        = 0x00000020;

    /** A ViewShell has been removed from one of the panes.  Note that for
        the ViewShell in the center pane bth this event type and
        EID_MAIN_VIEW_REMOVED is broadcasted.
    */
    static const EventId EID_VIEW_REMOVED           = 0x00000040;

    /** A new ViewShell is being displayed in one of the panes.  Note that
        for the ViewShell in the center pane both this event type and
        EID_MAIN_VIEW_ADDED is broadcasted.
    */
    static const EventId EID_VIEW_ADDED             = 0x00000080;

    /** The PaneManager is being destroyed.
    */
    static const EventId EID_PANE_MANAGER_DYING     = 0x00000100;

    /** Edit mode was (or is being) switched to normal mode.  Find
        EID_EDIT_MODE_MASTER below.
    */
    static const EventId EID_EDIT_MODE_NORMAL       = 0x00000200;

    /** One or more pages have been inserted into or deleted from the model.
    */
    static const EventId EID_PAGE_ORDER             = 0x00000400;

    /** Text editing in one of the shapes in the MainViewShell has started.
    */
    static const EventId EID_BEGIN_TEXT_EDIT        = 0x00000800;

    /** Text editing in one of the shapes in the MainViewShell has ended.
    */
    static const EventId EID_END_TEXT_EDIT          = 0x00001000;

    /** A UNO controller has been attached to the UNO frame.
    */
    static const EventId EID_CONTROLLER_ATTACHED    = 0x00002000;

    /** A UNO controller has been detached to the UNO frame.
    */
    static const EventId EID_CONTROLLER_DETACHED    = 0x00004000;

    /** The state of a shape has changed.  The page is available in the user data.
    */
    static const EventId EID_SHAPE_CHANGED          = 0x00008000;

    /** A shape has been inserted to a page.  The page is available in the
        user data.
    */
    static const EventId EID_SHAPE_INSERTED         = 0x00010000;

    /** A shape has been removed from a page.  The page is available in the
        user data.
    */
    static const EventId EID_SHAPE_REMOVED          = 0x00020000;

    /** A configuration update has been completed.
    */
    static const EventId EID_CONFIGURATION_UPDATED  = 0x00040000;

    /** Edit mode was (or is being) switched to master mode.
    */
    static const EventId EID_EDIT_MODE_MASTER       = 0x00080000;

    EventId meEventId;
    const void* mpUserData;

    EventMultiplexerEvent (
        EventId eEventId,
        const void* pUserData);
};

/** This convenience class makes it easy to listen to various events that
    originally are broadcasted via different channels.

    There is usually one EventMultiplexer instance per ViewShellBase().
    Call the laters GetEventMultiplexer() method to get access to that
    instance.

    When a listener is registered it can specify the events it
    wants to be informed of.  This can be done with code like the following:

    mrViewShellBase.GetEventMultiplexer().AddEventListener (
        LINK(this,MasterPagesSelector,EventMultiplexerListener),
        tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_REMOVED);
*/
class EventMultiplexer
{
public:
    /** Create new EventMultiplexer for the given ViewShellBase object.
    */
    EventMultiplexer (ViewShellBase& rBase);
    ~EventMultiplexer();

    /** Some constants that make it easier to remove a listener for all
        event types at once.
    */
    static const EventMultiplexerEvent::EventId EID_FULL_SET = 0xffffffff;
    static const EventMultiplexerEvent::EventId EID_EMPTY_SET = 0x00000000;

    /** Add an event listener that will be informed about the specified
        event types.
        @param rCallback
            The callback to call as soon as one of the event specified by
            aEventTypeSet is received by the EventMultiplexer.
        @param aEventTypeSet
            A, possibly empty, set of event types that the listener wants to
            be informed about.
    */
    void AddEventListener (
        const Link<EventMultiplexerEvent&,void>& rCallback,
        EventMultiplexerEvent::EventId aEventTypeSet);

    /** Remove an event listener for the specified event types.
        @param aEventTypeSet
            The listener will not be called anymore for any of the event
            types in this set.  Use EID_FULL_SET, the default value, to
            remove the listener for all event types it has been registered
            for.
    */
    void RemoveEventListener (
        const Link<EventMultiplexerEvent&,void>& rCallback,
        EventMultiplexerEvent::EventId aEventTypeSet = EID_FULL_SET);

    /** This method is used for out-of-line events.  An event of the
        specified type will be sent to all listeners that are registered for
        that type.
        @param eEventId
            The type of the event.
        @param pUserData
            Some data sent to the listeners along with the event.
    */
    void MultiplexEvent(
        EventMultiplexerEvent::EventId eEventId,
        void* pUserData = nullptr);

private:
    class Implementation;
    css::uno::Reference<Implementation> mpImpl;
};

} } // end of namespace ::sd::tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
