/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: security.c,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:19:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#include "osl/thread.h"
#include "osl/file.h"

#ifdef SOLARIS
#include <crypt.h>
#endif

#include "secimpl.h"

#ifndef PAM_BINARY_MSG
#define PAM_BINARY_MSG 6
#endif

extern oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);
extern void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName);
static oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
                  oslSecurity* pSecurity);
sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax);
static sal_Bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax);
static sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
static sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);

static oslSecurityImpl * newSecurityImpl(size_t * bufSize) {
#ifdef MACOSX
    /* #i64906#: sysconf(_SC_GETPW_R_SIZE_MAX) returns -1 on Mac OS X */
    size_t n = 1024;
#else
    size_t n = (size_t) sysconf(_SC_GETPW_R_SIZE_MAX);
#endif
    if (n <= SIZE_MAX - offsetof(oslSecurityImpl, m_buffer)) {
        *bufSize = n;
        n += offsetof(oslSecurityImpl, m_buffer);
    } else {
        *bufSize = SIZE_MAX - offsetof(oslSecurityImpl, m_buffer);
        n = SIZE_MAX;
    }
    return malloc(n);
}

static void deleteSecurityImpl(oslSecurityImpl * impl) {
    free(impl);
}

oslSecurity SAL_CALL osl_getCurrentSecurity()
{
    size_t n;
    oslSecurityImpl * p = newSecurityImpl(&n);
    if (p != NULL) {
        struct passwd * found;
        if (getpwuid_r(getuid(), &p->m_pPasswd, p->m_buffer, n, &found) != 0 ||
            found == NULL)
        {
            deleteSecurityImpl(p);
            p = 0;
        }
    }
    return p;
}


#ifdef LINUX

/*
 *
 * osl Routines for Pluggable Authentication Modules (PAM)
 * tested with Linux-PAM 0.66 on Redhat-6.0 and
 * Linux-PAM 0.64 on RedHat-5.2,
 * XXX Will probably not run on PAM 0.59 or prior, since
 *     number of pam_response* responses has changed
 *
 */

#include <security/pam_appl.h>

typedef struct {
    char* name;
    char* password;
} sal_PamData;

typedef struct {
    int (*pam_start)(const char *service_name, const char *user,
                      const struct pam_conv *pam_conversation,
                      pam_handle_t **pamh);
    int (*pam_end)          (pam_handle_t *pamh, int pam_status);
    int (*pam_authenticate) (pam_handle_t *pamh, int flags);
    int (*pam_acct_mgmt)    (pam_handle_t *pamh, int flags);
} sal_PamModule;

/*
 * Implement a pam-conversation callback-routine,
 * it just supply name and password instead of prompting the user.
 * I guess that echo-off means 'ask for password' and echo-on means
 * 'ask for user-name'. In fact I've never been asked anything else
 * than the password
 * XXX Please notice that if a pam-module does ask anything else, we
 *     are completely lost, and a pam-module is free to do so
 * XXX
 */

static int
osl_PamConversation (int num_msg, const struct pam_message **msgm,
                     struct pam_response **response, void *appdata_ptr)
{
    int         i;
    sal_Bool    error;
    sal_PamData         *pam_data;
    struct pam_response *p_reply;

    /* resource initialization */
    pam_data = (sal_PamData*) appdata_ptr;
    p_reply  = (struct pam_response *) calloc( num_msg,
                                               sizeof(struct pam_response));
    if ( p_reply == NULL || pam_data == NULL )
    {
        if ( p_reply != NULL )
            free ( p_reply );
        *response = NULL;
        return PAM_CONV_ERR;
    }

    /* pseudo dialog */
    error = sal_False;
    for ( i = 0; i < num_msg ; i++ )
    {
        switch ( msgm[ i ]->msg_style )
        {
            case PAM_PROMPT_ECHO_OFF:
                p_reply[ i ].resp_retcode = 0;
                p_reply[ i ].resp         = strdup( pam_data->password );
                 break;
            case PAM_PROMPT_ECHO_ON:
                p_reply[ i ].resp_retcode = 0;
                p_reply[ i ].resp       = strdup( pam_data->name );
                break;
            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
             case PAM_BINARY_PROMPT:
            case PAM_BINARY_MSG:
                p_reply[ i ].resp_retcode   = 0;
                p_reply[ i ].resp           = NULL;
                break;
            default:
                error = sal_True;
                break;
        }
    }

