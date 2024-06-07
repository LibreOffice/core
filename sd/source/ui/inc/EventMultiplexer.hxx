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

#pragma once

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <com/sun/star/uno/XInterface.hpp>

template <typename Arg, typename Ret> class Link;

namespace sd
{
class ViewShellBase;
}

enum class EventMultiplexerEventId
{
    /** The EventMultiplexer itself is being disposed.  Called for a live
        EventMultiplexer.  Removing a listener as response is not necessary,
        though.
    */
    Disposing,

    /** The selection in the center pane has changed.
    */
    EditViewSelection,

    /** The selection in the slide sorter has changed, regardless of whether
        the slide sorter is displayed in the left pane or the center pane.
    */
    SlideSortedSelection,

    /** The current page has changed.
    */
    CurrentPageChanged,

    /** The current MainViewShell (the ViewShell displayed in the center
        pane) has been removed.
    */
    MainViewRemoved,

    /** A new ViewShell has been made the MainViewShell.
    */
    MainViewAdded,

    /** A new ViewShell is being displayed in one of the panes.  Note that
        for the ViewShell in the center pane both this event type and
        EventId::MainViewAdded is broadcasted.
    */
    ViewAdded,

    /** Edit mode was (or is being) switched to normal mode.  Find
        EventId::EditModeMaster below.
    */
    EditModeNormal,

    /** One or more pages have been inserted into or deleted from the model.
    */
    PageOrder,

    /** Text editing in one of the shapes in the MainViewShell has started.
    */
    BeginTextEdit,

    /** Text editing in one of the shapes in the MainViewShell has ended.
    */
    EndTextEdit,

    /** A UNO controller has been attached to the UNO frame.
    */
    ControllerAttached,

    /** A UNO controller has been detached to the UNO frame.
    */
    ControllerDetached,

    /** The state of a shape has changed.  The page is available in the user data.
    */
    ShapeChanged,

    /** A shape has been inserted to a page.  The page is available in the
        user data.
    */
    ShapeInserted,

    /** A shape has been removed from a page.  The page is available in the
        user data.
    */
    ShapeRemoved,

    /** A configuration update has been completed.
    */
    ConfigurationUpdated,

    /** Edit mode was (or is being) switched to master mode.
    */
    EditModeMaster,

    /** Focus shifted between views.
     */
    FocusShifted,
};

namespace sd::tools
{
class EventMultiplexerEvent
{
public:
    EventMultiplexerEventId meEventId;
    const void* mpUserData;
    css::uno::Reference<css::uno::XInterface> mxUserData;

    EventMultiplexerEvent(EventMultiplexerEventId eEventId, const void* pUserData,
                          const css::uno::Reference<css::uno::XInterface>& xUserData = {});
};

/** This convenience class makes it easy to listen to various events that
    originally are broadcasted via different channels.

    There is usually one EventMultiplexer instance per ViewShellBase().
    Call the laters GetEventMultiplexer() method to get access to that
    instance.
*/
class EventMultiplexer
{
public:
    /** Create new EventMultiplexer for the given ViewShellBase object.
    */
    EventMultiplexer(ViewShellBase& rBase);
    ~EventMultiplexer();

    /** Add an event listener that will be informed about the specified
        event types.
        @param rCallback
            The callback to call as soon as one of the event specified by
            aEventTypeSet is received by the EventMultiplexer.
    */
    void AddEventListener(const Link<EventMultiplexerEvent&, void>& rCallback);

    /** Remove an event listener for the specified event types.
    */
    void RemoveEventListener(const Link<EventMultiplexerEvent&, void>& rCallback);

    /** This method is used for out-of-line events.  An event of the
        specified type will be sent to all listeners that are registered for
        that type.
        @param eEventId
            The type of the event.
        @param pUserData
            Some data sent to the listeners along with the event.
    */
    void MultiplexEvent(EventMultiplexerEventId eEventId, void const* pUserData,
                        const css::uno::Reference<css::uno::XInterface>& xUserData = {});

private:
    class Implementation;
    rtl::Reference<Implementation> mpImpl;
};

} // end of namespace ::sd::tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
