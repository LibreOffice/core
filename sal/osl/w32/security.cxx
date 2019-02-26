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

#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <systools/win32/uwinapi.h>
#include <sddl.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include "secimpl.hxx"

/* Data for use in (un)LoadProfile Functions */
/* Declarations based on USERENV.H for Windows 2000 Beta 2 */
#define PI_NOUI         0x00000001   // Prevents displaying of messages

typedef struct {
  DWORD    dwSize;          // Must be set to sizeof(PROFILEINFO)
  DWORD    dwFlags;         // See flags above
  LPWSTR   lpUserName;      // User name (required)
  LPWSTR   lpProfilePath;   // Roaming profile path
  LPWSTR   lpDefaultPath;   // Default user profile path
  LPWSTR   lpServerName;    // Validating DC name in netbios format
  LPWSTR   lpPolicyPath;    // Path to the NT4 style policy file
  HANDLE   hProfile;        // Registry key handle - filled by function
} PROFILEINFOW, FAR * LPPROFILEINFOW;

/* Typedefs for function pointers in USERENV.DLL */
typedef BOOL (STDMETHODCALLTYPE FAR * LPFNLOADUSERPROFILE) (
  HANDLE hToken,
  LPPROFILEINFOW lpProfileInfo
);

typedef BOOL (STDMETHODCALLTYPE FAR * LPFNUNLOADUSERPROFILE) (
  HANDLE hToken,
  HANDLE hProfile
);

typedef BOOL (STDMETHODCALLTYPE FAR * LPFNGETUSERPROFILEDIR) (
  HANDLE hToken,
  LPWSTR lpProfileDir,
  LPDWORD lpcchSize
);

/* To get an impersonation token we need to create an impersonation
   duplicate so every access token has to be created with duplicate
   access rights */

#define TOKEN_DUP_QUERY (TOKEN_QUERY|TOKEN_DUPLICATE)

static bool GetSpecialFolder(rtl_uString **strPath,int nFolder);
// We use LPCTSTR here, because we use it with SE_foo_NAME constants
// which are defined in winnt.h as UNICODE-dependent TEXT("PrivilegeName")
static BOOL Privilege(LPCTSTR pszPrivilege, BOOL bEnable);
static bool getUserNameImpl(oslSecurity Security, rtl_uString **strName, bool bIncludeDomain);

oslSecurity SAL_CALL osl_getCurrentSecurity(void)
{
    oslSecurityImpl* pSecImpl = static_cast<oslSecurityImpl *>(malloc(sizeof(oslSecurityImpl)));
    if (pSecImpl)
    {
        pSecImpl->m_pNetResource = nullptr;
        pSecImpl->m_User[0] = '\0';
        pSecImpl->m_hToken = nullptr;
        pSecImpl->m_hProfile = nullptr;
    }
    return pSecImpl;
}

oslSecurityError SAL_CALL osl_loginUser( rtl_uString *strUserName, rtl_uString *strPasswd, oslSecurity *pSecurity )
{
    oslSecurityError ret;

    sal_Unicode*    strUser;
    sal_Unicode*    strDomain = o3tl::toU(_wcsdup(o3tl::toW(rtl_uString_getStr(strUserName))));
    HANDLE  hUserToken;
    LUID luid;

    if (nullptr != (strUser = o3tl::toU(wcschr(o3tl::toW(strDomain), L'/'))))
        *strUser++ = L'\0';
    else
    {
        strUser   = strDomain;
        strDomain = nullptr;
    }

    // this process must have the right: 'act as a part of operatingsystem'
    OSL_ASSERT(LookupPrivilegeValue(nullptr, SE_TCB_NAME, &luid));
    (void) luid;

    if (LogonUserW(o3tl::toW(strUser), strDomain ? o3tl::toW(strDomain) : L"", o3tl::toW(rtl_uString_getStr(strPasswd)),
                  LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &hUserToken))
    {
        oslSecurityImpl* pSecImpl = static_cast<oslSecurityImpl *>(malloc(sizeof(oslSecurityImpl)));
        if (pSecImpl)
        {
            pSecImpl->m_pNetResource = nullptr;
            pSecImpl->m_hToken = hUserToken;
            pSecImpl->m_hProfile = nullptr;
            wcscpy(o3tl::toW(pSecImpl->m_User), o3tl::toW(strUser));
        }
        *pSecurity = pSecImpl;
        ret = pSecImpl ? osl_Security_E_None : osl_Security_E_Unknown;
    }
    else
    {
        ret = osl_Security_E_UserUnknown;
    }

    if (strDomain)
        free(strDomain);
    else
        free(strUser);

    return ret;
}

