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

#include "osl/process.h"

#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/module.h"
#include "osl/thread.h"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.h"

#include "file_path_helper.h"

#include "uunxapi.h"

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

/***************************************
  osl_bootstrap_getExecutableFile_Impl().

  @internal
  @see rtl_bootstrap
  @see #i37371#

 **************************************/

extern "C" oslProcessError SAL_CALL osl_bootstrap_getExecutableFile_Impl (
    rtl_uString ** ppFileURL
) SAL_THROW_EXTERN_C();


#if defined(MACOSX) || defined(IOS)
#include <mach-o/dyld.h>

oslProcessError SAL_CALL osl_bootstrap_getExecutableFile_Impl (
    rtl_uString ** ppFileURL
) SAL_THROW_EXTERN_C()
{
    oslProcessError result = osl_Process_E_NotFound;

    char   buffer[PATH_MAX];
    size_t buflen = sizeof(buffer);

    if (_NSGetExecutablePath (buffer, (uint32_t*)&buflen) == 0)
    {
        /* Determine absolute path. */
        char abspath[PATH_MAX];
        if (realpath (buffer, abspath) != 0)
        {
            /* Convert from utf8 to unicode. */
            rtl_uString * pAbsPath = 0;
            rtl_string2UString (
                &(pAbsPath),
                abspath, rtl_str_getLength (abspath),
                RTL_TEXTENCODING_UTF8,
                OSTRING_TO_OUSTRING_CVTFLAGS);

            if (pAbsPath)
            {
                /* Convert from path to url. */
                if (osl_getFileURLFromSystemPath (pAbsPath, ppFileURL) == osl_File_E_None)
                {
                    /* Success. */
                    result = osl_Process_E_None;
                }
                rtl_uString_release (pAbsPath);
            }
        }
    }

    return (result);
}

#else
#include <dlfcn.h>

oslProcessError SAL_CALL osl_bootstrap_getExecutableFile_Impl (
    rtl_uString ** ppFileURL
) SAL_THROW_EXTERN_C()
{
    oslProcessError result = osl_Process_E_NotFound;

#if defined(ANDROID) && !defined(DISABLE_DYNLOADING)
    /* On Android we in theory want the address of the "lo_main()"
     * function, as that is what corresponds to "main()" in
     * LibreOffice programs on normal desktop OSes.
     *
     * But that is true only for apps with a "native activity", using
     * <sal/main.h> and the org.libreoffice.android.Bootstrap
     * mechanism. For more normal (?) Android apps that just use
     * LibreOffice libraries (components) where the main program is in
     * Java, that just use LibreOffice libraries, there is no
     * lo_main(). (Note that we don't know for sure yet how
     * complicated it might be to write such Android apps...)
     *
     * Maybe best to just pick some function in liblo-bootstrap.so
     * which also such Java apps *must* load as the very first
     * LibreOffice native library. We store all LibreOffice native
     * shared libraries an app uses in the same folder anyway, so it
     * doesn't really matter.
     */
    void * addr = (void *) &lo_dlopen;
#else
    /* Determine address of "main()" function. */
    void * addr = dlsym (RTLD_DEFAULT, "main");
#endif
    if (addr != 0)
    {
        /* Determine module URL. */
        if (osl_getModuleURLFromAddress (addr, ppFileURL))
        {
            /* Success. */
            result = osl_Process_E_None;
        }
    }

    /* Fallback to ordinary osl_getExecutableFile(). */
    if (result == osl_Process_E_NotFound)
        result = osl_getExecutableFile (ppFileURL);

    return (result);
}

#endif

/***************************************
 CommandArgs_Impl.
 **************************************/
struct CommandArgs_Impl
{
    pthread_mutex_t m_mutex;
    sal_uInt32      m_nCount;
    rtl_uString **  m_ppArgs;
};

static struct CommandArgs_Impl g_command_args =
{
    PTHREAD_MUTEX_INITIALIZER,
    0,
    0
};

/***************************************
  osl_getExecutableFile().
 **************************************/
