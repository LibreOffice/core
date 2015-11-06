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
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

using namespace com::sun::star;

namespace desktop
{
    UnxSplashScreen::UnxSplashScreen( const uno::Reference< uno::XComponentContext >& xCtx )
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
    throw ( uno::RuntimeException, std::exception )
{
}

void SAL_CALL UnxSplashScreen::end()
    throw ( uno::RuntimeException, std::exception )
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
    throw ( uno::RuntimeException, std::exception )
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
    throw ( uno::RuntimeException, std::exception )
{
    // TODO?
}

void SAL_CALL UnxSplashScreen::setValue( sal_Int32 nValue )
    throw ( uno::RuntimeException, std::exception )
{
    if ( m_pOutFd )
    {
        fprintf( m_pOutFd, "%" SAL_PRIdINT32 "%%\n", nValue );
        fflush( m_pOutFd );
    }
}

// XInitialize
void SAL_CALL
UnxSplashScreen::initialize( const css::uno::Sequence< css::uno::Any>& )
    throw ( uno::RuntimeException, std::exception )
{
    for ( sal_uInt32 i = 0; i < osl_getCommandArgCount(); i++ )
    {
        OUString aArg;
        osl_getCommandArg( i, &aArg.pData );
        OUString aNum;
        if ( aArg.startsWithIgnoreAsciiCase("--splash-pipe=", &aNum) )
        {
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

OUString UnxSplashScreen::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return UnxSplash_getImplementationName();
}

sal_Bool UnxSplashScreen::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UnxSplashScreen::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return UnxSplash_getSupportedServiceNames();
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
            m_xINSTANCE = static_cast<cppu::OWeakObject*>(new UnxSplashScreen( xCtx ));
    }

    return m_xINSTANCE;
}

OUString UnxSplash_getImplementationName()
{
    return OUString( "com.sun.star.office.comp.PipeSplashScreen" );
}

uno::Sequence< OUString > UnxSplash_getSupportedServiceNames() throw()
{
    return uno::Sequence< OUString > { "com.sun.star.office.PipeSplashScreen" };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
