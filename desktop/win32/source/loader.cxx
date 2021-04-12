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

#include "loader.hxx"
#include <cassert>
#include <systools/win32/uwinapi.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <desktop/exithelper.h>
#include <tools/pathutils.hxx>

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace {

void fail()
{
    LPWSTR buf = nullptr;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        GetLastError(), 0, reinterpret_cast< LPWSTR >(&buf), 0, nullptr);
    MessageBoxW(nullptr, buf, nullptr, MB_OK | MB_ICONERROR);
    HeapFree(GetProcessHeap(), 0, buf);
    TerminateProcess(GetCurrentProcess(), 255);
}

LPWSTR* GetCommandArgs(int* pArgc) { return CommandLineToArgvW(GetCommandLineW(), pArgc); }

// tdf#120249: quotes in arguments need to be escaped; backslashes before quotes need doubling. See
// https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-commandlinetoargvw
std::wstring EscapeArg(LPCWSTR sArg)
{
    const size_t nOrigSize = wcslen(sArg);
    LPCWSTR const end = sArg + nOrigSize;
    std::wstring sResult(L"\"");

    LPCWSTR lastPosQuote = sArg;
    LPCWSTR posQuote;
    while ((posQuote = std::find(lastPosQuote, end, L'"')) != end)
    {
        LPCWSTR posBackslash = posQuote;
        while (posBackslash != lastPosQuote && *(posBackslash - 1) == L'\\')
            --posBackslash;

        sResult.append(lastPosQuote, posBackslash);
        sResult.append((posQuote - posBackslash) * 2 + 1, L'\\'); // 2n+1 '\' to escape the '"'
        sResult.append(1, L'"');
        lastPosQuote = posQuote + 1;
    }

    LPCWSTR posTrailingBackslashSeq = end;
    while (posTrailingBackslashSeq != lastPosQuote && *(posTrailingBackslashSeq - 1) == L'\\')
        --posTrailingBackslashSeq;
    sResult.append(lastPosQuote, posTrailingBackslashSeq);
    sResult.append((end - posTrailingBackslashSeq) * 2, L'\\'); // 2n '\' before closing '"'
    sResult.append(1, L'"');

    return sResult;
}

void AddEscapedArg(LPCWSTR sArg, std::vector<std::wstring>& aEscapedArgs,
                   std::size_t& iLengthAccumulator)
{
    std::wstring sEscapedArg = EscapeArg(sArg);
    aEscapedArgs.push_back(sEscapedArg);
    iLengthAccumulator += sEscapedArg.length() + 1; // a space between args
}

bool HasWildCard(LPCWSTR sArg)
{
    while (*sArg != L'\0')
    {
        if (*sArg == L'*' || *sArg == L'?')
            return true;
        sArg++;
    }
    return false;
}

}

