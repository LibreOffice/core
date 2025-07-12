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
#include <userenv.h>

#include <cassert>
#include <osl/security.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <systools/win32/comtools.hxx>
#include <systools/win32/uwinapi.h>
#include <sddl.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/temporary.hxx>
#include "secimpl.hxx"

namespace
{
/* To get an impersonation token we need to create an impersonation
   duplicate so every access token has to be created with duplicate
   access rights */

constexpr auto TOKEN_DUP_QUERY = TOKEN_QUERY | TOKEN_DUPLICATE;

OUString GetSpecialFolder(REFKNOWNFOLDERID rFolder)
{
    sal::systools::CoTaskMemAllocated<wchar_t> PathW;
    if (SUCCEEDED(SHGetKnownFolderPath(rFolder, KF_FLAG_CREATE, nullptr, &PathW)))
        return OUString(o3tl::toU(PathW));

    return {};
}

class SafeHandle
{
public:
    SafeHandle(HANDLE h = nullptr, bool close = true) : handle(h), needToClose(close) {}
    SafeHandle(const SafeHandle&) = delete;
    SafeHandle(SafeHandle&&) = delete;
    void operator=(const SafeHandle&) = delete;
    void operator=(SafeHandle&&) = delete;
    ~SafeHandle()
    {
        if (needToClose && handle)
            CloseHandle(handle);
    }
    operator HANDLE() const { return handle; }
    HANDLE* operator&() { return &handle; }

private:
    HANDLE handle;
    bool needToClose;
};

SafeHandle getAccessToken(const oslSecurity Security, DWORD desiredAccess = TOKEN_DUP_QUERY)
{
    if (auto* pSecImpl = static_cast<oslSecurityImpl*>(Security))
        if (pSecImpl->m_hToken)
            return SafeHandle(pSecImpl->m_hToken, false);

    HANDLE token = nullptr;
    OpenProcessToken(GetCurrentProcess(), desiredAccess, &token);
    return SafeHandle(token);
}

// We use LPCTSTR here, because we use it with SE_foo_NAME constants
// which are defined in winnt.h as UNICODE-dependent TEXT("PrivilegeName")
bool Privilege(LPCTSTR strPrivilege, bool bEnable)
{
    TOKEN_PRIVILEGES tp{ .PrivilegeCount = 1 };

    // get the luid
    if (!LookupPrivilegeValue(nullptr, strPrivilege, &tp.Privileges[0].Luid))
        return false;

    tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

    // obtain the processes token
    auto hToken = getAccessToken(nullptr, TOKEN_ADJUST_PRIVILEGES | TOKEN_DUP_QUERY);

    // enable or disable the privilege
    return hToken && AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr);
}

// Returned pointer must be free()d
PTOKEN_USER getUserTokenInfo(HANDLE hAccessToken)
{
    DWORD nInfoBuffer = 512;
    auto pInfoBuffer = static_cast<PTOKEN_USER>(malloc(nInfoBuffer));
    assert(pInfoBuffer);

    while (!GetTokenInformation(hAccessToken, TokenUser, pInfoBuffer, nInfoBuffer, &nInfoBuffer))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            if (auto p = static_cast<PTOKEN_USER>(realloc(pInfoBuffer, nInfoBuffer)))
            {
                pInfoBuffer = p;
                continue;
            }
        }

        free(pInfoBuffer);
        return nullptr;
    }

    return pInfoBuffer;
}

bool getUserNameFromWNet(rtl_uString** strName)
{
    DWORD needed = 0;
    WNetGetUserW(nullptr, nullptr, &needed);

    if (needed > 1 && needed <= o3tl::make_unsigned(SAL_MAX_INT32))
    {
        // OUStringBuffer has space for an extra zero character
        if (OUStringBuffer buffer(needed - 1);
            WNetGetUserW(nullptr, o3tl::toW(buffer.appendUninitialized(needed - 1)), &needed)
            == NO_ERROR)
        {
            rtl_uString_assign(strName, buffer.makeStringAndClear().pData);
            return true;
        }
    }
    return false;
}

