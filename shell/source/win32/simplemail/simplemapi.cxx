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

#include "simplemapi.hxx"

#include <string>
#include <stdexcept>

CSimpleMapi::CSimpleMapi() :
    m_lpfnMapiLogon(nullptr),
    m_lpfnMapiLogoff(nullptr),
    m_lpfnMapiSendMail(nullptr)
{
    m_hMapiDll = LoadLibraryW(L"mapi32.dll");
    if ((m_hMapiDll == INVALID_HANDLE_VALUE) || (m_hMapiDll == nullptr))
        throw std::runtime_error("Couldn't load MAPI library");

    m_lpfnMapiLogon = reinterpret_cast<LPMAPILOGON>(GetProcAddress(m_hMapiDll, "MAPILogon"));
    if (!m_lpfnMapiLogon)
        throw std::runtime_error("Couldn't find method MAPILogon");

    m_lpfnMapiLogoff = reinterpret_cast<LPMAPILOGOFF>(GetProcAddress(m_hMapiDll, "MAPILogoff"));
    if (!m_lpfnMapiLogoff)
        throw std::runtime_error("Couldn't find method MAPILogoff");

    m_lpfnMapiSendMail = reinterpret_cast<LPMAPISENDMAIL>(GetProcAddress(m_hMapiDll, "MAPISendMail"));
    if (!m_lpfnMapiSendMail)
        throw std::runtime_error("Couldn't find method MAPISendMail");
}

CSimpleMapi::~CSimpleMapi()
{
    FreeLibrary(m_hMapiDll);
}

ULONG CSimpleMapi::MAPILogon(
    ULONG ulUIParam,
    LPSTR lpszProfileName,
    LPSTR lpszPassword,
    FLAGS flFlags,
    ULONG ulReserved,
    LPLHANDLE lplhSession )
{
    return m_lpfnMapiLogon(
        ulUIParam,
        lpszProfileName,
        lpszPassword,
        flFlags,
        ulReserved,
        lplhSession );
}

ULONG CSimpleMapi::MAPILogoff(
    LHANDLE lhSession,
    ULONG ulUIParam,
    FLAGS flFlags,
    ULONG ulReserved )
{
    return m_lpfnMapiLogoff(lhSession, ulUIParam, flFlags, ulReserved);
}

ULONG CSimpleMapi::MAPISendMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved )
{
    return m_lpfnMapiSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
