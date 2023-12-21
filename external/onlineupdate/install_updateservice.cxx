/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstddef>
#include <limits>
#include <memory>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

#include <pathhash.h>

// Replacements for functions used in
// workdir/UnpackedTarball/onlineupdate/onlineupdate/source/update/common/pathhash.cpp but not
// available here:

extern "C" wchar_t* _wcslwr(wchar_t* str)
{
    for (auto p = str; *p != L'\0'; ++p)
    {
        if (*p >= L'A' && *p <= L'Z')
        {
            *p += L'a' - L'A';
        }
    }
    return str;
};

extern "C" wchar_t* wcsncpy(wchar_t* strDest, wchar_t const* strSource, std::size_t count)
{
    for (std::size_t i = 0; i != count; ++i)
    {
        strDest[i] = *strSource;
        if (*strSource != L'\0')
        {
            ++strSource;
        }
    }
    return strDest;
}

namespace
{
bool getInstallLocation(MSIHANDLE handle, std::wstring* installLocation)
{
    DWORD n = 0;
    if (MsiGetPropertyW(handle, L"INSTALLLOCATION", const_cast<wchar_t*>(L""), &n)
            != ERROR_MORE_DATA
        || n == std::numeric_limits<DWORD>::max())
    {
        return false;
    }
    ++n;
    auto buf = std::make_unique<wchar_t[]>(n);
    if (MsiGetPropertyW(handle, L"INSTALLLOCATION", buf.get(), &n) != ERROR_SUCCESS)
    {
        return false;
    }
    if (n != 0 && buf[n - 1] == L'\\')
    {
        --n;
    }
    installLocation->assign(buf.get(), n);
    return true;
}

typedef std::unique_ptr<void, decltype(&CloseHandle)> CloseHandleGuard;

CloseHandleGuard guard(HANDLE handle) { return CloseHandleGuard(handle, CloseHandle); }

bool runExecutable(std::wstring const& installLocation, wchar_t const* commandLine)
{
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW((installLocation + L"\\program\\update_service.exe").c_str(),
                        const_cast<LPWSTR>(commandLine), nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                        nullptr, nullptr, &si, &pi))
    {
        return false;
    }
    auto const g(guard(pi.hProcess));
    DWORD res = WaitForSingleObject(pi.hProcess, INFINITE);
    if (res != WAIT_OBJECT_0)
    {
        return false;
    }
    DWORD ec = 0;
    if (!GetExitCodeProcess(pi.hProcess, &ec))
    {
        return false;
    }
    if (ec != 0)
    {
        return false;
    }
    return true;
}
}

extern "C" __declspec(dllexport) UINT __stdcall InstallUpdateservice(MSIHANDLE handle)
{
    std::wstring loc;
    if (!getInstallLocation(handle, &loc))
    {
        return false;
    }
    if (!runExecutable(loc, L"install"))
    {
        return ERROR_INVALID_FUNCTION;
    }
    WCHAR maintenanceServiceKey[MAX_PATH + 1];
    if (!CalculateRegistryPathFromFilePath(loc.c_str(), maintenanceServiceKey))
    {
        return ERROR_INVALID_FUNCTION;
    }
    HKEY key;
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, (std::wstring(maintenanceServiceKey) + L"\\0").c_str(),
                        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, nullptr,
                        &key, nullptr)
        != ERROR_SUCCESS)
    {
        return ERROR_INVALID_FUNCTION;
    }
    if (RegSetValueExW(key, L"issuer", 0, REG_SZ,
                       reinterpret_cast<BYTE const*>(L"Certum Code Signing 2021 CA"),
                       sizeof L"Certum Code Signing 2021 CA")
        != ERROR_SUCCESS)
    {
        return ERROR_INVALID_FUNCTION;
    }
    if (RegSetValueExW(key, L"name", 0, REG_SZ,
                       reinterpret_cast<BYTE const*>(L"The Document Foundation"),
                       sizeof L"The Document Foundation")
        != ERROR_SUCCESS)
    {
        return ERROR_INVALID_FUNCTION;
    }
    if (RegCloseKey(key) != ERROR_SUCCESS)
    {
        return ERROR_INVALID_FUNCTION;
    }
    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall UninstallUpdateservice(MSIHANDLE handle)
{
    std::wstring loc;
    if (!getInstallLocation(handle, &loc))
    {
        return false;
    }
    if (!runExecutable(loc, L"uninstall"))
    {
        return ERROR_INVALID_FUNCTION;
    }
    WCHAR maintenanceServiceKey[MAX_PATH + 1];
    if (!CalculateRegistryPathFromFilePath(loc.c_str(), maintenanceServiceKey))
    {
        return ERROR_INVALID_FUNCTION;
    }
    if (RegDeleteTreeW(HKEY_LOCAL_MACHINE, maintenanceServiceKey) != ERROR_SUCCESS)
    {
        return ERROR_INVALID_FUNCTION;
    }
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
