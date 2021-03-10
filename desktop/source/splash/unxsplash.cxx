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
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace com::sun::star;

namespace desktop
{
    UnxSplashScreen::UnxSplashScreen()
    : m_pOutFd( nullptr )
{
}

UnxSplashScreen::~UnxSplashScreen()
{
    SAL_INFO("desktop.splash", "UnxSplashScreen::~UnxSplashScreen()");
    if ( m_pOutFd )
    {
        fclose( m_pOutFd );
        m_pOutFd = nullptr;
    }
}

void SAL_CALL UnxSplashScreen::start( const OUString& /*aText*/, sal_Int32 /*nRange*/ )
{
}

void SAL_CALL UnxSplashScreen::end()
{
    SAL_INFO("desktop.splash", "UnxSplashScreen::end()");
    if( !m_pOutFd )
        return;

    fprintf( m_pOutFd, "end\n" );
    fflush( m_pOutFd );
}

void SAL_CALL UnxSplashScreen::reset()
{
    SAL_INFO("desktop.splash", "UNXSplashScreen::reset()");
    if( !m_pOutFd )
        return;

    fprintf( m_pOutFd, "restart\n" );
    fflush( m_pOutFd );
}

void SAL_CALL UnxSplashScreen::setText( const OUString& /*aText*/ )
{
    // TODO?
}

void SAL_CALL UnxSplashScreen::setValue( sal_Int32 nValue )
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
{
    for ( sal_uInt32 i = 0; i < osl_getCommandArgCount(); i++ )
    {
        OUString aArg;
        osl_getCommandArg( i, &aArg.pData );
        OUString aNum;
        if ( aArg.startsWithIgnoreAsciiCase("--splash-pipe=", &aNum) )
        {
            auto fd = aNum.toUInt32();
            m_pOutFd = fdopen( fd, "w" );
            SAL_INFO("desktop.splash", "Got argument '--splash-pipe=" << fd << " ('"
                << aNum << "') ("
                << static_cast<void *>(m_pOutFd) << ")");
        }
    }
}

OUString UnxSplashScreen::getImplementationName()
{
    return "com.sun.star.office.comp.PipeSplashScreen";
}

sal_Bool UnxSplashScreen::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> UnxSplashScreen::getSupportedServiceNames()
{
    return { "com.sun.star.office.PipeSplashScreen" };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_UnxSplash_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new desktop::UnxSplashScreen());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
