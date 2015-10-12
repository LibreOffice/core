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
#include <cppuhelper/implbase.hxx>
#include <list>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

class SwView;

class SwXDispatchProviderInterceptor : public cppu::WeakImplHelper
<
    ::com::sun::star::frame::XDispatchProviderInterceptor,
    ::com::sun::star::lang::XEventListener,
    ::com::sun::star::lang::XUnoTunnel
>
{
    class DispatchMutexLock_Impl
    {
        //::osl::MutexGuard   aGuard; #102295# solar mutex has to be used currently
        SolarMutexGuard aGuard;
        DispatchMutexLock_Impl();
    public:
        DispatchMutexLock_Impl(SwXDispatchProviderInterceptor&);
        ~DispatchMutexLock_Impl();
    };
    friend class DispatchMutexLock_Impl;

//    ::osl::Mutex                     m_aMutex;#102295# solar mutex has to be used currently

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>   m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xMasterDispatcher;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>                   m_xDispatch;

    SwView* m_pView;

public:
    SwXDispatchProviderInterceptor(SwView& rView);
    virtual ~SwXDispatchProviderInterceptor();

    //XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // view destroyed
    void    Invalidate();
};

struct StatusStruct_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener> xListener;
    ::com::sun::star::util::URL                                                 aURL;
};
typedef std::list< StatusStruct_Impl > StatusListenerList;
class SwXDispatch : public cppu::WeakImplHelper
<
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::view::XSelectionChangeListener
>
{
    SwView*             m_pView;
    StatusListenerList  m_aListenerList;
    bool            m_bOldEnable;
    bool            m_bListenerAdded;
public:
    SwXDispatch(SwView& rView);
    virtual ~SwXDispatch();

    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static const sal_Char* GetDBChangeURL();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
