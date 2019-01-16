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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>

#ifdef IOS
#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif

#include "system.hxx"

#include <osl/security.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

#include <osl/thread.h>
#include <osl/file.h>

#if defined LINUX || defined __sun
#include <crypt.h>
#endif

#if defined HAIKU
#include <fs_info.h>
#include <FindDirectory.h>
#endif

#include "secimpl.hxx"

#ifdef ANDROID
#define getpwuid_r(uid, pwd, buf, buflen, result) (*(result) = getpwuid(uid), (*(result) ? (memcpy (buf, *(result), sizeof (struct passwd)), 0) : errno))
#endif

static bool osl_psz_getHomeDir(oslSecurity Security, OString* pszDirectory);
static bool osl_psz_getConfigDir(oslSecurity Security, OString* pszDirectory);

static bool sysconf_SC_GETPW_R_SIZE_MAX(std::size_t * value) {
#if defined _SC_GETPW_R_SIZE_MAX
    long m;
    errno = 0;
    m = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (m == -1) {
        /* _SC_GETPW_R_SIZE_MAX has no limit; some platforms like certain
           FreeBSD versions support sysconf(_SC_GETPW_R_SIZE_MAX) in a broken
           way and always set EINVAL, so be resilient here: */
        return false;
    }
    SAL_WARN_IF( m < 0 || static_cast<unsigned long>(m) >= std::numeric_limits<std::size_t>::max(), "sal.osl",
                "m < 0 || (unsigned long) m >= std::numeric_limits<std::size_t>::max()");
    *value = static_cast<std::size_t>(m);
    return true;
#else
    /* some platforms like macOS 1.3 do not define _SC_GETPW_R_SIZE_MAX: */
    return false;
#endif
}

static oslSecurityImpl * growSecurityImpl(
    oslSecurityImpl * impl, std::size_t * bufSize)
{
    std::size_t n = 0;
    oslSecurityImpl * p = nullptr;
    if (impl == nullptr) {
        if (!sysconf_SC_GETPW_R_SIZE_MAX(&n)) {
            /* choose something sensible (the callers of growSecurityImpl will
               detect it if the allocated buffer is too small: */
            n = 1024;
        }
    } else if (*bufSize <= std::numeric_limits<std::size_t>::max() / 2) {
        n = 2 * *bufSize;
    }
    if (n != 0) {
        if (n <= std::numeric_limits<std::size_t>::max()
            - offsetof(oslSecurityImpl, m_buffer))
        {
            *bufSize = n;
            n += offsetof(oslSecurityImpl, m_buffer);
        } else {
            *bufSize = std::numeric_limits<std::size_t>::max()
                - offsetof(oslSecurityImpl, m_buffer);
            n = std::numeric_limits<std::size_t>::max();
        }
        p = static_cast<oslSecurityImpl *>(realloc(impl, n));
        memset (p, 0, n);
    }
    if (p == nullptr) {
        free(impl);
    }
    return p;
}

static void deleteSecurityImpl(oslSecurityImpl * impl) {
    free(impl);
}

oslSecurity SAL_CALL osl_getCurrentSecurity()
{
    std::size_t n = 0;
    oslSecurityImpl * p = nullptr;
    for (;;) {
        struct passwd * found;
        p = growSecurityImpl(p, &n);
        if (p == nullptr) {
            return nullptr;
        }
        switch (getpwuid_r(getuid(), &p->m_pPasswd, p->m_buffer, n, &found)) {
        case ERANGE:
            break;
        case 0:
            if (found != nullptr) {
                return p;
            }
            [[fallthrough]];
        default:
            deleteSecurityImpl(p);
            return nullptr;
        }
    }
}

oslSecurityError SAL_CALL osl_loginUser(
    SAL_UNUSED_PARAMETER rtl_uString *,
    SAL_UNUSED_PARAMETER rtl_uString *,
    SAL_UNUSED_PARAMETER oslSecurity *
    )
{
    return osl_Security_E_None;
}

