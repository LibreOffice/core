/*************************************************************************
 *
 *  $RCSfile: security.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-26 16:17:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "system.h"

#include <osl/security.h>
#include <osl/diagnose.h>
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#ifdef SOLARIS
#include <crypt.h>
#endif

#include "secimpl.h"

#ifndef PAM_BINARY_MSG
#define PAM_BINARY_MSG 6
#endif

extern oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);
extern void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName);
extern oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
                  oslSecurity* pSecurity);
sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax);
sal_Bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax);
sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);
sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax);



oslSecurity SAL_CALL osl_getCurrentSecurity()
{

    oslSecurityImpl *pSecImpl = (oslSecurityImpl*) malloc(sizeof(oslSecurityImpl));
    struct passwd   *pPasswd  = getpwuid(getuid());

    if (pPasswd)
    {
        memcpy(&pSecImpl->m_pPasswd, pPasswd, sizeof(pSecImpl->m_pPasswd));
        pSecImpl->m_isValid = sal_True;
    }
    else
    {
        /* Some UNIX-OS don't implement getpwuid, e.g. NC OS (special NetBSD) 1.2.1 */
        /* so we have to catch this in this else branch */

        pSecImpl->m_pPasswd.pw_name     = "unknown";
        pSecImpl->m_pPasswd.pw_passwd   = NULL;
        pSecImpl->m_pPasswd.pw_uid      = getuid();
        pSecImpl->m_pPasswd.pw_gid      = getgid();
        pSecImpl->m_pPasswd.pw_gecos    = "unknown";
        pSecImpl->m_pPasswd.pw_dir      = "/tmp";
        pSecImpl->m_pPasswd.pw_shell    = "unknown";
        pSecImpl->m_isValid             = sal_False;
    }


    return ((oslSecurity)pSecImpl);
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
 * I guess that echo-off means ´ask for password´ and echo-on means
 * ´ask for user-name´. In fact I´ve never been asked anything else
 * than the password
 * XXX Please notice that if a pam-module does ask anything else, we
 *     are completely lost, and a pam-module is free to do so
 * XXX
 */

int
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
    error = False;
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
                error = True;
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

/*
 * avoid linking against libpam.so, since it is not available on all systems,
 * instead load-on-call, returns structure which holds pointer to
 * pam-functions,
 * library is never closed in case of success
 */

static sal_PamModule* osl_getPAM()
{
    static sal_PamModule *pam_module = NULL;
    static sal_Bool load_once = False;

    if ( !load_once )
    {
        /* get library-handle. cannot use osl-module, since
            RTLD_GLOBAL is required for PAM-0.64 RH 5.2
           (but not for PAM-0.66 RH 6.0) */
        void *pam_hdl;

        pam_hdl = dlopen( "libpam.so", RTLD_GLOBAL | RTLD_LAZY );

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
        load_once = True;
    }

    return pam_module;
}

/*
 * User Identification using PAM
 */

static sal_Bool
osl_PamAuthentification( const sal_Char* name, const sal_Char* password )
{
    sal_Bool success = False;

    sal_PamModule* pam_module;

    pam_module = osl_getPAM();
    if ( pam_module != NULL )
    {
        pam_handle_t   *pam_handle = NULL;
        struct pam_conv pam_conversation;
        sal_PamData     pam_data;

        int             return_value;

        pam_data.name     = (char*) name;
        pam_data.password = (char*) password;

        pam_conversation.conv        = osl_PamConversation;
        pam_conversation.appdata_ptr = (void*)(&pam_data);

          return_value = pam_module->pam_start( "su", name,
            &pam_conversation, &pam_handle);
        if (return_value == PAM_SUCCESS )
            return_value = pam_module->pam_authenticate(pam_handle, 0);
          if (return_value == PAM_SUCCESS )
            return_value = pam_module->pam_acct_mgmt(pam_handle, 0);
        pam_module->pam_end( pam_handle, return_value );

        success = (sal_Bool)(return_value == PAM_SUCCESS);
    }

      return success;
}