    /* free resources on error */
    if ( error )
    {
        for ( i = 0; i < num_msg ; i++ )
            if ( p_reply[ i ].resp )
            {
                memset ( p_reply[ i ].resp, 0,
                         strlen( p_reply[ i ].resp ) );
                free   ( p_reply[ i ].resp );
            }
        free ( p_reply );

        *response = NULL;
        return PAM_CONV_ERR;
    }

    /* well done */
    *response = p_reply;
    return PAM_SUCCESS;
}

#ifndef PAM_LINK
/*
 * avoid linking against libpam.so, since it is not available on all systems,
 * instead load-on-call, returns structure which holds pointer to
 * pam-functions,
 * library is never closed in case of success
 */

static sal_PamModule* osl_getPAM()
{
    static sal_PamModule *pam_module = NULL;
    static sal_Bool load_once = sal_False;

    if ( !load_once )
    {
        /* get library-handle. cannot use osl-module, since
            RTLD_GLOBAL is required for PAM-0.64 RH 5.2
           (but not for PAM-0.66 RH 6.0) */
        void *pam_hdl;

        pam_hdl = dlopen( "libpam.so.0", RTLD_GLOBAL | RTLD_LAZY );

        if ( pam_hdl != NULL )
            pam_module = (sal_PamModule*)calloc( 1, sizeof(sal_PamModule) );

        /* load functions */
        if ( pam_module  != NULL )
        {
            pam_module->pam_acct_mgmt = (int (*)(pam_handle_t *, int)) dlsym ( pam_hdl, "pam_acct_mgmt" );
            pam_module->pam_authenticate
                                      = (int (*)(pam_handle_t *, int)) dlsym ( pam_hdl, "pam_authenticate" );
            pam_module->pam_end       = (int (*)(pam_handle_t *, int)) dlsym ( pam_hdl, "pam_end" );
            pam_module->pam_start     = (int (*)(const char *, const char *, const struct pam_conv *, pam_handle_t **)) dlsym ( pam_hdl, "pam_start" );

            /* free resources, if not completely successful */
            if (   (pam_module->pam_start        == NULL)
                || (pam_module->pam_end          == NULL)
                || (pam_module->pam_authenticate == NULL)
                || (pam_module->pam_acct_mgmt    == NULL) )
            {
                free( pam_module );
                pam_module = NULL;
                dlclose( pam_hdl );
            }
        }

        /* never try again */
        load_once = sal_True;
    }

    return pam_module;
}
#endif

/*
 * User Identification using PAM
 */

static sal_Bool
osl_PamAuthentification( const sal_Char* name, const sal_Char* password )
{
    sal_Bool success = sal_False;

#ifndef PAM_LINK
    sal_PamModule* pam_module;

    pam_module = osl_getPAM();
    if ( pam_module != NULL )
    {
#endif
        pam_handle_t   *pam_handle = NULL;
        struct pam_conv pam_conversation;
        sal_PamData     pam_data;

        int             return_value;

        pam_data.name     = (char*) name;
        pam_data.password = (char*) password;

        pam_conversation.conv        = osl_PamConversation;
        pam_conversation.appdata_ptr = (void*)(&pam_data);

#ifndef PAM_LINK
          return_value = pam_module->pam_start( "su", name,
            &pam_conversation, &pam_handle);
#else
          return_value = pam_start( "su", name,
            &pam_conversation, &pam_handle);
#endif
        if (return_value == PAM_SUCCESS )
#ifndef PAM_LINK
            return_value = pam_module->pam_authenticate(pam_handle, 0);
#else
            return_value = pam_authenticate(pam_handle, 0);
#endif
          if (return_value == PAM_SUCCESS )
#ifndef PAM_LINK
            return_value = pam_module->pam_acct_mgmt(pam_handle, 0);
        pam_module->pam_end( pam_handle, return_value );
#else
            return_value = pam_acct_mgmt(pam_handle, 0);
        pam_end( pam_handle, return_value );
#endif

        success = (sal_Bool)(return_value == PAM_SUCCESS);
#ifndef PAM_LINK
    }
#endif

      return success;
}


