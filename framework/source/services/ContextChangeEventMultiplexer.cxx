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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/basemutex.hxx>

#include <algorithm>
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>

namespace cssl = css::lang;
namespace cssu = css::uno;

using namespace css;
using namespace css::uno;

namespace {

typedef ::cppu::WeakComponentImplHelper3 <
    css::ui::XContextChangeEventMultiplexer,
    css::lang::XServiceInfo,
    css::lang::XEventListener
    > ContextChangeEventMultiplexerInterfaceBase;

class ContextChangeEventMultiplexer
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public ContextChangeEventMultiplexerInterfaceBase
{
public:
    ContextChangeEventMultiplexer();
    virtual ~ContextChangeEventMultiplexer (void);

    virtual void SAL_CALL disposing (void);

    
    virtual void SAL_CALL addContextChangeEventListener (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL removeContextChangeEventListener (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL removeAllContextChangeEventListeners (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException);
    virtual void SAL_CALL broadcastContextChangeEvent (
        const css::ui::ContextChangeEventObject& rContextChangeEventObject,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException);

    
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (cssu::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService  (
        const ::rtl::OUString& rsServiceName)
        throw (cssu::RuntimeException);
    virtual cssu::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void)
        throw (cssu::RuntimeException);

    
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (cssu::RuntimeException);

private:
    typedef ::std::vector<cssu::Reference<css::ui::XContextChangeEventListener> > ListenerContainer;
    class FocusDescriptor
    {
    public:
        ListenerContainer maListeners;
        ::rtl::OUString msCurrentApplicationName;
        ::rtl::OUString msCurrentContextName;
    };
    typedef ::std::map<cssu::Reference<cssu::XInterface>, FocusDescriptor> ListenerMap;
    ListenerMap maListeners;

    /** Notify all listeners in the container that is associated with
        the given event focus.

        Typically called twice from broadcastEvent(), once for the
        given event focus and onece for NULL.
    */
    void BroadcastEventToSingleContainer (
        const css::ui::ContextChangeEventObject& rEventObject,
        const cssu::Reference<cssu::XInterface>& rxEventFocus);
    FocusDescriptor* GetFocusDescriptor (
        const cssu::Reference<cssu::XInterface>& rxEventFocus,
        const bool bCreateWhenMissing);
};

ContextChangeEventMultiplexer::ContextChangeEventMultiplexer()
    : ContextChangeEventMultiplexerInterfaceBase(m_aMutex),
      maListeners()
{
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
        
        Reference<lang::XComponent> xComponent (iContainer->first, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(this);

        
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


void SAL_CALL ContextChangeEventMultiplexer::addContextChangeEventListener (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw css::lang::IllegalArgumentException(
            "can not add an empty reference",
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
            
            
            throw cssl::IllegalArgumentException("listener added twice", static_cast<XWeak*>(this), 0);
        }
    }

    
    
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
            "can not remove an empty reference",
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

            
            
        }
    }

}

void SAL_CALL ContextChangeEventMultiplexer::removeAllContextChangeEventListeners (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(
            "can not remove an empty reference",
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

            
            
        }
    }
}

void SAL_CALL ContextChangeEventMultiplexer::broadcastContextChangeEvent (
    const css::ui::ContextChangeEventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException)
{
    
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
        
        Reference<lang::XComponent> xComponent (rxEventFocus, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);

        
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

OUString SAL_CALL ContextChangeEventMultiplexer::getImplementationName()
    throw(cssu::RuntimeException)
{
    return OUString("org.apache.openoffice.comp.framework.ContextChangeEventMultiplexer");
}

sal_Bool SAL_CALL ContextChangeEventMultiplexer::supportsService ( const ::rtl::OUString& rsServiceName)
    throw (cssu::RuntimeException)
{
    return cppu::supportsService(this, rsServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ContextChangeEventMultiplexer::getSupportedServiceNames()
    throw (cssu::RuntimeException)
{
    
    return css::uno::Sequence<OUString>();
}

void SAL_CALL ContextChangeEventMultiplexer::disposing ( const css::lang::EventObject& rEvent)
    throw (cssu::RuntimeException)
{
    ListenerMap::iterator iDescriptor (maListeners.find(rEvent.Source));

    if (iDescriptor == maListeners.end())
    {
        OSL_ASSERT(iDescriptor != maListeners.end());
        return;
    }

    

    maListeners.erase(iDescriptor);
}

struct Instance {
    explicit Instance():
        instance(static_cast<cppu::OWeakObject *>(
                    new ContextChangeEventMultiplexer()))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::Static<Instance, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_framework_ContextChangeEventMultiplexer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get().instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
