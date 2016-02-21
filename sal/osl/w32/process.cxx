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

#define UNICODE
#include "system.h"
#include <string.h>
#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <shellapi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <cassert>

#include <osl/diagnose.h>
#include <osl/security.h>
#include <osl/nlsupport.h>
#include <osl/mutex.h>
#include <osl/thread.h>
#include <sal/log.hxx>

#include "procimpl.h"
#include "sockimpl.h"
#include "file_url.h"
#include "path_helper.hxx"
#include <rtl/ustrbuf.h>
#include <rtl/alloc.h>

/***************************************************************************
 * Process.
 ***************************************************************************/

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == NULL)
        return osl_Process_E_Unknown;

    if (TerminateProcess(((oslProcessImpl*)Process)->m_hProcess, 0))
        return osl_Process_E_None;

    return osl_Process_E_Unknown;
}

/***************************************************************************/

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    oslProcessImpl* pProcImpl;
    HANDLE hProcess = OpenProcess(
        STANDARD_RIGHTS_REQUIRED | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, (DWORD)Ident);

    if (hProcess)
    {
        pProcImpl = reinterpret_cast< oslProcessImpl*>( rtl_allocateMemory(sizeof(oslProcessImpl)) );
        pProcImpl->m_hProcess  = hProcess;
        pProcImpl->m_IdProcess = Ident;
    }
    else
        pProcImpl = NULL;

    return pProcImpl;
}

/***************************************************************************/

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != NULL)
    {
        CloseHandle(((oslProcessImpl*)Process)->m_hProcess);

        rtl_freeMemory((oslProcessImpl*)Process);
    }
}

/***************************************************************************/

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields,
                                   oslProcessInfo* pInfo)
{
    HANDLE hProcess;
    DWORD  IdProcess;

    if (Process == NULL)
    {
        hProcess  = GetCurrentProcess();
        IdProcess = GetCurrentProcessId();
    }
    else
    {
        hProcess  = ((oslProcessImpl*)Process)->m_hProcess;
        IdProcess = ((oslProcessImpl*)Process)->m_IdProcess;
    }

    if (! pInfo || (pInfo->Size != sizeof(oslProcessInfo)))
        return osl_Process_E_Unknown;

    pInfo->Fields = 0;

    if (Fields & osl_Process_IDENTIFIER)
    {
        pInfo->Ident  = IdProcess;
        pInfo->Fields |= osl_Process_IDENTIFIER;
    }

    if (Fields & osl_Process_EXITCODE)
    {
        if (GetExitCodeProcess(hProcess, &(pInfo->Code)) && (pInfo->Code != STILL_ACTIVE))
            pInfo->Fields |= osl_Process_EXITCODE;
    }

    if (Fields & osl_Process_HEAPUSAGE)
    {
        void*   lpAddress=0;
        MEMORY_BASIC_INFORMATION Info;

        pInfo->HeapUsage = 0;

        do
        {
            if (VirtualQueryEx(hProcess, lpAddress, &Info, sizeof(Info)) == 0)
                break;

            if ((Info.State == MEM_COMMIT) && (Info.Type == MEM_PRIVATE))
                pInfo->HeapUsage += Info.RegionSize;

            lpAddress = (LPBYTE)lpAddress + Info.RegionSize;
        }
        while ((uintptr_t)lpAddress <= (uintptr_t)0x7FFFFFFF); // 2GB address space

        pInfo->Fields |= osl_Process_HEAPUSAGE;
    }

    if (Fields & osl_Process_CPUTIMES)
    {
        FILETIME CreationTime, ExitTime, KernelTime, UserTime;

        if (GetProcessTimes(hProcess, &CreationTime, &ExitTime,
                                      &KernelTime, &UserTime))
        {
            __int64 Value;

            Value = *((__int64 *)&UserTime);
            pInfo->UserTime.Seconds   = (unsigned long) (Value / 10000000L);
            pInfo->UserTime.Nanosec   = (unsigned long)((Value % 10000000L) * 100);

            Value = *((__int64 *)&KernelTime);
            pInfo->SystemTime.Seconds = (unsigned long) (Value / 10000000L);
            pInfo->SystemTime.Nanosec = (unsigned long)((Value % 10000000L) * 100);

            pInfo->Fields |= osl_Process_CPUTIMES;
        }
    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    return osl_joinProcessWithTimeout(Process, NULL);
}

/***************************************************************************/

oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    DWORD           timeout   = INFINITE;
    oslProcessError osl_error = osl_Process_E_None;
    DWORD           ret;

    if (NULL == Process)
        return osl_Process_E_Unknown;

    if (pTimeout)
        timeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000L;

    ret = WaitForSingleObject(((oslProcessImpl*)Process)->m_hProcess, timeout);

    if (WAIT_FAILED == ret)
        osl_error = osl_Process_E_Unknown;
    else if (WAIT_TIMEOUT == ret)
        osl_error = osl_Process_E_TimedOut;

    return osl_error;
}

