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
#include <unotools/bootstrap.hxx>
#include <osl/process.h>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <sfx2/sfx.hrc>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <rtl/logfile.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#define PIPE_ARG "--splash-pipe="

using namespace ::rtl;
using namespace ::com::sun::star::registry;

namespace desktop
{

UnxSplashScreen::UnxSplashScreen( const Reference< XMultiServiceFactory >& rSMgr )
    : m_rFactory( rSMgr ),
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

    fprintf( m_pOutFd, "end\n" );
    fflush( m_pOutFd );
}

void SAL_CALL UnxSplashScreen::reset()
    throw ( RuntimeException )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "UnxSplashScreen::reset()\n" );
#endif

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
        fprintf( m_pOutFd, "%"SAL_PRIdINT32"%%\n", nValue );
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
                     fd, (const sal_Char *)rtl::OUStringToOString( aNum, RTL_TEXTENCODING_UTF8 ),
                     m_pOutFd );
#endif
        }
    }
}

// get service instance...
UnxSplashScreen *UnxSplashScreen::m_pINSTANCE = NULL;
osl::Mutex UnxSplashScreen::m_aMutex;

Reference< XInterface > UnxSplashScreen::getInstance( const Reference< XMultiServiceFactory >& rSMgr )
{
    if ( m_pINSTANCE == NULL )
    {
        osl::MutexGuard guard( m_aMutex );
        if ( m_pINSTANCE == NULL )
            return (XComponent*) new UnxSplashScreen( rSMgr );
    }

    return (XComponent*)NULL;
}

// static service info...
const char* UnxSplashScreen::interfaces[] =
{
    "com.sun.star.task.XStartusIndicator",
    "com.sun.star.lang.XInitialization",
    NULL,
};
const sal_Char *UnxSplashScreen::serviceName = "com.sun.star.office.PipeSplashScreen";
const sal_Char *UnxSplashScreen::implementationName = "com.sun.star.office.comp.PipeSplashScreen";
const sal_Char *UnxSplashScreen::supportedServiceNames[] = { "com.sun.star.office.PipeSplashScreen", NULL };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
