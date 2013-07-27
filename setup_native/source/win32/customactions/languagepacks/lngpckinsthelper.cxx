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
    parallel e.g. StarOffice, StarSuite, OpenOffice.org, Apache OpenOffice.
    It will be     searched in this order for an installed product. If a product
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