namespace {

oslProcessError bootstrap_getExecutableFile(rtl_uString ** ppFileURL)
{
    oslProcessError result = osl_Process_E_NotFound;

    ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
    DWORD buflen = 0;

    if ((buflen = GetModuleFileNameW (0, ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer), aBuffer.getBufSizeInSymbols())) > 0)
    {
        rtl_uString * pAbsPath = 0;
        rtl_uString_newFromStr_WithLength (&(pAbsPath), aBuffer, buflen);
        if (pAbsPath)
        {
            /* Convert from path to url. */
            if (osl_getFileURLFromSystemPath (pAbsPath, ppFileURL) == osl_File_E_None)
            {
                /* Success. */
                result = osl_Process_E_None;
            }
            rtl_uString_release (pAbsPath);
        }
    }

    return result;
}

}

/***************************************************************************
 * Command Line Arguments.
 ***************************************************************************/

struct CommandArgs_Impl
{
    sal_uInt32     m_nCount;
    rtl_uString ** m_ppArgs;
};

static struct CommandArgs_Impl g_command_args =
{
    0,
    0
};

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4100 )
#endif
static rtl_uString ** osl_createCommandArgs_Impl (int argc, char **)
{
    rtl_uString ** ppArgs =
        (rtl_uString**)rtl_allocateZeroMemory (argc * sizeof(rtl_uString*));
    if (ppArgs != 0)
    {
        int i;
        int nArgs;
        LPWSTR *wargv = CommandLineToArgvW( GetCommandLineW(), &nArgs );
        assert( nArgs == argc );
        for (i = 0; i < nArgs; i++)
        {
            /* Convert to unicode */
            rtl_uString_newFromStr( &(ppArgs[i]), reinterpret_cast<const sal_Unicode*>(wargv[i]) );
        }
        if (ppArgs[0] != 0)
        {
            /* Ensure absolute path */
            ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
            DWORD dwResult = 0;

            dwResult = SearchPath (
                0, reinterpret_cast<LPCWSTR>(ppArgs[0]->buffer), L".exe", aBuffer.getBufSizeInSymbols(), ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer), 0);
            if ((0 < dwResult) && (dwResult < aBuffer.getBufSizeInSymbols()))
            {
                /* Replace argv[0] with its absolute path */
                rtl_uString_newFromStr_WithLength(
                    &(ppArgs[0]), aBuffer, dwResult);
            }
        }
        if (ppArgs[0] != 0)
        {
            /* Convert to FileURL, see @ osl_getExecutableFile() */
            rtl_uString * pResult = 0;
            osl_getFileURLFromSystemPath (ppArgs[0], &pResult);
            if (pResult != 0)
            {
                rtl_uString_assign (&(ppArgs[0]), pResult);
                rtl_uString_release (pResult);
            }
        }
    }
    return ppArgs;

}
#ifdef _MSC_VER
#pragma warning( pop )
#endif

/***************************************************************************/

oslProcessError SAL_CALL osl_getExecutableFile( rtl_uString **ppustrFile )
{
    osl_acquireMutex (*osl_getGlobalMutex());
    if (g_command_args.m_nCount == 0)
    {
        osl_releaseMutex (*osl_getGlobalMutex());
        return bootstrap_getExecutableFile(ppustrFile);
    }

    /* CommandArgs set. Obtain arv[0]. */
    rtl_uString_assign (ppustrFile, g_command_args.m_ppArgs[0]);
    osl_releaseMutex (*osl_getGlobalMutex());
    return osl_Process_E_None;
}

/***************************************************************************/

