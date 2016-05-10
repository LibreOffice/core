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

#include <cstddef>
#include <limits>

#ifdef IOS
#include <premac.h>
#import <Foundation/Foundation.h>
#include <postmac.h>
#endif

#include "system.hxx"

#include <osl/security.h>
#include <osl/diagnose.h>
#include <rtl/bootstrap.h>
#include <sal/log.hxx>

#include "osl/thread.h"
#include "osl/file.h"

#if defined LINUX || defined SOLARIS
#include <crypt.h>
#endif

#include "secimpl.hxx"

#ifdef ANDROID
#define getpwuid_r(uid, pwd, buf, buflen, result) (*(result) = getpwuid(uid), (*(result) ? (memcpy (buf, *(result), sizeof (struct passwd)), 0) : errno))
#endif

static oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
                  oslSecurity* pSecurity);
static bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax);
static bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
static bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);

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
    } else {
        SAL_WARN_IF( m < 0 || (unsigned long) m >= std::numeric_limits<std::size_t>::max(), "sal.osl", 
                "m < 0 || (unsigned long) m >= std::numeric_limits<std::size_t>::max()");
        *value = (std::size_t) m;
        return true;
    }
#else
    /* some platforms like Mac OS X 1.3 do not define _SC_GETPW_R_SIZE_MAX: */
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
            SAL_FALLTHROUGH;
        default:
            deleteSecurityImpl(p);
            return nullptr;
        }
    }
}