namespace desktop_win32 {

void extendLoaderEnvironment(WCHAR * binPath, WCHAR * iniDirectory) {
    if (!GetModuleFileNameW(nullptr, iniDirectory, MAX_PATH)) {
        fail();
    }
    WCHAR * iniDirEnd = tools::filename(iniDirectory);
    WCHAR name[MAX_PATH + MY_LENGTH(L".bin")];
        // hopefully std::size_t is large enough to not overflow
    WCHAR * nameEnd = name;
    for (WCHAR * p = iniDirEnd; *p != L'\0'; ++p) {
        *nameEnd++ = *p;
    }
    if (!(nameEnd - name >= 4 && nameEnd[-4] == L'.' &&
         (((nameEnd[-3] == L'E' || nameEnd[-3] == L'e') &&
           (nameEnd[-2] == L'X' || nameEnd[-2] == L'x') &&
           (nameEnd[-1] == L'E' || nameEnd[-1] == L'e')) ||
          ((nameEnd[-3] == L'C' || nameEnd[-3] == L'c') &&
           (nameEnd[-2] == L'O' || nameEnd[-2] == L'o') &&
           (nameEnd[-1] == L'M' || nameEnd[-1] == L'm')))))
    {
        *nameEnd = L'.';
        nameEnd += 4;
    }
    nameEnd[-3] = 'b';
    nameEnd[-2] = 'i';
    nameEnd[-1] = 'n';
    tools::buildPath(binPath, iniDirectory, iniDirEnd, name, nameEnd - name);
    *iniDirEnd = L'\0';
    std::size_t const maxEnv = 32767;
    WCHAR env[maxEnv];
    DWORD n = GetEnvironmentVariableW(L"PATH", env, maxEnv);
    if ((n >= maxEnv || n == 0) && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
        fail();
    }
    // must be first in PATH to override other entries
    assert(*(iniDirEnd - 1) == L'\\'); // hence -1 below
    if (wcsncmp(env, iniDirectory, iniDirEnd - iniDirectory - 1) != 0
        || env[iniDirEnd - iniDirectory - 1] != L';')
    {
        WCHAR pad[MAX_PATH + maxEnv];
            // hopefully std::size_t is large enough to not overflow
        WCHAR * p = commandLineAppend(pad, iniDirectory, iniDirEnd - iniDirectory - 1);
        if (n != 0) {
            *p++ = L';';
            for (DWORD i = 0; i <= n; ++i) {
                *p++ = env[i];
            }
        } else {
            *p++ = L'\0';
        }
        if (!SetEnvironmentVariableW(L"PATH", pad)) {
            fail();
        }
    }
}

int officeloader_impl(bool bAllowConsole)
{
    WCHAR szTargetFileName[MAX_PATH] = {};
    WCHAR szIniDirectory[MAX_PATH];
    STARTUPINFOW aStartupInfo;

    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    ZeroMemory(&aStartupInfo, sizeof(aStartupInfo));
    aStartupInfo.cb = sizeof(aStartupInfo);

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes
    GetStartupInfoW(&aStartupInfo);

    DWORD dwExitCode = DWORD(-1);

    bool fSuccess = false;
    LPWSTR lpCommandLine = nullptr;
    bool bFirst = true;
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH)
    {
        cwdLen = 0;
    }
    std::vector<std::wstring> aEscapedArgs;

    // read limit values from bootstrap.ini
    unsigned int nMaxMemoryInMB = 0;
    bool bExcludeChildProcesses = true;

    const WCHAR* szIniFile = L"\\bootstrap.ini";
    const size_t nDirLen = wcslen(szIniDirectory);
    if (wcslen(szIniFile) + nDirLen < MAX_PATH)
    {
        WCHAR szBootstrapIni[MAX_PATH];
        wcscpy(szBootstrapIni, szIniDirectory);
        wcscpy(&szBootstrapIni[nDirLen], szIniFile);

        try
        {
            boost::property_tree::ptree pt;
            std::ifstream aFile(szBootstrapIni);
            boost::property_tree::ini_parser::read_ini(aFile, pt);
            nMaxMemoryInMB = pt.get("Win32.LimitMaximumMemoryInMB", nMaxMemoryInMB);
            bExcludeChildProcesses = pt.get("Win32.ExcludeChildProcessesFromLimit", bExcludeChildProcesses);
        }
        catch (...)
        {
            nMaxMemoryInMB = 0;
        }
    }

