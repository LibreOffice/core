/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "system.h"

#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <systools/win32/uwinapi.h>
#include <sal/macros.h>
#include "secimpl.h"

/*****************************************************************************/
/* Data Type Definition */
/*****************************************************************************/


/* Data for use in (un)LoadProfile Functions */
/* Declarations based on USERENV.H for Windows 2000 Beta 2 */
#define PI_NOUI         0x00000001   // Prevents displaying of messages
#define PI_APPLYPOLICY  0x00000002   // Apply NT4 style policy

typedef struct _PROFILEINFOW {
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
  LPTSTR lpProfileDir,
  LPDWORD lpcchSize
);

/* To get an impersonation token we need to create an impersonation
   duplicate so every access token has to be created with duplicate
   access rights */

#define TOKEN_DUP_QUERY (TOKEN_QUERY|TOKEN_DUPLICATE)

/*****************************************************************************/
/* Static Module Function Declarations */
/*****************************************************************************/

static sal_Bool GetSpecialFolder(rtl_uString **strPath,int nFolder);
static BOOL Privilege(LPTSTR pszPrivilege, BOOL bEnable);
static sal_Bool SAL_CALL getUserNameImpl(oslSecurity Security, rtl_uString **strName, sal_Bool bIncludeDomain);

/*****************************************************************************/
/* Exported Module Functions */
/*****************************************************************************/

oslSecurity SAL_CALL osl_getCurrentSecurity(void)
{
    oslSecurityImpl* pSecImpl = malloc(sizeof(oslSecurityImpl));

    pSecImpl->m_pNetResource = NULL;
    pSecImpl->m_User[0] = '\0';
    pSecImpl->m_hToken = NULL;
    pSecImpl->m_hProfile = NULL;

    return ((oslSecurity)pSecImpl);
}

oslSecurityError SAL_CALL osl_loginUser( rtl_uString *strUserName, rtl_uString *strPasswd, oslSecurity *pSecurity )
{
    oslSecurityError ret;

    sal_Unicode*    strUser;
    sal_Unicode*    strDomain = _wcsdup(rtl_uString_getStr(strUserName));
    HANDLE  hUserToken;

    #if OSL_DEBUG_LEVEL > 0
        LUID luid;
    #endif

    if (NULL != (strUser = wcschr(strDomain, L'/')))
        *strUser++ = L'\0';
    else
    {
        strUser   = strDomain;
        strDomain = NULL;
    }

    // this process must have the right: 'act as a part of operatingsystem'
    OSL_ASSERT(LookupPrivilegeValue(NULL, SE_TCB_NAME, &luid));

    if (LogonUserW(strUser, strDomain ? strDomain : L"", rtl_uString_getStr(strPasswd),
                  LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &hUserToken))
    {
        oslSecurityImpl* pSecImpl = malloc(sizeof(oslSecurityImpl));

        pSecImpl->m_pNetResource = NULL;
        pSecImpl->m_hToken = hUserToken;
        pSecImpl->m_hProfile = NULL;
        wcscpy(pSecImpl->m_User, strUser);

        *pSecurity = (oslSecurity)pSecImpl;
        ret = osl_Security_E_None;
    }
    else
        ret = osl_Security_E_UserUnknown;

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

    remoteName  = malloc((rtl_uString_getLength(strFileServer) + rtl_uString_getLength(strUserName) + 4) * sizeof(sal_Unicode));
    userName    = malloc((rtl_uString_getLength(strFileServer) + rtl_uString_getLength(strUserName) + 2) * sizeof(sal_Unicode));

    wcscpy(remoteName, L"\\\\");
    wcscat(remoteName, rtl_uString_getStr(strFileServer));
    wcscat(remoteName, L"\\");
    wcscat(remoteName, rtl_uString_getStr(strUserName));

    wcscpy(userName, rtl_uString_getStr(strFileServer));
    wcscat(userName, L"\\");
    wcscat(userName, rtl_uString_getStr(strUserName));

    netResource.dwScope         = RESOURCE_GLOBALNET;
    netResource.dwType          = RESOURCETYPE_DISK;
    netResource.dwDisplayType   = RESOURCEDISPLAYTYPE_SHARE;
    netResource.dwUsage         = RESOURCEUSAGE_CONNECTABLE;
    netResource.lpLocalName     = NULL;
    netResource.lpRemoteName    = remoteName;
    netResource.lpComment       = NULL;
    netResource.lpProvider      = NULL;

    err = WNetAddConnection2W(&netResource, rtl_uString_getStr(strPasswd), userName, 0);

    if ((err == NO_ERROR) || (err == ERROR_ALREADY_ASSIGNED))
    {
        oslSecurityImpl* pSecImpl = malloc(sizeof(oslSecurityImpl));

        pSecImpl->m_pNetResource = malloc(sizeof(NETRESOURCE));
        *pSecImpl->m_pNetResource = netResource;

        pSecImpl->m_hToken = NULL;
        pSecImpl->m_hProfile = NULL;
        wcscpy(pSecImpl->m_User, rtl_uString_getStr(strUserName));

        *pSecurity = (oslSecurity)pSecImpl;

        ret = osl_Security_E_None;
    }
    else
        ret = osl_Security_E_UserUnknown;

    free(remoteName);
    free(userName);

    return ret;
}