oslSecurityError SAL_CALL osl_loginUserOnFileServer(rtl_uString *strUserName,
                                                    rtl_uString *strPasswd,
                                                    rtl_uString *strFileServer,
                                                    oslSecurity *pSecurity)
{
    oslSecurityError    ret;
    DWORD               err;
    NETRESOURCEW        netResource;
    sal_Unicode*                remoteName;
    sal_Unicode*                userName;

    remoteName  = static_cast<sal_Unicode *>(malloc((rtl_uString_getLength(strFileServer) + rtl_uString_getLength(strUserName) + 4) * sizeof(sal_Unicode)));
    userName    = static_cast<sal_Unicode *>(malloc((rtl_uString_getLength(strFileServer) + rtl_uString_getLength(strUserName) + 2) * sizeof(sal_Unicode)));

    wcscpy(o3tl::toW(remoteName), L"\\\\");
    wcscat(o3tl::toW(remoteName), o3tl::toW(rtl_uString_getStr(strFileServer)));
    wcscat(o3tl::toW(remoteName), L"\\");
    wcscat(o3tl::toW(remoteName), o3tl::toW(rtl_uString_getStr(strUserName)));

    wcscpy(o3tl::toW(userName), o3tl::toW(rtl_uString_getStr(strFileServer)));
    wcscat(o3tl::toW(userName), L"\\");
    wcscat(o3tl::toW(userName), o3tl::toW(rtl_uString_getStr(strUserName)));

    netResource.dwScope         = RESOURCE_GLOBALNET;
    netResource.dwType          = RESOURCETYPE_DISK;
    netResource.dwDisplayType   = RESOURCEDISPLAYTYPE_SHARE;
    netResource.dwUsage         = RESOURCEUSAGE_CONNECTABLE;
    netResource.lpLocalName     = nullptr;
    netResource.lpRemoteName    = o3tl::toW(remoteName);
    netResource.lpComment       = nullptr;
    netResource.lpProvider      = nullptr;

    err = WNetAddConnection2W(&netResource, o3tl::toW(rtl_uString_getStr(strPasswd)), o3tl::toW(userName), 0);

    if ((err == NO_ERROR) || (err == ERROR_ALREADY_ASSIGNED))
    {
        oslSecurityImpl* pSecImpl = static_cast<oslSecurityImpl *>(malloc(sizeof(oslSecurityImpl)));
        if (pSecImpl)
        {
            pSecImpl->m_pNetResource = static_cast<NETRESOURCEW *>(malloc(sizeof(NETRESOURCE)));
            if (pSecImpl->m_pNetResource)
            {
                *pSecImpl->m_pNetResource = netResource;
                pSecImpl->m_hToken = nullptr;
                pSecImpl->m_hProfile = nullptr;
                wcscpy(o3tl::toW(pSecImpl->m_User), o3tl::toW(rtl_uString_getStr(strUserName)));
            }
            else
            {
                free(pSecImpl);
                pSecImpl = nullptr;
            }
        }
        *pSecurity = pSecImpl;

        ret = pSecImpl ? osl_Security_E_None : osl_Security_E_Unknown;
    }
    else
    {
        ret = osl_Security_E_UserUnknown;
    }

    free(remoteName);
    free(userName);

    return ret;
}

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    if (Security != nullptr)
    {
        HANDLE                      hImpersonationToken = nullptr;
        PSID                        psidAdministrators;
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = { SECURITY_NT_AUTHORITY };
        bool                    bSuccess = false;

        /* If Security contains an access token we need to duplicate it to an impersonation
           access token. NULL works with CheckTokenMembership() as the current effective
           impersonation token
         */

        if ( static_cast<oslSecurityImpl*>(Security)->m_hToken )
        {
            if ( !DuplicateToken (static_cast<oslSecurityImpl*>(Security)->m_hToken, SecurityImpersonation, &hImpersonationToken) )
                return false;
        }

        /* CheckTokenMembership() can be used on W2K and higher (NT4 no longer supported by OOo)
           and also works on Vista to retrieve the effective user rights. Just checking for
           membership of Administrators group is not enough on Vista this would require additional
           complicated checks as described in KB article Q118626: http://support.microsoft.com/kb/118626/en-us
        */

        if (AllocateAndInitializeSid(&siaNtAuthority,
                                     2,
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &psidAdministrators))
        {
            BOOL fSuccess = FALSE;

            if (CheckTokenMembership(hImpersonationToken, psidAdministrators, &fSuccess) && fSuccess)
                bSuccess = true;

            FreeSid(psidAdministrators);
        }

        if (hImpersonationToken)
            CloseHandle(hImpersonationToken);

        return bSuccess;
    }
    else
    {
        return false;
    }
}

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    if (Security)
    {
        oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

        if (pSecImpl->m_pNetResource != nullptr)
        {
            WNetCancelConnection2W(pSecImpl->m_pNetResource->lpRemoteName, 0, true);

            free(pSecImpl->m_pNetResource->lpRemoteName);
            free(pSecImpl->m_pNetResource);
        }

        if (pSecImpl->m_hToken)
            CloseHandle(pSecImpl->m_hToken);

        if ( pSecImpl->m_hProfile )
            CloseHandle(pSecImpl->m_hProfile);

        free (pSecImpl);
    }
}

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **strIdent)
{
    if (Security != nullptr)
    {
        oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

        HANDLE hAccessToken = pSecImpl->m_hToken;

        if (hAccessToken == nullptr)
            OpenProcessToken(GetCurrentProcess(), TOKEN_DUP_QUERY, &hAccessToken);

        if (hAccessToken)
        {
            DWORD  nInfoBuffer = 512;
            UCHAR* pInfoBuffer = static_cast<UCHAR *>(malloc(nInfoBuffer));

            while (!GetTokenInformation(hAccessToken, TokenUser,
                                           pInfoBuffer, nInfoBuffer, &nInfoBuffer))
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (auto p = static_cast<UCHAR *>(realloc(pInfoBuffer, nInfoBuffer)))
                        pInfoBuffer = p;
                    else
                    {
                        free(pInfoBuffer);
                        pInfoBuffer = nullptr;
                        break;
                    }
                }
                else
                {
                    free(pInfoBuffer);
                    pInfoBuffer = nullptr;
                    break;
                }
            }

            if (pSecImpl->m_hToken == nullptr)
                CloseHandle(hAccessToken);

            if (pInfoBuffer)
            {
                PSID pSid = reinterpret_cast<PTOKEN_USER>(pInfoBuffer)->User.Sid;

                LPWSTR pSidStr = nullptr;
                BOOL bResult = ConvertSidToStringSidW(pSid, &pSidStr);
                if (bResult)
                {
                    rtl_uString_newFromStr(strIdent, o3tl::toU(pSidStr));
                    LocalFree(pSidStr);
                }
                else
                {
                    const DWORD dwError = GetLastError();
                    SAL_WARN(
                        "sal.osl",
                        "ConvertSidToStringSidW failed. GetLastError returned: " << dwError);
                }

                free(pInfoBuffer);

                return bResult != FALSE;
            }
        }
        else
        {
            DWORD needed = 0;

            WNetGetUserW(nullptr, nullptr, &needed);
            if (needed < 16)
                needed = 16;

            if (auto Ident = static_cast<sal_Unicode *>(malloc(needed*sizeof(sal_Unicode))))
            {
                if (WNetGetUserW(nullptr, o3tl::toW(Ident), &needed) != NO_ERROR)
                {
                    wcscpy(o3tl::toW(Ident), L"unknown");
                    Ident[7] = L'\0';
                }

                rtl_uString_newFromStr( strIdent, Ident);
                free(Ident);
                return true;
            }
        }
    }

    return false;
}

sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **strName)
{
    return getUserNameImpl(Security, strName, true);
}

sal_Bool SAL_CALL osl_getShortUserName(oslSecurity Security, rtl_uString **strName)
{
    return getUserNameImpl(Security, strName, false);
}

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    rtl_uString *ustrSysDir = nullptr;
    bool    bSuccess = false;

    if (Security != nullptr)
    {
        oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

        if (pSecImpl->m_pNetResource != nullptr)
        {
            rtl_uString_newFromStr( &ustrSysDir, o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName));

            bSuccess = osl_File_E_None == osl_getFileURLFromSystemPath( ustrSysDir, pustrDirectory );
        }
        else
        {
                bSuccess = GetSpecialFolder(&ustrSysDir, CSIDL_PERSONAL) &&
                                     (osl_File_E_None == osl_getFileURLFromSystemPath(ustrSysDir, pustrDirectory));
        }
    }

    if ( ustrSysDir )
        rtl_uString_release( ustrSysDir );

    return bSuccess;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    bool bSuccess = false;

    if (Security != nullptr)
    {
        oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

        if (pSecImpl->m_pNetResource != nullptr)
        {
            rtl_uString *ustrSysDir = nullptr;

            rtl_uString_newFromStr( &ustrSysDir, o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName));
            bSuccess = osl_File_E_None == osl_getFileURLFromSystemPath( ustrSysDir, pustrDirectory);

            if ( ustrSysDir )
                rtl_uString_release( ustrSysDir );
        }
        else
        {
            if (pSecImpl->m_hToken)
            {
                /* not implemented */
                OSL_ASSERT(false);
            }
            else
            {
                rtl_uString *ustrFile = nullptr;
                sal_Unicode sFile[_MAX_PATH];

                if ( !GetSpecialFolder( &ustrFile, CSIDL_APPDATA) )
                {
                    OSL_VERIFY(GetWindowsDirectoryW(o3tl::toW(sFile), _MAX_DIR) > 0);

                    rtl_uString_newFromStr( &ustrFile, sFile);
                }

                bSuccess = osl_File_E_None == osl_getFileURLFromSystemPath(ustrFile, pustrDirectory);

                if ( ustrFile )
                    rtl_uString_release( ustrFile );
            }
        }
    }

    return bSuccess;
}

