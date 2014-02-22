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


#include "ConfigurationControllerBroadcaster.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd { namespace framework {

ConfigurationControllerBroadcaster::ConfigurationControllerBroadcaster (
    const Reference<XConfigurationController>& rxController)
    : mxConfigurationController(rxController),
    maListenerMap()
{
}




void ConfigurationControllerBroadcaster::AddListener(
    const Reference<XConfigurationChangeListener>& rxListener,
    const OUString& rsEventType,
    const Any& rUserData)
{
    if ( ! rxListener.is())
        throw lang::IllegalArgumentException("invalid listener",
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
        throw lang::IllegalArgumentException("invalid listener",
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
    
    
    ConfigurationChangeEvent aEvent (rEvent);

    ListenerList::const_iterator iListener;
    for (iListener=rList.begin(); iListener!=rList.end(); ++iListener)
    {
        try
        {
            aEvent.UserData = iListener->maUserData;
            iListener->mxListener->notifyConfigurationChange(aEvent);
        }
        catch (const lang::DisposedException& rException)
        {
            
            
            if (rException.Context == iListener->mxListener)
                RemoveListener(iListener->mxListener);
        }
        catch (const RuntimeException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}




void ConfigurationControllerBroadcaster::NotifyListeners (const ConfigurationChangeEvent& rEvent)
{
    
    ListenerMap::const_iterator iMap (maListenerMap.find(rEvent.Type));
    if (iMap != maListenerMap.end())
    {
        
        
        ListenerList aList (iMap->second.begin(), iMap->second.end());
        NotifyListeners(aList,rEvent);
    }

    
    iMap = maListenerMap.find(OUString());
    if (iMap != maListenerMap.end())
    {
        
        
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
    catch (const lang::DisposedException&)
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
                
                
                
                try
                {
                    RemoveListener(iMap->second.front().mxListener);
                    xListener->disposing(aEvent);
                }
                catch (const RuntimeException&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            else
            {
                
                iMap->second.erase(iMap->second.begin());
            }
        }
    }
}




} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
