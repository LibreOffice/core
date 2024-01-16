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
#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <vector>
#include <desktop/exithelper.h>
#include <systools/win32/uwinapi.h>
#include <tools/pathutils.hxx>

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#define MY_LENGTH(s) (std::size(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

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

struct CommandArgs
{
    LPWSTR* argv;
    int argc;
    CommandArgs() { argv = CommandLineToArgvW(GetCommandLineW(), &argc); }
    ~CommandArgs() { LocalFree(argv); }
    auto begin() const { return argv; }
    auto end() const { return begin() + argc; }
};

// tdf#120249: quotes in arguments need to be escaped; backslashes before quotes need doubling. See
// https://docs.microsoft.com/en-us/windows/desktop/api/shellapi/nf-shellapi-commandlinetoargvw
std::wstring EscapeArg(std::wstring_view sArg)
{
    std::wstring sResult(L"\"");
    for (size_t lastPosQuote = 0; lastPosQuote <= sArg.size();)
    {
        const size_t posQuote = std::min(sArg.find(L'"', lastPosQuote), sArg.size());
        size_t posBackslash = posQuote;
        while (posBackslash != lastPosQuote && sArg[posBackslash - 1] == L'\\')
            --posBackslash;
        // 2n+1 '\' to escape internal '"'; 2n '\' before closing '"'
        const size_t nEscapes = (posQuote - posBackslash) * 2 + (posQuote < sArg.size() ? 1 : 0);

        sResult.append(sArg.begin() + lastPosQuote, sArg.begin() + posBackslash);
        sResult.append(nEscapes, L'\\');
        sResult.append(1, L'"');
        lastPosQuote = posQuote + 1;
    }
    return sResult;
}

std::wstring getCWDarg()
{
    std::wstring s(L" \"-env:OOO_CWD=");

    WCHAR cwd[MAX_PATH];
    DWORD cwdLen = GetCurrentDirectoryW(MAX_PATH, cwd);
    if (cwdLen == 0 || cwdLen >= MAX_PATH)
    {
        s += L'0';
    }
    else
    {
        s += L'2';

        size_t n = 0; // number of trailing backslashes
        for (auto* p = cwd; *p; ++p)
        {
            WCHAR c = *p;
            if (c == L'$')
            {
                s += L"\\$";
                n = 0;
            }
            else if (c == L'\\')
            {
                s += L"\\\\";
                n += 2;
            }
            else
            {
                s += c;
                n = 0;
            }
        }
        // The command line will continue with a double quote, so double any
        // preceding backslashes as required by Windows:
        s.append(n, L'\\');
    }
    s += L'"';
    return s;
}

WCHAR* commandLineAppend(WCHAR* buffer, std::wstring_view text)
{
    auto ret = std::copy_n(text.begin(), text.size(), buffer);
    *ret = 0; // trailing null
    return ret;
}

// Set the PATH environment variable in the current (loader) process, so that a
// following CreateProcess has the necessary environment:
// @param binPath
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full path to the "bin" file corresponding to the current
// executable.
// @param iniDirectory
// Must point to an array of size at least MAX_PATH.  Is filled with the null
// terminated full directory path (ending in "\") to the "ini" file
// corresponding to the current executable.
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
    std::wstring_view iniDirView(iniDirectory, iniDirEnd - iniDirectory - 1);
    if (!std::wstring_view(env, n).starts_with(iniDirView) || env[iniDirView.size()] != L';')
    {
        WCHAR pad[MAX_PATH + maxEnv];
            // hopefully std::size_t is large enough to not overflow
        WCHAR* p = commandLineAppend(pad, iniDirView);
        if (n != 0) {
            *p++ = L';';
            commandLineAppend(p, std::wstring_view(env, n));
        }
        if (!SetEnvironmentVariableW(L"PATH", pad)) {
            fail();
        }
    }
}

}