oslSecurityError SAL_CALL osl_loginUserOnFileServer(
    SAL_UNUSED_PARAMETER rtl_uString *,
    SAL_UNUSED_PARAMETER rtl_uString *,
    SAL_UNUSED_PARAMETER rtl_uString *,
    SAL_UNUSED_PARAMETER oslSecurity *
    )
{
    return osl_Security_E_UserUnknown;
}

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **ustrIdent)
{
    bool     bRet = false;
    sal_Char pszIdent[1024];

    pszIdent[0] = '\0';

    bRet = osl_psz_getUserIdent(Security,pszIdent,sizeof(pszIdent));

    rtl_string2UString( ustrIdent, pszIdent, rtl_str_getLength( pszIdent ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    SAL_WARN_IF(*ustrIdent == nullptr, "sal.osl", "*ustrIdent == NULL");

    return bRet;
}

bool osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax)
{
    sal_Char  buffer[32];
    sal_Int32 nChr;

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr)
        return false;

    nChr = snprintf(buffer, sizeof(buffer), "%u", pSecImpl->m_pPasswd.pw_uid);
    if ( nChr < 0 || sal::static_int_cast<sal_uInt32>(nChr) >= sizeof(buffer)
         || sal::static_int_cast<sal_uInt32>(nChr) >= nMax )
        return false; /* leave *pszIdent unmodified in case of failure */

    memcpy(pszIdent, buffer, nChr+1);
    return true;
}

sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **ustrName)
{
    bool     bRet = false;
    sal_Char * pszName;
    sal_Int32 len;

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl != nullptr && pSecImpl->m_pPasswd.pw_name != nullptr) {
        pszName = pSecImpl->m_pPasswd.pw_name;
        auto const n = std::strlen(pszName);
        if (n <= sal_uInt32(std::numeric_limits<sal_Int32>::max())) {
            len = n;
            bRet = true;
        }
    }

    if (!bRet) {
        pszName = nullptr;
        len = 0;
    }

    rtl_string2UString( ustrName, pszName, len, osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    SAL_WARN_IF(*ustrName == nullptr, "sal.osl", "ustrName == NULL");

    return bRet;
}

sal_Bool SAL_CALL osl_getShortUserName(oslSecurity Security, rtl_uString **ustrName)
{
    return osl_getUserName(Security, ustrName); // No domain name on unix
}

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    bool     bRet = false;
    OString pszDirectory;

    bRet = osl_psz_getHomeDir(Security,&pszDirectory);

    if ( bRet )
    {
        rtl_string2UString( pustrDirectory, pszDirectory.getStr(), pszDirectory.getLength(), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        SAL_WARN_IF(*pustrDirectory == nullptr, "sal.osl", "*pustrDirectory == NULL");
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

static bool osl_psz_getHomeDir(oslSecurity Security, OString* pszDirectory)
{
    assert(pszDirectory != nullptr);

    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr)
        return false;

#ifdef HAIKU
    dev_t volume = dev_for_path("/boot");
    sal_Char homeDir[B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];
    status_t result = find_directory(B_USER_DIRECTORY, volume, false, homeDir,
                                     sizeof(homeDir));
    if (result == B_OK) {
        static_assert(
            B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH <= std::numeric_limits<sal_Int32>::max());
        *pszDirectory = OString(homeDir, std::strlen(homeDir));
        return true;
    }
    return false;
#endif

#ifdef ANDROID
{
    OUString pValue;

    if (rtl::Bootstrap::get("HOME", pValue))
    {
        auto const pStrValue = OUStringToOString(pValue, RTL_TEXTENCODING_UTF8);
        if (!pStrValue.isEmpty())
        {
            *pszDirectory = pStrValue;
            return true;
        }
    }
}
#endif

#ifdef IOS
    {
        // Let's pretend the app-specific "Documents" directory is the home directory for now
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *userDirectory = [paths objectAtIndex:0];
        auto const len = [userDirectory length];
        if (len <= std::numeric_limits<sal_Int32>::max())
        {
            *pszDirectory = OString([userDirectory UTF8String], len);
            return sal_True;
        }
    }
#endif

    /* if current user, check also environment for HOME */
    if (getuid() == pSecImpl->m_pPasswd.pw_uid)
    {
        sal_Char *pStr = nullptr;
#ifdef __sun
        char    buffer[8192];

        struct passwd pwd;
        struct passwd *ppwd;

#ifdef _POSIX_PTHREAD_SEMANTICS
        if ( 0 != getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer), &ppwd ) )
            ppwd = NULL;
#else
        ppwd = getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer) );
#endif

        if ( ppwd )
            pStr = ppwd->pw_dir;
#else
        pStr = getenv("HOME");