oslProcessError SAL_CALL osl_getExecutableFile (rtl_uString ** ppustrFile)
{
    oslProcessError result = osl_Process_E_NotFound;

    pthread_mutex_lock (&(g_command_args.m_mutex));
    OSL_ASSERT(g_command_args.m_nCount > 0);
    if (g_command_args.m_nCount > 0)
    {
        /* CommandArgs set. Obtain argv[0]. */
        rtl_uString_assign (ppustrFile, g_command_args.m_ppArgs[0]);
        result = osl_Process_E_None;
    }
    pthread_mutex_unlock (&(g_command_args.m_mutex));

    return (result);
}

/***************************************
 osl_getCommandArgCount().
 **************************************/
sal_uInt32 SAL_CALL osl_getCommandArgCount (void)
{
    sal_uInt32 result = 0;

    pthread_mutex_lock (&(g_command_args.m_mutex));
    if (g_command_args.m_nCount == 0) {
        OSL_TRACE(
            OSL_LOG_PREFIX
            "osl_getCommandArgCount w/o prior call to osl_setCommandArgs");
    }
    if (g_command_args.m_nCount > 0)
        result = g_command_args.m_nCount - 1;
    pthread_mutex_unlock (&(g_command_args.m_mutex));

    return (result);
}

/***************************************
 osl_getCommandArg().
 **************************************/
oslProcessError SAL_CALL osl_getCommandArg (sal_uInt32 nArg, rtl_uString ** strCommandArg)
{
    oslProcessError result = osl_Process_E_NotFound;

    pthread_mutex_lock (&(g_command_args.m_mutex));
    OSL_ASSERT(g_command_args.m_nCount > 0);
    if (g_command_args.m_nCount > (nArg + 1))
    {
        rtl_uString_assign (strCommandArg, g_command_args.m_ppArgs[nArg + 1]);
        result = osl_Process_E_None;
    }
    pthread_mutex_unlock (&(g_command_args.m_mutex));

    return (result);
}

/***************************************
 osl_setCommandArgs().
 **************************************/
void SAL_CALL osl_setCommandArgs (int argc, char ** argv)
{
    OSL_ASSERT(argc > 0);
    pthread_mutex_lock (&(g_command_args.m_mutex));
    OSL_ENSURE (g_command_args.m_nCount == 0, "osl_setCommandArgs(): CommandArgs already set.");
    if (g_command_args.m_nCount == 0)
    {
        rtl_uString** ppArgs = (rtl_uString**)rtl_allocateZeroMemory (argc * sizeof(rtl_uString*));
        if (ppArgs != 0)
        {
            rtl_TextEncoding encoding = osl_getThreadTextEncoding();
            for (int i = 0; i < argc; i++)
            {
                rtl_string2UString (
                    &(ppArgs[i]),
                    argv[i], rtl_str_getLength (argv[i]), encoding,
                    OSTRING_TO_OUSTRING_CVTFLAGS);
            }
            if (ppArgs[0] != 0)
            {
#if !defined(ANDROID) && !defined(IOS) // No use searching PATH on Android or iOS
                /* see @ osl_getExecutableFile(). */
                if (rtl_ustr_indexOfChar (rtl_uString_getStr(ppArgs[0]), sal_Unicode('/')) == -1)
                {
                    const rtl::OUString PATH (RTL_CONSTASCII_USTRINGPARAM("PATH"));

                    rtl_uString * pSearchPath = 0;
                    osl_getEnvironment (PATH.pData, &pSearchPath);
                    if (pSearchPath)
                    {
                        rtl_uString * pSearchResult = 0;
                        osl_searchPath (ppArgs[0], pSearchPath, &pSearchResult);
                        if (pSearchResult)
                        {
                            rtl_uString_assign (&(ppArgs[0]), pSearchResult);
                            rtl_uString_release (pSearchResult);
                        }
                        rtl_uString_release (pSearchPath);
                    }
                }
#endif
                rtl_uString * pArg0 = 0;
                if (realpath_u (ppArgs[0], &pArg0))
                {
                    osl_getFileURLFromSystemPath (pArg0, &(ppArgs[0]));
                    rtl_uString_release (pArg0);
                }
            }
            g_command_args.m_nCount = argc;
            g_command_args.m_ppArgs = ppArgs;
        }
    }
    pthread_mutex_unlock (&(g_command_args.m_mutex));
}

