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


#include "vcl/svapp.hxx"
#include "osl/mutex.hxx"
#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/awt/XRequestCallback.hpp"


// component helper namespace
namespace comp_AsyncCallback {

// component and service helper functions:
::rtl::OUString SAL_CALL _getImplementationName();
css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace



/// anonymous implementation namespace
namespace {

class AsyncCallback:
    public ::cppu::WeakImplHelper2<
        css::lang::XServiceInfo,
        css::awt::XRequestCallback>
{
public:
    explicit AsyncCallback(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::awt::XRequestCallback:
    virtual void SAL_CALL addCallback(const css::uno::Reference< css::awt::XCallback > & xCallback, const ::com::sun::star::uno::Any & aData) throw (css::uno::RuntimeException);

private:

    struct CallbackData
    {
        CallbackData( const css::uno::Reference< css::awt::XCallback >& rCallback, const css::uno::Any& rAny ) :
            xCallback( rCallback ), aData( rAny ) {}

        css::uno::Reference< css::awt::XCallback > xCallback;
        css::uno::Any                              aData;
    };

    DECL_STATIC_LINK( AsyncCallback, Notify_Impl, CallbackData* );

    AsyncCallback(AsyncCallback &); // not defined
    void operator =(AsyncCallback &); // not defined

    virtual ~AsyncCallback() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

AsyncCallback::AsyncCallback(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL AsyncCallback::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_AsyncCallback::_getImplementationName();
}

::sal_Bool SAL_CALL AsyncCallback::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    const css::uno::Sequence< ::rtl::OUString > serviceNames = comp_AsyncCallback::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL AsyncCallback::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_AsyncCallback::_getSupportedServiceNames();
}

// ::com::sun::star::awt::XRequestCallback:
void SAL_CALL AsyncCallback::addCallback(const css::uno::Reference< css::awt::XCallback > & xCallback, const ::com::sun::star::uno::Any & aData) throw (css::uno::RuntimeException)
{
    if ( Application::IsInMain() )
    {
        SolarMutexGuard aSolarGuard;

        CallbackData* pCallbackData = new CallbackData( xCallback, aData );
        Application::PostUserEvent( STATIC_LINK( this, AsyncCallback, Notify_Impl ), pCallbackData );
    }
}

// private asynchronous link to call reference to the callback object
IMPL_STATIC_LINK_NOINSTANCE( AsyncCallback, Notify_Impl, CallbackData*, pCallbackData )
{
    try
    {
        // Asynchronous execution
        // Check pointer and reference before!
        if ( pCallbackData && pCallbackData->xCallback.is() )
            pCallbackData->xCallback->notify( pCallbackData->aData );
    }
    catch ( css::uno::Exception& )
    {
    }

    delete pCallbackData;
    return 0;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_AsyncCallback {

OUString SAL_CALL _getImplementationName() {
    return OUString("com.sun.star.awt.comp.AsyncCallback");
}

css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s(1);
    s[0] = "com.sun.star.awt.AsyncCallback";
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new AsyncCallback(context));
}

} // closing component helper namespace

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_AsyncCallback::_create,
      &comp_AsyncCallback::_getImplementationName,
      &comp_AsyncCallback::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

void * SAL_CALL comp_AsyncCallback_component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
