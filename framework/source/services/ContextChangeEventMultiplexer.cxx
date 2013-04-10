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
#include "services/ContextChangeEventMultiplexer.hxx"
#include "services.h"

using ::rtl::OUString;

#define A2S(s) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

using namespace css;
using namespace cssu;

namespace framework {

#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.framework.ContextChangeEventMultiplexer"
#define SERVICE_NAME "com.sun.star.ui.ContextChangeEventMultiplexer"
#define SINGLETON_NAME "org.apache.openoffice.comp.framework.ContextChangeEventMultiplexerSigleton"


ContextChangeEventMultiplexer::ContextChangeEventMultiplexer (
    const cssu::Reference<cssu::XComponentContext>& rxContext)
    : ContextChangeEventMultiplexerInterfaceBase(m_aMutex),
      maListeners()
{
    (void)rxContext;
}




ContextChangeEventMultiplexer::~ContextChangeEventMultiplexer (void)
{
}




void SAL_CALL ContextChangeEventMultiplexer::disposing (void)
{
    ListenerMap aListeners;
    aListeners.swap(maListeners);

    cssu::Reference<cssu::XInterface> xThis (static_cast<XWeak*>(this));
    css::lang::EventObject aEvent (xThis);
    for (ListenerMap::const_iterator iContainer(aListeners.begin()), iEnd(aListeners.end());
         iContainer!=iEnd;
         ++iContainer)
    {
        // Unregister from the focus object.
        Reference<lang::XComponent> xComponent (iContainer->first, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(this);

        // Tell all listeners that we are being disposed.
        const FocusDescriptor& rFocusDescriptor (iContainer->second);
        for (ListenerContainer::const_iterator
                 iListener(rFocusDescriptor.maListeners.begin()),
                 iContainerEnd(rFocusDescriptor.maListeners.end());
             iListener!=iContainerEnd;
             ++iListener)
        {
            (*iListener)->disposing(aEvent);
        }
    }
}




// XContextChangeEventMultiplexer

void SAL_CALL ContextChangeEventMultiplexer::addContextChangeEventListener (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw css::lang::IllegalArgumentException(
            A2S("can not add an empty reference"),
            static_cast<XWeak*>(this),
            0);

    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, true);
    if (pFocusDescriptor != NULL)
    {
        ListenerContainer& rContainer (pFocusDescriptor->maListeners);
        if (::std::find(rContainer.begin(), rContainer.end(), rxListener) == rContainer.end())
            rContainer.push_back(rxListener);
        else
        {
            // The listener was added for the same event focus
            // previously.  That is an error.
            throw cssl::IllegalArgumentException(A2S("listener added twice"), static_cast<XWeak*>(this), 0);
        }
    }

    // Send out an initial event that informs the new listener about
    // the current context.
    if (rxEventFocus.is() && pFocusDescriptor!=NULL)
    {
        css::ui::ContextChangeEventObject aEvent (
            NULL,
            pFocusDescriptor->msCurrentApplicationName,
            pFocusDescriptor->msCurrentContextName);
        rxListener->notifyContextChangeEvent(aEvent);
    }
}




void SAL_CALL ContextChangeEventMultiplexer::removeContextChangeEventListener (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(
            A2S("can not remove an empty reference"),
            static_cast<XWeak*>(this), 0);

    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, false);
    if (pFocusDescriptor != NULL)
    {
        ListenerContainer& rContainer (pFocusDescriptor->maListeners);
        const ListenerContainer::iterator iListener (
            ::std::find(rContainer.begin(), rContainer.end(), rxListener));
        if (iListener != rContainer.end())
        {
            rContainer.erase(iListener);

            // We hold on to the focus descriptor even when the last listener has been removed.
            // This allows us to keep track of the current context and send it to new listeners.
        }
    }

}




void SAL_CALL ContextChangeEventMultiplexer::removeAllContextChangeEventListeners (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(
            A2S("can not remove an empty reference"),
            static_cast<XWeak*>(this), 0);

    for (ListenerMap::iterator
             iContainer(maListeners.begin()),
             iEnd(maListeners.end());
         iContainer!=iEnd;
         ++iContainer)
    {
        const ListenerContainer::iterator iListener (
            ::std::find(iContainer->second.maListeners.begin(), iContainer->second.maListeners.end(), rxListener));
        if (iListener != iContainer->second.maListeners.end())
        {
            iContainer->second.maListeners.erase(iListener);

            // We hold on to the focus descriptor even when the last listener has been removed.
            // This allows us to keep track of the current context and send it to new listeners.
        }
    }
}




void SAL_CALL ContextChangeEventMultiplexer::broadcastContextChangeEvent (
    const css::ui::ContextChangeEventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException)
{
    // Remember the current context.
    if (rxEventFocus.is())
    {
        FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, true);
        if (pFocusDescriptor != NULL)
        {
            pFocusDescriptor->msCurrentApplicationName = rEventObject.ApplicationName;
            pFocusDescriptor->msCurrentContextName = rEventObject.ContextName;
        }
    }

    BroadcastEventToSingleContainer(rEventObject, rxEventFocus);
    if (rxEventFocus.is())
        BroadcastEventToSingleContainer(rEventObject, NULL);
}




