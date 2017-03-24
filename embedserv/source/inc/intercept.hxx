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
#include <comphelper/interfacecontainer2.hxx>
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
        bool bLink );

    ~Interceptor() override;

    void DisconnectDocHolder();

    void generateFeatureStateEvent();

    // overwritten to release the statuslistener.


    // XComponent
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL
    addEventListener(
        const css::uno::Reference< css::lang::XEventListener >& xListener );

    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener );

    /// @throws css::uno::RuntimeException
    void SAL_CALL
    dispose();


    //XDispatch
    virtual void SAL_CALL
    dispatch(
        const css::util::URL& URL,
        const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;

    virtual void SAL_CALL
    addStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL ) override;

    virtual void SAL_CALL
    removeStatusListener(
        const css::uno::Reference< css::frame::XStatusListener >& Control,
        const css::util::URL& URL ) override;

    //XInterceptorInfo
    virtual css::uno::Sequence< OUString >
    SAL_CALL getInterceptedURLs(  ) override;


    //XDispatchProvider ( inherited by XDispatchProviderInterceptor )
    virtual css::uno::Reference<
    css::frame::XDispatch > SAL_CALL
    queryDispatch(
        const css::util::URL& URL,
        const OUString& TargetFrameName,
        sal_Int32 SearchFlags ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const css::uno::Sequence<
        css::frame::DispatchDescriptor >& Requests ) override;


    //XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getSlaveDispatchProvider(  ) override;

    virtual void SAL_CALL
    setSlaveDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewDispatchProvider ) override;

    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL
    getMasterDispatchProvider(  ) override;

    virtual void SAL_CALL
    setMasterDispatchProvider(
        const css::uno::Reference< css::frame::XDispatchProvider >& NewSupplier ) override;


private:

    osl::Mutex   m_aMutex;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOleAccess;

    css::uno::WeakReference< css::uno::XInterface > m_xDocHLocker;
    DocumentHolder*       m_pDocH;

    css::uno::Reference< css::frame::XDispatchProvider > m_xSlaveDispatchProvider;

    css::uno::Reference< css::frame::XDispatchProvider > m_xMasterDispatchProvider;

    static css::uno::Sequence< OUString > m_aInterceptedURL;

    comphelper::OInterfaceContainerHelper2*    m_pDisposeEventListeners;
    StatusChangeListenerContainer*      m_pStatCL;

    bool m_bLink;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
