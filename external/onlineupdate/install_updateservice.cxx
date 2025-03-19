/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <shlobj.h>

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
bool getProperty(MSIHANDLE handle, wchar_t const* name, std::wstring* value)
{
    DWORD n = 0;
    if (MsiGetPropertyW(handle, name, const_cast<wchar_t*>(L""), &n) != ERROR_MORE_DATA
        || n == std::numeric_limits<DWORD>::max())
    {
        return false;
    }
    ++n;
    auto buf = std::make_unique<wchar_t[]>(n);
    if (MsiGetPropertyW(handle, name, buf.get(), &n) != ERROR_SUCCESS)
    {
        return false;
    }
    if (n != 0 && buf[n - 1] == L'\\')
    {
        --n;
    }
    value->assign(buf.get(), n);
    return true;
}

typedef std::unique_ptr<void, decltype(&CloseHandle)> CloseHandleGuard;

CloseHandleGuard guard(HANDLE handle) { return CloseHandleGuard(handle, CloseHandle); }

bool runExecutable(std::wstring const& installLocation, wchar_t const* argument)
{
    bool use = false;
    PWSTR progPath;
    if (SHGetKnownFolderPath(FOLDERID_ProgramFiles, 0, nullptr, &progPath) == S_OK)
    {
        auto const n = wcslen(progPath);
        // For SHGetKnownFolderPath it is guaranteed that "The returned path does not include a
        // trailing backslash":
        use = (installLocation.size() == n
               || (installLocation.size() > n && installLocation[n] == L'\\'))
              && _wcsnicmp(progPath, installLocation.data(), n) == 0;
    }
    CoTaskMemFree(progPath);
    if (!use)
    {
        return true;
    }

    std::wstring cmdline(L"\"");
    cmdline += installLocation;
    cmdline += L"\\program\\update_service.exe\" ";
    cmdline += argument;
    auto const n = cmdline.size() + 1;
    auto const buf = std::make_unique<wchar_t[]>(n);
    std::copy_n(cmdline.data(), n, buf.get());
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(nullptr, buf.get(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                        nullptr, &si, &pi))
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

bool writeRegistry(std::wstring const& installLocation)
{
    WCHAR path[MAX_PATH + 1];
    if (!CalculateRegistryPathFromFilePath(installLocation.c_str(), path))
    {
        return false;
    }
    HKEY key;
    if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, (std::wstring(path) + L"\\0").c_str(), 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_WOW64_64KEY, nullptr, &key,
                        nullptr)
        != ERROR_SUCCESS)
    {
        return false;
    }
    auto ok = true;
    if (RegSetValueExW(key, L"issuer", 0, REG_SZ,
                       reinterpret_cast<BYTE const*>(L"Certum Code Signing 2021 CA"),
                       sizeof L"Certum Code Signing 2021 CA")
        != ERROR_SUCCESS)
    {
        ok = false;
    }
    if (RegSetValueExW(key, L"name", 0, REG_SZ,
                       reinterpret_cast<BYTE const*>(L"The Document Foundation"),
                       sizeof L"The Document Foundation")
        != ERROR_SUCCESS)
    {
        ok = false;
    }
    if (RegCloseKey(key) != ERROR_SUCCESS)
    {
        ok = false;
    }
    return ok;
}

bool deleteRegistry(std::wstring const& installLocation)
{
    WCHAR path[MAX_PATH + 1];
    if (!CalculateRegistryPathFromFilePath(installLocation.c_str(), path))
    {
        return false;
    }
    if (RegDeleteTreeW(HKEY_LOCAL_MACHINE, path) != ERROR_SUCCESS)
    {
        return false;
    }
    return true;
}
}

extern "C" __declspec(dllexport) UINT __stdcall PrepareUpdateservice(MSIHANDLE handle)
{
    std::wstring loc;
    if (!getProperty(handle, L"INSTALLLOCATION", &loc))
    {
        return ERROR_INSTALL_FAILURE;
    }
    auto ok = true;
    if (MsiSetPropertyW(handle, L"install_updateservice", loc.c_str()) != ERROR_SUCCESS)
    {
        ok = false;
    }
    if (MsiSetPropertyW(handle, L"remove_updateservice", loc.c_str()) != ERROR_SUCCESS)
    {
        ok = false;
    }
    if (MsiSetPropertyW(handle, L"uninstall_updateservice", loc.c_str()) != ERROR_SUCCESS)
    {
        ok = false;
    }
    return ok ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

extern "C" __declspec(dllexport) UINT __stdcall InstallUpdateservice(MSIHANDLE handle)
{
    std::wstring loc;
    if (!getProperty(handle, L"CustomActionData", &loc))
    {
        return ERROR_INSTALL_FAILURE;
    }
    auto ok = true;
    if (!runExecutable(loc, L"install"))
    {
        ok = false;
    }
    if (!writeRegistry(loc))
    {
        ok = false;
    }
    return ok ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

extern "C" __declspec(dllexport) UINT __stdcall RemoveUpdateservice(MSIHANDLE handle)
{
    std::wstring sInstallPath;
    if (!getProperty(handle, L"CustomActionData", &sInstallPath))
    {
        return ERROR_INSTALL_FAILURE;
    }

    const wchar_t* strarray[] = { L"\\program\\mar.exe", L"\\program\\update_service.exe",
                                  L"\\program\\updater.exe", L"\\program\\updater.ini" };

    for (const wchar_t* file : strarray)
    {
        std::wstring sFilePath = sInstallPath + file;
        WIN32_FIND_DATAW aFindData;
        HANDLE hFind = FindFirstFileW(sFilePath.c_str(), &aFindData);
        if (INVALID_HANDLE_VALUE != hFind)
        {
            FindClose(hFind);
            SetFileAttributesW(sFilePath.c_str(), FILE_ATTRIBUTE_NORMAL);
            DeleteFileW(sFilePath.c_str());
        }
    }
    return ERROR_SUCCESS;
}

extern "C" __declspec(dllexport) UINT __stdcall UninstallUpdateservice(MSIHANDLE handle)
{
    std::wstring loc;
    if (!getProperty(handle, L"CustomActionData", &loc))
    {
        return ERROR_INSTALL_FAILURE;
    }
    auto ok = true;
    if (!runExecutable(loc, L"uninstall"))
    {
        ok = false;
    }
    if (!deleteRegistry(loc))
    {
        ok = false;
    }
    return ok ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