#ifndef CRYPT_LINK
/* dummy crypt, matches the interface of
   crypt() but does not encrypt at all */
static const sal_Char* SAL_CALL
osl_noCrypt ( const sal_Char *key, const sal_Char *salt )
{
    (void) salt; /* unused */
    return key;
}

/* load-on-call crypt library and crypt symbol */
static void*  SAL_CALL
osl_getCrypt()
{
    static char* (*crypt_sym)(const char*, const char*) = NULL;
    static sal_Bool load_once  = sal_False;

    if ( !load_once )
    {
        oslModule crypt_library;

        crypt_library = osl_psz_loadModule( "libcrypt.so.1", SAL_LOADMODULE_DEFAULT );  /* never closed */
        if ( crypt_library != NULL )
            crypt_sym = (char* (*)(const char *, const char *)) osl_psz_getSymbol(crypt_library, "crypt" );
        if ( crypt_sym == NULL ) /* no libcrypt or libcrypt without crypt */
            crypt_sym = (char* (*)(const char *, const char *)) &osl_noCrypt;

        load_once = sal_True;
    }

    return (void*)crypt_sym;
}

/* replacement for crypt function for password encryption, uses either
   strong encryption of dlopen'ed libcrypt.so.1 or dummy implementation
   with no encryption. Objective target is to avoid linking against
   libcrypt (not available on caldera open linux 2.2 #63822#) */
static sal_Char* SAL_CALL
osl_dynamicCrypt ( const sal_Char *key, const sal_Char *salt )
{
    char* (*dynamic_crypt)(char *, char *);

    dynamic_crypt = (char * (*)(char *, char *)) osl_getCrypt();

    return dynamic_crypt( (sal_Char*)key, (sal_Char*)salt );
}
#endif

/*
 * compare an encrypted and an unencrypted password for equality
 * returns true if passwords are equal, false otherwise
 * Note: uses crypt() and a mutex instead of crypt_r() since crypt_r needs
 * more than 128KByte of external buffer for struct crypt_data
 */

static sal_Bool SAL_CALL
osl_equalPasswords ( const sal_Char *pEncryptedPassword, const sal_Char *pPlainPassword )
{
    static pthread_mutex_t crypt_mutex = PTHREAD_MUTEX_INITIALIZER;

    sal_Bool  success;
    sal_Char  salt[3];
    sal_Char *encrypted_plain;

    salt[0] = pEncryptedPassword[0];
    salt[1] = pEncryptedPassword[1];
    salt[2] = '\0';

    pthread_mutex_lock(&crypt_mutex);

#ifndef CRYPT_LINK
    encrypted_plain = (sal_Char *)osl_dynamicCrypt( pPlainPassword, salt );
#else
    encrypted_plain = (sal_Char *)crypt( pPlainPassword, salt );
#endif
    success = (sal_Bool) (strcmp(pEncryptedPassword, encrypted_plain) == 0);

    pthread_mutex_unlock(&crypt_mutex);

    return success;
}

#endif
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
#if defined NETBSD || defined SCO || defined AIX || defined FREEBSD || \
    defined MACOSX

    return osl_Security_E_None;

