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
#include "precompiled_framework.hxx"

#include "services/EventMultiplexer.hxx"
#include "services.h"

using ::rtl::OUString;

#define A2S(s) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace framework {

#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.framework.EventMultiplexer"
#define SERVICE_NAME "com.sun.star.ui.ContextChangeEventMultiplexer"
#define SINGLETON_NAME "org.apache.openoffice.comp.framework.EventMultiplexer"


EventMultiplexer::EventMultiplexer (const cssu::Reference<css::uno::XComponentContext>& rxContext)
    : EventMultiplexerInterfaceBase(m_aMutex),
      maListeners()
{
    (void)rxContext;
}




EventMultiplexer::~EventMultiplexer (void)
{
    maListeners.clear();
}




// XEventMultiplexer

void SAL_CALL EventMultiplexer::addEventListener (
    const cssu::Reference<css::util::XEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw css::lang::IllegalArgumentException(A2S("can not add an empty reference"), static_cast<XWeak*>(this), 0);

    ListenerMap::iterator iListenerContainer (maListeners.find(rxEventFocus));
    if (iListenerContainer == maListeners.end())
    {
        // Create a new listener container for the event focus.
        iListenerContainer = maListeners.insert(
            ListenerMap::value_type(
                rxEventFocus,
                ListenerContainer())).first;
    }
    if (iListenerContainer != maListeners.end())
    {
        ListenerContainer& rContainer (iListenerContainer->second);
        if (::std::find(rContainer.begin(), rContainer.end(), rxListener) == rContainer.end())
            rContainer.push_back(rxListener);
        else
        {
            // The listener was added for the same event focus
            // previously.  That is an error.
            throw cssl::IllegalArgumentException(A2S("listener added twice"), static_cast<XWeak*>(this), 0);
        }
    }
}




void SAL_CALL EventMultiplexer::removeEventListener (
    const cssu::Reference<css::util::XEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(A2S("can not remove an empty reference"), static_cast<XWeak*>(this), 0);

    ListenerMap::iterator iListenerContainer (maListeners.find(rxEventFocus));
    if (iListenerContainer != maListeners.end())
    {
        ListenerContainer& rContainer (iListenerContainer->second);
        const ListenerContainer::iterator iListener (::std::find(rContainer.begin(), rContainer.end(), rxListener));
        if (iListener != rContainer.end())
            rContainer.erase(iListener);
    }

}




void SAL_CALL EventMultiplexer::removeAllEventListeners (
    const cssu::Reference<css::util::XEventListener>& rxListener)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(A2S("can not remove an empty reference"), static_cast<XWeak*>(this), 0);

    for (ListenerMap::iterator
             iContainer(maListeners.begin()),
             iEnd(maListeners.end());
         iContainer!=iEnd;
         ++iContainer)
    {
        const ListenerContainer::iterator iListener (::std::find(iContainer->second.begin(), iContainer->second.end(), rxListener));
        if (iListener != iContainer->second.end())
            iContainer->second.erase(iListener);
    }
}





void SAL_CALL EventMultiplexer::broadcastEvent (
    const cssl::EventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException)
{
    BroadcastEventToSingleContainer(rEventObject, rxEventFocus);
    if (rxEventFocus.is())
        BroadcastEventToSingleContainer(rEventObject, NULL);
}




void EventMultiplexer::BroadcastEventToSingleContainer (
    const cssl::EventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
{
    ListenerMap::iterator iListenerContainer (maListeners.find(rxEventFocus));
    if (iListenerContainer != maListeners.end())
    {
        // Create a copy of the listener container to avoid problems
        // when one of the called listeners calls add... or remove...
        ListenerContainer aContainer (iListenerContainer->second);
        for (ListenerContainer::const_iterator
                 iListener(aContainer.begin()),
                 iEnd(aContainer.end());
             iListener!=iEnd;
             ++iListener)
        {
            (*iListener)->notifyEvent(rEventObject);
        }
    }
}




// XSingleComponentFactory
cssu::Reference<cssu::XInterface> SAL_CALL EventMultiplexer::createInstanceWithContext (
    const cssu::Reference<cssu::XComponentContext>& rxContext)
    throw (cssu::Exception, cssu::RuntimeException)
{
    return cssu::Reference<cssu::XInterface>();
}




cssu::Reference<cssu::XInterface > SAL_CALL EventMultiplexer::createInstanceWithArgumentsAndContext (
    const cssu::Sequence<cssu::Any>& rArguments,
    const cssu::Reference<cssu::XComponentContext>& rxContext)
    throw (cssu::Exception, cssu::RuntimeException)
{
    return cssu::Reference<cssu::XInterface>();
}




// XServiceInfo

::rtl::OUString SAL_CALL EventMultiplexer::getImplementationName (void)
    throw(cssu::RuntimeException)
{
    return impl_getStaticImplementationName();
}





sal_Bool SAL_CALL EventMultiplexer::supportsService (
    const ::rtl::OUString& rsServiceName)
    throw (cssu::RuntimeException)
{
    return ::comphelper::findValue(static_GetSupportedServiceNames(), rsServiceName, sal_True).getLength() != 0;
}




cssu::Sequence<OUString> SAL_CALL EventMultiplexer::getSupportedServiceNames (void)
    throw (cssu::RuntimeException)
{
    return static_GetSupportedServiceNames();
}




// Local and static methods.

OUString SAL_CALL EventMultiplexer::impl_getStaticImplementationName (void)
{
    return A2S(IMPLEMENTATION_NAME);
}




cssu::Sequence<OUString> SAL_CALL EventMultiplexer::static_GetSupportedServiceNames (void)
{
    cssu::Sequence<OUString> aServiceNames (2);
    aServiceNames[0] = A2S(SERVICE_NAME);
    aServiceNames[1] = A2S(SINGLETON_NAME);
    return aServiceNames;
}




cssu::Reference<cssu::XInterface> EventMultiplexer::impl_createFactory (
    const cssu::Reference<cssl::XMultiServiceFactory>& rxServiceManager)
{
    return cppu::createSingleComponentFactory(
        EventMultiplexer::static_CreateInstance,
        EventMultiplexer::impl_getStaticImplementationName(),
        EventMultiplexer::static_GetSupportedServiceNames()
        );
}




cssu::Reference<cssu::XInterface> SAL_CALL EventMultiplexer::static_CreateInstance (
    const cssu::Reference<cssu::XComponentContext>& rxComponentContext)
    throw (cssu::Exception)
{
    EventMultiplexer* pObject = new EventMultiplexer(rxComponentContext);
    cssu::Reference<cssu::XInterface> xService (static_cast<XWeak*>(pObject), cssu::UNO_QUERY);
    return xService;
}

}  // end of namespace framework