oslSecurityError SAL_CALL osl_loginUser(
    rtl_uString *ustrUserName,
    rtl_uString *ustrPassword,
    oslSecurity *pSecurity
    )
{
    oslSecurityError Error;
    rtl_String* strUserName=nullptr;
    rtl_String* strPassword=nullptr;
    sal_Char* pszUserName=nullptr;
    sal_Char* pszPassword=nullptr;

    if ( ustrUserName != nullptr )
    {
        rtl_uString2String( &strUserName,
                            rtl_uString_getStr(ustrUserName),
                            rtl_uString_getLength(ustrUserName),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszUserName = rtl_string_getStr(strUserName);
    }

    if ( ustrPassword != nullptr )
    {
        rtl_uString2String( &strPassword,
                            rtl_uString_getStr(ustrPassword),
                            rtl_uString_getLength(ustrPassword),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszPassword = rtl_string_getStr(strPassword);
    }

    Error=osl_psz_loginUser(pszUserName,pszPassword,pSecurity);

    if ( strUserName != nullptr )
    {
        rtl_string_release(strUserName);
    }

    if ( strPassword)
    {
        rtl_string_release(strPassword);
    }

    return Error;
}

static oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
               oslSecurity* pSecurity)
{
    (void)pszUserName;
    (void)pszPasswd;
    (void)pSecurity;

    return osl_Security_E_None;
}

oslSecurityError SAL_CALL osl_loginUserOnFileServer(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    rtl_uString *strFileServer,
    oslSecurity *pSecurity
    )
{
    (void) strUserName; /* unused */
    (void) strPasswd; /* unused */
    (void) strFileServer; /* unused */
    (void) pSecurity; /* unused */
    return osl_Security_E_UserUnknown;
}

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **ustrIdent)
{
    bool     bRet = false;
    sal_Char pszIdent[1024];

    pszIdent[0] = '\0';

    bRet = osl_psz_getUserIdent(Security,pszIdent,sizeof(pszIdent));

    rtl_string2UString( ustrIdent, pszIdent, rtl_str_getLength( pszIdent ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
    SAL_WARN_IF(*ustrIdent == nullptr, "sal.osl", "*ustrIdent == NULL");

    return bRet;
}

bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax)
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
    sal_Char pszName[1024];

    pszName[0] = '\0';

    bRet = osl_psz_getUserName(Security,pszName,sizeof(pszName));

    rtl_string2UString( ustrName, pszName, rtl_str_getLength( pszName ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
    SAL_WARN_IF(*ustrName == nullptr, "sal.osl", "ustrName == NULL");

    return bRet;
}

static bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax)
{
    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr || pSecImpl->m_pPasswd.pw_name == nullptr)
        return false;

    strncpy(pszName, pSecImpl->m_pPasswd.pw_name, nMax);

    return true;
}

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    bool     bRet = false;
    sal_Char pszDirectory[PATH_MAX];

    pszDirectory[0] = '\0';

    bRet = osl_psz_getHomeDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet )
    {
        rtl_string2UString( pustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
        SAL_WARN_IF(*pustrDirectory == nullptr, "sal.osl", "*pustrDirectory == NULL");
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

static bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr)
        return false;

#ifdef ANDROID
{
    sal_Bool bRet = sal_False;
    rtl_uString *pName = 0, *pValue = 0;

    rtl_uString_newFromAscii(&pName, "HOME");

    if (rtl_bootstrap_get(pName, &pValue, NULL))
    {
        rtl_String *pStrValue = 0;
        if (pValue && pValue->length > 0)
        {
            rtl_uString2String(&pStrValue, pValue->buffer,
                               pValue->length, RTL_TEXTENCODING_UTF8,
                               OUSTRING_TO_OSTRING_CVTFLAGS);
            if (pStrValue && pStrValue->length > 0)
            {
                sal_Int32 nCopy = (sal_Int32)(nMax-1) < pStrValue->length ? (sal_Int32)(nMax-1) : pStrValue->length ;
                strncpy (pszDirectory, pStrValue->buffer, nCopy);
                pszDirectory[nCopy] = '\0';
                bRet = (std::size_t)pStrValue->length < nMax;
            }
            rtl_string_release(pStrValue);
        }
        rtl_uString_release(pName);
    }
    if (bRet)
        return bRet;
}
#endif

#ifdef IOS
    {
        // Let's pretend the app-specific "Documents" directory is the home directory for now
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *userDirectory = [paths objectAtIndex:0];
        if ([userDirectory length] < nMax)
        {
            strcpy(pszDirectory, [userDirectory UTF8String]);
            return sal_True;
        }
    }
#endif

    /* if current user, check also environment for HOME */
    if (getuid() == pSecImpl->m_pPasswd.pw_uid)
    {
        sal_Char *pStr = nullptr;
#ifdef SOLARIS
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

        if (pStr != nullptr && strlen(pStr) > 0 && access(pStr, 0) == 0)
            strncpy(pszDirectory, pStr, nMax);
        else if (pSecImpl->m_pPasswd.pw_dir != nullptr)
            strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
        else
            return false;
    }
    else if (pSecImpl->m_pPasswd.pw_dir != nullptr)
        strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
    else
        return false;

    return true;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    bool     bRet = false;
    sal_Char pszDirectory[PATH_MAX];

    pszDirectory[0] = '\0';

    bRet = osl_psz_getConfigDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet )
    {
        rtl_string2UString( pustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
        SAL_WARN_IF(*pustrDirectory == nullptr, "sal.osl", "*pustrDirectory == NULL");
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

#if !defined(MACOSX) && !defined(IOS)

#define DOT_CONFIG "/.config"

static bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    sal_Char *pStr = getenv("XDG_CONFIG_HOME");

    if (pStr == nullptr || strlen(pStr) == 0 || access(pStr, 0) != 0)
    {
        std::size_t n = 0;

        // a default equal to $HOME/.config should be used.
        if (!osl_psz_getHomeDir(Security, pszDirectory, nMax))
            return false;
        n = strlen(pszDirectory);
        if (n + sizeof(DOT_CONFIG) < nMax)
        {
            strncpy(pszDirectory+n, DOT_CONFIG, sizeof(DOT_CONFIG));

            // try to create dir if not present
            bool dirOK = true;
            if (mkdir(pszDirectory, S_IRWXU) != 0)
            {
                int e = errno;
                if (e != EEXIST)
                {
                    SAL_WARN(
                        "sal.osl",
                        "mkdir(" << pszDirectory << "): errno=" << e);
                    dirOK = false;
                }
            }
            if (dirOK)
            {
                // check file type and permissions
                struct stat st;
                if (stat(pszDirectory, &st) != 0)
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

            // resort to HOME
            if (!dirOK)
                pszDirectory[n] = '\0';
        }
    }
    else
        strncpy(pszDirectory, pStr, nMax);

    return true;
}

#undef DOT_CONFIG

#else

/*
 * FIXME: rewrite to use more flexible
 * NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES)
 * as soon as we can bumb the baseline to Tiger (for NSApplicationSupportDirectory) and have
 * support for Objective-C in the build environment
 */

#define MACOSX_CONFIG_DIR "/Library/Application Support" /* Used on iOS, too */
static bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    if( osl_psz_getHomeDir(Security, pszDirectory, nMax - sizeof(MACOSX_CONFIG_DIR) + 1) )
    {
        strcat( pszDirectory, MACOSX_CONFIG_DIR );
        return true;
    }

    return false;
}

#endif

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    oslSecurityImpl *pSecImpl = static_cast<oslSecurityImpl *>(Security);

    if (pSecImpl == nullptr)
        return sal_False;

    if (pSecImpl->m_pPasswd.pw_uid != 0)
        return sal_False;

    return sal_True;
}

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    deleteSecurityImpl(static_cast<oslSecurityImpl *>(Security));
}

sal_Bool SAL_CALL osl_loadUserProfile(oslSecurity Security)
{
    (void) Security; /* unused */
    return sal_False;
}

void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    (void) Security; /* unused */
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
