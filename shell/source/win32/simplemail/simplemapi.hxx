/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplemapi.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:08:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_SIMPLEMAPI_HXX
#define INCLUDED_SIMPLEMAPI_HXX

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mapi.h>
#include <mapix.h>

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