    // create a Windows JobObject with a memory limit
    HANDLE hJobObject = nullptr;
    if (nMaxMemoryInMB > 0)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION aJobLimit;
        aJobLimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_JOB_MEMORY;
        if (bExcludeChildProcesses)
            aJobLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
        aJobLimit.JobMemoryLimit = nMaxMemoryInMB * 1024 * 1024;
        hJobObject = CreateJobObjectW(nullptr, nullptr);
        if (hJobObject != nullptr)
            SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &aJobLimit,
                                    sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
    }

    do
    {
        if (bFirst)
        {
            int argc = 0;
            LPWSTR* argv = GetCommandArgs(&argc);
            std::size_t n = 0;
            for (int i = 0; i < argc; ++i)
            {
                // check for wildCards in arguments- windows does not expand automatically
                if (HasWildCard(argv[i]))
                {
                    WIN32_FIND_DATAW aFindData;
                    HANDLE h = FindFirstFileW(argv[i], &aFindData);
                    if (h == INVALID_HANDLE_VALUE)
                    {
                        AddEscapedArg(argv[i], aEscapedArgs, n);
                    }
                    else
                    {
                        const int nPathSize = 32 * 1024;
                        wchar_t drive[nPathSize];
                        wchar_t dir[nPathSize];
                        wchar_t path[nPathSize];
                        _wsplitpath_s(argv[i], drive, nPathSize, dir, nPathSize, nullptr, 0,
                                      nullptr, 0);
                        _wmakepath_s(path, nPathSize, drive, dir, aFindData.cFileName, nullptr);
                        AddEscapedArg(path, aEscapedArgs, n);

                        while (FindNextFileW(h, &aFindData))
                        {
                            _wmakepath_s(path, nPathSize, drive, dir, aFindData.cFileName, nullptr);
                            AddEscapedArg(path, aEscapedArgs, n);
                        }
                        FindClose(h);
                    }
                }
                else
                {
                    AddEscapedArg(argv[i], aEscapedArgs, n);
                }
            }
            LocalFree(argv);
            n += MY_LENGTH(L" \"-env:OOO_CWD=2") + 4 * cwdLen + MY_LENGTH(L"\"") + 1;
            // 4 * cwdLen: each char preceded by backslash, each trailing
            // backslash doubled
            lpCommandLine = new WCHAR[n];
        }
        WCHAR* p = desktop_win32::commandLineAppend(lpCommandLine, aEscapedArgs[0].c_str(),
                                                    aEscapedArgs[0].length());
        for (size_t i = 1; i < aEscapedArgs.size(); ++i)
        {
            const std::wstring& rArg = aEscapedArgs[i];
            if (bFirst || EXITHELPER_NORMAL_RESTART == dwExitCode
                || wcsncmp(rArg.c_str(), MY_STRING(L"\"-env:")) == 0)
            {
                p = desktop_win32::commandLineAppend(p, MY_STRING(L" "));
                p = desktop_win32::commandLineAppend(p, rArg.c_str(), rArg.length());
            }
        }

        p = desktop_win32::commandLineAppend(p, MY_STRING(L" \"-env:OOO_CWD="));
        if (cwdLen == 0)
        {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"0"));
        }
        else
        {
            p = desktop_win32::commandLineAppend(p, MY_STRING(L"2"));
            p = desktop_win32::commandLineAppendEncoded(p, cwd);
        }
        desktop_win32::commandLineAppend(p, MY_STRING(L"\""));
        bFirst = false;

        WCHAR szParentProcessId[64]; // This is more than large enough for a 128 bit decimal value
        bool bHeadlessMode(false);

        {
            // Check command line arguments for "--headless" parameter. We only
            // set the environment variable "ATTACHED_PARENT_PROCESSID" for the headless
            // mode as self-destruction of the soffice.bin process can lead to
            // certain side-effects (log-off can result in data-loss, ".lock" is not deleted.
            // See 138244 for more information.
            int argc2;
            LPWSTR* argv2 = GetCommandArgs(&argc2);

            if (argc2 > 1)
            {
                int n;

                for (n = 1; n < argc2; n++)
                {
                    if (0 == wcsnicmp(argv2[n], L"-headless", 9)
                        || 0 == wcsnicmp(argv2[n], L"--headless", 10))
                    {
                        bHeadlessMode = true;
                    }
                }
            }

            LocalFree(argv2);
        }

        if (_ltow(static_cast<long>(GetCurrentProcessId()), szParentProcessId, 10) && bHeadlessMode)
            SetEnvironmentVariableW(L"ATTACHED_PARENT_PROCESSID", szParentProcessId);

        PROCESS_INFORMATION aProcessInfo;

        fSuccess = CreateProcessW(szTargetFileName, lpCommandLine, nullptr, nullptr, TRUE,
                                  bAllowConsole ? 0 : DETACHED_PROCESS, nullptr, szIniDirectory,
                                  &aStartupInfo, &aProcessInfo);

        if (fSuccess)
        {
            DWORD dwWaitResult;

            if (hJobObject)
                AssignProcessToJobObject(hJobObject, aProcessInfo.hProcess);

            do
            {
                // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWith" so
                // we have to do so as if we were processing any messages

                dwWaitResult = MsgWaitForMultipleObjects(1, &aProcessInfo.hProcess, FALSE, INFINITE,
                                                         QS_ALLEVENTS);

                if (WAIT_OBJECT_0 + 1 == dwWaitResult)
                {
                    MSG msg;

                    PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
                }
            } while (WAIT_OBJECT_0 + 1 == dwWaitResult);

            dwExitCode = 0;
            GetExitCodeProcess(aProcessInfo.hProcess, &dwExitCode);

            CloseHandle(aProcessInfo.hProcess);
            CloseHandle(aProcessInfo.hThread);
        }
    } while (fSuccess
             && (EXITHELPER_CRASH_WITH_RESTART == dwExitCode
                 || EXITHELPER_NORMAL_RESTART == dwExitCode));

    if (hJobObject)
        CloseHandle(hJobObject);

    delete[] lpCommandLine;

    return fSuccess ? dwExitCode : -1;
}

