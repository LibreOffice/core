/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lngpckinsthelper.cxx,v $
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

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <tchar.h>
#include <string>
#include <stdexcept>
#include <vector>

class RegistryKeyGuard
{
public:
    RegistryKeyGuard(HKEY hkey = 0) :
        hkey_(hkey)
    {
    }

    ~RegistryKeyGuard()
    {
        if (hkey_)
            RegCloseKey(hkey_);
    }
private:
    HKEY hkey_;

private:
    RegistryKeyGuard(const RegistryKeyGuard&);
    RegistryKeyGuard& operator=(const RegistryKeyGuard&);
};

typedef std::vector<TCHAR> CharacterBuffer_t;

/* throws std::runtime_error when the value "Path" could
   not be found or contains an empty string or is not of
   type REG_SZ. All such conditions are invalid for a
   properly installed product. */
std::string FindProductInstallationPath(HKEY hkey)
{
    DWORD nSubKeys;
    DWORD lLongestSubKey;

    if (RegQueryInfoKey(hkey, NULL, NULL, NULL, &nSubKeys, &lLongestSubKey, NULL, NULL, NULL, NULL, NULL, NULL) !=
        ERROR_SUCCESS)
        throw std::runtime_error("Cannot query info for registery key");

    CharacterBuffer_t buff(lLongestSubKey + 1);

    for (DWORD i = 0; i < nSubKeys; i++)
    {
        buff[0] = 0;
        LONG ret = RegEnumKey(hkey, i, &buff[0], buff.size());

        if ((ret != ERROR_SUCCESS) && (ret != ERROR_MORE_DATA))
            throw std::runtime_error("Error enumerating registry key");

        HKEY hSubKey;
        if (RegOpenKey(hkey, &buff[0], &hSubKey) != ERROR_SUCCESS)
            continue;

        RegistryKeyGuard guard(hSubKey);

        DWORD type;
        TCHAR pbuff[MAX_PATH];
        DWORD size = sizeof(pbuff);
        if ((RegQueryValueEx(
                hSubKey, TEXT("Path"), NULL, &type, reinterpret_cast<LPBYTE>(pbuff), &size) != ERROR_SUCCESS) ||
                (type != REG_SZ))
            continue;

        std::string path(pbuff);
        std::string::size_type idx = path.rfind("program\\soffice.exe");
        if (idx != std::string::npos)
            return path.substr(0, idx);
    } // for

    throw std::runtime_error("No valid product path found");
}

UINT GetInstallProperty(MSIHANDLE handle, LPCTSTR name, CharacterBuffer_t* buffer)
{
    DWORD size = buffer->size();
    UINT ret = MsiGetProperty(handle, name, &(*buffer)[0], &size);

    if (ret == ERROR_MORE_DATA)
    {
        buffer->resize(size + 1);
        size = buffer->size();
        ret = MsiGetProperty(handle, name, &(*buffer)[0], &size);
    }
    return ret;
}

/*
    Try to find the installation path to an already installed product.
    The installation path will be written in the Windows registry
    during the installation. There may exist different products in
    parallel e.g. StarOffice, StarSuite, OpenOffice.org. It will be
    searched in this order for an installed product. If a product
    will be found the path to the product will be set in the property
    "INSTALLLOCATION" else nothing will be done.
*/
extern "C" UINT __stdcall SetProductInstallationPath(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("SetProductInstallationPath"), TEXT("Language Pack Installation Helper"), MB_OK | MB_ICONINFORMATION);

    try
    {
        CharacterBuffer_t regKeyProdPath(MAX_PATH);

        GetInstallProperty(handle, TEXT("REGKEYPRODPATH"), &regKeyProdPath);

        HKEY hKey;
        if ((RegOpenKey(HKEY_CURRENT_USER, &regKeyProdPath[0], &hKey) == ERROR_SUCCESS) ||
            (RegOpenKey(HKEY_LOCAL_MACHINE, &regKeyProdPath[0], &hKey) == ERROR_SUCCESS))
        {
            RegistryKeyGuard guard(hKey);
            std::string path = FindProductInstallationPath(hKey);
            MsiSetProperty(handle, TEXT("INSTALLLOCATION"), path.c_str());
        }
    }
    catch(std::runtime_error& ex)
    {
        ex = ex; // no warnings
    }
    return ERROR_SUCCESS;
}

void MakeCfgimportCommandLine(CharacterBuffer_t* productPath)
{
    char* p = &(*productPath)[0] + lstrlen(&(*productPath)[0]) - 1;

    if (*p != '\\')
        lstrcat(&(*productPath)[0], "\\program\\configimport.exe --spool");
    else
        lstrcat(&(*productPath)[0], "program\\configimport.exe --spool");
}

/*
    Calls configimport.exe --spool
*/
extern "C" UINT __stdcall RegisterLanguagePack(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("RegisterLanguagePack"), TEXT("Language Pack Installation Helper"), MB_OK | MB_ICONINFORMATION);

    CharacterBuffer_t productPath(MAX_PATH);
    GetInstallProperty(handle, TEXT("INSTALLLOCATION"), &productPath);
    MakeCfgimportCommandLine(&productPath);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (CreateProcess(
            NULL, &productPath[0], NULL, NULL,
            FALSE, CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS, NULL,
            NULL, &si, &pi))
    {
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return ERROR_SUCCESS;
}

