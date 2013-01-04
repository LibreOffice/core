/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "sysmapi.hxx"

#include <string>
#include <stdexcept>

namespace shell
{

WinSysMapi::WinSysMapi() :
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

WinSysMapi::~WinSysMapi()
{
    FreeLibrary(m_hMapiDll);
}

ULONG WinSysMapi::MAPILogon(
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

ULONG WinSysMapi::MAPILogoff(
    LHANDLE lhSession,
    ULONG ulUIParam,
    FLAGS flFlags,
    ULONG ulReserved )
{
    return m_lpfnMapiLogoff(lhSession, ulUIParam, flFlags, ulReserved);
}

ULONG WinSysMapi::MAPISendMail(
    LHANDLE lhSession,
    ULONG ulUIParam,
    lpMapiMessage lpMessage,
    FLAGS flFlags,
    ULONG ulReserved )
{
    return m_lpfnMapiSendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

}

