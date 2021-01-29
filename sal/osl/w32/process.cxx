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

#include "system.h"
#include <string.h>

#include <shellapi.h>

#include <cassert>

#include <osl/mutex.hxx>
#include <osl/nlsupport.h>
#include <o3tl/char16_t2wchar_t.hxx>

#include "filetime.hxx"
#include "nlsupport.hxx"
#include "procimpl.hxx"
#include "file_url.hxx"
#include "path_helper.hxx"
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process)
{
    if (Process == nullptr)
        return osl_Process_E_Unknown;

    HANDLE hProcess = static_cast<oslProcessImpl*>(Process)->m_hProcess;
    DWORD dwPID = GetProcessId(hProcess);

    // cannot be System Process (0x00000000)
    if (dwPID == 0x0)
        return osl_Process_E_InvalidError;

    // Test to see if we can create a thread in a process... adapted from:
    // * https://support.microsoft.com/en-us/help/178893/how-to-terminate-an-application-cleanly-in-win32
    // * http://www.drdobbs.com/a-safer-alternative-to-terminateprocess/184416547

    // TODO: we really should firstly check to see if we have access to create threads and only
    // duplicate the handle with elevated access if we don't have access... this can be done, but
    // it's not exactly easy - an example can be found here:
    // http://windowsitpro.com/site-files/windowsitpro.com/files/archive/windowsitpro.com/content/content/15989/listing_01.txt

    HANDLE hDupProcess = nullptr;


    // we need to make sure we can create a thread in the remote process, if the handle was created
    // in something that doesn't give us appropriate levels of access then we will need to give it the
    // desired level of access - if the process handle was grabbed from OpenProcess it's quite possible
    // that the handle doesn't have the appropriate level of access...

    // see https://msdn.microsoft.com/en-au/library/windows/desktop/ms684880(v=vs.85).aspx
    DWORD const dwAccessFlags = (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION
                                    | PROCESS_VM_WRITE | PROCESS_VM_READ);

    bool bHaveDuplHdl = DuplicateHandle(GetCurrentProcess(),    // handle to process that has handle
                                    hProcess,                   // handle to be duplicated
                                    GetCurrentProcess(),        // process that will get the dup handle
                                    &hDupProcess,               // store duplicate process handle here
                                    dwAccessFlags,              // desired access
                                    FALSE,                      // handle can't be inherited
                                    0);                         // zero means no additional action needed

    if (bHaveDuplHdl)
        hProcess = hDupProcess;     // so we were able to duplicate the handle, all good...
    else
        SAL_WARN("sal.osl", "Could not duplicate process handle, let's hope for the best...");

    DWORD dwProcessStatus = 0;
    HANDLE hRemoteThread = nullptr;

    if (GetExitCodeProcess(hProcess, &dwProcessStatus) && (dwProcessStatus == STILL_ACTIVE))
    {
        // We need to get the address of the Win32 procedure ExitProcess, can't call it
        // directly because we'll be calling the thunk and that will probably lead to an
        // access violation. Once we have the address, then we need to create a new
        // thread in the process (which we might need to run in the address space of
        // another process) and then call on ExitProcess to try to cleanly terminate that
        // process

        DWORD dwTID = 0;    // dummy variable as we don't need to track the thread ID

        // Note: we want to call on ExitProcess() and not TerminateProcess() - this is
        // because with ExitProcess() Windows notifies all attached dlls that the process
        // is detaching from the dll, but TerminateProcess() terminates all threads
        // immediately, doesn't call any termination handlers and doesn't notify any dlls
        // that it is detaching from them

        HINSTANCE hKernel = GetModuleHandleW(L"kernel32.dll");
        FARPROC pfnExitProc = GetProcAddress(hKernel, "ExitProcess");
        hRemoteThread = CreateRemoteThread(
                            hProcess,           /* process handle */
                            nullptr,               /* default security descriptor */
                            0,                  /* initial size of stack in bytes is default
                                                   size for executable */
                            reinterpret_cast<LPTHREAD_START_ROUTINE>(pfnExitProc), /* Win32 ExitProcess() */
                            reinterpret_cast<PVOID>(UINT(0)), /* ExitProcess(UINT uExitCode) argument */
                            0,                  /* value of 0 tells thread to run immediately
                                                   after creation */
                            &dwTID);            /* new remote thread's identifier */

    }

    bool bHasExited = false;

    if (hRemoteThread)
    {
        WaitForSingleObject(hProcess, INFINITE); // wait for process to terminate, never stop waiting...
        CloseHandle(hRemoteThread);              // close the thread handle to allow the process to exit
        bHasExited = true;
    }

    // need to close this duplicated process handle...
    if (bHaveDuplHdl)
        CloseHandle(hProcess);

    if (bHasExited)
        return osl_Process_E_None;

    // fallback - given that we wait for an infinite time on WaitForSingleObject, this should
    // never occur... unless CreateRemoteThread failed
    SAL_WARN("sal.osl", "TerminateProcess(hProcess, 0) called - we should never get here!");
    return (TerminateProcess(hProcess, 0) == FALSE) ? osl_Process_E_Unknown : osl_Process_E_None;
}

oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident)
{
    oslProcessImpl* pProcImpl;
    HANDLE hProcess = OpenProcess(
        STANDARD_RIGHTS_REQUIRED | PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, static_cast<DWORD>(Ident));

    if (hProcess)
    {
        pProcImpl = static_cast< oslProcessImpl*>( malloc(sizeof(oslProcessImpl)) );
        pProcImpl->m_hProcess  = hProcess;
        pProcImpl->m_IdProcess = Ident;
    }
    else
        pProcImpl = nullptr;

    return pProcImpl;
}

void SAL_CALL osl_freeProcessHandle(oslProcess Process)
{
    if (Process != nullptr)
    {
        CloseHandle(static_cast<oslProcessImpl*>(Process)->m_hProcess);

        free(Process);
    }
}

oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields,
                                   oslProcessInfo* pInfo)
{
    HANDLE hProcess;
    DWORD  IdProcess;

    if (Process == nullptr)
    {
        hProcess  = GetCurrentProcess();
        IdProcess = GetCurrentProcessId();
    }
    else
    {
        hProcess  = static_cast<oslProcessImpl*>(Process)->m_hProcess;
        IdProcess = static_cast<oslProcessImpl*>(Process)->m_IdProcess;
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
        void*   lpAddress=nullptr;
        MEMORY_BASIC_INFORMATION Info;

        pInfo->HeapUsage = 0;

        do
        {
            if (VirtualQueryEx(hProcess, lpAddress, &Info, sizeof(Info)) == 0)
                break;

            if ((Info.State == MEM_COMMIT) && (Info.Type == MEM_PRIVATE))
                pInfo->HeapUsage += Info.RegionSize;

            lpAddress = static_cast<LPBYTE>(lpAddress) + Info.RegionSize;
        }
        while (reinterpret_cast<uintptr_t>(lpAddress) <= 0x7FFFFFFFU); // 2GB address space

        pInfo->Fields |= osl_Process_HEAPUSAGE;
    }

    if (Fields & osl_Process_CPUTIMES)
    {
        FILETIME CreationTime, ExitTime, KernelTime, UserTime;

        if (GetProcessTimes(hProcess, &CreationTime, &ExitTime,
                                      &KernelTime, &UserTime))
        {
            __int64 Value;

            Value = osl::detail::getFiletime(UserTime);
            pInfo->UserTime.Seconds   = static_cast<unsigned long>(Value / 10000000L);
            pInfo->UserTime.Nanosec   = static_cast<unsigned long>((Value % 10000000L) * 100);

            Value = osl::detail::getFiletime(KernelTime);
            pInfo->SystemTime.Seconds = static_cast<unsigned long>(Value / 10000000L);
            pInfo->SystemTime.Nanosec = static_cast<unsigned long>((Value % 10000000L) * 100);

            pInfo->Fields |= osl_Process_CPUTIMES;
        }
    }

    return (pInfo->Fields == Fields) ? osl_Process_E_None : osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_joinProcess(oslProcess Process)
{
    return osl_joinProcessWithTimeout(Process, nullptr);
}

oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout)
{
    DWORD           timeout   = INFINITE;
    oslProcessError osl_error = osl_Process_E_None;
    DWORD           ret;

    if (nullptr == Process)
        return osl_Process_E_Unknown;

    if (pTimeout)
        timeout = pTimeout->Seconds * 1000 + pTimeout->Nanosec / 1000000L;

    ret = WaitForSingleObject(static_cast<oslProcessImpl*>(Process)->m_hProcess, timeout);

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

    if ((buflen = GetModuleFileNameW (nullptr, o3tl::toW(aBuffer), aBuffer.getBufSizeInSymbols())) > 0)
    {
        rtl_uString * pAbsPath = nullptr;
        rtl_uString_newFromStr_WithLength (&pAbsPath, aBuffer, buflen);
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

struct CommandArgs_Impl
{
    sal_uInt32     m_nCount;
    rtl_uString ** m_ppArgs;
};

}

static struct CommandArgs_Impl g_command_args =
{
    0,
    nullptr
};

static rtl_uString ** osl_createCommandArgs_Impl (int argc, char **)
{
    rtl_uString ** ppArgs =
        static_cast<rtl_uString**>(rtl_allocateZeroMemory (argc * sizeof(rtl_uString*)));
    if (ppArgs != nullptr)
    {
        int i;
        int nArgs;
        LPWSTR *wargv = CommandLineToArgvW( GetCommandLineW(), &nArgs );
        assert( nArgs == argc );
        for (i = 0; i < nArgs; i++)
        {
            /* Convert to unicode */
            rtl_uString_newFromStr( &(ppArgs[i]), o3tl::toU(wargv[i]) );
        }
        if (ppArgs[0] != nullptr)
        {
            /* Ensure absolute path */
            ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
            DWORD dwResult
                = GetModuleFileNameW(nullptr, o3tl::toW(aBuffer), aBuffer.getBufSizeInSymbols());
            if ((0 < dwResult) && (dwResult < aBuffer.getBufSizeInSymbols()))
            {
                /* Replace argv[0] with its absolute path */
                rtl_uString_newFromStr_WithLength(
                    &(ppArgs[0]), aBuffer, dwResult);
            }
        }
        if (ppArgs[0] != nullptr)
        {
            /* Convert to FileURL, see @ osl_getExecutableFile() */
            rtl_uString * pResult = nullptr;
            osl_getFileURLFromSystemPath (ppArgs[0], &pResult);
            if (pResult != nullptr)
            {
                rtl_uString_assign (&(ppArgs[0]), pResult);
                rtl_uString_release (pResult);
            }
        }
    }
    return ppArgs;

}

oslProcessError SAL_CALL osl_getExecutableFile( rtl_uString **ppustrFile )
{
    {
        osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
        if (g_command_args.m_nCount > 0)
        {
            /* CommandArgs set. Obtain arv[0]. */
            rtl_uString_assign(ppustrFile, g_command_args.m_ppArgs[0]);
            return osl_Process_E_None;
        }
    }
    return bootstrap_getExecutableFile(ppustrFile);
}

sal_uInt32 SAL_CALL osl_getCommandArgCount()
{
    sal_uInt32 result = 0;

    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
    SAL_INFO_IF(
        g_command_args.m_nCount == 0, "sal.osl",
        "osl_getCommandArgCount w/o prior call to osl_setCommandArgs");
    if (g_command_args.m_nCount > 0)
    {
        /* We're not counting argv[0] here. */
        result = g_command_args.m_nCount - 1;
    }

    return result;
}

oslProcessError SAL_CALL osl_getCommandArg( sal_uInt32 nArg, rtl_uString **strCommandArg)
{
    oslProcessError result = osl_Process_E_NotFound;

    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
    assert(g_command_args.m_nCount > 0);
    if (g_command_args.m_nCount > (nArg + 1))
    {
        /* We're not counting argv[0] here. */
        rtl_uString_assign (strCommandArg, g_command_args.m_ppArgs[nArg + 1]);
        result = osl_Process_E_None;
    }

    return result;
}

void SAL_CALL osl_setCommandArgs (int argc, char ** argv)
{
    assert(argc > 0);
    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
    SAL_WARN_IF(g_command_args.m_nCount != 0, "sal.osl", "args already set");
    if (g_command_args.m_nCount == 0)
    {
        rtl_uString** ppArgs = osl_createCommandArgs_Impl (argc, argv);
        if (ppArgs != nullptr)
        {
            g_command_args.m_nCount = argc;
            g_command_args.m_ppArgs = ppArgs;
        }
    }
}

/* TODO because of an issue with GetEnvironmentVariableW we have to
   allocate a buffer large enough to hold the requested environment
   variable instead of testing for the required size. This wastes
   some stack space, maybe we should revoke this work around if
   this is no longer a problem */
#define ENV_BUFFER_SIZE (32*1024-1)

oslProcessError SAL_CALL osl_getEnvironment(rtl_uString *ustrVar, rtl_uString **ustrValue)
{
    WCHAR buff[ENV_BUFFER_SIZE];

    if (GetEnvironmentVariableW(o3tl::toW(ustrVar->buffer), buff, ENV_BUFFER_SIZE) > 0)
    {
        rtl_uString_newFromStr(ustrValue, o3tl::toU(buff));
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_setEnvironment(rtl_uString *ustrVar, rtl_uString *ustrValue)
{
    // set Windows environment variable
    if (SetEnvironmentVariableW(o3tl::toW(ustrVar->buffer), o3tl::toW(ustrValue->buffer)))
    {
        OUString sAssign = OUString::unacquired(&ustrVar) + "=" + OUString::unacquired(&ustrValue);
        _wputenv(o3tl::toW(sAssign.getStr()));
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_clearEnvironment(rtl_uString *ustrVar)
{
    // delete the variable from the current process environment
    // by setting SetEnvironmentVariable's second parameter to NULL
    if (SetEnvironmentVariableW(o3tl::toW(ustrVar->buffer), nullptr))
    {
        OUString sAssign = OUString::unacquired(&ustrVar) + "=";
        _wputenv(o3tl::toW(sAssign.getStr()));
        return osl_Process_E_None;
    }
    return osl_Process_E_Unknown;
}

oslProcessError SAL_CALL osl_getProcessWorkingDir( rtl_uString **pustrWorkingDir )
{
    ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
    DWORD   dwLen = 0;

    osl_acquireMutex( g_CurrentDirectoryMutex );
    dwLen = GetCurrentDirectoryW( aBuffer.getBufSizeInSymbols(), o3tl::toW(aBuffer) );
    osl_releaseMutex( g_CurrentDirectoryMutex );

    if ( dwLen && dwLen < aBuffer.getBufSizeInSymbols() )
    {
        oslFileError    eError;
        rtl_uString     *ustrTemp = nullptr;

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

static rtl_Locale * g_theProcessLocale = nullptr;

oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale )
{
    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());

    /* determine the users default locale */
    if( nullptr == g_theProcessLocale )
        imp_getProcessLocale( &g_theProcessLocale );

    /* or return the cached value */
    *ppLocale = g_theProcessLocale;

    return osl_Process_E_None;
}

oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale )
{
    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());

    /* check if locale is supported */
    if( RTL_TEXTENCODING_DONTKNOW == osl_getTextEncodingFromLocale( pLocale ) )
        return osl_Process_E_Unknown;

    /* just remember the locale here */
    g_theProcessLocale = pLocale;

    return osl_Process_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
