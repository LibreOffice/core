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
#include <systools/win32/extended_max_path.hxx>
#include <systools/win32/uwinapi.h>
#include <tools/pathutils.hxx>

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

// For PathCchCanonicalizeEx
#include <pathcch.h>
#pragma comment(lib, "Pathcch.lib")

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

    DWORD cwdLen = GetCurrentDirectoryW(0, nullptr);
    std::vector<WCHAR> cwd(cwdLen);
    cwdLen = GetCurrentDirectoryW(cwdLen, cwd.data());
    if (cwdLen == 0 || cwdLen >= cwd.size())
    {
        s += L'0';
    }
    else
    {
        s += L'2';

        size_t n = 0; // number of trailing backslashes
        for (auto* p = cwd.data(); *p; ++p)
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
// returns a pair of strings { binPath, iniDirectory }
// * binPath is the full path to the "bin" file corresponding to the current executable.
// * iniDirectory is the full directory path (ending in "\") to the "ini" file corresponding to the
// current executable.
[[nodiscard]] std::pair<std::wstring, std::wstring> extendLoaderEnvironment()
{
    std::vector<wchar_t> executable_path(EXTENDED_MAX_PATH);
    DWORD exe_len;
    for (;;)
    {
        exe_len = GetModuleFileNameW(nullptr, executable_path.data(), executable_path.size());
        if (!exe_len)
            fail();
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            executable_path.resize(exe_len + 4); // to accommodate a possible ".bin" in the end
            break;
        }
        executable_path.resize(executable_path.size() * 2);
    }
    WCHAR* iniDirEnd = tools::filename(executable_path.data());
    std::wstring_view iniDirView(executable_path.data(), iniDirEnd);
    WCHAR* nameEnd = executable_path.data() + exe_len;
    if (!(nameEnd - iniDirEnd >= 4 && nameEnd[-4] == L'.' &&
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
    std::wstring_view nameView(iniDirEnd, nameEnd);

    WCHAR env[32767];
    DWORD n = GetEnvironmentVariableW(L"PATH", env, std::size(env));
    if ((n >= std::size(env) || n == 0) && GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
        fail();
    }
    std::wstring_view envView(env, n);
    // must be first in PATH to override other entries
    assert(iniDirView.back() == L'\\'); // hence -1 below
    std::wstring_view iniDirView1(iniDirView.substr(0, iniDirView.size() - 1));
    if (!envView.starts_with(iniDirView1) || env[iniDirView1.size()] != L';')
    {
        std::wstring pad(iniDirView1);
        if (n != 0) {
            pad += L';';
            pad += envView;
        }
        if (!SetEnvironmentVariableW(L"PATH", pad.data())) {
            fail();
        }
    }

    return { tools::buildPath(iniDirView, nameView), std::wstring(iniDirView) };
}

}

namespace desktop_win32 {

int officeloader_impl(bool bAllowConsole)
{
    const auto& [szTargetFileName, szIniDirectory] = extendLoaderEnvironment();

    STARTUPINFOW aStartupInfo{ .cb = sizeof(aStartupInfo) };

    // Create process with same command line, environment and stdio handles which
    // are directed to the created pipes
    GetStartupInfoW(&aStartupInfo);

    DWORD dwExitCode = DWORD(-1);

    bool fSuccess = false;
    bool bFirst = true;

    // read limit values from fundamental.override.ini
    unsigned int nMaxMemoryInMB = 0;
    bool bExcludeChildProcesses = true;

    try
    {
        boost::property_tree::ptree pt;
        std::ifstream aFile(szIniDirectory + L"\\fundamental.override.ini");
        boost::property_tree::ini_parser::read_ini(aFile, pt);
        nMaxMemoryInMB = pt.get("Bootstrap.LimitMaximumMemoryInMB", nMaxMemoryInMB);
        bExcludeChildProcesses = pt.get("Bootstrap.ExcludeChildProcessesFromLimit", bExcludeChildProcesses);
    }
    catch (...)
    {
        nMaxMemoryInMB = 0;
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
    const size_t nPathSize = 32 * 1024;
    for (std::wstring_view arg : CommandArgs())
    {
        // Check command line arguments for "--headless" parameter. We only set the environment
        // variable "ATTACHED_PARENT_PROCESSID" for the headless mode as self-destruction of the
        // soffice.bin process can lead to certain side-effects (log-off can result in data-loss,
        // ".lock" is not deleted). See 138244 for more information.
        if (arg == L"-headless" || arg == L"--headless")
            bHeadlessMode = true;
        // check for wildcards in arguments - Windows does not expand automatically
        else if (arg.size() < nPathSize && arg.find_first_of(L"*?") != std::wstring_view::npos)
        {
            const wchar_t* path(arg.data());
            // 1. PathCchCanonicalizeEx only works with backslashes, so preprocess to comply
            wchar_t buf1[nPathSize], buf2[nPathSize];
            arg.copy(buf1, arg.size());
            buf1[arg.size()] = '\0';
            std::replace(buf1, buf1 + arg.size(), '/', '\\');
            // 2. Canonicalize the path: if needed, drop the .. and . segments; if long, make sure
            //    that path has \\?\ long path prefix present (required for FindFirstFileW)
            if (SUCCEEDED(
                    PathCchCanonicalizeEx(buf2, std::size(buf1), buf1, PATHCCH_ALLOW_LONG_PATHS)))
                path = buf2;
            // 3. Expand the wildcards
            WIN32_FIND_DATAW aFindData;
            HANDLE h = FindFirstFileW(path, &aFindData);
            if (h != INVALID_HANDLE_VALUE)
            {
                wchar_t drive[3];
                bool splitted = _wsplitpath_s(path, drive, std::size(drive), buf1, std::size(buf1),
                                              nullptr, 0, nullptr, 0) == 0;
                if (splitted)
                {
                    do
                    {
                        if (_wmakepath_s(buf2, drive, buf1, aFindData.cFileName, nullptr) == 0)
                            aEscapedArgs.push_back(EscapeArg(buf2));
                    } while (FindNextFileW(h, &aFindData));
                }
                FindClose(h);
                if (splitted)
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

        fSuccess = CreateProcessW(szTargetFileName.data(), lpCommandLine, nullptr, nullptr, TRUE,
                                  bAllowConsole ? 0 : DETACHED_PROCESS, nullptr,
                                  szIniDirectory.data(), &aStartupInfo, &aProcessInfo);

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
    const auto& [szTargetFileName, szIniDirectory] = extendLoaderEnvironment();

    STARTUPINFOW aStartupInfo{ .cb = sizeof(aStartupInfo) };
    GetStartupInfoW(&aStartupInfo);

    DWORD   dwExitCode = DWORD(-1);

    std::wstring sCWDarg = getCWDarg();
    DWORD dummy;
    std::wstring redirect = tools::buildPath(szIniDirectory, L"redirect.ini");
    bool hasRedirect = !redirect.empty() &&
            (GetBinaryTypeW(redirect.data(), &dummy) || // cheaper check for file existence?
                GetLastError() != ERROR_FILE_NOT_FOUND);
    LPWSTR cl1 = GetCommandLineW();
    std::wstring cl2 = cl1;
    if (hasRedirect)
        cl2 += L" \"-env:INIFILENAME=vnd.sun.star.pathname:" + redirect + L"\"";
    cl2 += sCWDarg;

    PROCESS_INFORMATION aProcessInfo;

    bool fSuccess = CreateProcessW(
        szTargetFileName.data(),
        cl2.data(),
        nullptr,
        nullptr,
        TRUE,
        bAllowConsole ? 0 : DETACHED_PROCESS,
        nullptr,
        szIniDirectory.data(),
        &aStartupInfo,
        &aProcessInfo);

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