namespace desktop_win32 {

int officeloader_impl(bool bAllowConsole)
{
    WCHAR szTargetFileName[MAX_PATH] = {};
    WCHAR szIniDirectory[MAX_PATH];

    extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    STARTUPINFOW aStartupInfo;
    ZeroMemory(&aStartupInfo, sizeof(aStartupInfo));
    aStartupInfo.cb = sizeof(aStartupInfo);

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes
    GetStartupInfoW(&aStartupInfo);

    DWORD dwExitCode = DWORD(-1);

    bool fSuccess = false;
    bool bFirst = true;

    // read limit values from fundamental.override.ini
    unsigned int nMaxMemoryInMB = 0;
    bool bExcludeChildProcesses = true;

    const WCHAR* szIniFile = L"\\fundamental.override.ini";
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
            nMaxMemoryInMB = pt.get("Bootstrap.LimitMaximumMemoryInMB", nMaxMemoryInMB);
            bExcludeChildProcesses = pt.get("Bootstrap.ExcludeChildProcessesFromLimit", bExcludeChildProcesses);
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

    std::vector<std::wstring> aEscapedArgs;
    bool bHeadlessMode = false;
    for (std::wstring_view arg : CommandArgs())
    {
        // Check command line arguments for "--headless" parameter. We only set the environment
        // variable "ATTACHED_PARENT_PROCESSID" for the headless mode as self-destruction of the
        // soffice.bin process can lead to certain side-effects (log-off can result in data-loss,
        // ".lock" is not deleted). See 138244 for more information.
        if (arg == L"-headless" || arg == L"--headless")
            bHeadlessMode = true;
        // check for wildcards in arguments - Windows does not expand automatically
        else if (arg.find_first_of(L"*?") != std::wstring_view::npos)
        {
            WIN32_FIND_DATAW aFindData;
            HANDLE h = FindFirstFileW(arg.data(), &aFindData);
            if (h != INVALID_HANDLE_VALUE)
            {
                const int nPathSize = 32 * 1024;
                wchar_t drive[3];
                wchar_t dir[nPathSize];
                wchar_t path[nPathSize];
                _wsplitpath_s(arg.data(), drive, std::size(drive), dir, std::size(dir), nullptr, 0,
                              nullptr, 0);
                do
                {
                    _wmakepath_s(path, std::size(path), drive, dir, aFindData.cFileName, nullptr);
                    aEscapedArgs.push_back(EscapeArg(path));
                } while (FindNextFileW(h, &aFindData));
                FindClose(h);
                continue;
            }
        }

        aEscapedArgs.push_back(EscapeArg(arg));
    }
    size_t n = std::accumulate(aEscapedArgs.begin(), aEscapedArgs.end(), aEscapedArgs.size(),
                               [](size_t a, const std::wstring& s) { return a + s.size(); });
    std::wstring sCWDarg = getCWDarg();
    n += sCWDarg.size() + 1;
    LPWSTR lpCommandLine = new WCHAR[n];

    if (bHeadlessMode)
    {
        WCHAR szParentProcessId[64]; // This is more than large enough for a 128 bit decimal value
        if (_ltow(static_cast<long>(GetCurrentProcessId()), szParentProcessId, 10))
            SetEnvironmentVariableW(L"ATTACHED_PARENT_PROCESSID", szParentProcessId);
    }

    do
    {
        WCHAR* p = commandLineAppend(lpCommandLine, aEscapedArgs[0]);
        for (size_t i = 1; i < aEscapedArgs.size(); ++i)
        {
            const std::wstring& rArg = aEscapedArgs[i];
            if (bFirst || EXITHELPER_NORMAL_RESTART == dwExitCode || rArg.starts_with(L"\"-env:"))
            {
                p = commandLineAppend(p, L" ");
                p = commandLineAppend(p, rArg);
            }
        }

        commandLineAppend(p, sCWDarg);
        bFirst = false;

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
    extendLoaderEnvironment(szTargetFileName, szIniDirectory);

    STARTUPINFOW aStartupInfo{};
    aStartupInfo.cb = sizeof(aStartupInfo);
    GetStartupInfoW(&aStartupInfo);

    DWORD   dwExitCode = DWORD(-1);

    size_t iniDirLen = wcslen(szIniDirectory);
    std::wstring sCWDarg = getCWDarg();
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
            sCWDarg.size() + 1];
    // 4 * cwdLen: each char preceded by backslash, each trailing backslash
    // doubled
    WCHAR* p = commandLineAppend(cl2, cl1);
    if (hasRedirect) {
        p = commandLineAppend(p, L" \"-env:INIFILENAME=vnd.sun.star.pathname:");
        p = commandLineAppend(p, szIniDirectory);
        p = commandLineAppend(p, L"redirect.ini\"");
    }
    commandLineAppend(p, sCWDarg);

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
