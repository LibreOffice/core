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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_INTERCEPT_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_INTERCEPT_HXX

#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

#include <rtl/ref.hxx>
#include "embeddocaccess.hxx"


class StatusChangeListenerContainer;
class EmbedDocument_Impl;
class DocumentHolder;

class Interceptor
    : public ::cppu::WeakImplHelper<
                   css::frame::XDispatchProviderInterceptor,
                   css::frame::XInterceptorInfo,
                   css::frame::XDispatch>
{
public:

    Interceptor(
        const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess,
        DocumentHolder* pDocH,
        sal_Bool bLink );

    ~Interceptor();

    void DisconnectDocHolder();

    void generateFeatureStateEvent();

    // overwritten to release the statuslistner.


    // XComponent
    virtual void SAL_CALL
    addEventListener(
        const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
        throw( css::uno::RuntimeException );

    void SAL_CALL
    dispose() throw(css::uno::RuntimeException);



    //XDispatch
    virtual void SAL_CALL
    dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments )
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL
    addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL )
        throw (
            css::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL )
        throw (
            css::uno::RuntimeException
        );

    //XInterceptorInfo
    virtual css::uno::Sequence< OUString >
    SAL_CALL getInterceptedURLs(  )
        throw (
            css::uno::RuntimeException
        );


    //XDispatchProvider ( inherited by XDispatchProviderInterceptor )
    virtual css::uno::Reference<
    css::frame::XDispatch > SAL_CALL
    queryDispatch(
        const css::util::URL& URL,
        const OUString& TargetFrameName,
        sal_Int32 SearchFlags )
        throw (
            css::uno::RuntimeException
        );

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const css::uno::Sequence<
        css::frame::DispatchDescriptor >& Requests )
        throw (
            css::uno::RuntimeException
        );


    //XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getSlaveDispatchProvider(  )
        throw (
            css::uno::RuntimeException
        );

    virtual void SAL_CALL
    setSlaveDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewDispatchProvider )
        throw (
            css::uno::RuntimeException
        );

    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getMasterDispatchProvider(  )
        throw (
            css::uno::RuntimeException
        );

    virtual void SAL_CALL
    setMasterDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewSupplier )
        throw (
            css::uno::RuntimeException
        );


private:

    osl::Mutex   m_aMutex;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOleAccess;

    css::uno::WeakReference< css::uno::XInterface > m_xDocHLocker;
    DocumentHolder*       m_pDocH;

    css::uno::Reference< css::frame::XDispatchProvider > m_xSlaveDispatchProvider;

    css::uno::Reference< css::frame::XDispatchProvider > m_xMasterDispatchProvider;

    static css::uno::Sequence< OUString > m_aInterceptedURL;

    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    StatusChangeListenerContainer*      m_pStatCL;

    sal_Bool m_bLink;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
