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
#ifndef __FRAMEWORK_SERVICES_CONTEXT_CHANGE_EVENT_MULTIPLEXER_HXX_
#define __FRAMEWORK_SERVICES_CONTEXT_CHANGE_EVENT_MULTIPLEXER_HXX_

#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>

#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/basemutex.hxx>

#include "macros/xserviceinfo.hxx"

#include <map>
#include <boost/noncopyable.hpp>


namespace
{
    typedef ::cppu::WeakComponentImplHelper4 <
        css::ui::XContextChangeEventMultiplexer,
        css::lang::XSingleComponentFactory,
        css::lang::XServiceInfo,
        css::lang::XEventListener
        > ContextChangeEventMultiplexerInterfaceBase;
}


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
namespace cssl = ::com::sun::star::lang;

namespace framework {

class ContextChangeEventMultiplexer
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public ContextChangeEventMultiplexerInterfaceBase
{
public:
    ContextChangeEventMultiplexer(const cssu::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~ContextChangeEventMultiplexer (void);

    virtual void SAL_CALL disposing (void);

    // XContextChangeEventMultiplexer
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

    // XSingleComponentFactory
    virtual cssu::Reference<cssu::XInterface> SAL_CALL createInstanceWithContext (
        const cssu::Reference<cssu::XComponentContext>& rxContext)
        throw (cssu::Exception, cssu::RuntimeException);
    virtual cssu::Reference<cssu::XInterface > SAL_CALL createInstanceWithArgumentsAndContext (
        const cssu::Sequence<cssu::Any>& rArguments,
        const cssu::Reference<cssu::XComponentContext>& rxContext)
        throw (cssu::Exception, cssu::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (cssu::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService  (
        const ::rtl::OUString& rsServiceName)
        throw (cssu::RuntimeException);
    virtual cssu::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames (void)
        throw (cssu::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (cssu::RuntimeException);

    static ::rtl::OUString SAL_CALL impl_getStaticImplementationName (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL impl_createFactory (
        const cssu::Reference<cssl::XMultiServiceFactory>& xServiceManager);

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

    static cssu::Sequence< ::rtl::OUString > SAL_CALL static_GetSupportedServiceNames (void);
    static cssu::Reference<cssu::XInterface> SAL_CALL static_CreateInstance (
        const cssu::Reference<cssu::XComponentContext>& rxComponentContext)
        throw (cssu::Exception);
};

} // end of namespace framework

#endif