static BOOL WINAPI CheckTokenMembership_Stub( HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember )
{
    typedef BOOL (WINAPI *CheckTokenMembership_PROC)( HANDLE, PSID, PBOOL );

    static HMODULE  hModule = NULL;
    static CheckTokenMembership_PROC    pCheckTokenMembership = NULL;

    if ( !hModule )
    {
        /* SAL is always linked against ADVAPI32 so we can rely on that it is already mapped */

        hModule = GetModuleHandleA( "ADVAPI32.DLL" );

        pCheckTokenMembership = (CheckTokenMembership_PROC)GetProcAddress( hModule, "CheckTokenMembership" );
    }

    if ( pCheckTokenMembership )
        return pCheckTokenMembership( TokenHandle, SidToCheck, IsMember );
    else
    {
        SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
        return FALSE;
    }

}


sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    if (Security != NULL)
    {
        HANDLE                      hImpersonationToken = NULL;
        PSID                        psidAdministrators;
        SID_IDENTIFIER_AUTHORITY    siaNtAuthority = { SECURITY_NT_AUTHORITY };
        sal_Bool                    bSuccess = sal_False;


        /* If Security contains an access token we need to duplicate it to an impersonation
           access token. NULL works with CheckTokenMembership() as the current effective
           impersonation token
         */

        if ( ((oslSecurityImpl*)Security)->m_hToken )
        {
            if ( !DuplicateToken (((oslSecurityImpl*)Security)->m_hToken, SecurityImpersonation, &hImpersonationToken) )
                return sal_False;
        }

        /* CheckTokenMembership() can be used on W2K and higher (NT4 no longer supported by OOo)
           and also works on Vista to retrieve the effective user rights. Just checking for
           membership of Administrators group is not enough on Vista this would require additional
           complicated checks as described in KB arcticle Q118626: http://support.microsoft.com/kb/118626/en-us
        */

        if (AllocateAndInitializeSid(&siaNtAuthority,
                                     2,
                                      SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS,
                                      0, 0, 0, 0, 0, 0,
                                      &psidAdministrators))
        {
            BOOL    fSuccess = FALSE;

            if ( CheckTokenMembership_Stub( hImpersonationToken, psidAdministrators, &fSuccess ) && fSuccess )
                bSuccess = sal_True;

            FreeSid(psidAdministrators);
        }

        if ( hImpersonationToken )
            CloseHandle( hImpersonationToken );

        return (bSuccess);
    }
    else
        return (sal_False);
}