/* dummy crypt, matches the interface of
   crypt() but does not encrypt at all */
static const sal_Char* SAL_CALL
osl_noCrypt ( const sal_Char *key, const sal_Char *salt )
{
    return key;
}

/* load-on-call crypt library and crypt symbol */
static void*  SAL_CALL
osl_getCrypt()
{
    static char* (*crypt_sym)(const char*, const char*) = NULL;
    static sal_Bool load_once  = False;

    if ( !load_once )
    {
        oslModule crypt_library;

        crypt_library = osl_psz_loadModule( "libcrypt.so", SAL_LOADMODULE_DEFAULT );  /* never closed */
        if ( crypt_library != NULL )
            crypt_sym = (char* (*)(const char *, const char *)) osl_psz_getSymbol(crypt_library, "crypt" );
        if ( crypt_sym == NULL ) /* no libcrypt or libcrypt without crypt */
            crypt_sym = (char* (*)(const char *, const char *)) &osl_noCrypt;

        load_once = True;
    }

    return (void*)crypt_sym;
}

/* replacement for crypt function for password encryption, uses either
   strong encryption of dlopen´ed libcrypt.so or dummy implementation
   with no encryption. Objective target is to avoid linking against
   libcrypt (not available on caldera open linux 2.2 #63822#) */
static sal_Char* SAL_CALL
osl_dynamicCrypt ( const sal_Char *key, const sal_Char *salt )
{
    char* (*dynamic_crypt)(char *key, char *salt);

    dynamic_crypt = (char * (*)(char *, char *)) osl_getCrypt();

    return dynamic_crypt( (sal_Char*)key, (sal_Char*)salt );
}

/*
 * compare an encrypted and an unencrypted password for equality
 * returns true if passwords are equal, false otherwise
 * Note: uses crypt() and a mutex instead of crypt_r() since crypt_r needs
 * more than 128KByte of external buffer for struct crypt_data
 */

sal_Bool SAL_CALL
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

    encrypted_plain = (sal_Char *)osl_dynamicCrypt( pPlainPassword, salt );
    success = (sal_Bool) (strcmp(pEncryptedPassword, encrypted_plain) == 0);

    pthread_mutex_unlock(&crypt_mutex);

    return success;
}

/*
 * the oslSecurityImpl structure is merely a struct passwd plus addons like a
 * buffer, which may be used in *_r calls. Copying is less nifty since struct
 * passwd has some member (pw_name, pw_passwd ...) wich point into this buffer.
 * osl_copySecurity copies the data from the src->passwd entries (which do or
 * do not point to src->buffer) one by one into the dst->buffer and make the
 * dst->passwd entries point to the corresponding spots in dst->buffer
 * NOTE: the function implies that PASSWD_BUFFER_SIZE is sufficient to hold the
 *       memory, otherwise getpwnam_r fails anyway and this routine
 *       isn't called at all. If this doesn't hold some entries will contain
 *       garbage, but memory is kept clean and does not leak.
 */

/* wraper around memcpy which maintains buffer integrity */
sal_Char* SAL_CALL
osl_copyPasswdEntry( sal_Char *dst, sal_Char *src, sal_Int32 *size )
{
    sal_Int32 len;

    len = strlen( src ) + 1;
    *size += len;
    if ( *size < PASSWD_BUFFER_SIZE )
    {
        memcpy( dst, src, len );
        dst += len;
    }

    return dst;
}

