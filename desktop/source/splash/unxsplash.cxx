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

#include "unxsplash.hxx"
#include <stdio.h>
#include <osl/process.h>
//#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/implementationentry.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#define PIPE_ARG "--splash-pipe="

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
        OUString aArg;
        if ( osl_getCommandArg( i, &aArg.pData ) )
            break;
        if ( aArg.matchIgnoreAsciiCaseAsciiL( PIPE_ARG, sizeof( PIPE_ARG ) - 1, 0 ) )
        {
            OUString aNum = aArg.copy( sizeof( PIPE_ARG ) - 1 );
            int fd = aNum.toInt32();
            m_pOutFd = fdopen( fd, "w" );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Got argument '--splash-pipe=%d ('%s') (%p)\n",
                     fd, OUStringToOString( aNum, RTL_TEXTENCODING_UTF8 ).getStr(),
                     m_pOutFd );
#endif
        }
    }
}
}

using namespace desktop;

// get service instance...
static uno::Reference< uno::XInterface > m_xINSTANCE;

uno::Reference< uno::XInterface > UnxSplash_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
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
    return OUString( "com.sun.star.office.comp.PipeSplashScreen" );
}

uno::Sequence< OUString > UnxSplash_getSupportedServiceNames() throw()
{
    const OUString aServiceName( "com.sun.star.office.PipeSplashScreen" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
