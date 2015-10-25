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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONCONTROLLERBROADCASTER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_CONFIGURATION_CONFIGURATIONCONTROLLERBROADCASTER_HXX

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/ConfigurationChangeEvent.hpp>

#include <unordered_map>
#include <vector>

namespace sd { namespace framework {

/** This class manages the set of XConfigurationChangeListeners and
    calls them when the ConfigurationController wants to broadcast an
    event.

    For every registered combination of listener and event type a user data
    object is stored.  This user data object is then given to the listener
    whenever it is called for an event.  With this the listener can use
    a switch statement to handle different event types.
*/
class ConfigurationControllerBroadcaster
{
public:
    /** The given controller is used as origin of thrown exceptions.
    */
    explicit ConfigurationControllerBroadcaster (
        const css::uno::Reference<
            css::drawing::framework::XConfigurationController>& rxController);

    /** Add a listener for one type of event.  When one listener is
        interested in more than one event type this method has to be called
        once for every event type.  Alternatively it can register as
        universal listener that will be called for all event types.
        @param rxListener
            A valid reference to a listener.
        @param rsEventType
            The type of event that the listener will be called for.  The
            empty string is a special value in that the listener will be
            called for all event types.
        @param rUserData
            This object is passed to the listener whenever it is called for
            the specified event type.  For different event types different
            user data objects can be provided.
        @throws IllegalArgumentException
            when an empty listener reference is given.
    */
    void AddListener(
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener,
        const OUString& rsEventType,
        const css::uno::Any& rUserData);

    /** Remove all references to the given listener.  When one listener has
        been registered for more than one type of event then it is removed
        for all of them.
        @param rxListener
            A valid reference to a listener.
        @throws IllegalArgumentException
            when an empty listener reference is given.
    */
    void RemoveListener(
        const css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener>& rxListener);

    /** Broadcast the given event to all listeners that have been registered
        for its type of event as well as all universal listeners.

        When calling a listener results in a DisposedException being thrown
        the listener is unregistered automatically.
    */
    void NotifyListeners (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent);

    /** This convenience variant of NotifyListeners create the event from
        the given arguments.
    */
    void NotifyListeners (
        const OUString& rsEventType,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const css::uno::Reference<css::drawing::framework::XResource>& rxResourceObject);

    /** Call all listeners and inform them that the
        ConfigurationController is being disposed.  When this method returns
        the list of registered listeners is empty.  Further calls to
        RemoveListener() are not necessary but do not result in an error.
    */
    void DisposeAndClear();

private:
    css::uno::Reference<
        com::sun::star::drawing::framework::XConfigurationController> mxConfigurationController;
    class ListenerDescriptor {public:
        css::uno::Reference<
            css::drawing::framework::XConfigurationChangeListener> mxListener;
        css::uno::Any maUserData;
    };
    typedef std::vector<ListenerDescriptor> ListenerList;
    typedef std::unordered_map
        <OUString,
         ListenerList,
         OUStringHash> ListenerMap;
    ListenerMap maListenerMap;

    /** Broadcast the given event to all listeners in the given list.

        When calling a listener results in a DisposedException being thrown
        the listener is unregistered automatically.
    */
    void NotifyListeners (
        const ListenerList& rList,
        const css::drawing::framework::ConfigurationChangeEvent& rEvent);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
