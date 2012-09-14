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

#include <stddef.h>

/* Solaris 8 has no C99 stdint.h, and Solaris generally seems not to miss it for
   SIZE_MAX: */
#if !defined __SUNPRO_C
#include <stdint.h>
#endif

#include "system.h"

#include <osl/security.h>
#include <osl/diagnose.h>
#include <rtl/bootstrap.h>

#include "osl/thread.h"
#include "osl/file.h"

#if defined LINUX || defined SOLARIS
#include <crypt.h>
#endif

#include "secimpl.h"

#ifdef ANDROID
#define getpwuid_r(uid, pwd, buf, buflen, result) (*(result) = getpwuid(uid), (*(result) ? (memcpy (buf, *(result), sizeof (struct passwd)), 0) : errno))
#endif


static oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
                  oslSecurity* pSecurity);
sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax);
static sal_Bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax);
static sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
static sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);

static sal_Bool sysconf_SC_GETPW_R_SIZE_MAX(size_t * value) {
#if defined _SC_GETPW_R_SIZE_MAX
    long m;
    errno = 0;
    m = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (m == -1) {
        /* _SC_GETPW_R_SIZE_MAX has no limit; some platforms like certain
           FreeBSD versions support sysconf(_SC_GETPW_R_SIZE_MAX) in a broken
           way and always set EINVAL, so be resilient here: */
        return sal_False;
    } else {
        OSL_ASSERT(m >= 0 && (unsigned long) m < SIZE_MAX);
        *value = (size_t) m;
        return sal_True;
    }
#else
    /* some platforms like Mac OS X 1.3 do not define _SC_GETPW_R_SIZE_MAX: */
    return sal_False;
#endif
}

static oslSecurityImpl * growSecurityImpl(
    oslSecurityImpl * impl, size_t * bufSize)
{
    size_t n = 0;
    oslSecurityImpl * p = NULL;
    if (impl == NULL) {
        if (!sysconf_SC_GETPW_R_SIZE_MAX(&n)) {
            /* choose something sensible (the callers of growSecurityImpl will
               detect it if the allocated buffer is too small: */
            n = 1024;
        }
    } else if (*bufSize <= SIZE_MAX / 2) {
        n = 2 * *bufSize;
    }
    if (n != 0) {
        if (n <= SIZE_MAX - offsetof(oslSecurityImpl, m_buffer)) {
            *bufSize = n;
            n += offsetof(oslSecurityImpl, m_buffer);
        } else {
            *bufSize = SIZE_MAX - offsetof(oslSecurityImpl, m_buffer);
            n = SIZE_MAX;
        }
        p = realloc(impl, n);
        memset (p, 0, n);
    }
    if (p == NULL) {
        free(impl);
    }
    return p;
}

static void deleteSecurityImpl(oslSecurityImpl * impl) {
    free(impl);
}