/***************************************
 osl_getEnvironment().
 **************************************/
oslProcessError SAL_CALL osl_getEnvironment(rtl_uString* pustrEnvVar, rtl_uString** ppustrValue)
{
    oslProcessError  result   = osl_Process_E_NotFound;
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    rtl_String* pstr_env_var  = 0;

    OSL_PRECOND(pustrEnvVar, "osl_getEnvironment(): Invalid parameter");
    OSL_PRECOND(ppustrValue, "osl_getEnvironment(): Invalid parameter");

    rtl_uString2String(
        &pstr_env_var,
        rtl_uString_getStr(pustrEnvVar), rtl_uString_getLength(pustrEnvVar), encoding,
        OUSTRING_TO_OSTRING_CVTFLAGS);
    if (pstr_env_var != 0)
    {
        const char* p_env_var = getenv (rtl_string_getStr (pstr_env_var));
        if (p_env_var != 0)
        {
            rtl_string2UString(
                ppustrValue,
                p_env_var, strlen(p_env_var), encoding,
                OSTRING_TO_OUSTRING_CVTFLAGS);
            OSL_ASSERT(*ppustrValue != NULL);

            result = osl_Process_E_None;
        }
        rtl_string_release(pstr_env_var);
    }

    return (result);
}

/***************************************
 osl_setEnvironment().
 **************************************/
oslProcessError SAL_CALL osl_setEnvironment(rtl_uString* pustrEnvVar, rtl_uString* pustrValue)
{
    oslProcessError  result   = osl_Process_E_Unknown;
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    rtl_String* pstr_env_var  = 0;
    rtl_String* pstr_val  = 0;

    OSL_PRECOND(pustrEnvVar, "osl_setEnvironment(): Invalid parameter");
    OSL_PRECOND(pustrValue, "osl_setEnvironment(): Invalid parameter");

    rtl_uString2String(
        &pstr_env_var,
        rtl_uString_getStr(pustrEnvVar), rtl_uString_getLength(pustrEnvVar), encoding,
        OUSTRING_TO_OSTRING_CVTFLAGS);

    rtl_uString2String(
        &pstr_val,
        rtl_uString_getStr(pustrValue), rtl_uString_getLength(pustrValue), encoding,
        OUSTRING_TO_OSTRING_CVTFLAGS);

    if (pstr_env_var != 0 && pstr_val != 0)
    {
#if defined (SOLARIS)
        rtl_String * pBuffer = NULL;

        sal_Int32 nCapacity = rtl_stringbuffer_newFromStringBuffer( &pBuffer,
            rtl_string_getLength(pstr_env_var) + rtl_string_getLength(pstr_val) + 1,
            pstr_env_var );
        rtl_stringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length, "=", 1);
        rtl_stringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length,
            rtl_string_getStr(pstr_val), rtl_string_getLength(pstr_val) );

        rtl_string_acquire(pBuffer); // argument to putenv must leak on success

        if (putenv(rtl_string_getStr(pBuffer)) == 0)
            result = osl_Process_E_None;
        else
            rtl_string_release(pBuffer);
#else
        if (setenv(rtl_string_getStr(pstr_env_var), rtl_string_getStr(pstr_val), 1) == 0)
            result = osl_Process_E_None;
#endif
    }

    if (pstr_val)
        rtl_string_release(pstr_val);

    if (pstr_env_var != 0)
        rtl_string_release(pstr_env_var);

    return (result);
}

/***************************************
 osl_clearEnvironment().
 **************************************/
