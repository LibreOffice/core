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

#ifdef _MSC_VER
#pragma warning( disable: 4668 )
#endif

#include <sal/types.h>
#include <sal/macros.h>

#include <onlinecheck.hxx>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>

// #i71984
extern "C" bool SAL_CALL WNT_hasInternetConnection()
{
    DWORD   dwFlags;
    TCHAR   szConnectionName[1024];

    __try {
    BOOL fIsConnected = InternetGetConnectedStateEx(
        &dwFlags,
        szConnectionName,
        SAL_N_ELEMENTS(szConnectionName),
        0 );

    return fIsConnected;

    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
