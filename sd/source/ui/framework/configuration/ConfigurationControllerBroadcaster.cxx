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

#include "precompiled_sd.hxx"

#include "ConfigurationControllerBroadcaster.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using rtl::OUString;

namespace sd { namespace framework {

ConfigurationControllerBroadcaster::ConfigurationControllerBroadcaster (
    const Reference<XConfigurationController>& rxController)
    : mxConfigurationController(rxController),
    maListenerMap()
{
}




void ConfigurationControllerBroadcaster::AddListener(
    const Reference<XConfigurationChangeListener>& rxListener,
    const ::rtl::OUString& rsEventType,
    const Any& rUserData)
{
    if ( ! rxListener.is())
        throw lang::IllegalArgumentException(
            OUString::createFromAscii("invalid listener"),
            mxConfigurationController,
            0);

    if (maListenerMap.find(rsEventType) == maListenerMap.end())
        maListenerMap[rsEventType] = ListenerList();
    ListenerDescriptor aDescriptor;
    aDescriptor.mxListener = rxListener;
    aDescriptor.maUserData = rUserData;
    maListenerMap[rsEventType].push_back(aDescriptor);
}




void ConfigurationControllerBroadcaster::RemoveListener(
    const Reference<XConfigurationChangeListener>& rxListener)
{
    if ( ! rxListener.is())
        throw lang::IllegalArgumentException(
            OUString::createFromAscii("invalid listener"),
            mxConfigurationController,
            0);

    ListenerMap::iterator iMap;
    ListenerList::iterator iList;
    for (iMap=maListenerMap.begin(); iMap!=maListenerMap.end(); ++iMap)
    {
        for (iList=iMap->second.begin(); iList!=iMap->second.end(); ++iList)
        {
            if (iList->mxListener == rxListener)
            {
                iMap->second.erase(iList);
                break;
            }
        }
    }
}




void ConfigurationControllerBroadcaster::NotifyListeners (
    const ListenerList& rList,
    const ConfigurationChangeEvent& rEvent)
{
    // Create a local copy of the event in which the user data is modified
    // for every listener.
    ConfigurationChangeEvent aEvent (rEvent);

    ListenerList::const_iterator iListener;
    for (iListener=rList.begin(); iListener!=rList.end(); ++iListener)
    {
        try
        {
            aEvent.UserData = iListener->maUserData;
            iListener->mxListener->notifyConfigurationChange(aEvent);
        }
        catch (lang::DisposedException& rException)
        {
            // When the exception comes from the listener itself then
            // unregister it.
            if (rException.Context == iListener->mxListener)
                RemoveListener(iListener->mxListener);
        }
        catch(RuntimeException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}




void ConfigurationControllerBroadcaster::NotifyListeners (const ConfigurationChangeEvent& rEvent)
{
    // Notify the specialized listeners.
    ListenerMap::const_iterator iMap (maListenerMap.find(rEvent.Type));
    if (iMap != maListenerMap.end())
    {
        // Create a local list of the listeners to avoid problems with
        // concurrent changes and to be able to remove disposed listeners.
        ListenerList aList (iMap->second.begin(), iMap->second.end());
        NotifyListeners(aList,rEvent);
    }

    // Notify the universal listeners.
    iMap = maListenerMap.find(OUString());
    if (iMap != maListenerMap.end())
    {
        // Create a local list of the listeners to avoid problems with
        // concurrent changes and to be able to remove disposed listeners.
        ListenerList aList (iMap->second.begin(), iMap->second.end());
        NotifyListeners(aList,rEvent);
    }
}




void ConfigurationControllerBroadcaster::NotifyListeners (
    const OUString& rsEventType,
    const Reference<XResourceId>& rxResourceId,
    const Reference<XResource>& rxResourceObject)
{
    ConfigurationChangeEvent aEvent;
    aEvent.Type = rsEventType;
    aEvent.ResourceId = rxResourceId;
    aEvent.ResourceObject = rxResourceObject;
    try
    {
        NotifyListeners(aEvent);
    }
    catch (lang::DisposedException)
    {
    }
}





void ConfigurationControllerBroadcaster::DisposeAndClear (void)
{
    lang::EventObject aEvent;
    aEvent.Source = mxConfigurationController;
    while (maListenerMap.size() > 0)
    {
        ListenerMap::iterator iMap (maListenerMap.begin());
        if (iMap == maListenerMap.end())
            break;

        // When the first vector is empty then remove it from the map.
        if (iMap->second.size() == 0)
        {
            maListenerMap.erase(iMap);
            continue;
        }
        else
        {
            Reference<lang::XEventListener> xListener (
                iMap->second.front().mxListener, UNO_QUERY);
            if (xListener.is())
            {
                // Tell the listener that the configuration controller is
                // being disposed and remove the listener (for all event
                // types).
                try
                {
                    RemoveListener(iMap->second.front().mxListener);
                    xListener->disposing(aEvent);
                }
                catch (RuntimeException&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            else
            {
                // Remove just this reference to the listener.
                iMap->second.erase(iMap->second.begin());
            }
        }
    }
}




} } // end of namespace sd::framework

