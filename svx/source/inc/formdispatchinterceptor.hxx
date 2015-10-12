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

#ifndef INCLUDED_SVX_SOURCE_INC_FORMDISPATCHINTERCEPTOR_HXX
#define INCLUDED_SVX_SOURCE_INC_FORMDISPATCHINTERCEPTOR_HXX

#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>


namespace svxform
{

    class DispatchInterceptor
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> interceptedQueryDispatch(
            const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException ) = 0;

        virtual ::osl::Mutex* getInterceptorMutex() = 0;

    protected:
        DispatchInterceptor() {}

        ~DispatchInterceptor() {}
    };


    //=

    typedef ::cppu::WeakComponentImplHelper<   ::com::sun::star::frame::XDispatchProviderInterceptor
                                            ,   ::com::sun::star::lang::XEventListener
                                            >   DispatchInterceptionMultiplexer_BASE;

    class DispatchInterceptionMultiplexer : public DispatchInterceptionMultiplexer_BASE
    {
        ::osl::Mutex    m_aFallback;
        ::osl::Mutex*   m_pMutex;

        // the component which's dispatches we're intercepting
        ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProviderInterception >
                        m_xIntercepted;
        bool        m_bListening;

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
        DECLARE_UNO3_DEFAULTS(DispatchInterceptionMultiplexer, DispatchInterceptionMultiplexer_BASE)

        // ::com::sun::star::frame::XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::frame::XDispatchProviderInterceptor
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewDispatchProvider ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSupplier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    protected:
        void ImplDetach();
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FORMDISPATCHINTERCEPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
