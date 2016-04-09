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

#include <helper/mischelper.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/basemutex.hxx>

#include <algorithm>
#include <map>
#include <vector>

namespace cssl = css::lang;
namespace cssu = css::uno;

using namespace css;
using namespace css::uno;

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::ui::XContextChangeEventMultiplexer,
    css::lang::XServiceInfo,
    css::lang::XEventListener
    > ContextChangeEventMultiplexerInterfaceBase;

class ContextChangeEventMultiplexer
    : private ::cppu::BaseMutex,
      public ContextChangeEventMultiplexerInterfaceBase
{
public:
    ContextChangeEventMultiplexer();
    virtual ~ContextChangeEventMultiplexer();
    ContextChangeEventMultiplexer(const ContextChangeEventMultiplexer&) = delete;
    ContextChangeEventMultiplexer& operator=(const ContextChangeEventMultiplexer&) = delete;

    virtual void SAL_CALL disposing() override;

    // XContextChangeEventMultiplexer
    virtual void SAL_CALL addContextChangeEventListener (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL removeContextChangeEventListener (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL removeAllContextChangeEventListeners (
        const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener)
        throw(cssu::RuntimeException, cssl::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL broadcastContextChangeEvent (
        const css::ui::ContextChangeEventObject& rContextChangeEventObject,
        const cssu::Reference<cssu::XInterface>& rxEventFocus)
        throw(cssu::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (cssu::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService  (
        const ::rtl::OUString& rsServiceName)
        throw (cssu::RuntimeException, std::exception) override;
    virtual cssu::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames()
        throw (cssu::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (cssu::RuntimeException, std::exception) override;

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
        given event focus and once for NULL.
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

ContextChangeEventMultiplexer::~ContextChangeEventMultiplexer()
{
}

void SAL_CALL ContextChangeEventMultiplexer::disposing()
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
    throw(cssu::RuntimeException,cssl::IllegalArgumentException, std::exception)
{
    if ( ! rxListener.is())
        throw css::lang::IllegalArgumentException(
            "can not add an empty reference",
            static_cast<XWeak*>(this),
            0);

    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, true);
    if (pFocusDescriptor != nullptr)
    {
        ListenerContainer& rContainer (pFocusDescriptor->maListeners);
        if (::std::find(rContainer.begin(), rContainer.end(), rxListener) == rContainer.end())
            rContainer.push_back(rxListener);
        else
        {
            // The listener was added for the same event focus
            // previously.  That is an error.
            throw cssl::IllegalArgumentException("listener added twice", static_cast<XWeak*>(this), 0);
        }
    }

    // Send out an initial event that informs the new listener about
    // the current context.
    if (rxEventFocus.is() && pFocusDescriptor!=nullptr)
    {
        css::ui::ContextChangeEventObject aEvent (
            nullptr,
            pFocusDescriptor->msCurrentApplicationName,
            pFocusDescriptor->msCurrentContextName);
        rxListener->notifyContextChangeEvent(aEvent);
    }
}

void SAL_CALL ContextChangeEventMultiplexer::removeContextChangeEventListener (
    const cssu::Reference<css::ui::XContextChangeEventListener>& rxListener,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException,cssl::IllegalArgumentException, std::exception)
{
    if ( ! rxListener.is())
        throw cssl::IllegalArgumentException(
            "can not remove an empty reference",
            static_cast<XWeak*>(this), 0);

    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, false);
    if (pFocusDescriptor != nullptr)
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
    throw(cssu::RuntimeException,cssl::IllegalArgumentException, std::exception)
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

            // We hold on to the focus descriptor even when the last listener has been removed.
            // This allows us to keep track of the current context and send it to new listeners.
        }
    }
}

void SAL_CALL ContextChangeEventMultiplexer::broadcastContextChangeEvent (
    const css::ui::ContextChangeEventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
    throw(cssu::RuntimeException, std::exception)
{
    // Remember the current context.
    if (rxEventFocus.is())
    {
        FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, true);
        if (pFocusDescriptor != nullptr)
        {
            pFocusDescriptor->msCurrentApplicationName = rEventObject.ApplicationName;
            pFocusDescriptor->msCurrentContextName = rEventObject.ContextName;
        }
    }

    BroadcastEventToSingleContainer(rEventObject, rxEventFocus);
    if (rxEventFocus.is())
        BroadcastEventToSingleContainer(rEventObject, nullptr);
}

void ContextChangeEventMultiplexer::BroadcastEventToSingleContainer (
    const css::ui::ContextChangeEventObject& rEventObject,
    const cssu::Reference<cssu::XInterface>& rxEventFocus)
{
    FocusDescriptor* pFocusDescriptor = GetFocusDescriptor(rxEventFocus, false);
    if (pFocusDescriptor != nullptr)
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
        return nullptr;
}

OUString SAL_CALL ContextChangeEventMultiplexer::getImplementationName()
    throw(cssu::RuntimeException, std::exception)
{
    return OUString("org.apache.openoffice.comp.framework.ContextChangeEventMultiplexer");
}

sal_Bool SAL_CALL ContextChangeEventMultiplexer::supportsService ( const ::rtl::OUString& rsServiceName)
    throw (cssu::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rsServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ContextChangeEventMultiplexer::getSupportedServiceNames()
    throw (cssu::RuntimeException, std::exception)
{
    // it's a singleton, not a service
    return css::uno::Sequence<OUString>();
}

void SAL_CALL ContextChangeEventMultiplexer::disposing ( const css::lang::EventObject& rEvent)
    throw (cssu::RuntimeException, std::exception)
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

namespace framework {

// right now we assume there's one matching listener
uno::Reference<ui::XContextChangeEventListener> GetFirstListenerWith_ImplImpl(
    uno::Reference<uno::XInterface> const& xEventFocus,
    std::function<bool (uno::Reference<ui::XContextChangeEventListener> const&)> const& rPredicate)
{
    assert(xEventFocus.is()); // in current usage it's a bug if the XController is null here
    uno::Reference<ui::XContextChangeEventListener> xRet;

    ContextChangeEventMultiplexer *const pMultiplexer(
        dynamic_cast<ContextChangeEventMultiplexer *>(Singleton::get().instance.get()));
    assert(pMultiplexer);

    ContextChangeEventMultiplexer::FocusDescriptor const*const pFocusDescriptor(
        pMultiplexer->GetFocusDescriptor(xEventFocus, false));
    if (!pFocusDescriptor)
        return xRet;

    for (auto & xListener : pFocusDescriptor->maListeners)
    {
        if (rPredicate(xListener))
        {
            assert(!xRet.is()); // generalize this if it is used for more than 1:1 mapping?
            xRet = xListener;
        }
    }
    return xRet;
}

namespace {

struct Hook
{
    Hook() { g_pGetMultiplexerListener = &GetFirstListenerWith_ImplImpl; }
    ~Hook() { g_pGetMultiplexerListener = nullptr; }
};

static Hook g_hook;

}

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