#else

    oslSecurityError nError = osl_Security_E_Unknown;
    oslSecurityImpl * p = NULL;
    if (pszUserName != NULL && pszPasswd != NULL && pSecurity != NULL) {
        size_t n;
        p = newSecurityImpl(&n);
        if (p != NULL) {
            /* get nis or normal password, should succeed for any known user,
               but perhaps the password is wrong (i.e. 'x') if shadow passwords
               are in use or authentication must be done by PAM */
            struct passwd * found;
            if (getpwnam_r(pszUserName, &p->m_pPasswd, p->m_buffer, n, &found)
                == 0)
            {
                if (found == NULL) {
                    nError = osl_Security_E_UserUnknown;
                } else {
#if defined LINUX
                    /* only root is able to read the /etc/shadow passwd, a
                       normal user even can't read his own encrypted passwd */
                    if (osl_equalPasswords(p->m_pPasswd.pw_passwd, pszPasswd) ||
                        osl_PamAuthentification(pszUserName, pszPasswd))
                    {
                        nError = osl_Security_E_None;
                    } else {
                        char buffer[1024];
                        struct spwd result_buf;
                        struct spwd * pShadowPasswd;
                        buffer[0] = '\0';
                        if (getspnam_r(
                                pszUserName, &result_buf, buffer, sizeof buffer,
                                &pShadowPasswd) == 0 &&
                            pShadowPasswd != NULL)
                        {
                            nError =
                                osl_equalPasswords(
                                    pShadowPasswd->sp_pwdp, pszPasswd)
                                ? osl_Security_E_None
                                : osl_Security_E_WrongPassword;
                        } else if (getuid() == 0) {
                            /* mfe: Try to verify the root-password via nis */
                            if (getspnam_r(
                                    "root", &result_buf, buffer, sizeof buffer,
                                    &pShadowPasswd) == 0 &&
                                pShadowPasswd != NULL &&
                                osl_equalPasswords(
                                    pShadowPasswd->sp_pwdp, pszPasswd))
                            {
                                nError = osl_Security_E_None;
                            } else {
                                /* mfe: we can't get via nis (glibc2.0.x has bug
                                   in getspnam_r) we try it with the normal
                                   getspnam */
                                static pthread_mutex_t pwmutex =
                                    PTHREAD_MUTEX_INITIALIZER;
                                pthread_mutex_lock(&pwmutex);
                                pShadowPasswd = getspnam("root");
                                pthread_mutex_unlock(&pwmutex);
                                nError =
                                    ((pShadowPasswd != NULL &&
                                      osl_equalPasswords(
                                          pShadowPasswd->sp_pwdp, pszPasswd)) ||
                                     osl_PamAuthentification("root", pszPasswd))
                                    ? osl_Security_E_None
                                    : osl_Security_E_WrongPassword;
                            }
                        }
                    }
#else
                    char buffer[1024];
                    struct spwd spwdStruct;
                    buffer[0] = '\0';
                    if (getspnam_r(
                            pszUserName, &spwdStruct, buffer, sizeof buffer) !=
                        NULL)
                    {
                        char salt[3];
                        char * cryptPasswd;
                        strncpy(salt, spwdStruct.sp_pwdp, 2);
                        salt[2] = '\0';
                        cryptPasswd = (char *) crypt(pszPasswd, salt);
                        if (strcmp(spwdStruct.sp_pwdp, cryptPasswd) == 0) {
                            nError = osl_Security_E_None;
                        } else if (getuid() == 0 &&
                                   (getspnam_r(
                                       "root", &spwdStruct, buffer,
                                       sizeof buffer)
                                    != NULL))
                        {
                            /* if current process is running as root, allow to
                               logon as any other user */
                            strncpy(salt, spwdStruct.sp_pwdp, 2);
                            salt[2] = '\0';
                            cryptPasswd = (char *) crypt(pszPasswd, salt);
                            if (strcmp(spwdStruct.sp_pwdp, cryptPasswd) == 0) {
                                nError = osl_Security_E_None;
                            }
                        } else {
                            nError = osl_Security_E_WrongPassword;
                        }
                    }
#endif
                }
            }
        }
    }
    if (nError == osl_Security_E_None) {
        *pSecurity = p;
    } else {
        deleteSecurityImpl(p);
        *pSecurity = NULL;
    }
    return nError;

#endif
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

    if (pSecImpl == NULL)
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

        if ((pStr != NULL) && (strlen(pStr) > 0) &&
            (access(pStr, 0) == 0))
            strncpy(pszDirectory, pStr, nMax);
        else
            strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
    }
    else
        strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);

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

static sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    return (osl_psz_getHomeDir(Security, pszDirectory, nMax));
}

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
        return (sal_False);

    return (sal_True);
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
