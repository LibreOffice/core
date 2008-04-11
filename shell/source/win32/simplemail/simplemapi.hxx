/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simplemapi.hxx,v $
 * $Revision: 1.6 $
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

#ifndef INCLUDED_SIMPLEMAPI_HXX
#define INCLUDED_SIMPLEMAPI_HXX

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <mapi.h>
#ifndef __MINGW32__
#include <mapix.h>
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
