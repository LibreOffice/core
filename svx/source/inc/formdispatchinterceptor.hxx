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
        /// @throws css::uno::RuntimeException
        virtual css::uno::Reference< css::frame::XDispatch> interceptedQueryDispatch(
            const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) = 0;

        virtual ::osl::Mutex* getInterceptorMutex() = 0;

    protected:
        DispatchInterceptor() {}

        ~DispatchInterceptor() {}
    };


    //=

    typedef ::cppu::WeakComponentImplHelper<   css::frame::XDispatchProviderInterceptor
                                            ,   css::lang::XEventListener
                                            >   DispatchInterceptionMultiplexer_BASE;

    class DispatchInterceptionMultiplexer : public DispatchInterceptionMultiplexer_BASE
    {
        ::osl::Mutex    m_aFallback;
        ::osl::Mutex*   m_pMutex;

        // the component which's dispatches we're intercepting
        css::uno::WeakReference< css::frame::XDispatchProviderInterception >
                        m_xIntercepted;
        bool            m_bListening;

        // the real interceptor
        DispatchInterceptor*            m_pMaster;

        // chaining
        css::uno::Reference< css::frame::XDispatchProvider>           m_xSlaveDispatcher;
        css::uno::Reference< css::frame::XDispatchProvider>           m_xMasterDispatcher;

        virtual ~DispatchInterceptionMultiplexer() override;

    public:
        css::uno::Reference< css::frame::XDispatchProviderInterception> getIntercepted() const { return m_xIntercepted; }

    public:
        DispatchInterceptionMultiplexer(
            const css::uno::Reference< css::frame::XDispatchProviderInterception>& _rToIntercept,
            DispatchInterceptor* _pMaster
        );

        // css::frame::XDispatchProvider
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch( const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts ) override;

        // css::frame::XDispatchProviderInterceptor
        virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getSlaveDispatchProvider(  ) override;
        virtual void SAL_CALL setSlaveDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewDispatchProvider ) override;
        virtual css::uno::Reference< css::frame::XDispatchProvider > SAL_CALL getMasterDispatchProvider(  ) override;
        virtual void SAL_CALL setMasterDispatchProvider( const css::uno::Reference< css::frame::XDispatchProvider >& xNewSupplier ) override;

        // css::lang::XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    protected:
        void ImplDetach();
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_FORMDISPATCHINTERCEPTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
