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

#include <sal/config.h>

#if defined(LINUX)
#  include <stdio.h>
#endif
#if defined(__FreeBSD__)
#  include <sys/utsname.h>
#endif

#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif
#include <unx/geninst.h>

// SalYieldMutex

SalYieldMutex::SalYieldMutex()
{
#if HAVE_FEATURE_OPENGL
    SetBeforeReleaseHandler( &OpenGLContext::prepareForYield );
#endif
}

SalYieldMutex::~SalYieldMutex()
{
}

SalGenericInstance::~SalGenericInstance()
{
}

OUString SalGenericInstance::getOSVersion()
{
    OUString aKernelVer = "unknown";

#if defined(LINUX)
    FILE* pVersion = fopen( "/proc/version", "r" );
    if ( pVersion )
    {
        char aVerBuffer[512];
        if ( fgets ( aVerBuffer, 511, pVersion ) )
        {
            aKernelVer = OUString::createFromAscii( aVerBuffer );
            // "Linux version 3.16.7-29-desktop ..."
            OUString aVers = aKernelVer.getToken( 2, ' ' );
            // "3.16.7-29-desktop ..."
            sal_Int32 nTooDetailed = aVers.indexOf( '.', 2);
            if (nTooDetailed < 1 || nTooDetailed > 8)
                aKernelVer = "Linux (misparsed version)";
            else // "3.16.7-29-desktop ..."
                aKernelVer = OUString::Concat("Linux ") + aVers.subView(0, nTooDetailed);
        }
        fclose( pVersion );
    }
    return aKernelVer;
#elif defined(__FreeBSD__)
    struct utsname stName;
    if ( uname( &stName ) != 0 )
        return aKernelVer;

    sal_Int32 nDots = 0;
    sal_Int32 nIndex = 0;
    aKernelVer = OUString::createFromAscii( stName.release );
    while ( nIndex++ < aKernelVer.getLength() )
    {
        const char c = stName.release[ nIndex ];
        if ( c == ' ' || c == '-' || ( c == '.' && nDots++ > 0 ) )
            break;
    }
    return OUString::createFromAscii( stName.sysname ) + " " +
        aKernelVer.copy( 0, nIndex );
#else
    return aKernelVer;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
