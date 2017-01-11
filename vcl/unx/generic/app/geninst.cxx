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

#include <cassert>
#include <string.h>
#include <stdlib.h>

#if defined(LINUX)
#  include <stdio.h>
#endif

#include <osl/module.hxx>
#include <comphelper/solarmutex.hxx>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <vcl/opengl/OpenGLContext.hxx>
#endif
#include "unx/geninst.h"

// SalYieldMutex

SalYieldMutex::SalYieldMutex()
{
    mnCount     = 0;
    mnThreadId  = 0;
    ::comphelper::SolarMutex::setSolarMutex( this );
}

SalYieldMutex::~SalYieldMutex()
{
    ::comphelper::SolarMutex::setSolarMutex( nullptr );
}

void SalYieldMutex::acquire()
{
    m_mutex.acquire();
    mnThreadId = osl::Thread::getCurrentIdentifier();
    mnCount++;
}

void SalYieldMutex::release()
{
    assert(mnCount != 0);
    assert(mnThreadId == osl::Thread::getCurrentIdentifier());
    if ( mnCount == 1 )
    {
#if HAVE_FEATURE_OPENGL
        OpenGLContext::prepareForYield();
#endif
        mnThreadId = 0;
    }
    mnCount--;
    m_mutex.release();
}

bool SalYieldMutex::tryToAcquire()
{
    if ( m_mutex.tryToAcquire() )
    {
        mnThreadId = osl::Thread::getCurrentIdentifier();
        mnCount++;
        return true;
    }
    else
        return false;
}

comphelper::SolarMutex* SalGenericInstance::GetYieldMutex()
{
    return mpSalYieldMutex.get();
}

sal_uLong SalGenericInstance::ReleaseYieldMutex()
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex.get();
    if ( pYieldMutex->GetThreadId() ==
         osl::Thread::getCurrentIdentifier() )
    {
        sal_uLong nCount = pYieldMutex->GetAcquireCount();
        sal_uLong n = nCount;
        while ( n )
        {
            pYieldMutex->release();
            n--;
        }

        return nCount;
    }
    else
        return 0;
}

void SalGenericInstance::AcquireYieldMutex( sal_uLong nCount )
{
    SalYieldMutex* pYieldMutex = mpSalYieldMutex.get();
    while ( nCount )
    {
        pYieldMutex->acquire();
        nCount--;
    }
}

bool SalGenericInstance::CheckYieldMutex()
{
    bool bRet = true;

    SalYieldMutex* pYieldMutex = mpSalYieldMutex.get();
    if ( pYieldMutex->GetThreadId() != osl::Thread::getCurrentIdentifier() )
    {
        SAL_WARN("vcl", "CheckYieldMutex: " << pYieldMutex->GetThreadId() << "!=" << osl::Thread::getCurrentIdentifier() );
        bRet = false;
    }

    return bRet;
}

SalGenericInstance::~SalGenericInstance()
{
}

OUString SalGenericInstance::getOSVersion()
{
    OUString aKernelVer = "unknown";

// not so generic, but at least shared between all unix backend
#if defined(LINUX)
    FILE* pVersion = fopen( "/proc/version", "r" );
    if ( pVersion )
    {
        char aVerBuffer[512];
        if ( fgets ( aVerBuffer, 511, pVersion ) )
        {
            aKernelVer = OUString::createFromAscii( aVerBuffer );
            sal_Int32 nIndex = 0;
            // "Linux version 3.16.7-29-desktop ..."
            OUString aVers = aKernelVer.getToken( 2, ' ', nIndex );
            // "3.16.7-29-desktop ..."
            sal_Int32 nTooDetailed = aVers.indexOf( '.', 2);
            if (nTooDetailed < 1 || nTooDetailed > 8)
                aKernelVer = "Linux (misparsed version)";
            else // "3.16.7-29-desktop ..."
                aKernelVer = "Linux " + aVers.copy(0, nTooDetailed);
        }
        fclose( pVersion );
    }
#endif
    return aKernelVer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