#endif

        if (pStr != nullptr && pStr[0] != '\0' && access(pStr, 0) == 0)
        {
            auto const len = std::strlen(pStr);
            if (len > sal_uInt32(std::numeric_limits<sal_Int32>::max())) {
                return false;
            }
            *pszDirectory = OString(pStr, len);
            return true;
        }
    }
    if (pSecImpl->m_pPasswd.pw_dir != nullptr)
    {
        auto const len = std::strlen(pSecImpl->m_pPasswd.pw_dir);
        if (len > sal_uInt32(std::numeric_limits<sal_Int32>::max())) {
            return false;
        }
        *pszDirectory = OString(pSecImpl->m_pPasswd.pw_dir, len);
    }
    else
        return false;

    return true;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    bool     bRet = false;
    OString pszDirectory;

    bRet = osl_psz_getConfigDir(Security,&pszDirectory);

    if ( bRet )
    {
        rtl_string2UString( pustrDirectory, pszDirectory.getStr(), pszDirectory.getLength(), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        SAL_WARN_IF(*pustrDirectory == nullptr, "sal.osl", "*pustrDirectory == NULL");
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

#if defined HAIKU

static bool osl_psz_getConfigDir(oslSecurity Security, OString* pszDirectory)
{
    assert(pszDirectory != nullptr);
    (void) Security;
    dev_t volume = dev_for_path("/boot");
    sal_Char configDir[B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];
    status_t result = find_directory(B_USER_SETTINGS_DIRECTORY, volume, false,
                                     configDir, sizeof(configDir));
    if (result == B_OK) {
        auto const len = strlen(configDir);
        if (len <= sal_uInt32(std::numeric_limits<sal_Int32>::max())) {
            *pszDirectory = OString(configDir, len);
            return true;
        }
    }
    return false;
}

#elif !defined(MACOSX) && !defined(IOS)

static bool osl_psz_getConfigDir(oslSecurity Security, OString* pszDirectory)
{
    assert(pszDirectory != nullptr);

    sal_Char *pStr = getenv("XDG_CONFIG_HOME");

    if (pStr == nullptr || pStr[0] == '\0' || access(pStr, 0) != 0)
    {
        // a default equal to $HOME/.config should be used.
        OString home;
        if (!osl_psz_getHomeDir(Security, &home))
            return false;
        auto const config = OString(home + "/.config");

        // try to create dir if not present
        bool dirOK = true;
        if (mkdir(config.getStr(), S_IRWXU) != 0)
        {
            int e = errno;
            if (e != EEXIST)
            {
                SAL_WARN(
                    "sal.osl",
                    "mkdir(" << config << "): errno=" << e);
                dirOK = false;
            }
        }
        if (dirOK)
        {
            // check file type and permissions
            struct stat st;
            if (stat(config.getStr(), &st) != 0)
            {
                SAL_INFO("sal.osl","Could not stat $HOME/.config");
                dirOK = false;
            }
            else
            {
                if (!S_ISDIR(st.st_mode))
                {
                    SAL_INFO("sal.osl", "$HOME/.config is not a directory");
                    dirOK = false;
                }
                if (!(st.st_mode & S_IRUSR && st.st_mode & S_IWUSR && st.st_mode & S_IXUSR))
                {
                    SAL_INFO("sal.osl", "$HOME/.config has bad permissions");
                    dirOK = false;
                }
            }
        }

        // if !dirOK, resort to HOME
        if (dirOK)
            home = config;
        *pszDirectory = home;
    }
    else
    {
        auto const len = std::strlen(pStr);
        if (len > sal_uInt32(std::numeric_limits<sal_Int32>::max())) {
            return false;
        }
        *pszDirectory = OString(pStr, len);
    }

    return true;
}

#else

/*
 * FIXME: rewrite to use more flexible
 * NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES)
 * as soon as we can bump the baseline to Tiger (for NSApplicationSupportDirectory) and have
 * support for Objective-C in the build environment
 */

static bool osl_psz_getConfigDir(oslSecurity Security, OString* pszDirectory)
{
    assert(pszDirectory != nullptr);

    OString home;
    if( osl_psz_getHomeDir(Security, &home) )
    {
        *pszDirectory = home + "/Library/Application Support"; /* Used on iOS, too */
        return true;
    }

    return false;
}

#endif

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr)
        return false;

    if (pSecImpl->m_pPasswd.pw_uid != 0)
        return false;

    return true;
}

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    deleteSecurityImpl(static_cast<oslSecurityImpl *>(Security));
}

sal_Bool SAL_CALL osl_loadUserProfile(SAL_UNUSED_PARAMETER oslSecurity)
{
    return false;
}

void SAL_CALL osl_unloadUserProfile(SAL_UNUSED_PARAMETER oslSecurity) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
