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

#ifndef INCLUDED_SIMPLEMAPI_HXX
#define INCLUDED_SIMPLEMAPI_HXX

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <mapi.h>
#ifndef __MINGW32__
#if NTDDI_VERSION < NTDDI_WIN8
#include <mapix.h>
#endif
#endif
#if defined _MSC_VER
#pragma warning(pop)
#endif

class CSimpleMapi
{
public:
    /**
        @throws std::runtime_error
        if either the mapi32.dll could not be loaded at all
        or necessary function exports are missing
    */
    CSimpleMapi(); // throws std::runtime_error;

    ~CSimpleMapi();

    ULONG MAPILogon(
        ULONG ulUIParam,
        LPTSTR lpszProfileName,
        LPTSTR lpszPassword,
        FLAGS flFlags,
        ULONG ulReserved,
        LPLHANDLE lplhSession );

    ULONG MAPILogoff(
        LHANDLE lhSession,
        ULONG ulUIParam,
        FLAGS flFlags,
        ULONG ulReserved );

    ULONG MAPISendMail(
        LHANDLE lhSession,
        ULONG ulUIParam,
        lpMapiMessage lpMessage,
        FLAGS flFlags,
        ULONG ulReserved );

private:
    HMODULE         m_hMapiDll;
    LPMAPILOGON     m_lpfnMapiLogon;
    LPMAPILOGOFF    m_lpfnMapiLogoff;
    LPMAPISENDMAIL  m_lpfnMapiSendMail;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
