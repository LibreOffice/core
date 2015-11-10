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
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <ucbhelper/resultsethelper.hxx>
#include <ucbhelper/getcomponentcontext.hxx>

#include "osl/diagnose.h"

using namespace com::sun::star;




// ResultSetImplHelper Implementation.




namespace ucbhelper {


ResultSetImplHelper::ResultSetImplHelper(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const com::sun::star::ucb::OpenCommandArgument2& rCommand )
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
    delete m_pDisposeEventListeners;
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
    throw( css::uno::RuntimeException, std::exception )
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
                      com::sun::star::ucb::XDynamicResultSet );



// XServiceInfo methods.



XSERVICEINFO_NOFACTORY_IMPL_1( ResultSetImplHelper,
                               OUString(
                                   "ResultSetImplHelper" ),
                               DYNAMICRESULTSET_SERVICE_NAME );



// XComponent methods.



// virtual
void SAL_CALL ResultSetImplHelper::dispose()
    throw( uno::RuntimeException, std::exception )
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
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}


// virtual
void SAL_CALL ResultSetImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}



// XDynamicResultSet methods.



// virtual
uno::Reference< sdbc::XResultSet > SAL_CALL
ResultSetImplHelper::getStaticResultSet()
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    init( true );
    return m_xResultSet1;
}


// virtual
void SAL_CALL ResultSetImplHelper::setListener(
        const uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >&
            Listener )
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           uno::RuntimeException, std::exception )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    if ( m_bStatic || m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    m_xListener = Listener;


    // Create "welcome event" and send it to listener.


    // Note: We only have the implementation for a static result set at the
    //       moment (src590). The dynamic result sets passed to the listener
    //       are a fake. This implementation will never call "notify" at the
    //       listener to propagate any changes!!!

    init( false );

    uno::Any aInfo;
    aInfo <<= com::sun::star::ucb::WelcomeDynamicResultSetStruct(
        m_xResultSet1 /* "old" */,
        m_xResultSet2 /* "new" */ );

    uno::Sequence< com::sun::star::ucb::ListAction > aActions( 1 );
    aActions.getArray()[ 0 ]
        = com::sun::star::ucb::ListAction(
            0, // Position; not used
            0, // Count; not used
            com::sun::star::ucb::ListActionType::WELCOME,
            aInfo );
    aGuard.clear();

    Listener->notify(
        com::sun::star::ucb::ListEvent(
            static_cast< cppu::OWeakObject * >( this ), aActions ) );
}


// virtual
sal_Int16 SAL_CALL ResultSetImplHelper::getCapabilities()
    throw( uno::RuntimeException, std::exception )
{
    // ! com::sun::star::ucb::ContentResultSetCapability::SORTED
    return 0;
}


// virtual
void SAL_CALL ResultSetImplHelper::connectToCache(
        const uno::Reference< com::sun::star::ucb::XDynamicResultSet > &
            xCache )
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           com::sun::star::ucb::AlreadyInitializedException,
           com::sun::star::ucb::ServiceNotFoundException,
           uno::RuntimeException, std::exception )
{
    if ( m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    if ( m_bStatic )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    uno::Reference< com::sun::star::ucb::XSourceInitialization >
        xTarget( xCache, uno::UNO_QUERY );
    if ( xTarget.is() )
    {
        uno::Reference<
            com::sun::star::ucb::XCachedDynamicResultSetStubFactory >
                xStubFactory;
        try
        {
            xStubFactory
                = com::sun::star::ucb::CachedDynamicResultSetStubFactory::create(
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
    throw com::sun::star::ucb::ServiceNotFoundException();
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
