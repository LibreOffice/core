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


/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is far away from completion. It has no interface
   for changes notifications etc.

 *************************************************************************/
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <ucbhelper/resultsethelper.hxx>
#include <ucbhelper/getcomponentcontext.hxx>

#include "osl/diagnose.h"

using namespace com::sun::star;


// ResultSetImplHelper Implementation.


namespace ucbhelper {


ResultSetImplHelper::ResultSetImplHelper(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const css::ucb::OpenCommandArgument2& rCommand )
: m_pDisposeEventListeners( nullptr ),
  m_bStatic( false ),
  m_bInitDone( false ),
  m_aCommand( rCommand ),
  m_xContext( rxContext )
{
}


// virtual
ResultSetImplHelper::~ResultSetImplHelper()
{
}


// XInterface methods.
void SAL_CALL ResultSetImplHelper::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ResultSetImplHelper::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ResultSetImplHelper::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< lang::XTypeProvider* >(this)),
                                               (static_cast< lang::XServiceInfo* >(this)),
                                               (static_cast< lang::XComponent* >(this)),
                                               (static_cast< css::ucb::XDynamicResultSet* >(this))
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_3( ResultSetImplHelper,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      css::ucb::XDynamicResultSet );


// XServiceInfo methods.

OUString SAL_CALL ResultSetImplHelper::getImplementationName()
{
    return OUString( "ResultSetImplHelper" );
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
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
}


// virtual
void SAL_CALL ResultSetImplHelper::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners.reset(new cppu::OInterfaceContainerHelper( m_aMutex ));

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL ResultSetImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


// XDynamicResultSet methods.


// virtual
uno::Reference< sdbc::XResultSet > SAL_CALL
ResultSetImplHelper::getStaticResultSet()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw css::ucb::ListenerAlreadySetException();

    init( true );
    return m_xResultSet1;
}


// virtual
void SAL_CALL ResultSetImplHelper::setListener(
        const uno::Reference< css::ucb::XDynamicResultSetListener >& Listener )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

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
    aGuard.clear();

    Listener->notify(
        css::ucb::ListEvent(
            static_cast< cppu::OWeakObject * >( this ), aActions ) );
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
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_bInitDone )
    {
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
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
