/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _UNODISPATCH_HXX
#define _UNODISPATCH_HXX

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <list>
#include <osl/mutex.hxx>

namespace binfilter {

class SwView;

class SwXDispatchProviderInterceptor : public cppu::WeakImplHelper3
<
    ::com::sun::star::frame::XDispatchProviderInterceptor,
    ::com::sun::star::lang::XEventListener,
    ::com::sun::star::lang::XUnoTunnel
>
{
    class DispatchMutexLock_Impl
    {
        //::osl::MutexGuard   aGuard; #102295# solar mutex has to be used currently
        osl::MutexGuard         aGuard;
        DispatchMutexLock_Impl();
    public:
        DispatchMutexLock_Impl(SwXDispatchProviderInterceptor&);
        ~DispatchMutexLock_Impl();
    };
    friend class DispatchMutexLock_Impl;

//    ::osl::Mutex                     m_aMutex;#102295# solar mutex has to be used currently

    // the component which's dispatches we're intercepting
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>	m_xIntercepted;

    // chaining
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>			m_xSlaveDispatcher;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>			m_xMasterDispatcher;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>                   m_xDispatch;

    SwView* m_pView;

public:
    SwXDispatchProviderInterceptor(SwView& rView);
    ~SwXDispatchProviderInterceptor();

    //XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

    //XDispatchProviderInterceptor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    //XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // view destroyed
    void    Invalidate();
};

struct StatusStruct_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener> xListener;
    ::com::sun::star::util::URL                                                 aURL;
};
typedef std::list< StatusStruct_Impl > StatusListenerList;
class SwXDispatch : public cppu::WeakImplHelper2
<
    ::com::sun::star::frame::XDispatch,
    ::com::sun::star::view::XSelectionChangeListener
>
{
    SwView*             m_pView;
    StatusListenerList  m_aListenerList;
    sal_Bool            m_bOldEnable;
    sal_Bool            m_bListenerAdded;
public:
    SwXDispatch(SwView& rView);
    ~SwXDispatch();

    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);

    //XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    //XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    static const sal_Char* GetDBChangeURL();
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