void ContextChangeEventMultiplexer::BroadcastEventToSingleContainer (
    const css::ui::ContextChangeEventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
{
    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, false);
    if (pFocusDescriptor != NULL)
    {
        // Create a copy of the listener container to avoid problems
        // when one of the called listeners calls add... or remove...
        ListenerContainer aContainer (pFocusDescriptor->maListeners);
        for (ListenerContainer::const_iterator
                 iListener(aContainer.begin()),
                 iEnd(aContainer.end());
             iListener!=iEnd;
             ++iListener)
        {
            (*iListener)->notifyContextChangeEvent(rEventObject);
        }
    }
}




ContextChangeEventMultiplexer::FocusDescriptor* ContextChangeEventMultiplexer::GetFocusDescriptor (
    const cssu::Reference<cssu::XInterface>& rxEventFocus,
    const bool bCreateWhenMissing)
{
    ListenerMap::iterator iDescriptor (maListeners.find(rxEventFocus));
    if (iDescriptor == maListeners.end() && bCreateWhenMissing)
    {
        // Listen for the focus being disposed.
        Reference<lang::XComponent> xComponent (rxEventFocus, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);

        // Create a new listener container for the event focus.
        iDescriptor = maListeners.insert(
            ListenerMap::value_type(
                rxEventFocus,
                FocusDescriptor())).first;
    }
    if (iDescriptor != maListeners.end())
        return &iDescriptor->second;
    else
        return NULL;
}




// XSingleComponentFactory

cssu::Reference<cssu::XInterface> SAL_CALL ContextChangeEventMultiplexer::createInstanceWithContext (
    const cssu::Reference<cssu::XComponentContext>& rxContext)
    throw (cssu::Exception, cssu::RuntimeException)
{
    (void)rxContext;
    return cssu::Reference<cssu::XInterface>();
}




cssu::Reference<cssu::XInterface > SAL_CALL ContextChangeEventMultiplexer::createInstanceWithArgumentsAndContext (
    const cssu::Sequence<cssu::Any>& rArguments,
    const cssu::Reference<cssu::XComponentContext>& rxContext)
    throw (cssu::Exception, cssu::RuntimeException)
{
    (void)rArguments;
    (void)rxContext;
    return cssu::Reference<cssu::XInterface>();
}




// XServiceInfo

::rtl::OUString SAL_CALL ContextChangeEventMultiplexer::getImplementationName (void)
    throw(cssu::RuntimeException)
{
    return impl_getStaticImplementationName();
}





sal_Bool SAL_CALL ContextChangeEventMultiplexer::supportsService (
    const ::rtl::OUString& rsServiceName)
    throw (cssu::RuntimeException)
{
    return ::comphelper::findValue(static_GetSupportedServiceNames(), rsServiceName, sal_True).getLength() != 0;
}




cssu::Sequence<OUString> SAL_CALL ContextChangeEventMultiplexer::getSupportedServiceNames (void)
    throw (cssu::RuntimeException)
{
    return static_GetSupportedServiceNames();
}




void SAL_CALL ContextChangeEventMultiplexer::disposing (
    const css::lang::EventObject& rEvent)
    throw (cssu::RuntimeException)
{
    ListenerMap::iterator iDescriptor (maListeners.find(rEvent.Source));

    if (iDescriptor == maListeners.end())
    {
        OSL_ASSERT(iDescriptor != maListeners.end());
        return;
    }

    // Should we notify the remaining listeners?

    maListeners.erase(iDescriptor);
}




// Local and static methods.

OUString SAL_CALL ContextChangeEventMultiplexer::impl_getStaticImplementationName (void)
{
    return A2S(IMPLEMENTATION_NAME);
}




cssu::Sequence<OUString> SAL_CALL ContextChangeEventMultiplexer::static_GetSupportedServiceNames (void)
{
    cssu::Sequence<OUString> aServiceNames (2);
    aServiceNames[0] = A2S(SERVICE_NAME);
    aServiceNames[1] = A2S(SINGLETON_NAME);
    return aServiceNames;
}




cssu::Reference<cssu::XInterface> ContextChangeEventMultiplexer::impl_createFactory (
    const cssu::Reference<cssl::XMultiServiceFactory>& rxServiceManager)
{
    (void)rxServiceManager;
    return cppu::createSingleComponentFactory(
        ContextChangeEventMultiplexer::static_CreateInstance,
        ContextChangeEventMultiplexer::impl_getStaticImplementationName(),
        ContextChangeEventMultiplexer::static_GetSupportedServiceNames()
        );
}




cssu::Reference<cssu::XInterface> SAL_CALL ContextChangeEventMultiplexer::static_CreateInstance (
    const cssu::Reference<cssu::XComponentContext>& rxComponentContext)
    throw (cssu::Exception)
{
    ContextChangeEventMultiplexer* pObject = new ContextChangeEventMultiplexer(rxComponentContext);
    cssu::Reference<cssu::XInterface> xService (static_cast<XWeak*>(pObject), cssu::UNO_QUERY);
    return xService;
}

}  // end of namespace framework
