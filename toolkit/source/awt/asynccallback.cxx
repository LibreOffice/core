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


#include "vcl/svapp.hxx"
#include "osl/mutex.hxx"
#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase2.hxx"
#include <cppuhelper/supportsservice.hxx>
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/awt/XRequestCallback.hpp"

/
namespace {

class AsyncCallback:
    public ::cppu::WeakImplHelper2<
        css::lang::XServiceInfo,
        css::awt::XRequestCallback>
{
public:
    AsyncCallback() {}

    
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    
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

    AsyncCallback(AsyncCallback &); 
    void operator =(AsyncCallback &); 

    virtual ~AsyncCallback() {}
};


OUString SAL_CALL AsyncCallback::getImplementationName() throw (css::uno::RuntimeException)
{
    return OUString("com.sun.star.awt.comp.AsyncCallback");
}

::sal_Bool SAL_CALL AsyncCallback::supportsService(OUString const & serviceName) throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL AsyncCallback::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > s(1);
    s[0] = "com.sun.star.awt.AsyncCallback";
    return s;
}


void SAL_CALL AsyncCallback::addCallback(const css::uno::Reference< css::awt::XCallback > & xCallback, const ::com::sun::star::uno::Any & aData) throw (css::uno::RuntimeException)
{
    if ( Application::IsInMain() )
    {
        SolarMutexGuard aSolarGuard;

        CallbackData* pCallbackData = new CallbackData( xCallback, aData );
        Application::PostUserEvent( STATIC_LINK( this, AsyncCallback, Notify_Impl ), pCallbackData );
    }
}


IMPL_STATIC_LINK_NOINSTANCE( AsyncCallback, Notify_Impl, CallbackData*, pCallbackData )
{
    try
    {
        
        
        if ( pCallbackData && pCallbackData->xCallback.is() )
            pCallbackData->xCallback->notify( pCallbackData->aData );
    }
    catch ( css::uno::Exception& )
    {
    }

    delete pCallbackData;
    return 0;
}

} 

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_awt_comp_AsyncCallback_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new AsyncCallback());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
