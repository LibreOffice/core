/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef SVX_FORMDISPATCHINTERCEPTOR_HXX
#define SVX_FORMDISPATCHINTERCEPTOR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
/** === end UNO includes === **/

#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= DispatchInterceptor
    //====================================================================
    class DispatchInterceptor
    {
    public:
        DispatchInterceptor() { }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> interceptedQueryDispatch(
            const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException ) = 0;

        virtual ::osl::Mutex* getInterceptorMutex() = 0;
    };

    //====================================================================
    //=
    //====================================================================
    typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::frame::XDispatchProviderInterceptor
                                            ,   ::com::sun::star::lang::XEventListener
                                            >   DispatchInterceptionMultiplexer_BASE;

    class DispatchInterceptionMultiplexer : public DispatchInterceptionMultiplexer_BASE
    {
        ::osl::Mutex    m_aFallback;
        ::osl::Mutex*   m_pMutex;

        // the component which's dispatches we're intercepting
        ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProviderInterception >
                        m_xIntercepted;
        sal_Bool        m_bListening;

        // the real interceptor
        DispatchInterceptor*            m_pMaster;

        // chaining
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xSlaveDispatcher;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider>           m_xMasterDispatcher;

        virtual ~DispatchInterceptionMultiplexer();

    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception> getIntercepted() const { return m_xIntercepted; }

    public:
        DispatchInterceptionMultiplexer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception>& _rToIntercept,
            DispatchInterceptor* _pMaster
        );

        // UNO
        DECLARE_UNO3_DEFAULTS(DispatchInterceptionMultiplexer, DispatchInterceptionMultiplexer_BASE);

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::frame::XDispatchProviderInterceptor
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        void ImplDetach();
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FORMDISPATCHINTERCEPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
