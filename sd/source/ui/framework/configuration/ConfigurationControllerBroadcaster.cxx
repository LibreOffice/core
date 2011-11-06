/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    while (!maListenerMap.empty())
    {
        ListenerMap::iterator iMap (maListenerMap.begin());
        if (iMap == maListenerMap.end())
            break;

        // When the first vector is empty then remove it from the map.
        if (iMap->second.empty())
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