sal_Bool SAL_CALL osl_loadUserProfile(oslSecurity Security)
{
    /*  CreateProcessAsUser does not load the specified user's profile
        into the HKEY_USERS registry key. This means that access to information
        in the HKEY_CURRENT_USER registry key may not produce results consistent
        with a normal interactive logon.
        It is your responsibility to load the user's registry hive into HKEY_USERS
        with the LoadUserProfile function before calling CreateProcessAsUser.
    */
    bool bOk = false;

    RegCloseKey(HKEY_CURRENT_USER);

    if (Privilege(SE_RESTORE_NAME, TRUE))
    {
        HMODULE                 hUserEnvLib         = nullptr;
        LPFNLOADUSERPROFILE     fLoadUserProfile    = nullptr;
        LPFNUNLOADUSERPROFILE   fUnloadUserProfile  = nullptr;
        HANDLE                  hAccessToken        = static_cast<oslSecurityImpl*>(Security)->m_hToken;

        /* try to create user profile */
        if ( !hAccessToken )
        {
            /* retrieve security handle if not done before e.g. osl_getCurrentSecurity()
            */
            HANDLE hProcess = GetCurrentProcess();

            if (hProcess != nullptr)
            {
                OpenProcessToken(hProcess, TOKEN_IMPERSONATE, &hAccessToken);
                CloseHandle(hProcess);
            }
        }

        hUserEnvLib = LoadLibraryW(L"userenv.dll");

        if (hUserEnvLib)
        {
            fLoadUserProfile = reinterpret_cast<LPFNLOADUSERPROFILE>(GetProcAddress(hUserEnvLib, "LoadUserProfileW"));
            fUnloadUserProfile = reinterpret_cast<LPFNUNLOADUSERPROFILE>(GetProcAddress(hUserEnvLib, "UnloadUserProfile"));

            if (fLoadUserProfile && fUnloadUserProfile)
            {
                rtl_uString *buffer = nullptr;
                PROFILEINFOW pi;

                getUserNameImpl(Security, &buffer, false);

                ZeroMemory(&pi, sizeof(pi));
                pi.dwSize = sizeof(pi);
                pi.lpUserName = o3tl::toW(rtl_uString_getStr(buffer));
                pi.dwFlags = PI_NOUI;

                if (fLoadUserProfile(hAccessToken, &pi))
                {
                    fUnloadUserProfile(hAccessToken, pi.hProfile);

                    bOk = true;
                }

                rtl_uString_release(buffer);
            }

            FreeLibrary(hUserEnvLib);
        }

        if (hAccessToken && (hAccessToken != static_cast<oslSecurityImpl*>(Security)->m_hToken))
            CloseHandle(hAccessToken);
    }

    return bOk;
}

