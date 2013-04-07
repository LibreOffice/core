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

#include "formdispatchinterceptor.hxx"

#include <tools/debug.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::frame::XDispatchProviderInterception;
    using ::com::sun::star::frame::XDispatchProviderInterceptor;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::util::URL;
    using ::com::sun::star::frame::XDispatch;
    using ::com::sun::star::frame::DispatchDescriptor;
    using ::com::sun::star::frame::XDispatchProvider;
    using ::com::sun::star::lang::EventObject;

    //========================================================================
    //= DispatchInterceptionMultiplexer
    //========================================================================

    DBG_NAME(DispatchInterceptionMultiplexer)
    //------------------------------------------------------------------------
    DispatchInterceptionMultiplexer::DispatchInterceptionMultiplexer(
                const Reference< XDispatchProviderInterception >& _rxToIntercept, DispatchInterceptor* _pMaster )
        :DispatchInterceptionMultiplexer_BASE(_pMaster && _pMaster->getInterceptorMutex() ? *_pMaster->getInterceptorMutex() : m_aFallback)
        ,m_aFallback()
        ,m_pMutex( _pMaster && _pMaster->getInterceptorMutex() ? _pMaster->getInterceptorMutex() : &m_aFallback )
        ,m_xIntercepted(_rxToIntercept)
        ,m_bListening(sal_False)
        ,m_pMaster(_pMaster)
    {
        DBG_CTOR(DispatchInterceptionMultiplexer,NULL);

        ::osl::MutexGuard aGuard( *m_pMutex );
        ::comphelper::increment(m_refCount);
        if (_rxToIntercept.is())
        {
            _rxToIntercept->registerDispatchProviderInterceptor((XDispatchProviderInterceptor*)this);
            // this should make us the top-level dispatch-provider for the component, via a call to our
            // setDispatchProvider we should have got an fallback for requests we (i.e. our master) cannot fullfill
            Reference< XComponent> xInterceptedComponent(_rxToIntercept, UNO_QUERY);
            if (xInterceptedComponent.is())
            {
                xInterceptedComponent->addEventListener(this);
                m_bListening = sal_True;
            }
        }
        ::comphelper::decrement(m_refCount);
    }

    //------------------------------------------------------------------------
    DispatchInterceptionMultiplexer::~DispatchInterceptionMultiplexer()
    {
        if (!rBHelper.bDisposed)
            dispose();

        DBG_DTOR(DispatchInterceptionMultiplexer,NULL);
    }

    //------------------------------------------------------------------------------
    Reference< XDispatch > SAL_CALL DispatchInterceptionMultiplexer::queryDispatch( const URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        Reference< XDispatch> xResult;
        // ask our 'real' interceptor
        if (m_pMaster)
            xResult = m_pMaster->interceptedQueryDispatch( aURL, aTargetFrameName, nSearchFlags);

        // ask our slave provider
        if (!xResult.is() && m_xSlaveDispatcher.is())
            xResult = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

        return xResult;
    }

    //------------------------------------------------------------------------------
    Sequence< Reference< XDispatch > > SAL_CALL
    DispatchInterceptionMultiplexer::queryDispatches( const Sequence< DispatchDescriptor >& aDescripts ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        Sequence< Reference< XDispatch> > aReturn(aDescripts.getLength());
        Reference< XDispatch>* pReturn = aReturn.getArray();
        const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
        for (sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts)
        {
            *pReturn = queryDispatch(pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags);
        }
        return aReturn;
    }

    //------------------------------------------------------------------------------
    Reference< XDispatchProvider > SAL_CALL DispatchInterceptionMultiplexer::getSlaveDispatchProvider(  ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        return m_xSlaveDispatcher;
    }

    //------------------------------------------------------------------------------
    void SAL_CALL DispatchInterceptionMultiplexer::setSlaveDispatchProvider(const Reference< XDispatchProvider>& xNewDispatchProvider) throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        m_xSlaveDispatcher = xNewDispatchProvider;
    }

    //------------------------------------------------------------------------------
    Reference< XDispatchProvider> SAL_CALL DispatchInterceptionMultiplexer::getMasterDispatchProvider(void) throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        return m_xMasterDispatcher;
    }

    //------------------------------------------------------------------------------
    void SAL_CALL DispatchInterceptionMultiplexer::setMasterDispatchProvider(const Reference< XDispatchProvider>& xNewSupplier) throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        m_xMasterDispatcher = xNewSupplier;
    }

    //------------------------------------------------------------------------------
    void SAL_CALL DispatchInterceptionMultiplexer::disposing(const EventObject& Source) throw( RuntimeException )
    {
        if (m_bListening)
        {
            Reference< XDispatchProviderInterception > xIntercepted(m_xIntercepted.get(), UNO_QUERY);
            if (Source.Source == xIntercepted)
                ImplDetach();
        }
    }

    //------------------------------------------------------------------------------
    void DispatchInterceptionMultiplexer::ImplDetach()
    {
        ::osl::MutexGuard aGuard( *m_pMutex );
        OSL_ENSURE(m_bListening, "DispatchInterceptionMultiplexer::ImplDetach: invalid call!");

        // deregister ourself from the interception component
        Reference< XDispatchProviderInterception > xIntercepted(m_xIntercepted.get(), UNO_QUERY);
        if (xIntercepted.is())
            xIntercepted->releaseDispatchProviderInterceptor(static_cast<XDispatchProviderInterceptor*>(this));

    //  m_xIntercepted = Reference< XDispatchProviderInterception >();
            // Don't reset m_xIntercepted: It may be needed by our owner to check for which object we were
            // responsible. As we hold the object with a weak reference only, this should be no problem.
            // 88936 - 23.07.2001 - frank.schoenheit@sun.com
        m_pMaster = NULL;
        m_pMutex = &m_aFallback;
        m_bListening = sal_False;
    }

    //------------------------------------------------------------------------------
    void DispatchInterceptionMultiplexer::disposing()
    {
        // remove ourself as event listener from the interception component
        if (m_bListening)
        {
            Reference< XComponent> xInterceptedComponent(m_xIntercepted.get(), UNO_QUERY);
            if (xInterceptedComponent.is())
                xInterceptedComponent->removeEventListener(static_cast<XEventListener*>(this));

            // detach from the interception component
            ImplDetach();
        }
    }

//........................................................................
} // namespace svxform
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
