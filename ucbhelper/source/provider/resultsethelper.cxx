/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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



//

//



namespace ucbhelper {


ResultSetImplHelper::ResultSetImplHelper(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const com::sun::star::ucb::OpenCommandArgument2& rCommand )
: m_pDisposeEventListeners( 0 ),
  m_bStatic( false ),
  m_bInitDone( false ),
  m_aCommand( rCommand ),
  m_xContext( rxContext )
{
}



ResultSetImplHelper::~ResultSetImplHelper()
{
    delete m_pDisposeEventListeners;
}


//

//


XINTERFACE_IMPL_4( ResultSetImplHelper,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   lang::XComponent, /* base of XDynamicResultSet */
                   com::sun::star::ucb::XDynamicResultSet );


//

//


XTYPEPROVIDER_IMPL_3( ResultSetImplHelper,
                      lang::XTypeProvider,
                         lang::XServiceInfo,
                      com::sun::star::ucb::XDynamicResultSet );


//

//


XSERVICEINFO_NOFACTORY_IMPL_1( ResultSetImplHelper,
                               OUString(
                                   "ResultSetImplHelper" ),
                               OUString(
                                   DYNAMICRESULTSET_SERVICE_NAME ) );


//

//



void SAL_CALL ResultSetImplHelper::dispose()
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast< lang::XComponent * >( this );
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
}



void SAL_CALL ResultSetImplHelper::addEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners
            = new cppu::OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}



void SAL_CALL ResultSetImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}


//

//



uno::Reference< sdbc::XResultSet > SAL_CALL
ResultSetImplHelper::getStaticResultSet()
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    init( true );
    return m_xResultSet1;
}



void SAL_CALL ResultSetImplHelper::setListener(
        const uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >&
            Listener )
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           uno::RuntimeException )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    if ( m_bStatic || m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    m_xListener = Listener;

    
    
    

    
    
    
    

    init( false );

    uno::Any aInfo;
    aInfo <<= com::sun::star::ucb::WelcomeDynamicResultSetStruct(
        m_xResultSet1 /* "old" */,
        m_xResultSet2 /* "new" */ );

    uno::Sequence< com::sun::star::ucb::ListAction > aActions( 1 );
    aActions.getArray()[ 0 ]
        = com::sun::star::ucb::ListAction(
            0, 
            0, 
            com::sun::star::ucb::ListActionType::WELCOME,
            aInfo );
    aGuard.clear();

    Listener->notify(
        com::sun::star::ucb::ListEvent(
            static_cast< cppu::OWeakObject * >( this ), aActions ) );
}



sal_Int16 SAL_CALL ResultSetImplHelper::getCapabilities()
    throw( uno::RuntimeException )
{
    
    return 0;
}



void SAL_CALL ResultSetImplHelper::connectToCache(
        const uno::Reference< com::sun::star::ucb::XDynamicResultSet > &
            xCache )
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           com::sun::star::ucb::AlreadyInitializedException,
           com::sun::star::ucb::ServiceNotFoundException,
           uno::RuntimeException )
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
                                  this, xCache, m_aCommand.SortingInfo, 0 );
            return;
        }
    }
    throw com::sun::star::ucb::ServiceNotFoundException();
}


//

//


void ResultSetImplHelper::init( bool bStatic )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_bInitDone )
    {
        if ( bStatic )
        {
            
            initStatic();

            OSL_ENSURE( m_xResultSet1.is(),
                        "ResultSetImplHelper::init - No 1st result set!" );
            m_bStatic = true;
        }
        else
        {
            
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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
