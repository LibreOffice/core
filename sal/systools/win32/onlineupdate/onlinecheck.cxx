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
#include "precompiled_sal.hxx"
#ifdef _MSC_VER
#pragma warning( disable: 4668 )
#endif

#include <sal/types.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>
#ifdef __MINGW32__
#include <excpt.h>
#endif

// #i71984
extern "C" sal_Bool SAL_CALL hasInternetConnection()
{
    DWORD   dwFlags;
    TCHAR   szConnectionName[1024];

#ifdef __MINGW32__
        jmp_buf jmpbuf;
        __SEHandler han;
        if (__builtin_setjmp(jmpbuf) == 0)
        {
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER);
#else
    __try {
#endif
    BOOL fIsConnected = InternetGetConnectedStateEx(
        &dwFlags,
        szConnectionName,
        SAL_N_ELEMENTS(szConnectionName),
        0 );

    return fIsConnected ? sal_True : sal_False;

#ifdef __MINGW32__
        }
        else return sal_False;
        han.Reset();
#else
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        return sal_False;
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