void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    if ( static_cast<oslSecurityImpl*>(Security)->m_hProfile != nullptr )
    {
        HMODULE                 hUserEnvLib         = nullptr;
        LPFNUNLOADUSERPROFILE   fUnloadUserProfile  = nullptr;
        HANDLE                  hAccessToken        = static_cast<oslSecurityImpl*>(Security)->m_hToken;

        if ( !hAccessToken )
        {
            /* retrieve security handle if not done before e.g. osl_getCurrentSecurity()
            */
            HANDLE hProcess = GetCurrentProcess();

            if (hProcess != nullptr)
            {
                OpenProcessToken(hProcess, TOKEN_IMPERSONATE, &hAccessToken);
                CloseHandle(hProcess);
            }
        }

        hUserEnvLib = LoadLibraryW(L"userenv.dll");

        if (hUserEnvLib)
        {
            fUnloadUserProfile = reinterpret_cast<LPFNUNLOADUSERPROFILE>(GetProcAddress(hUserEnvLib, "UnloadUserProfile"));

            if (fUnloadUserProfile)
            {
                /* unloading the user profile */
                fUnloadUserProfile(hAccessToken, static_cast<oslSecurityImpl*>(Security)->m_hProfile);
            }

            FreeLibrary(hUserEnvLib);
        }

        static_cast<oslSecurityImpl*>(Security)->m_hProfile = nullptr;

        if (hAccessToken && (hAccessToken != static_cast<oslSecurityImpl*>(Security)->m_hToken))
            CloseHandle(hAccessToken);
    }
}

static bool GetSpecialFolder(rtl_uString **strPath, int nFolder)
{
    bool bRet = false;
    HINSTANCE hLibrary = LoadLibraryW(L"shell32.dll");

    if (hLibrary != nullptr)
    {
        sal_Unicode PathW[_MAX_PATH];
        BOOL (WINAPI *pSHGetSpecialFolderPathW)(HWND, LPWSTR, int, BOOL) = reinterpret_cast<BOOL (WINAPI *)(HWND, LPWSTR, int, BOOL)>(GetProcAddress(hLibrary, "SHGetSpecialFolderPathW"));

        if (pSHGetSpecialFolderPathW)
        {
            if (pSHGetSpecialFolderPathW(GetActiveWindow(), o3tl::toW(PathW), nFolder, TRUE))
            {
                rtl_uString_newFromStr( strPath, PathW);
                bRet = true;
            }
        }
        else
        {
            HRESULT (WINAPI *pSHGetSpecialFolderLocation)(HWND, int, LPITEMIDLIST *) = reinterpret_cast<HRESULT (WINAPI *)(HWND, int, LPITEMIDLIST *)>(GetProcAddress(hLibrary, "SHGetSpecialFolderLocation"));
            BOOL (WINAPI *pSHGetPathFromIDListW)(LPCITEMIDLIST, LPWSTR) = reinterpret_cast<BOOL (WINAPI *)(LPCITEMIDLIST, LPWSTR)>(GetProcAddress(hLibrary, "SHGetPathFromIDListW"));
            HRESULT (WINAPI *pSHGetMalloc)(LPMALLOC *) = reinterpret_cast<HRESULT (WINAPI *)(LPMALLOC *)>(GetProcAddress(hLibrary, "SHGetMalloc"));

            if (pSHGetSpecialFolderLocation && pSHGetPathFromIDListW && pSHGetMalloc )
            {
                LPITEMIDLIST pidl;
                LPMALLOC pMalloc;
                HRESULT  hr;

                hr = pSHGetSpecialFolderLocation(GetActiveWindow(), nFolder, &pidl);

                /* Get SHGetSpecialFolderLocation fails if directory does not exists. */
                /* If it fails we try to create the directory and redo the call */
                if (! SUCCEEDED(hr))
                {
                    HKEY hRegKey;

                    if (RegOpenKeyW(HKEY_CURRENT_USER,
                                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                                   &hRegKey) == ERROR_SUCCESS)
                    {
                        LONG lRet;
                        DWORD lSize = sizeof(PathW);
                        DWORD Type = REG_SZ;

                        switch (nFolder)
                        {
                            case CSIDL_APPDATA:
                                lRet = RegQueryValueExW(hRegKey, L"AppData", nullptr, &Type, reinterpret_cast<LPBYTE>(PathW), &lSize);
                                  break;

                            case CSIDL_PERSONAL:
                                lRet = RegQueryValueExW(hRegKey, L"Personal", nullptr, &Type, reinterpret_cast<LPBYTE>(PathW), &lSize);
                                break;

                            default:
                                lRet = -1l;
                        }

                        if ((lRet == ERROR_SUCCESS) && (Type == REG_SZ))
                        {
                            if (_waccess(o3tl::toW(PathW), 0) < 0)
                                CreateDirectoryW(o3tl::toW(PathW), nullptr);

                            hr = pSHGetSpecialFolderLocation(GetActiveWindow(), nFolder, &pidl);
                        }

                        RegCloseKey(hRegKey);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if (pSHGetPathFromIDListW(pidl, o3tl::toW(PathW)))
                    {
                        /* if directory does not exist, create it */
                        if (_waccess(o3tl::toW(PathW), 0) < 0)
                            CreateDirectoryW(o3tl::toW(PathW), nullptr);

                        rtl_uString_newFromStr( strPath, PathW);
                        bRet = true;
                    }
                }

                if (SUCCEEDED(pSHGetMalloc(&pMalloc)))
                {
                    pMalloc->Free(pidl);
                    pMalloc->Release();
                }
            }
        }

        FreeLibrary(hLibrary);
    }

    return bRet;
}

// We use LPCTSTR here, because we use it with SE_foo_NAME constants
// which are defined in winnt.h as UNICODE-dependent TEXT("PrivilegeName")
static BOOL Privilege(LPCTSTR strPrivilege, BOOL bEnable)
{
    HANDLE           hToken;
    TOKEN_PRIVILEGES tp;

    // obtain the processes token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_DUP_QUERY, &hToken))
        return FALSE;

    // get the luid
    if (!LookupPrivilegeValue(nullptr, strPrivilege, &tp.Privileges[0].Luid))
        return FALSE;

    tp.PrivilegeCount = 1;

    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // enable or disable the privilege
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr))
        return FALSE;

    if (!CloseHandle(hToken))
        return FALSE;

    return TRUE;
}

