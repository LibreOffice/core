/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationControllerBroadcaster.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:29:04 $
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

#include "precompiled_sd.hxx"

#include "ConfigurationControllerBroadcaster.hxx"

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#include <tools/debug.hxx>

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
            DBG_ASSERT(false,
                "ConfigurationController: caught exception while notifying listeners");
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
                    DBG_ASSERT(false,
                        "ConfigurationController: caught exception while notifying dispose");
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