bool getUserNameImpl(oslSecurity Security, rtl_uString** strName, bool bIncludeDomain)
{
    if (!Security)
        return false;

    if (auto hAccessToken = getAccessToken(Security))
    {
        PTOKEN_USER pInfoBuffer = getUserTokenInfo(hAccessToken);
        if (!pInfoBuffer)
            return false;

        sal_Unicode UserName[256];
        sal_Unicode DomainName[256];
        DWORD nUserName = std::size(UserName);
        DWORD nDomainName = std::size(DomainName);

        bool bResult = LookupAccountSidW(nullptr, pInfoBuffer->User.Sid, o3tl::toW(UserName),
                                         &nUserName, o3tl::toW(DomainName), &nDomainName,
                                         &o3tl::temporary(SID_NAME_USE()));
        free(pInfoBuffer);
        if (!bResult)
            return false;

        OUString aResult
            = bIncludeDomain ? OUString::Concat(DomainName) + "/" + UserName : OUString(UserName);
        rtl_uString_assign(strName, aResult.pData);
        return true;
    }

    if (getUserNameFromWNet(strName))
        return true;

    if (auto pSecImpl = static_cast<oslSecurityImpl*>(Security); pSecImpl->m_User[0] != '\0')
    {
        rtl_uString_newFromStr(strName, o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName));
        return true;
    }

    return false;
}
} // namespace

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

    wchar_t* strDomain = _wcsdup(o3tl::toW(rtl_uString_getStr(strUserName)));
    wchar_t* strUser = wcschr(strDomain, L'/');

    if (strUser)
        *strUser++ = L'\0';
    else
    {
        strUser   = strDomain;
        strDomain = nullptr;
    }

    // this process must have the right: 'act as a part of operatingsystem'
    OSL_ASSERT(LookupPrivilegeValue(nullptr, SE_TCB_NAME, &o3tl::temporary(LUID())));

    HANDLE  hUserToken;
    if (LogonUserW(strUser, strDomain ? strDomain : L"", o3tl::toW(rtl_uString_getStr(strPasswd)),
                  LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &hUserToken))
    {
        oslSecurityImpl* pSecImpl = static_cast<oslSecurityImpl *>(malloc(sizeof(oslSecurityImpl)));
        if (pSecImpl)
        {
            pSecImpl->m_pNetResource = nullptr;
            pSecImpl->m_hToken = hUserToken;
            pSecImpl->m_hProfile = nullptr;
            wcscpy(o3tl::toW(pSecImpl->m_User), strUser);
        }
        else
            CloseHandle(hUserToken);
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
    OUString remoteName
        = "\\\\" + OUString::unacquired(&strFileServer) + "\\" + OUString::unacquired(&strUserName);
    OUString userName
        = OUString::unacquired(&strFileServer) + "\\" + OUString::unacquired(&strUserName);

    NETRESOURCEW netResource{ .dwScope = RESOURCE_GLOBALNET,
                              .dwType = RESOURCETYPE_DISK,
                              .dwDisplayType = RESOURCEDISPLAYTYPE_SHARE,
                              .dwUsage = RESOURCEUSAGE_CONNECTABLE,
                              .lpRemoteName = o3tl::toW(rtl_uString_getStr(remoteName.pData)) };

    DWORD err = WNetAddConnection2W(&netResource, o3tl::toW(rtl_uString_getStr(strPasswd)),
                                    o3tl::toW(userName.getStr()), 0);

    if ((err == NO_ERROR) || (err == ERROR_ALREADY_ASSIGNED))
    {
        // Allocate it all in one memory block
        const size_t implSize = sizeof(oslSecurityImpl) + sizeof(NETRESOURCEW)
                                + (remoteName.getLength() + 1) * sizeof(wchar_t);
        if (oslSecurityImpl* pSecImpl = static_cast<oslSecurityImpl*>(malloc(implSize)))
        {
            pSecImpl->m_pNetResource = reinterpret_cast<NETRESOURCEW*>(pSecImpl + 1);
            *pSecImpl->m_pNetResource = netResource;
            pSecImpl->m_pNetResource->lpRemoteName
                = reinterpret_cast<wchar_t*>(pSecImpl->m_pNetResource + 1);
            wcscpy(pSecImpl->m_pNetResource->lpRemoteName, o3tl::toW(remoteName.getStr()));
            pSecImpl->m_hToken = nullptr;
            pSecImpl->m_hProfile = nullptr;
            assert(o3tl::make_unsigned(strUserName->length) < std::size(pSecImpl->m_User));
            wcscpy(o3tl::toW(pSecImpl->m_User), o3tl::toW(rtl_uString_getStr(strUserName)));

            *pSecurity = pSecImpl;
            return osl_Security_E_None;
        }
    }

    return osl_Security_E_UserUnknown;
}

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    if (!Security)
        return false;

    /* If Security contains an access token we need to duplicate it to an impersonation
       access token. NULL works with CheckTokenMembership() as the current effective
       impersonation token
     */

    SafeHandle hImpersonationToken;
    if (HANDLE hToken = static_cast<oslSecurityImpl*>(Security)->m_hToken)
    {
        if (!DuplicateToken(hToken, SecurityImpersonation, &hImpersonationToken))
            return false;
    }

    /* CheckTokenMembership() can be used on W2K and higher (NT4 no longer supported by OOo)
       and also works on Vista to retrieve the effective user rights. Just checking for
       membership of Administrators group is not enough on Vista this would require additional
       complicated checks as described in KB article Q118626: http://support.microsoft.com/kb/118626/en-us
    */

    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = { SECURITY_NT_AUTHORITY };
    bool bSuccess = false;
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

    return bSuccess;
}

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    if (!Security)
        return;

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

    if (pSecImpl->m_pNetResource != nullptr)
        WNetCancelConnection2W(pSecImpl->m_pNetResource->lpRemoteName, 0, true);

    if (pSecImpl->m_hToken)
        CloseHandle(pSecImpl->m_hToken);

    if ( pSecImpl->m_hProfile )
        CloseHandle(pSecImpl->m_hProfile);

    free (pSecImpl);
}

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **strIdent)
{
    if (!Security)
       return false;

    if (auto hAccessToken = getAccessToken(Security))
    {
        if (PTOKEN_USER pInfoBuffer = getUserTokenInfo(hAccessToken))
        {
            LPWSTR pSidStr = nullptr;
            bool bResult = ConvertSidToStringSidW(pInfoBuffer->User.Sid, &pSidStr);
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

            return bResult;
        }
    }
    else
    {
        if (!getUserNameFromWNet(strIdent))
            rtl_uString_newFromStr(strIdent, u"unknown");
        return true;
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
    if (!Security)
        return false;

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

    OUString aSysDir = pSecImpl->m_pNetResource
                           ? OUString(o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName))
                           : GetSpecialFolder(FOLDERID_Documents);

    return !aSysDir.isEmpty()
           && osl_File_E_None == osl_getFileURLFromSystemPath(aSysDir.pData, pustrDirectory);
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    if (!Security)
        return false;

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl*>(Security);

    if (pSecImpl->m_pNetResource != nullptr)
    {
        OUString aSysDir(o3tl::toU(pSecImpl->m_pNetResource->lpRemoteName));
        return osl_File_E_None == osl_getFileURLFromSystemPath(aSysDir.pData, pustrDirectory);
    }

    if (pSecImpl->m_hToken)
    {
        /* not implemented */
        OSL_ASSERT(false);
    }
    else
    {
        OUString aFile = GetSpecialFolder(FOLDERID_RoamingAppData);
        if (aFile.isEmpty())
        {
            sal_Unicode sFile[_MAX_PATH];
            OSL_VERIFY(GetWindowsDirectoryW(o3tl::toW(sFile), std::size(sFile)) > 0);
            aFile = sFile;
        }

        return osl_File_E_None == osl_getFileURLFromSystemPath(aFile.pData, pustrDirectory);
    }

    return false;
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

    RegCloseKey(HKEY_CURRENT_USER);

    if (!Privilege(SE_RESTORE_NAME, true))
        return false;

    auto hAccessToken = getAccessToken(Security, TOKEN_IMPERSONATE);

    /* try to create user profile */

    OUString buffer;
    getUserNameImpl(Security, &buffer.pData, false);

    PROFILEINFOW pi{ .dwSize = sizeof(pi),
                     .dwFlags = PI_NOUI,
                     .lpUserName = o3tl::toW(rtl_uString_getStr(buffer.pData)) };

    if (LoadUserProfileW(hAccessToken, &pi))
    {
        UnloadUserProfile(hAccessToken, pi.hProfile);
        return true;
    }

    return false;
}

void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    if ( static_cast<oslSecurityImpl*>(Security)->m_hProfile == nullptr )
        return;

    auto hAccessToken = getAccessToken(Security, TOKEN_IMPERSONATE);

    /* unloading the user profile */
    UnloadUserProfile(hAccessToken, static_cast<oslSecurityImpl*>(Security)->m_hProfile);

    static_cast<oslSecurityImpl*>(Security)->m_hProfile = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
