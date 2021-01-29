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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNODISPATCH_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNODISPATCH_HXX

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <vcl/svapp.hxx>

class SwView;

class SwXDispatchProviderInterceptor final : public cppu::WeakImplHelper
<
    css::frame::XDispatchProviderInterceptor,
    css::lang::XEventListener,
    css::lang::XUnoTunnel,
    css::frame::XInterceptorInfo
>
{
    class DispatchMutexLock_Impl
    {
        //::osl::MutexGuard   aGuard; #102295# solar mutex has to be used currently
        SolarMutexGuard aGuard;
    public:
        DispatchMutexLock_Impl();
        ~DispatchMutexLock_Impl();
    };
    friend class DispatchMutexLock_Impl;

//    ::osl::Mutex                     m_aMutex;#102295# solar mutex has to be used currently

    // the component which's dispatches we're intercepting
    css::uno::Reference< css::frame::XDispatchProviderInterception>   m_xIntercepted;

    // chaining
    css::uno::Reference< css::frame::XDispatchProvider>           m_xSlaveDispatcher;
    css::uno::Reference< css::frame::XDispatchProvider>           m_xMasterDispatcher;

    css::uno::Reference< css::frame::XDispatch>                   m_xDispatch;

    SwView* m_pView;

public:
    SwXDispatchProviderInterceptor(SwView& rView);
    virtual ~SwXDispatchProviderInterceptor() override;

    //XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) override;

    //XDispatchProviderInterceptor
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) override;
    virtual void SAL_CALL setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewDispatchProvider ) override;
    virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) override;
    virtual void SAL_CALL setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSupplier ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    //XUnoTunnel
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // XInterceptorInfo
    virtual css::uno::Sequence<OUString> SAL_CALL getInterceptedURLs() override;

    // view destroyed
    void    Invalidate();
};

struct StatusStruct_Impl
{
    css::uno::Reference< css::frame::XStatusListener> xListener;
    css::util::URL                                    aURL;
};
class SwXDispatch final : public cppu::WeakImplHelper
<
    css::frame::XDispatch,
    css::view::XSelectionChangeListener
>
{
    SwView*             m_pView;
    std::vector< StatusStruct_Impl > m_aStatusListenerVector;
    bool            m_bOldEnable;
    bool            m_bListenerAdded;
public:
    SwXDispatch(SwView& rView);
    virtual ~SwXDispatch() override;

    virtual void SAL_CALL dispatch( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual void SAL_CALL addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;
    virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xControl, const css::util::URL& aURL ) override;

    //XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) override;

    //XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    static const char* GetDBChangeURL();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
