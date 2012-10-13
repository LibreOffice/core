/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright 2010, Novell Inc.
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
 * Contributor(s): Jan Holesovsky <kendy@novell.com>
 *
 ************************************************************************/
#include "unxsplash.hxx"
#include <stdio.h>
#include <osl/process.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <rtl/logfile.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#define PIPE_ARG "--splash-pipe="

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::registry;

namespace desktop
{
    UnxSplashScreen::UnxSplashScreen( const Reference< uno::XComponentContext >& xCtx )
    : m_xCtx( xCtx ),
      m_pOutFd( NULL )
{
}

UnxSplashScreen::~UnxSplashScreen()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "UnxSplashScreen::~UnxSplashScreen()\n" );
#endif

    if ( m_pOutFd )
    {
        fclose( m_pOutFd );
        m_pOutFd = NULL;
    }
}

void SAL_CALL UnxSplashScreen::start( const OUString& /*aText*/, sal_Int32 /*nRange*/ )
    throw ( RuntimeException )
{
}

void SAL_CALL UnxSplashScreen::end()
    throw ( RuntimeException )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "UnxSplashScreen::end()\n" );
#endif
    if( !m_pOutFd )
        return;

    fprintf( m_pOutFd, "end\n" );
    fflush( m_pOutFd );
}

void SAL_CALL UnxSplashScreen::reset()
    throw ( RuntimeException )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "UnxSplashScreen::reset()\n" );
#endif
    if( !m_pOutFd )
        return;

    fprintf( m_pOutFd, "restart\n" );
    fflush( m_pOutFd );
}

void SAL_CALL UnxSplashScreen::setText( const OUString& /*aText*/ )
    throw ( RuntimeException )
{
    // TODO?
}

void SAL_CALL UnxSplashScreen::setValue( sal_Int32 nValue )
    throw ( RuntimeException )
{
    if ( m_pOutFd )
    {
        fprintf( m_pOutFd, "%" SAL_PRIdINT32 "%%\n", nValue );
        fflush( m_pOutFd );
    }
}

// XInitialize
void SAL_CALL
UnxSplashScreen::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& )
    throw ( RuntimeException )
{
    for ( sal_uInt32 i = 0; i < osl_getCommandArgCount(); i++ )
    {
        rtl::OUString aArg;
        if ( osl_getCommandArg( i, &aArg.pData ) )
            break;
        if ( aArg.matchIgnoreAsciiCaseAsciiL( PIPE_ARG, sizeof( PIPE_ARG ) - 1, 0 ) )
        {
            OUString aNum = aArg.copy( sizeof( PIPE_ARG ) - 1 );
            int fd = aNum.toInt32();
            m_pOutFd = fdopen( fd, "w" );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Got argument '--splash-pipe=%d ('%s') (%p)\n",
                     fd, rtl::OUStringToOString( aNum, RTL_TEXTENCODING_UTF8 ).getStr(),
                     m_pOutFd );
#endif
        }
    }
}
}

using namespace desktop;

// get service instance...
static uno::Reference< uno::XInterface > m_xINSTANCE;

uno::Reference< uno::XInterface > SAL_CALL UnxSplash_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
{
    static osl::Mutex m_aMutex;
    if ( !m_xINSTANCE.is() )
    {
        osl::MutexGuard guard( m_aMutex );
        if ( !m_xINSTANCE.is() )
            m_xINSTANCE = (cppu::OWeakObject*) new UnxSplashScreen( xCtx );
    }

    return m_xINSTANCE;
}

OUString UnxSplash_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.comp.PipeSplashScreen" ) );
}

uno::Sequence< OUString > SAL_CALL UnxSplash_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.PipeSplashScreen" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

::cppu::ImplementationEntry aEntries[] =
{
    {
        UnxSplash_createInstance, UnxSplash_getImplementationName,
        UnxSplash_getSupportedServiceNames,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL splash_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper( pImplName, pServiceManager,
                                               pRegistryKey, aEntries );
}

} // extern "C"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