sal_uInt32 SAL_CALL osl_getCommandArgCount()
{
    sal_uInt32 result = 0;

    osl_acquireMutex (*osl_getGlobalMutex());
    SAL_INFO_IF(
        g_command_args.m_nCount == 0, "sal.osl",
        "osl_getCommandArgCount w/o prior call to osl_setCommandArgs");
    if (g_command_args.m_nCount > 0)
    {
        /* We're not counting argv[0] here. */
        result = g_command_args.m_nCount - 1;
    }
    osl_releaseMutex (*osl_getGlobalMutex());

    return result;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_getCommandArg( sal_uInt32 nArg, rtl_uString **strCommandArg)
{
    oslProcessError result = osl_Process_E_NotFound;

    osl_acquireMutex (*osl_getGlobalMutex());
    assert(g_command_args.m_nCount > 0);
    if (g_command_args.m_nCount > (nArg + 1))
    {
        /* We're not counting argv[0] here. */
        rtl_uString_assign (strCommandArg, g_command_args.m_ppArgs[nArg + 1]);
        result = osl_Process_E_None;
    }
    osl_releaseMutex (*osl_getGlobalMutex());

    return result;
}

/***************************************************************************/

void SAL_CALL osl_setCommandArgs (int argc, char ** argv)
{
    assert(argc > 0);
    osl_acquireMutex (*osl_getGlobalMutex());
    SAL_WARN_IF(g_command_args.m_nCount != 0, "sal.osl", "args already set");
    if (g_command_args.m_nCount == 0)
    {
        rtl_uString** ppArgs = osl_createCommandArgs_Impl (argc, argv);
        if (ppArgs != 0)
        {
            g_command_args.m_nCount = argc;
            g_command_args.m_ppArgs = ppArgs;
        }
    }
    osl_releaseMutex (*osl_getGlobalMutex());
}

/***************************************************************************
 * Environment
 ***************************************************************************/
#define ENV_BUFFER_SIZE (32*1024-1)

oslProcessError SAL_CALL osl_getEnvironment(rtl_uString *ustrVar, rtl_uString **ustrValue)
{
    WCHAR buff[ENV_BUFFER_SIZE];

    if (GetEnvironmentVariableW(reinterpret_cast<LPCWSTR>(ustrVar->buffer), buff, ENV_BUFFER_SIZE) > 0)
    {
        rtl_uString_newFromStr(ustrValue, reinterpret_cast<const sal_Unicode*>(buff));
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_setEnvironment(rtl_uString *ustrVar, rtl_uString *ustrValue)
{
    // set Windows environment variable
    LPCWSTR lpName = reinterpret_cast<LPCWSTR>(ustrVar->buffer);
    LPCWSTR lpValue = reinterpret_cast<LPCWSTR>(ustrValue->buffer);
    if (SetEnvironmentVariableW(lpName, lpValue))
    {
        wchar_t *buffer = new wchar_t[wcslen(lpName) + 1 + wcslen(lpValue) + 1];
        wcscpy(buffer, lpName);
        wcscat(buffer, L"=");
        wcscat(buffer, lpValue);
        _wputenv(buffer);
        delete[] buffer;
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_clearEnvironment(rtl_uString *ustrVar)
{
    // delete the variable from the current process environment
    // by setting SetEnvironmentVariable's second parameter to NULL
    LPCWSTR lpName = reinterpret_cast<LPCWSTR>(ustrVar->buffer);
    if (SetEnvironmentVariableW(lpName, NULL))
    {
        wchar_t *buffer = new wchar_t[wcslen(lpName) + 1 + 1];
        wcscpy(buffer, lpName);
        wcscat(buffer, L"=");
        _wputenv(buffer);
        delete[] buffer;
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

/***************************************************************************
 * Current Working Directory.
 ***************************************************************************/

extern "C" oslMutex g_CurrentDirectoryMutex;

oslProcessError SAL_CALL osl_getProcessWorkingDir( rtl_uString **pustrWorkingDir )
{
    ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
    DWORD   dwLen = 0;

    osl_acquireMutex( g_CurrentDirectoryMutex );
    dwLen = GetCurrentDirectory( aBuffer.getBufSizeInSymbols(), ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer) );
    osl_releaseMutex( g_CurrentDirectoryMutex );

    if ( dwLen && dwLen < aBuffer.getBufSizeInSymbols() )
    {
        oslFileError    eError;
        rtl_uString     *ustrTemp = NULL;

        rtl_uString_newFromStr_WithLength( &ustrTemp, aBuffer, dwLen );
        eError = osl_getFileURLFromSystemPath( ustrTemp, pustrWorkingDir );

        rtl_uString_release( ustrTemp );

        if ( osl_File_E_None != eError )
            return osl_Process_E_Unknown;
        else
            return osl_Process_E_None;
    }
    else
        return osl_Process_E_Unknown;
}

/***************************************************************************
 * Process Locale.
 ***************************************************************************/

extern "C" void _imp_getProcessLocale( rtl_Locale ** ppLocale );

static rtl_Locale * g_theProcessLocale = NULL;

/***************************************************************************/

oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale )
{
    osl_acquireMutex( *osl_getGlobalMutex() );

    /* determine the users default locale */
    if( NULL == g_theProcessLocale )
        _imp_getProcessLocale( &g_theProcessLocale );

    /* or return the cached value */
    *ppLocale = g_theProcessLocale;

    osl_releaseMutex( *osl_getGlobalMutex() );
    return osl_Process_E_None;
}

/***************************************************************************/

oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale )
{
    osl_acquireMutex( *osl_getGlobalMutex() );

    /* check if locale is supported */
    if( RTL_TEXTENCODING_DONTKNOW == osl_getTextEncodingFromLocale( pLocale ) )
        return osl_Process_E_Unknown;

    /* just remember the locale here */
    g_theProcessLocale = pLocale;

    osl_releaseMutex( *osl_getGlobalMutex() );
    return osl_Process_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