/* copy a security by using the oslSecurityImpl.m_buffer for mem requirements */
oslSecurity SAL_CALL
osl_copySecurity ( oslSecurityImpl* src )
{
    sal_Int32        size = 0;
    oslSecurityImpl *dst;
    sal_Char        *buffer;

       dst = (oslSecurityImpl*) malloc( sizeof(oslSecurityImpl) );
    buffer = (sal_Char*)&(dst->m_buffer);

    buffer = osl_copyPasswdEntry ( dst->m_pPasswd.pw_name = buffer,
                                   src->m_pPasswd.pw_name, &size );
    buffer = osl_copyPasswdEntry ( dst->m_pPasswd.pw_passwd = buffer,
                                   src->m_pPasswd.pw_passwd, &size );
    buffer = osl_copyPasswdEntry ( dst->m_pPasswd.pw_gecos = buffer,
                                   src->m_pPasswd.pw_gecos, &size );
    buffer = osl_copyPasswdEntry ( dst->m_pPasswd.pw_dir = buffer,
                                   src->m_pPasswd.pw_dir, &size );
    buffer = osl_copyPasswdEntry ( dst->m_pPasswd.pw_shell = buffer,
                                   src->m_pPasswd.pw_shell, &size );

    dst->m_pPasswd.pw_uid = src->m_pPasswd.pw_uid;
    dst->m_pPasswd.pw_gid = src->m_pPasswd.pw_gid;

    dst->m_isValid = src->m_isValid;

    return (oslSecurity)dst ;
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


oslSecurityError SAL_CALL
osl_psz_loginUser(const sal_Char* pszUserName, const sal_Char* pszPasswd,
               oslSecurity* pSecurity)
{
#if defined NETBSD || defined SCO || defined AIX || defined FREEBSD || \
    defined MACOSX

    return osl_Security_E_None;

#elif defined HPUX

    oslSecurityImpl *pSecImpl = NULL;

    if ((pszUserName == NULL) || (pszPasswd == NULL) || (pSecurity == NULL))
        return osl_Security_E_Unknown;

    pSecImpl = malloc(sizeof(oslSecurityImpl));

    pSecImpl->m_isValid = sal_False;

    if (getpwnam_r(pszUserName, &pSecImpl->m_pPasswd,
                   pSecImpl->m_buffer, PASSWD_BUFFER_SIZE) == 0)
    {
        sal_Char        salt[3], *cryptPasswd;
        CRYPTD      aCRYPTD;
        struct spwd *pSpwd;

        strncpy(salt, pSecImpl->m_pPasswd.pw_passwd, 2);
        salt[2] = '\0';
        cryptPasswd = crypt_r((sal_Char*)pszPasswd, salt, &aCRYPTD);

        if (strcmp(pSecImpl->m_pPasswd.pw_passwd, cryptPasswd) == 0)
        {
            pSecImpl->m_isValid = sal_True;
            *pSecurity = (oslSecurity) pSecImpl;
            return osl_Security_E_None;
        }

        /* check for trusted systems */
        if (pSpwd = getspnam((sal_Char*)pszUserName))
        {
            strncpy(salt, pSpwd->sp_pwdp, 2);
            salt[2] = '\0';
            cryptPasswd = crypt_r((sal_Char*)pszPasswd, salt, &aCRYPTD);

            if (strcmp(pSpwd->sp_pwdp, cryptPasswd) == 0)
            {
                pSecImpl->m_isValid = sal_True;
                strcpy(pSecImpl->m_pPasswd.pw_passwd, pSpwd->sp_pwdp);
                *pSecurity = (oslSecurity)pSecImpl;
                return osl_Security_E_None;
            }
        }

        free(pSecImpl);
        *pSecurity = NULL;
        return osl_Security_E_WrongPassword;
    }
    else
    {
        free(pSecImpl);
        *pSecurity = NULL;
        return osl_Security_E_UserUnknown;
    }

#elif defined IRIX
/* getpwnam_r (_POSIX1C) returns 25 (ENOTTY, i.e. Inapprop. I/O control op.) */

    struct passwd           *pPasswd;
    static pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;

    if ((pszUserName == NULL) || (pszPasswd == NULL) || (pSecurity == NULL))
        return osl_Security_E_Unknown;

    pthread_mutex_lock(&mutex);

    pPasswd = getpwnam(pszUserName);
    if (pPasswd)
    {
        sal_Char salt[3], *cryptPasswd;

        strncpy(salt, pPasswd->pw_passwd, 2);
        salt[2] = '\0';
        cryptPasswd = crypt((sal_Char*)pszPasswd, salt);

        if (strcmp(pPasswd->pw_passwd, cryptPasswd) == 0)
        {
            if (strlen(pPasswd->pw_name) + strlen(pPasswd->pw_passwd) +
                strlen(pPasswd->pw_age) + strlen(pPasswd->pw_comment) +
                strlen(pPasswd->pw_gecos) + strlen(pPasswd->pw_dir) +
                strlen(pPasswd->pw_shell) + 7 <= PASSWD_BUFFER_SIZE)
            {
                sal_Char *p;
                oslSecurityImpl *pSecImpl = malloc(sizeof(oslSecurityImpl));

                pSecImpl->m_pPasswd = *pPasswd;

                p = pSecImpl->m_buffer;
                pSecImpl->m_pPasswd.pw_name = p;
                strcpy(p, pPasswd->pw_name);

                p += strlen(pPasswd->pw_name) + 1;
                pSecImpl->m_pPasswd.pw_passwd = p;
                strcpy(p, pPasswd->pw_passwd);

                p += strlen(pPasswd->pw_passwd) + 1;
                pSecImpl->m_pPasswd.pw_age = p;
                strcpy(p, pPasswd->pw_age);

                p += strlen(pPasswd->pw_age) + 1;
                pSecImpl->m_pPasswd.pw_comment = p;
                strcpy(p, pPasswd->pw_comment);

                p += strlen(pPasswd->pw_comment) + 1;
                pSecImpl->m_pPasswd.pw_gecos = p;
                strcpy(p, pPasswd->pw_gecos);

                p += strlen(pPasswd->pw_gecos) + 1;
                pSecImpl->m_pPasswd.pw_dir = p;
                strcpy(p, pPasswd->pw_dir);

                p += strlen(pPasswd->pw_dir) + 1;
                pSecImpl->m_pPasswd.pw_shell = p;
                strcpy(p, pPasswd->pw_shell);

                pSecImpl->m_isValid = sal_True;
                *pSecurity = (oslSecurity)pSecImpl;
                pthread_mutex_unlock(&mutex);
                return osl_Security_E_None;
            }
            else
            {
                *pSecurity = NULL;
                pthread_mutex_unlock(&mutex);
                return osl_Security_E_Unknown;
            }
        }
        else
        {
            *pSecurity = NULL;
            pthread_mutex_unlock(&mutex);
            return osl_Security_E_WrongPassword;
        }
    }
    else
    {
        *pSecurity = NULL;
        pthread_mutex_unlock(&mutex);
        return osl_Security_E_UserUnknown;
    }

#elif defined S390

    struct passwd           *pPasswd;

    if ((pszUserName == NULL) || (pszPasswd == NULL) || (pSecurity == NULL))
        return osl_SEUNKNOWN;

    OSL_TRACE("osl_loginUser %s (for now without password validation!)\n", pszUserName);

    pPasswd = getpwnam(pszUserName);
    if (pPasswd)
    {
        int   iResult = 0;
/* the function __passwd is not correctly implemented now; we have to wait for the next OS/390 release...
        sal_Char* _pszUserName = strdup(pszUserName);
        sal_Char* _pszPasswd = strdup(pszPasswd);
        asc2ebc(_pszUserName);
        asc2ebc(_pszPasswd);

        iResult = __passwd(_pszUserName, _pszPasswd, NULL);
        if(iResult == -1)
        {
            OSL_TRACE("__passwd result: %d", errno);
            perror(NULL);
        }

        free(_pszUserName);
        free(_pszPasswd);
*/
        if (iResult == 0)
        {
            if (strlen(pPasswd->pw_name) +
                strlen(pPasswd->pw_dir) +
                strlen(pPasswd->pw_shell) + 7 <= PASSWD_BUFFER_SIZE)
            {
                sal_Char *p;
                oslSecurityImpl *pSecImpl = malloc(sizeof(oslSecurityImpl));

                pSecImpl->m_pPasswd = *pPasswd;

                p = pSecImpl->m_buffer;
                pSecImpl->m_pPasswd.pw_name = p;
                strcpy(p, pPasswd->pw_name);

                p += strlen(pPasswd->pw_name) + 1;
                pSecImpl->m_pPasswd.pw_dir = p;
                strcpy(p, pPasswd->pw_dir);

                p += strlen(pPasswd->pw_dir) + 1;
                pSecImpl->m_pPasswd.pw_shell = p;
                strcpy(p, pPasswd->pw_shell);

                pSecImpl->m_isValid = sal_True;
                *pSecurity = (oslSecurity)pSecImpl;

                OSL_TRACE("Homedirectory of %s: %s\n", pSecImpl->m_pPasswd.pw_name, pSecImpl->m_pPasswd.pw_dir);

                return osl_Security_E_None;
            }
            else
            {
                *pSecurity = NULL;
                return osl_SEUNKNOWN;
            }
        }
        else
        {
            *pSecurity = NULL;
            return osl_Security_E_WrongPassword;
        }
    }
    else
    {
        *pSecurity = NULL;
        return osl_Security_E_UserUnknown;
    }


#elif (LINUX && (GLIBC >= 2))

    struct passwd   *pPasswd;

    oslSecurityImpl  aSecurityBuffer;
    oslSecurityError nError = osl_Security_E_Unknown;

    if ((pszUserName == NULL) || (pszPasswd == NULL) || (pSecurity == NULL))
        return osl_Security_E_Unknown;

    /* get nis or normal password, should succeed for any known user,
     * but perhaps the password is wrong (i.e. 'x') if shadow passwords
     * are in use or authentication must be done by PAM */
    if (0 == getpwnam_r ( pszUserName,
                     &(aSecurityBuffer.m_pPasswd),
                      (char*)&(aSecurityBuffer.m_buffer), PASSWD_BUFFER_SIZE,
                     &pPasswd ) )
    {
        /* only root is able to read the /etc/shadow passwd,
         * a normal user even can't read his own encrypted passwd */
        sal_Char     buffer[ 1024 ];
        struct spwd  result_buf;
        struct spwd *pShadowPasswd=0;
        int nRet=0;

        buffer[0] = '\0';

        /* mfe: here we try to get the shadowpassword though it's used later */
        nRet=getspnam_r ( pszUserName, &result_buf, buffer, sizeof(buffer), &pShadowPasswd);
        /* right password ? */
        if ( osl_equalPasswords(pPasswd->pw_passwd, pszPasswd) )
        {
            nError = osl_Security_E_None;
        }
        else
        /* otherwise try Pluggable Authentication Modules (PAM) */
        if ( osl_PamAuthentification( pszUserName, pszPasswd ) )
        {
            nError = osl_Security_E_None;
        }
        else
        /* otherwise try shadow passwd */
          if ( pShadowPasswd != 0 )
        {
            if ( osl_equalPasswords(pShadowPasswd->sp_pwdp, pszPasswd ) )
            {
                nError = osl_Security_E_None;
            }
            else
            {
                nError = osl_Security_E_WrongPassword;
            }
        }
        else
        if ( getuid() == 0 )
        {

            /* mfe: Try to verify the root-password via nis */
              if ( getspnam_r ( "root", &result_buf, buffer, sizeof(buffer), &pShadowPasswd) &&
                 osl_equalPasswords(pShadowPasswd->sp_pwdp, pszPasswd ) )
            {
                    nError = osl_Security_E_None;
            }
            else
            {   /*
                 *  mfe: we can't get via nis (glibc2.0.x has bug in getspnam_r)
                 *  we try it with the normal getspnam
                 */
                static pthread_mutex_t pwmutex = PTHREAD_MUTEX_INITIALIZER;

                pthread_mutex_lock(&pwmutex);
                pShadowPasswd=getspnam("root");
                pthread_mutex_unlock(&pwmutex);

                if ( osl_equalPasswords(pShadowPasswd->sp_pwdp, pszPasswd ) )
                {
                    nError = osl_Security_E_None;
                }
                else
                /* mfe: once more via PAM */
                if ( osl_PamAuthentification( "root", pszPasswd ) )
                {
                    nError = osl_Security_E_None;
                }
                else
                {
                    /* mfe: it didn't' succeed at all */
                    nError = osl_Security_E_WrongPassword;
                }
            }
        }
    }
    else
    {
        nError = osl_Security_E_UserUnknown;
    }

    /* return nError as well as a properly built pSecurity. if we've used
     * the shadow passwd, aSecurityBuffer doesn't contain the right password! */
    if ( nError == osl_Security_E_None )
    {
        aSecurityBuffer.m_isValid = sal_True;
        *pSecurity = osl_copySecurity( &aSecurityBuffer );
    }
    else
    {
        *pSecurity = NULL;
    }

    return nError;

#else

    struct passwd* pPasswd;
    struct spwd    spwdStruct;
    sal_Char           buffer[PASSWD_BUFFER_SIZE];

    oslSecurityImpl* pSecImpl = NULL;

    buffer[0] = '\0';

    if ((pszUserName == NULL) || (pszPasswd == NULL) || (pSecurity == NULL))
        return osl_Security_E_Unknown;

    pSecImpl = (oslSecurityImpl*) malloc(sizeof(oslSecurityImpl));

    if (getpwnam_r(pszUserName,
                   &(pSecImpl->m_pPasswd),
                   pSecImpl->m_buffer,
                   PASSWD_BUFFER_SIZE,
                   &pPasswd) == 0)
    {
        sal_Char salt[3], *cryptPasswd;

        if (getspnam_r(pszUserName, &spwdStruct,
                       buffer, PASSWD_BUFFER_SIZE) != NULL)
        {
            strncpy(salt, spwdStruct.sp_pwdp, 2);
            salt[2] = '\0';
            cryptPasswd = (sal_Char *)crypt(pszPasswd, salt);

            if (strcmp(spwdStruct.sp_pwdp, cryptPasswd) == 0)
            {
                 *pSecurity = (oslSecurity) pSecImpl;
                return osl_Security_E_None;
            }
            else
            {
                /* if current process is running as root,
                   allow to logon as any other user */
                if ((getuid() == 0) &&
                    (getspnam_r("root", &spwdStruct,
                                buffer, PASSWD_BUFFER_SIZE) != NULL))
                {
                    strncpy(salt, spwdStruct.sp_pwdp, 2);
                    salt[2] = '\0';
                    cryptPasswd = (sal_Char *)crypt(pszPasswd, salt);

                    if (strcmp(spwdStruct.sp_pwdp, cryptPasswd) == 0)
                    {
                         *pSecurity = (oslSecurity) pSecImpl;
                        return osl_Security_E_None;
                    }
                }

                free(pSecImpl);
                *pSecurity = NULL;
                return osl_Security_E_WrongPassword;
            }
        }
        else
        {
             free(pSecImpl);
            *pSecurity = NULL;
            return osl_Security_E_UserUnknown;
        }
    }
    else
    {
        free(pSecImpl);
        *pSecurity = NULL;
        return osl_Security_E_UserUnknown;
    }
#endif
}

oslSecurityError SAL_CALL osl_loginUserOnFileServer(
    rtl_uString *strUserName,
    rtl_uString *strPasswd,
    rtl_uString *strFileServer,
    oslSecurity *pSecurity
    )
{
    oslSecurityError erg;
    return erg = osl_Security_E_UserUnknown;
}


oslSecurityError SAL_CALL osl_psz_loginUserOnFileServer( const sal_Char*  pszUserName,
                                                     const sal_Char*  pszPasswd,
                                                     const sal_Char*  pszFileServer,
                                                     oslSecurity*     pSecurity )
{
    oslSecurityError erg;
    return erg = osl_Security_E_UserUnknown;
}

sal_Bool SAL_CALL osl_getUserIdent(oslSecurity Security, rtl_uString **ustrIdent)
{
    sal_Bool bRet=sal_False;
    sal_Char pszIdent[1024];

    pszIdent[0] = '\0';

    bRet = osl_psz_getUserIdent(Security,pszIdent,sizeof(pszIdent));

    rtl_string2UString( ustrIdent, pszIdent, rtl_str_getLength( pszIdent ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    return bRet;
}


sal_Bool SAL_CALL osl_psz_getUserIdent(oslSecurity Security, sal_Char *pszIdent, sal_uInt32 nMax)
{
    sal_Char buffer[32];

    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    buffer[0] = '\0';


    if (pSecImpl == NULL)
        return sal_False;

    sprintf(buffer, "%u", pSecImpl->m_pPasswd.pw_uid);

    strncpy(pszIdent, buffer, nMax);

    return sal_True;
}

sal_Bool SAL_CALL osl_getUserName(oslSecurity Security, rtl_uString **ustrName)
{
    sal_Bool bRet=sal_False;
    sal_Char pszName[1024];

    pszName[0] = '\0';

    bRet = osl_psz_getUserName(Security,pszName,sizeof(pszName));

    rtl_string2UString( ustrName, pszName, rtl_str_getLength( pszName ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    return bRet;
}



sal_Bool SAL_CALL osl_psz_getUserName(oslSecurity Security, sal_Char* pszName, sal_uInt32  nMax)
{
    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if ((pSecImpl == NULL) || (! pSecImpl->m_isValid))
        return sal_False;

    strncpy(pszName, pSecImpl->m_pPasswd.pw_name, nMax);

    return sal_True;
}

sal_Bool SAL_CALL osl_getHomeDir(oslSecurity Security, rtl_uString **ustrDirectory)
{
    sal_Bool bRet=sal_False;
    sal_Char pszDirectory[PATH_MAX];
    sal_Char pszUncPath[PATH_MAX+4];

    pszDirectory[0] = '\0';
    pszUncPath[0] = '\0';

    bRet = osl_psz_getHomeDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet == sal_True )
    {
        strcpy(pszUncPath,"//.");
        strcat(pszUncPath,pszDirectory);
    }

    rtl_string2UString( ustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );
    return bRet;
}


sal_Bool SAL_CALL osl_psz_getHomeDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    oslSecurityImpl *pSecImpl = (oslSecurityImpl *)Security;

    if (pSecImpl == NULL)
        return sal_False;

    /* if current user, check also environment for HOME */
    if (getuid() == pSecImpl->m_pPasswd.pw_uid)
    {
        sal_Char *pStr = getenv("HOME");

        if ((pStr != NULL) && (strlen(pStr) > 0) &&
            (access(pStr, 0) == 0))
            strncpy(pszDirectory, pStr, nMax);
        else
            if (pSecImpl->m_isValid)
                strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);
            else
                return sal_False;
    }
    else
        strncpy(pszDirectory, pSecImpl->m_pPasswd.pw_dir, nMax);

    return sal_True;
}

sal_Bool SAL_CALL osl_getConfigDir(oslSecurity Security, rtl_uString **ustrDirectory)
{
    sal_Bool bRet = sal_False;
    sal_Char pszDirectory[PATH_MAX];
    sal_Char pszUncPath[PATH_MAX+4];

    pszDirectory[0] = '\0';
    pszUncPath[0] = '\0';

    bRet = osl_psz_getConfigDir(Security,pszDirectory,sizeof(pszDirectory));

    if ( bRet == sal_True )
    {
        strcpy(pszUncPath,"//.");
        strcat(pszUncPath,pszDirectory);
    }

    rtl_string2UString( ustrDirectory, pszDirectory, rtl_str_getLength( pszDirectory ), osl_getThreadTextEncoding(), OUSTRING_TO_OSTRING_CVTFLAGS );

    return bRet;
}


sal_Bool SAL_CALL osl_psz_getConfigDir(oslSecurity Security, sal_Char* pszDirectory, sal_uInt32 nMax)
{
    return (osl_psz_getHomeDir(Security, pszDirectory, nMax));
}

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
    if (Security)
        free ((oslSecurityImpl*)Security);
}


sal_Bool SAL_CALL osl_loadUserProfile(oslSecurity Security)
{
    return sal_False;
}

void SAL_CALL osl_unloadUserProfile(oslSecurity Security)
{
    return;
}