oslProcessError SAL_CALL osl_clearEnvironment(rtl_uString* pustrEnvVar)
{
    oslProcessError  result   = osl_Process_E_Unknown;
    rtl_TextEncoding encoding = osl_getThreadTextEncoding();
    rtl_String* pstr_env_var  = 0;

    OSL_PRECOND(pustrEnvVar, "osl_setEnvironment(): Invalid parameter");

    rtl_uString2String(
        &pstr_env_var,
        rtl_uString_getStr(pustrEnvVar), rtl_uString_getLength(pustrEnvVar), encoding,
        OUSTRING_TO_OSTRING_CVTFLAGS);

    if (pstr_env_var)
    {
#if defined (SOLARIS)
        rtl_String * pBuffer = NULL;

        sal_Int32 nCapacity = rtl_stringbuffer_newFromStringBuffer( &pBuffer,
            rtl_string_getLength(pstr_env_var) + 1, pstr_env_var );
        rtl_stringbuffer_insert( &pBuffer, &nCapacity, pBuffer->length, "=", 1);

        rtl_string_acquire(pBuffer); // argument to putenv must leak on success

        if (putenv(rtl_string_getStr(pBuffer)) == 0)
            result = osl_Process_E_None;
        else
            rtl_string_release(pBuffer);
#elif (defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        //MacOSX baseline is 10.4, which has an old-school void return
        //for unsetenv.
                //See: http://developer.apple.com/mac/library/documentation/Darwin/Reference/ManPages/10.4/man3/unsetenv.3.html?useVersion=10.4
        unsetenv(rtl_string_getStr(pstr_env_var));
        result = osl_Process_E_None;
#else
        if (unsetenv(rtl_string_getStr(pstr_env_var)) == 0)
            result = osl_Process_E_None;
#endif
        rtl_string_release(pstr_env_var);
    }

    return (result);
}


/***************************************
 osl_getProcessWorkingDir().
 **************************************/
oslProcessError SAL_CALL osl_getProcessWorkingDir(rtl_uString **ppustrWorkingDir)
{
    oslProcessError result = osl_Process_E_Unknown;
    char buffer[PATH_MAX];

    OSL_PRECOND(ppustrWorkingDir, "osl_getProcessWorkingDir(): Invalid parameter");

    if (getcwd (buffer, sizeof(buffer)) != 0)
    {
        rtl_uString* ustrTmp = 0;

        rtl_string2UString(
            &ustrTmp,
            buffer, strlen(buffer), osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS);
        if (ustrTmp != 0)
        {
            if (osl_getFileURLFromSystemPath (ustrTmp, ppustrWorkingDir) == osl_File_E_None)
                result = osl_Process_E_None;
            rtl_uString_release (ustrTmp);
        }
    }

    return (result);
}

/******************************************************************************
 *
 *              new functions to set/return the current process locale
 *
 *****************************************************************************/

struct ProcessLocale_Impl
{
    pthread_mutex_t m_mutex;
    rtl_Locale *    m_pLocale;
};

static struct ProcessLocale_Impl g_process_locale =
{
    PTHREAD_MUTEX_INITIALIZER,
    0
};

extern "C" void _imp_getProcessLocale( rtl_Locale ** );
extern "C" int  _imp_setProcessLocale( rtl_Locale * );

/**********************************************
 osl_getProcessLocale().
 *********************************************/
oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale )
{
    oslProcessError result = osl_Process_E_Unknown;
    OSL_PRECOND(ppLocale, "osl_getProcessLocale(): Invalid parameter.");
    if (ppLocale)
    {
        pthread_mutex_lock(&(g_process_locale.m_mutex));

        if (g_process_locale.m_pLocale == 0)
            _imp_getProcessLocale (&(g_process_locale.m_pLocale));
        *ppLocale = g_process_locale.m_pLocale;
        result = osl_Process_E_None;

        pthread_mutex_unlock (&(g_process_locale.m_mutex));
    }
    return (result);
}

/**********************************************
 osl_setProcessLocale().
 *********************************************/
oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale )
{
    oslProcessError result = osl_Process_E_Unknown;

    OSL_PRECOND(pLocale, "osl_setProcessLocale(): Invalid parameter.");

    pthread_mutex_lock(&(g_process_locale.m_mutex));
    if (_imp_setProcessLocale (pLocale) == 0)
    {
        g_process_locale.m_pLocale = pLocale;
        result = osl_Process_E_None;
    }
    pthread_mutex_unlock (&(g_process_locale.m_mutex));

    return (result);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
