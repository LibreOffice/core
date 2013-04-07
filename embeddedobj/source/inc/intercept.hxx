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

#ifndef _INTERCEPT_HXX_
#define _INTERCEPT_HXX_

#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/frame/XDispatch.hpp>


class StatusChangeListenerContainer;
class DocumentHolder;

class Interceptor : public ::cppu::WeakImplHelper3< ::com::sun::star::frame::XDispatchProviderInterceptor,
                                                       ::com::sun::star::frame::XInterceptorInfo,
                                                       ::com::sun::star::frame::XDispatch>
{
public:

    Interceptor( DocumentHolder* pDocHolder );
    ~Interceptor();

    void DisconnectDocHolder();
    // overwritten to release the statuslistner.

    // XComponent
    virtual void SAL_CALL
    addEventListener(
        const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& xListener )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference< com::sun::star::lang::XEventListener >& aListener )
        throw( com::sun::star::uno::RuntimeException );

    //XDispatch
    virtual void SAL_CALL
    dispatch(
        const ::com::sun::star::util::URL& URL,
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::beans::PropertyValue >& Arguments )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    addStatusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    removeStatusListener(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XStatusListener >& Control,
        const ::com::sun::star::util::URL& URL )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    //XInterceptorInfo
    virtual ::com::sun::star::uno::Sequence< OUString >
    SAL_CALL getInterceptedURLs(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    //XDispatchProvider ( inherited by XDispatchProviderInterceptor )
    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatch > SAL_CALL
    queryDispatch(
        const ::com::sun::star::util::URL& URL,
        const OUString& TargetFrameName,
        sal_Int32 SearchFlags )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual ::com::sun::star::uno::Sequence<
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatch > > SAL_CALL
    queryDispatches(
        const ::com::sun::star::uno::Sequence<
        ::com::sun::star::frame::DispatchDescriptor >& Requests )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


    //XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    getSlaveDispatchProvider(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setSlaveDispatchProvider(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider >& NewDispatchProvider )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    getMasterDispatchProvider(  )
        throw (
            ::com::sun::star::uno::RuntimeException
        );

    virtual void SAL_CALL
    setMasterDispatchProvider(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDispatchProvider >& NewSupplier )
        throw (
            ::com::sun::star::uno::RuntimeException
        );


private:

    osl::Mutex   m_aMutex;

    DocumentHolder*   m_pDocHolder;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xSlaveDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xMasterDispatchProvider;

    static ::com::sun::star::uno::Sequence< OUString > m_aInterceptedURL;

    cppu::OInterfaceContainerHelper*    m_pDisposeEventListeners;
    StatusChangeListenerContainer*    m_pStatCL;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
