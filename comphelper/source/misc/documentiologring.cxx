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


#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper_module.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "documentiologring.hxx"

using namespace ::com::sun::star;

namespace comphelper
{

// ----------------------------------------------------------
OSimpleLogRing::OSimpleLogRing()
: m_aMessages( SIMPLELOGRING_SIZE )
, m_bInitialized( sal_False )
, m_bFull( sal_False )
, m_nPos( 0 )
{
}

// ----------------------------------------------------------
OSimpleLogRing::~OSimpleLogRing()
{
}

// ----------------------------------------------------------
uno::Sequence< OUString > SAL_CALL OSimpleLogRing::getSupportedServiceNames_static()
{
    uno::Sequence< OUString > aResult( 1 );
    aResult[0] = getServiceName_static();
    return aResult;
}

// ----------------------------------------------------------
OUString SAL_CALL OSimpleLogRing::getImplementationName_static()
{
    return OUString( "com.sun.star.comp.logging.SimpleLogRing" );
}

// ----------------------------------------------------------
OUString SAL_CALL OSimpleLogRing::getSingletonName_static()
{
    return OUString( "com.sun.star.logging.DocumentIOLogRing" );
}

// ----------------------------------------------------------
OUString SAL_CALL OSimpleLogRing::getServiceName_static()
{
    return OUString( "com.sun.star.logging.SimpleLogRing" );
}

// ----------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OSimpleLogRing::Create( SAL_UNUSED_PARAMETER const uno::Reference< uno::XComponentContext >& )
{
    return static_cast< cppu::OWeakObject* >( new OSimpleLogRing );
}

// XSimpleLogRing
// ----------------------------------------------------------
void SAL_CALL OSimpleLogRing::logString( const OUString& aMessage ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_aMessages[m_nPos] = aMessage;
    if ( ++m_nPos >= m_aMessages.getLength() )
    {
        m_nPos = 0;
        m_bFull = sal_True;
    }

    // if used once then default initialized
    m_bInitialized = sal_True;
}

// ----------------------------------------------------------
uno::Sequence< OUString > SAL_CALL OSimpleLogRing::getCollectedLog() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nResLen = m_bFull ? m_aMessages.getLength() : m_nPos;
    sal_Int32 nStart = m_bFull ? m_nPos : 0;
    uno::Sequence< OUString > aResult( nResLen );

    for ( sal_Int32 nInd = 0; nInd < nResLen; nInd++ )
        aResult[nInd] = m_aMessages[ ( nStart + nInd ) % m_aMessages.getLength() ];

    // if used once then default initialized
    m_bInitialized = sal_True;

    return aResult;
}

// XInitialization
// ----------------------------------------------------------
void SAL_CALL OSimpleLogRing::initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    sal_Int32 nLen = 0;
    if ( aArguments.getLength() == 1 && ( aArguments[0] >>= nLen ) && nLen )
        m_aMessages.realloc( nLen );
    else
        throw lang::IllegalArgumentException(
                OUString( "Nonnull size is expected as the first argument!" ),
                uno::Reference< uno::XInterface >(),
                0 );

    m_bInitialized = sal_True;
}

// XServiceInfo
OUString SAL_CALL OSimpleLogRing::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_static();
}

::sal_Bool SAL_CALL OSimpleLogRing::supportsService( const OUString& aServiceName ) throw (uno::RuntimeException)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence< OUString > SAL_CALL OSimpleLogRing::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static();
}

} // namespace comphelper

void createRegistryInfo_OSimpleLogRing()
{
    static ::comphelper::module::OAutoRegistration< ::comphelper::OSimpleLogRing > aAutoRegistration;
    static ::comphelper::module::OSingletonRegistration< ::comphelper::OSimpleLogRing > aSingletonRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