static bool getUserNameImpl(oslSecurity Security, rtl_uString **strName,  bool bIncludeDomain)
{
    if (Security != nullptr)
    {
        oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

        HANDLE hAccessToken = pSecImpl->m_hToken;

        if (hAccessToken == nullptr)
            OpenProcessToken(GetCurrentProcess(), TOKEN_DUP_QUERY, &hAccessToken);

        if (hAccessToken)
        {
            DWORD  nInfoBuffer = 512;
            UCHAR* pInfoBuffer = static_cast<UCHAR *>(malloc(nInfoBuffer));

            while (!GetTokenInformation(hAccessToken, TokenUser,
                                           pInfoBuffer, nInfoBuffer, &nInfoBuffer))
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (auto p = static_cast<UCHAR *>(realloc(pInfoBuffer, nInfoBuffer)))
                        pInfoBuffer = p;
                    else
                    {
                        free(pInfoBuffer);
                        pInfoBuffer = nullptr;
                        break;
                    }
                }
                else
                {
                    free(pInfoBuffer);
                    pInfoBuffer = nullptr;
                    break;
                }
            }

            if (pSecImpl->m_hToken == nullptr)
                CloseHandle(hAccessToken);

            if (pInfoBuffer)
            {
                sal_Unicode  UserName[128];
                sal_Unicode  DomainName[128];
                sal_Unicode  Name[257];
                DWORD nUserName   = SAL_N_ELEMENTS(UserName);
                DWORD nDomainName = SAL_N_ELEMENTS(DomainName);
                SID_NAME_USE sUse;

                if (LookupAccountSidW(nullptr, reinterpret_cast<PTOKEN_USER>(pInfoBuffer)->User.Sid,
                                        o3tl::toW(UserName), &nUserName,
                                        o3tl::toW(DomainName), &nDomainName, &sUse))
                {
                    if (bIncludeDomain)
                    {
                        wcscpy(o3tl::toW(Name), o3tl::toW(DomainName));
                        wcscat(o3tl::toW(Name), L"/");
                        wcscat(o3tl::toW(Name), o3tl::toW(UserName));
                    }
                    else
                    {
                        wcscpy(o3tl::toW(Name), o3tl::toW(UserName));
                    }

                    rtl_uString_newFromStr(strName, Name);
                    free(pInfoBuffer);
                    return true;
                }
            }
        }
        else
        {
            DWORD needed=0;
            sal_Unicode *pNameW=nullptr;

            WNetGetUserW(nullptr, nullptr, &needed);
            pNameW = static_cast<sal_Unicode *>(malloc (needed*sizeof(sal_Unicode)));

            if (WNetGetUserW(nullptr, o3tl::toW(pNameW), &needed) == NO_ERROR)
            {
                rtl_uString_newFromStr( strName, pNameW);

                if (pNameW)
                    free(pNameW);
                return true;
            }
            else if (pSecImpl->m_User[0] != '\0')
            {
                rtl_uString_newFromStr(strName, o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName));

                if (pNameW)
                    free(pNameW);

                return true;
            }

            if (pNameW)
                free(pNameW);
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
