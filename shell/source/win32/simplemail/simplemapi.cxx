/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simplemapi.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_shell.hxx"
#include "simplemapi.hxx"

#include <string>
#include <stdexcept>

CSimpleMapi::CSimpleMapi() :
    m_lpfnMapiLogon(NULL),
    m_lpfnMapiLogoff(NULL),
    m_lpfnMapiSendMail(NULL)
{
    m_hMapiDll = LoadLibrary("mapi32.dll");
    if ((m_hMapiDll == INVALID_HANDLE_VALUE) || (m_hMapiDll == NULL))
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
    LPTSTR lpszProfileName,
    LPTSTR lpszPassword,
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