int unopkgloader_impl(bool bAllowConsole)
{
    WCHAR        szTargetFileName[MAX_PATH];
    WCHAR        szIniDirectory[MAX_PATH];
    desktop_win32::extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    STARTUPINFOW aStartupInfo{};
    aStartupInfo.cb = sizeof(aStartupInfo);
    GetStartupInfoW(&aStartupInfo);

    DWORD   dwExitCode = DWORD(-1);

    size_t iniDirLen = wcslen(szIniDirectory);
    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen >= MAX_PATH) {
        cwdLen = 0;
    }
    WCHAR redirect[MAX_PATH];
    DWORD dummy;
    bool hasRedirect =
        tools::buildPath(
            redirect, szIniDirectory, szIniDirectory + iniDirLen,
            MY_STRING(L"redirect.ini")) != nullptr &&
            (GetBinaryTypeW(redirect, &dummy) || // cheaper check for file existence?
                GetLastError() != ERROR_FILE_NOT_FOUND);
    LPWSTR cl1 = GetCommandLineW();
    WCHAR* cl2 = new WCHAR[
        wcslen(cl1) +
            (hasRedirect
                ? (MY_LENGTH(L" \"-env:INIFILENAME=vnd.sun.star.pathname:") +
                    iniDirLen + MY_LENGTH(L"redirect.ini\""))
                : 0) +
            MY_LENGTH(L" \"-env:OOO_CWD=2") + 4 * cwdLen + MY_LENGTH(L"\"") + 1];
    // 4 * cwdLen: each char preceded by backslash, each trailing backslash
    // doubled
    WCHAR* p = desktop_win32::commandLineAppend(cl2, cl1);
    if (hasRedirect) {
        p = desktop_win32::commandLineAppend(
            p, MY_STRING(L" \"-env:INIFILENAME=vnd.sun.star.pathname:"));
        p = desktop_win32::commandLineAppend(p, szIniDirectory);
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"redirect.ini\""));
    }
    p = desktop_win32::commandLineAppend(p, MY_STRING(L" \"-env:OOO_CWD="));
    if (cwdLen == 0) {
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"0"));
    }
    else {
        p = desktop_win32::commandLineAppend(p, MY_STRING(L"2"));
        p = desktop_win32::commandLineAppendEncoded(p, cwd);
    }
    desktop_win32::commandLineAppend(p, MY_STRING(L"\""));

    PROCESS_INFORMATION aProcessInfo;

    bool fSuccess = CreateProcessW(
        szTargetFileName,
        cl2,
        nullptr,
        nullptr,
        TRUE,
        bAllowConsole ? 0 : DETACHED_PROCESS,
        nullptr,
        szIniDirectory,
        &aStartupInfo,
        &aProcessInfo);

    delete[] cl2;

    if (fSuccess)
    {
        DWORD   dwWaitResult;

        do
        {
            // On Windows XP it seems as the desktop calls WaitForInputIdle after "OpenWidth" so we have to do so
            // as if we were processing any messages

            dwWaitResult = MsgWaitForMultipleObjects(1, &aProcessInfo.hProcess, FALSE, INFINITE, QS_ALLEVENTS);

            if (WAIT_OBJECT_0 + 1 == dwWaitResult)
            {
                MSG msg;

                PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
            }
        } while (WAIT_OBJECT_0 + 1 == dwWaitResult);

        dwExitCode = 0;
        GetExitCodeProcess(aProcessInfo.hProcess, &dwExitCode);

        CloseHandle(aProcessInfo.hProcess);
        CloseHandle(aProcessInfo.hThread);
    }

    return dwExitCode;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
