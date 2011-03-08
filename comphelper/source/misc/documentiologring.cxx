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
#include "precompiled_comphelper.hxx"

#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper_module.hxx>

#include "documentiologring.hxx"

using namespace ::com::sun::star;

namespace comphelper
{

// ----------------------------------------------------------
OSimpleLogRing::OSimpleLogRing( const uno::Reference< uno::XComponentContext >& /*xContext*/ )
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
uno::Sequence< ::rtl::OUString > SAL_CALL OSimpleLogRing::getSupportedServiceNames_static()
{
    uno::Sequence< rtl::OUString > aResult( 1 );
    aResult[0] = getServiceName_static();
    return aResult;
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OSimpleLogRing::getImplementationName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.logging.SimpleLogRing" ) );
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OSimpleLogRing::getSingletonName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.DocumentIOLogRing" ) );
}

// ----------------------------------------------------------
::rtl::OUString SAL_CALL OSimpleLogRing::getServiceName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.logging.SimpleLogRing" ) );
}

// ----------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OSimpleLogRing::Create( const uno::Reference< uno::XComponentContext >& rxContext )
{
    return static_cast< cppu::OWeakObject* >( new OSimpleLogRing( rxContext ) );
}

// XSimpleLogRing
// ----------------------------------------------------------
void SAL_CALL OSimpleLogRing::logString( const ::rtl::OUString& aMessage ) throw (uno::RuntimeException)
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
uno::Sequence< ::rtl::OUString > SAL_CALL OSimpleLogRing::getCollectedLog() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nResLen = m_bFull ? m_aMessages.getLength() : m_nPos;
    sal_Int32 nStart = m_bFull ? m_nPos : 0;
    uno::Sequence< ::rtl::OUString > aResult( nResLen );

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
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Nonnull size is expected as the first argument!" ) ),
                uno::Reference< uno::XInterface >(),
                0 );

    m_bInitialized = sal_True;
}

// XServiceInfo
// ----------------------------------------------------------
::rtl::OUString SAL_CALL OSimpleLogRing::getImplementationName() throw (uno::RuntimeException)
{
    return getImplementationName_static();
}

// ----------------------------------------------------------
::sal_Bool SAL_CALL OSimpleLogRing::supportsService( const ::rtl::OUString& aServiceName ) throw (uno::RuntimeException)
{
    const uno::Sequence< rtl::OUString > & aSupportedNames = getSupportedServiceNames_static();
    for ( sal_Int32 nInd = 0; nInd < aSupportedNames.getLength(); nInd++ )
    {
        if ( aSupportedNames[ nInd ].equals( aServiceName ) )
            return sal_True;
    }

    return sal_False;
}

// ----------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OSimpleLogRing::getSupportedServiceNames() throw (uno::RuntimeException)
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