void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    if (Security)
    {
        oslSecurityImpl *pSecImpl = (oslSecurityImpl*)Security;

        if (pSecImpl->m_pNetResource != NULL)
        {
            WNetCancelConnection2W(pSecImpl->m_pNetResource->lpRemoteName, 0, sal_True);

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
    if (Security != NULL)
    {
        oslSecurityImpl *pSecImpl = (oslSecurityImpl*)Security;

        HANDLE hAccessToken = pSecImpl->m_hToken;

        if (hAccessToken == NULL)
            OpenProcessToken(GetCurrentProcess(), TOKEN_DUP_QUERY, &hAccessToken);

        if (hAccessToken)
        {
            sal_Char        *Ident;
            DWORD  nInfoBuffer = 512;
            UCHAR* pInfoBuffer = malloc(nInfoBuffer);


            while (!GetTokenInformation(hAccessToken, TokenUser,
                                           pInfoBuffer, nInfoBuffer, &nInfoBuffer))
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    pInfoBuffer = realloc(pInfoBuffer, nInfoBuffer);
                else
                {
                    free(pInfoBuffer);
                    pInfoBuffer = NULL;
                    break;
                }
            }

            if (pSecImpl->m_hToken == NULL)
                CloseHandle(hAccessToken);

            if (pInfoBuffer)
            {
                PSID pSid = ((PTOKEN_USER)pInfoBuffer)->User.Sid;
                PSID_IDENTIFIER_AUTHORITY psia;
                DWORD dwSubAuthorities;
                DWORD dwSidRev=SID_REVISION;
                DWORD dwCounter;
                DWORD dwSidSize;
                PUCHAR pSSACount;

                /* obtain SidIdentifierAuthority */
                psia=GetSidIdentifierAuthority(pSid);

                /* obtain sidsubauthority count */
                pSSACount = GetSidSubAuthorityCount(pSid);
                dwSubAuthorities = (*pSSACount < 5) ? *pSSACount : 5;

                /* buffer length: S-SID_REVISION- + identifierauthority- + subauthorities- + NULL */
                Ident=malloc(88*sizeof(sal_Char));

                /* prepare S-SID_REVISION- */
                dwSidSize=wsprintf(Ident, TEXT("S-%lu-"), dwSidRev);

                /* prepare SidIdentifierAuthority */
                if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
                {
                    dwSidSize+=wsprintf(Ident + strlen(Ident),
                                TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                                (USHORT)psia->Value[0],
                                (USHORT)psia->Value[1],
                                (USHORT)psia->Value[2],
                                (USHORT)psia->Value[3],
                                (USHORT)psia->Value[4],
                                (USHORT)psia->Value[5]);
                }
                else
                {
                    dwSidSize+=wsprintf(Ident + strlen(Ident),
                                TEXT("%lu"),
                                (ULONG)(psia->Value[5]      )   +
                                (ULONG)(psia->Value[4] <<  8)   +
                                (ULONG)(psia->Value[3] << 16)   +
                                (ULONG)(psia->Value[2] << 24)   );
                }

                /* loop through SidSubAuthorities */
                for (dwCounter=0; dwCounter < dwSubAuthorities; dwCounter++)
                {
                    dwSidSize+=wsprintf(Ident + dwSidSize, TEXT("-%lu"),
                                *GetSidSubAuthority(pSid, dwCounter) );
                }

                rtl_uString_newFromAscii( strIdent, Ident );

                free(pInfoBuffer);
                free(Ident);

                return (sal_True);
            }
        }
        else
        {
            DWORD needed=0;
            sal_Unicode     *Ident;

            WNetGetUserA(NULL, NULL, &needed);
            if (needed < 16)
            {
                needed = 16;
            }
            Ident=malloc(needed*sizeof(sal_Unicode));

            if (WNetGetUserW(NULL, Ident, &needed) != NO_ERROR)
            {
                wcscpy(Ident, L"unknown");
                Ident[7] = L'\0';
            }

            rtl_uString_newFromStr( strIdent, Ident);

            free(Ident);

            return sal_True;
        }
    }

    return sal_False;
}



sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **strName)
{
    return getUserNameImpl(Security, strName, sal_True);
}


sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    rtl_uString *ustrSysDir = NULL;
    sal_Bool    bSuccess = sal_False;

    if (Security != NULL)
    {
        oslSecurityImpl *pSecImpl = (oslSecurityImpl*)Security;

        if (pSecImpl->m_pNetResource != NULL)
        {
            rtl_uString_newFromStr( &ustrSysDir, pSecImpl->m_pNetResource->lpRemoteName);

            bSuccess = (sal_Bool)(osl_File_E_None == osl_getFileURLFromSystemPath( ustrSysDir, pustrDirectory ));
        }
        else
        {
                bSuccess = (sal_Bool)(GetSpecialFolder(&ustrSysDir, CSIDL_PERSONAL) &&
                                     (osl_File_E_None == osl_getFileURLFromSystemPath(ustrSysDir, pustrDirectory)));
        }
    }

    if ( ustrSysDir )
        rtl_uString_release( ustrSysDir );

    return bSuccess;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    sal_Bool    bSuccess = sal_False;

    if (Security != NULL)
    {
        oslSecurityImpl *pSecImpl = (oslSecurityImpl*)Security;

        if (pSecImpl->m_pNetResource != NULL)
        {
            rtl_uString *ustrSysDir = NULL;

            rtl_uString_newFromStr( &ustrSysDir, pSecImpl->m_pNetResource->lpRemoteName);
            bSuccess = (sal_Bool)(osl_File_E_None == osl_getFileURLFromSystemPath( ustrSysDir, pustrDirectory));

            if ( ustrSysDir )
                rtl_uString_release( ustrSysDir );
        }
        else
        {
            if (pSecImpl->m_hToken)
            {
                /* not implemented */
                OSL_ASSERT(sal_False);
            }
            else
            {
                rtl_uString *ustrFile = NULL;
                sal_Unicode sFile[_MAX_PATH];

                if ( !GetSpecialFolder( &ustrFile, CSIDL_APPDATA) )
                {
                    OSL_VERIFY(GetWindowsDirectoryW(sFile, _MAX_DIR) > 0);

                    rtl_uString_newFromStr( &ustrFile, sFile);
                }

                bSuccess = (sal_Bool)(osl_File_E_None == osl_getFileURLFromSystemPath(ustrFile, pustrDirectory));

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
    BOOL bOk = FALSE;

    RegCloseKey(HKEY_CURRENT_USER);

    if (Privilege(SE_RESTORE_NAME, TRUE))
    {
        HMODULE                 hUserEnvLib         = NULL;
        LPFNLOADUSERPROFILE     fLoadUserProfile    = NULL;
        LPFNUNLOADUSERPROFILE   fUnloadUserProfile  = NULL;
        HANDLE                  hAccessToken        = ((oslSecurityImpl*)Security)->m_hToken;

        /* try to create user profile */
        if ( !hAccessToken )
        {
            /* retrieve security handle if not done before e.g. osl_getCurrentSecurity()
            */
            HANDLE hProcess = GetCurrentProcess();

            if (hProcess != NULL)
            {
                OpenProcessToken(hProcess, TOKEN_IMPERSONATE, &hAccessToken);
                CloseHandle(hProcess);
            }
        }

        hUserEnvLib = LoadLibraryA("userenv.dll");

        if (hUserEnvLib)
        {
            fLoadUserProfile = (LPFNLOADUSERPROFILE)GetProcAddress(hUserEnvLib, "LoadUserProfileW");
            fUnloadUserProfile = (LPFNUNLOADUSERPROFILE)GetProcAddress(hUserEnvLib, "UnloadUserProfile");

            if (fLoadUserProfile && fUnloadUserProfile)
            {
                rtl_uString     *buffer = 0;
                PROFILEINFOW    pi;

                getUserNameImpl(Security, &buffer, sal_False);

                ZeroMemory( &pi, sizeof(pi) );
                  pi.dwSize = sizeof(pi);
                pi.lpUserName = rtl_uString_getStr(buffer);
                pi.dwFlags = PI_NOUI;

                if (fLoadUserProfile(hAccessToken, &pi))
                {
                    fUnloadUserProfile(hAccessToken, pi.hProfile);

                    bOk = TRUE;
                }

                rtl_uString_release(buffer);
            }

            FreeLibrary(hUserEnvLib);
        }

        if (hAccessToken && (hAccessToken != ((oslSecurityImpl*)Security)->m_hToken))
            CloseHandle(hAccessToken);
    }

    return (sal_Bool)bOk;
}


void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    if ( ((oslSecurityImpl*)Security)->m_hProfile != NULL )
    {
        HMODULE                 hUserEnvLib         = NULL;
        LPFNLOADUSERPROFILE     fLoadUserProfile    = NULL;
        LPFNUNLOADUSERPROFILE   fUnloadUserProfile  = NULL;
        HANDLE                  hAccessToken        = ((oslSecurityImpl*)Security)->m_hToken;

        if ( !hAccessToken )
        {
            /* retrieve security handle if not done before e.g. osl_getCurrentSecurity()
            */
            HANDLE hProcess = GetCurrentProcess();

            if (hProcess != NULL)
            {
                OpenProcessToken(hProcess, TOKEN_IMPERSONATE, &hAccessToken);
                CloseHandle(hProcess);
            }
        }

        hUserEnvLib = LoadLibrary("userenv.dll");

        if (hUserEnvLib)
        {
            fLoadUserProfile = (LPFNLOADUSERPROFILE)GetProcAddress(hUserEnvLib, "LoadUserProfileA");
            fUnloadUserProfile = (LPFNUNLOADUSERPROFILE)GetProcAddress(hUserEnvLib, "UnloadUserProfile");

            if (fLoadUserProfile && fUnloadUserProfile)
            {
                /* unloading the user profile */
                if (fLoadUserProfile && fUnloadUserProfile)
                    fUnloadUserProfile(hAccessToken, ((oslSecurityImpl*)Security)->m_hProfile);

                if (hUserEnvLib)
                    FreeLibrary(hUserEnvLib);
            }
        }

        ((oslSecurityImpl*)Security)->m_hProfile = NULL;

        if (hAccessToken && (hAccessToken != ((oslSecurityImpl*)Security)->m_hToken))
        {
            CloseHandle(hAccessToken);
        }
    }
}

/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/


static sal_Bool GetSpecialFolder(rtl_uString **strPath, int nFolder)
{
    sal_Bool bRet = sal_False;
    HINSTANCE hLibrary;
    sal_Char PathA[_MAX_PATH];
    sal_Unicode PathW[_MAX_PATH];

    if ((hLibrary = LoadLibrary("shell32.dll")) != NULL)
    {
        BOOL (WINAPI *pSHGetSpecialFolderPathA)(HWND, LPSTR, int, BOOL);
        BOOL (WINAPI *pSHGetSpecialFolderPathW)(HWND, LPWSTR, int, BOOL);

        pSHGetSpecialFolderPathA = (BOOL (WINAPI *)(HWND, LPSTR, int, BOOL))GetProcAddress(hLibrary, "SHGetSpecialFolderPathA");
        pSHGetSpecialFolderPathW = (BOOL (WINAPI *)(HWND, LPWSTR, int, BOOL))GetProcAddress(hLibrary, "SHGetSpecialFolderPathW");

        if (pSHGetSpecialFolderPathA)
        {
            if (pSHGetSpecialFolderPathA(GetActiveWindow(), PathA, nFolder, TRUE))
            {
                rtl_string2UString( strPath, PathA, strlen(PathA), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS);
                OSL_ASSERT(*strPath != NULL);
                bRet = sal_True;
            }
        }
        else if (pSHGetSpecialFolderPathW)
        {
            if (pSHGetSpecialFolderPathW(GetActiveWindow(), PathW, nFolder, TRUE))
            {
                rtl_uString_newFromStr( strPath, PathW);
                bRet = sal_True;
            }
        }
        else
        {
            HRESULT (WINAPI *pSHGetSpecialFolderLocation)(HWND, int, LPITEMIDLIST *) = (HRESULT (WINAPI *)(HWND, int, LPITEMIDLIST *))GetProcAddress(hLibrary, "SHGetSpecialFolderLocation");
            BOOL (WINAPI *pSHGetPathFromIDListA)(LPCITEMIDLIST, LPSTR) = (BOOL (WINAPI *)(LPCITEMIDLIST, LPSTR))GetProcAddress(hLibrary, "SHGetPathFromIDListA");
            BOOL (WINAPI *pSHGetPathFromIDListW)(LPCITEMIDLIST, LPWSTR) = (BOOL (WINAPI *)(LPCITEMIDLIST, LPWSTR))GetProcAddress(hLibrary, "SHGetPathFromIDListW");
             HRESULT (WINAPI *pSHGetMalloc)(LPMALLOC *) = (HRESULT (WINAPI *)(LPMALLOC *))GetProcAddress(hLibrary, "SHGetMalloc");


            if (pSHGetSpecialFolderLocation && (pSHGetPathFromIDListA || pSHGetPathFromIDListW ) && pSHGetMalloc )
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

                    if (RegOpenKey(HKEY_CURRENT_USER,
                                   "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                                   &hRegKey) == ERROR_SUCCESS)
                    {
                        LONG lRet;
                        DWORD lSize = SAL_N_ELEMENTS(PathA);
                        DWORD Type = REG_SZ;

                        switch (nFolder)
                        {
                            case CSIDL_APPDATA:
                                lRet = RegQueryValueEx(hRegKey, "AppData", NULL, &Type, (LPBYTE)PathA, &lSize);
                                  break;

                            case CSIDL_PERSONAL:
                                lRet = RegQueryValueEx(hRegKey, "Personal", NULL, &Type, (LPBYTE)PathA, &lSize);
                                break;

                            default:
                                lRet = -1l;
                        }

                        if ((lRet == ERROR_SUCCESS) && (Type == REG_SZ))
                        {
                            if (_access(PathA, 0) < 0)
                                CreateDirectory(PathA, NULL);

                               hr = pSHGetSpecialFolderLocation(GetActiveWindow(), nFolder, &pidl);
                        }

                        RegCloseKey(hRegKey);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    if (pSHGetPathFromIDListW && pSHGetPathFromIDListW(pidl, PathW))
                       {
                        /* if directory does not exist, create it */
                        if (_waccess(PathW, 0) < 0)
                            CreateDirectoryW(PathW, NULL);

                        rtl_uString_newFromStr( strPath, PathW);
                        bRet = sal_True;
                       }
                    else if (pSHGetPathFromIDListA && pSHGetPathFromIDListA(pidl, PathA))
                    {
                        /* if directory does not exist, create it */
                        if (_access(PathA, 0) < 0)
                            CreateDirectoryA(PathA, NULL);

                        rtl_string2UString( strPath, PathA, strlen(PathA), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS);
                        OSL_ASSERT(*strPath != NULL);
                        bRet = sal_True;
                    }
                   }

                   if (SUCCEEDED(pSHGetMalloc(&pMalloc)))
                {
                       pMalloc->lpVtbl->Free(pMalloc, pidl);
                    pMalloc->lpVtbl->Release(pMalloc);
                }
            }
        }
    }

    FreeLibrary(hLibrary);

    return (bRet);
}


static BOOL Privilege(LPTSTR strPrivilege, BOOL bEnable)
{
    HANDLE           hToken;
    TOKEN_PRIVILEGES tp;

    /*
        obtain the processes token
    */
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_DUP_QUERY, &hToken))
        return FALSE;

    /*
        get the luid
    */
    if (!LookupPrivilegeValue(NULL, strPrivilege, &tp.Privileges[0].Luid))
        return FALSE;

    tp.PrivilegeCount = 1;

    if (bEnable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    /*
        enable or disable the privilege
    */
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
        return FALSE;

    if (!CloseHandle(hToken))
        return FALSE;

    return TRUE;
}

static sal_Bool SAL_CALL getUserNameImpl(oslSecurity Security, rtl_uString **strName,  sal_Bool bIncludeDomain)
{
    if (Security != NULL)
    {
        oslSecurityImpl *pSecImpl = (oslSecurityImpl*)Security;

        HANDLE hAccessToken = pSecImpl->m_hToken;

        if (hAccessToken == NULL)
            OpenProcessToken(GetCurrentProcess(), TOKEN_DUP_QUERY, &hAccessToken);

        if (hAccessToken)
        {
            DWORD  nInfoBuffer = 512;
            UCHAR* pInfoBuffer = malloc(nInfoBuffer);

            while (!GetTokenInformation(hAccessToken, TokenUser,
                                           pInfoBuffer, nInfoBuffer, &nInfoBuffer))
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                    pInfoBuffer = realloc(pInfoBuffer, nInfoBuffer);
                else
                {
                    free(pInfoBuffer);
                    pInfoBuffer = NULL;
                    break;
                }
            }

            if (pSecImpl->m_hToken == NULL)
                CloseHandle(hAccessToken);

            if (pInfoBuffer)
            {
                sal_Unicode  UserName[128];
                sal_Unicode  DomainName[128];
                sal_Unicode  Name[257];
                DWORD nUserName   = sizeof(UserName);
                DWORD nDomainName = sizeof(DomainName);
                SID_NAME_USE sUse;

                if (LookupAccountSidW(NULL, ((PTOKEN_USER)pInfoBuffer)->User.Sid,
                                        UserName, &nUserName,
                                        DomainName, &nDomainName, &sUse))
                {
                    if (bIncludeDomain)
                    {
                        wcscpy(Name, DomainName);
                        wcscat(Name, L"/");
                        wcscat(Name, UserName);
                    }
                    else
                    {
                        wcscpy(Name, UserName);
                    }
                   }
                rtl_uString_newFromStr( strName, Name);

                free(pInfoBuffer);

                return (sal_True);
            }
        }
        else
        {
            DWORD needed=0;
            sal_Unicode         *pNameW=NULL;

            WNetGetUserW(NULL, NULL, &needed);
            pNameW = malloc (needed*sizeof(sal_Unicode));

            if (WNetGetUserW(NULL, pNameW, &needed) == NO_ERROR)
            {
                rtl_uString_newFromStr( strName, pNameW);

                if (pNameW)
                    free(pNameW);
                return (sal_True);
            }
            else
                if (wcslen(pSecImpl->m_User) > 0)
                {
                    rtl_uString_newFromStr( strName, pSecImpl->m_pNetResource->lpRemoteName);

                    if (pNameW)
                        free(pNameW);

                    return (sal_True);
                }

            if (pNameW)
                free(pNameW);
        }
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
