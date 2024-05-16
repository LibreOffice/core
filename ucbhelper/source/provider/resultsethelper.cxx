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


/* TODO
 - This implementation is far away from completion. It has no interface
   for changes notifications etc.
 */

#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/resultsethelper.hxx>
#include <utility>

#include <osl/diagnose.h>

using namespace com::sun::star;


// ResultSetImplHelper Implementation.


namespace ucbhelper {


ResultSetImplHelper::ResultSetImplHelper(
    uno::Reference< uno::XComponentContext > xContext,
    css::ucb::OpenCommandArgument2 aCommand )
: m_bStatic( false ),
  m_bInitDone( false ),
  m_aCommand(std::move( aCommand )),
  m_xContext(std::move( xContext ))
{
}


// virtual
ResultSetImplHelper::~ResultSetImplHelper()
{
}


// XServiceInfo methods.

OUString SAL_CALL ResultSetImplHelper::getImplementationName()
{
    return u"ResultSetImplHelper"_ustr;
}

sal_Bool SAL_CALL ResultSetImplHelper::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL ResultSetImplHelper::getSupportedServiceNames()
{
    return { DYNAMICRESULTSET_SERVICE_NAME };
}

// XComponent methods.


// virtual
void SAL_CALL ResultSetImplHelper::dispose()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_aDisposeEventListeners.getLength(aGuard) )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_aDisposeEventListeners.disposeAndClear( aGuard, aEvt );
    }
}


// virtual
void SAL_CALL ResultSetImplHelper::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.addInterface( aGuard, Listener );
}


// virtual
void SAL_CALL ResultSetImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    m_aDisposeEventListeners.removeInterface( aGuard, Listener );
}


// XDynamicResultSet methods.


// virtual
uno::Reference< sdbc::XResultSet > SAL_CALL
ResultSetImplHelper::getStaticResultSet()
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw css::ucb::ListenerAlreadySetException();

    init( true );
    return m_xResultSet1;
}


// virtual
void SAL_CALL ResultSetImplHelper::setListener(
        const uno::Reference< css::ucb::XDynamicResultSetListener >& Listener )
{
    std::unique_lock aGuard( m_aMutex );

    if ( m_bStatic || m_xListener.is() )
        throw css::ucb::ListenerAlreadySetException();

    m_xListener = Listener;


    // Create "welcome event" and send it to listener.


    // Note: We only have the implementation for a static result set at the
    //       moment (src590). The dynamic result sets passed to the listener
    //       are a fake. This implementation will never call "notify" at the
    //       listener to propagate any changes!!!

    init( false );

    uno::Any aInfo;
    aInfo <<= css::ucb::WelcomeDynamicResultSetStruct(
        m_xResultSet1 /* "old" */,
        m_xResultSet2 /* "new" */ );

    uno::Sequence< css::ucb::ListAction > aActions {
         css::ucb::ListAction(
            0, // Position; not used
            0, // Count; not used
            css::ucb::ListActionType::WELCOME,
            aInfo ) };
    aGuard.unlock();

    Listener->notify(
        css::ucb::ListEvent(
            getXWeak(), aActions ) );
}


// virtual
sal_Int16 SAL_CALL ResultSetImplHelper::getCapabilities()
{
    // ! css::ucb::ContentResultSetCapability::SORTED
    return 0;
}


// virtual
void SAL_CALL ResultSetImplHelper::connectToCache(
        const uno::Reference< css::ucb::XDynamicResultSet > & xCache )
{
    if ( m_xListener.is() )
        throw css::ucb::ListenerAlreadySetException();

    if ( m_bStatic )
        throw css::ucb::ListenerAlreadySetException();

    uno::Reference< css::ucb::XSourceInitialization > xTarget( xCache, uno::UNO_QUERY );
    if ( xTarget.is() )
    {
        uno::Reference< css::ucb::XCachedDynamicResultSetStubFactory >  xStubFactory;
        try
        {
            xStubFactory
                = css::ucb::CachedDynamicResultSetStubFactory::create(
                      m_xContext );
        }
        catch ( uno::Exception const & )
        {
        }

        if ( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                                  this, xCache, m_aCommand.SortingInfo, nullptr );
            return;
        }
    }
    throw css::ucb::ServiceNotFoundException();
}


// Non-interface methods.


void ResultSetImplHelper::init( bool bStatic )
{
    if ( m_bInitDone )
        return;

    if ( bStatic )
    {
        // virtual... derived class fills m_xResultSet1
        initStatic();

        OSL_ENSURE( m_xResultSet1.is(),
                    "ResultSetImplHelper::init - No 1st result set!" );
        m_bStatic = true;
    }
    else
    {
        // virtual... derived class fills m_xResultSet1 and m_xResultSet2
        initDynamic();

        OSL_ENSURE( m_xResultSet1.is(),
                    "ResultSetImplHelper::init - No 1st result set!" );
        OSL_ENSURE( m_xResultSet2.is(),
                    "ResultSetImplHelper::init - No 2nd result set!" );
        m_bStatic = false;
    }
    m_bInitDone = true;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
