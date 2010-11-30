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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is far away from completion. It has no interface
   for changes notifications etc.

 *************************************************************************/
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <ucbhelper/resultsethelper.hxx>

#include "osl/diagnose.h"

using namespace com::sun::star;

//=========================================================================
//=========================================================================
//
// ResultSetImplHelper Implementation.
//
//=========================================================================
//=========================================================================

namespace ucbhelper {

//=========================================================================
ResultSetImplHelper::ResultSetImplHelper(
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr )
: m_pDisposeEventListeners( 0 ),
  m_bStatic( sal_False ),
  m_bInitDone( sal_False ),
  m_xSMgr( rxSMgr )
{
}

//=========================================================================
ResultSetImplHelper::ResultSetImplHelper(
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
    const com::sun::star::ucb::OpenCommandArgument2& rCommand )
: m_pDisposeEventListeners( 0 ),
  m_bStatic( sal_False ),
  m_bInitDone( sal_False ),
  m_aCommand( rCommand ),
  m_xSMgr( rxSMgr )
{
}

//=========================================================================
// virtual
ResultSetImplHelper::~ResultSetImplHelper()
{
    delete m_pDisposeEventListeners;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( ResultSetImplHelper,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   lang::XComponent, /* base of XDynamicResultSet */
                   com::sun::star::ucb::XDynamicResultSet );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ResultSetImplHelper,
                      lang::XTypeProvider,
                         lang::XServiceInfo,
                      com::sun::star::ucb::XDynamicResultSet );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( ResultSetImplHelper,
                               rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                   "ResultSetImplHelper" )),
                               rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                   DYNAMICRESULTSET_SERVICE_NAME )) );

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
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

//=========================================================================
// virtual
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

//=========================================================================
// virtual
void SAL_CALL ResultSetImplHelper::removeEventListener(
        const uno::Reference< lang::XEventListener >& Listener )
    throw( uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XDynamicResultSet methods.
//
//=========================================================================

// virtual
uno::Reference< sdbc::XResultSet > SAL_CALL
ResultSetImplHelper::getStaticResultSet()
    throw( com::sun::star::ucb::ListenerAlreadySetException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw com::sun::star::ucb::ListenerAlreadySetException();

    init( sal_True );
    return m_xResultSet1;
}

//=========================================================================
// virtual
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

    //////////////////////////////////////////////////////////////////////
    // Create "welcome event" and send it to listener.
    //////////////////////////////////////////////////////////////////////

    // Note: We only have the implementation for a static result set at the
    //       moment (src590). The dynamic result sets passed to the listener
    //       are a fake. This implementation will never call "notify" at the
    //       listener to propagate any changes!!!

    init( sal_False );

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

//=========================================================================
// virtual
sal_Int16 SAL_CALL ResultSetImplHelper::getCapabilities()
    throw( uno::RuntimeException )
{
    // ! com::sun::star::ucb::ContentResultSetCapability::SORTED
    return 0;
}

//=========================================================================
// virtual
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
                = uno::Reference<
                    com::sun::star::ucb::XCachedDynamicResultSetStubFactory >(
                        m_xSMgr->createInstance(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.ucb.CachedDynamicResultSetStubFactory" )) ),
                uno::UNO_QUERY );
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

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void ResultSetImplHelper::init( sal_Bool bStatic )
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
            m_bStatic = sal_True;
        }
        else
        {
            // virtual... derived class fills m_xResultSet1 and m_xResultSet2
            initDynamic();

            OSL_ENSURE( m_xResultSet1.is(),
                        "ResultSetImplHelper::init - No 1st result set!" );
            OSL_ENSURE( m_xResultSet2.is(),
                        "ResultSetImplHelper::init - No 2nd result set!" );
            m_bStatic = sal_False;
        }
        m_bInitDone = sal_True;
    }
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