oslSecurity SAL_CALL osl_getCurrentSecurity()
{
    size_t n = 0;
    oslSecurityImpl * p = NULL;
    for (;;) {
        struct passwd * found;
        p = growSecurityImpl(p, &n);
        if (p == NULL) {
            return NULL;
        }
        switch (getpwuid_r(getuid(), &p->m_pPasswd, p->m_buffer, n, &found)) {
        case ERANGE:
            break;
        case 0:
            if (found != NULL) {
                return p;
            }
            /* fall through */
        default:
            deleteSecurityImpl(p);
            return NULL;
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
    rtl_String* strUserName=0;
    rtl_String* strPassword=0;
    sal_Char* pszUserName=0;
    sal_Char* pszPassword=0;

    if ( ustrUserName != 0 )
    {
        rtl_uString2String( &strUserName,
                            rtl_uString_getStr(ustrUserName),
                            rtl_uString_getLength(ustrUserName),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszUserName = rtl_string_getStr(strUserName);
    }


    if ( ustrPassword != 0 )
    {
        rtl_uString2String( &strPassword,
                            rtl_uString_getStr(ustrPassword),
                            rtl_uString_getLength(ustrPassword),
                            RTL_TEXTENCODING_UTF8,
                            OUSTRING_TO_OSTRING_CVTFLAGS );
        pszPassword = rtl_string_getStr(strPassword);
    }


    Error=osl_psz_loginUser(pszUserName,pszPassword,pSecurity);

    if ( strUserName != 0 )
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
    sal_Bool bRet=sal_False;
    sal_Char pszIdent[1024];

    pszIdent[0] = '\0';

    bRet = osl_psz_getUserIdent(Security,pszIdent,sizeof(pszIdent));

    rtl_string2UString( ustrIdent, pszIdent, rtl_str_getLength( pszIdent ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
    OSL_ASSERT(*ustrIdent != NULL);

    return bRet;
}


sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax)
{
    sal_Char  buffer[32];
    sal_Int32 nChr;

    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if (pSecImpl == NULL)
        return sal_False;

    nChr = snprintf(buffer, sizeof(buffer), "%u", pSecImpl->m_pPasswd.pw_uid);
    if ( nChr < 0 || SAL_INT_CAST(sal_uInt32, nChr) >= sizeof(buffer)
         || SAL_INT_CAST(sal_uInt32, nChr) >= nMax )
        return sal_False; /* leave *pszIdent unmodified in case of failure */

    memcpy(pszIdent, buffer, nChr+1);
    return sal_True;
}

sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **ustrName)
{
    sal_Bool bRet=sal_False;
    sal_Char pszName[1024];

    pszName[0] = '\0';

    bRet = osl_psz_getUserName(Security,pszName,sizeof(pszName));

    rtl_string2UString( ustrName, pszName, rtl_str_getLength( pszName ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
    OSL_ASSERT(*ustrName != NULL);

    return bRet;
}



static sal_Bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax)
{
    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if (pSecImpl == NULL || pSecImpl->m_pPasswd.pw_name == NULL)
        return sal_False;

    strncpy(pszName, pSecImpl->m_pPasswd.pw_name, nMax);

    return sal_True;
}

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    sal_Bool bRet=sal_False;
    sal_Char pszDirectory[PATH_MAX];

    pszDirectory[0] = '\0';

    bRet = osl_psz_getHomeDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet == sal_True )
    {
        rtl_string2UString( pustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
        OSL_ASSERT(*pustrDirectory != NULL);
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

static sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if (pSecImpl == NULL)
        return sal_False;

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
                bRet = (size_t)pStrValue->length < nMax;
            }
            rtl_string_release(pStrValue);
        }
        rtl_uString_release(pName);
    }
    if (bRet)
        return bRet;
}
#endif

    /* if current user, check also environment for HOME */
    if (getuid() == pSecImpl->m_pPasswd.pw_uid)
    {
        sal_Char *pStr = NULL;
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

        if (pStr != NULL && strlen(pStr) > 0 && access(pStr, 0) == 0)
            strncpy(pszDirectory, pStr, nMax);
        else if (pSecImpl->m_pPasswd.pw_dir != NULL)
            strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
        else
            return sal_False;
    }
    else if (pSecImpl->m_pPasswd.pw_dir != NULL)
        strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
    else
        return sal_False;

    return sal_True;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **pustrDirectory)
{
    sal_Bool bRet = sal_False;
    sal_Char pszDirectory[PATH_MAX];

    pszDirectory[0] = '\0';

    bRet = osl_psz_getConfigDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet == sal_True )
    {
        rtl_string2UString( pustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
        OSL_ASSERT(*pustrDirectory != NULL);
        osl_getFileURLFromSystemPath( *pustrDirectory, pustrDirectory );
    }

    return bRet;
}

#ifndef MACOSX

#define DOT_CONFIG "/.config"

static sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    sal_Char *pStr = getenv("XDG_CONFIG_HOME");

    if (pStr == NULL || strlen(pStr) == 0 || access(pStr, 0) != 0)
    {
        size_t n = 0;
        // a default equal to $HOME/.config should be used.
        if (!osl_psz_getHomeDir(Security, pszDirectory, nMax))
            return sal_False;
        n = strlen(pszDirectory);
        if (n + sizeof(DOT_CONFIG) < nMax)
        {
            strncpy(pszDirectory+n, DOT_CONFIG, sizeof(DOT_CONFIG));
            if (access(pszDirectory, 0) != 0)
            {
                // resort to HOME
                pszDirectory[n] = '\0';
            }
        }
    }
    else
        strncpy(pszDirectory, pStr, nMax);

    return sal_True;
}

#undef DOT_CONFIG

#else

/*
 * FIXME: rewrite to use more flexible
 * NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES)
 * as soon as we can bumb the baseline to Tiger (for NSApplicationSupportDirectory) and have
 * support for Objective-C in the build environment
 */

#define MACOSX_CONFIG_DIR "/Library/Application Support"
static sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    if( osl_psz_getHomeDir(Security, pszDirectory, nMax - sizeof(MACOSX_CONFIG_DIR) + 1) )
    {
        strcat( pszDirectory, MACOSX_CONFIG_DIR );
        return sal_True;
    }

    return sal_False;
}

#endif

sal_Bool SAL_CALL osl_isAdministrator(oslSecurity Security)
{
    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if (pSecImpl == NULL)
        return sal_False;

    if (pSecImpl->m_pPasswd.pw_uid != 0)
        return sal_False;

    return sal_True;
}

void SAL_CALL osl_freeSecurityHandle(oslSecurity Security)
{
    deleteSecurityImpl(Security);
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
