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
#include "sal/log.hxx"

#include "file_path_helper.hxx"

#include "uunxapi.hxx"
#include "nlsupport.hxx"

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#if defined(MACOSX) || defined(IOS)
#include <mach-o/dyld.h>

namespace {

oslProcessError SAL_CALL bootstrap_getExecutableFile(rtl_uString ** ppFileURL)
{
    oslProcessError result = osl_Process_E_NotFound;

    char   buffer[PATH_MAX];
    uint32_t buflen = sizeof(buffer);

    if (_NSGetExecutablePath (buffer, &buflen) == 0)
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

    return result;
}

}

#else
#include <dlfcn.h>

namespace {

oslProcessError SAL_CALL bootstrap_getExecutableFile(rtl_uString ** ppFileURL)
{
    oslProcessError result = osl_Process_E_NotFound;

#ifdef ANDROID
    /* Now with just a single DSO, this one from lo-bootstrap.c is as good as
     * any */
    void * addr = dlsym (RTLD_DEFAULT, "JNI_OnLoad");
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

    return result;
}

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
    pthread_mutex_lock (&(g_command_args.m_mutex));
    if (g_command_args.m_nCount == 0)
    {
        pthread_mutex_unlock (&(g_command_args.m_mutex));
        return bootstrap_getExecutableFile(ppustrFile);
    }

    /* CommandArgs set. Obtain argv[0]. */
    rtl_uString_assign (ppustrFile, g_command_args.m_ppArgs[0]);
    pthread_mutex_unlock (&(g_command_args.m_mutex));
    return osl_Process_E_None;
}

/***************************************
 osl_getCommandArgCount().
 **************************************/
sal_uInt32 SAL_CALL osl_getCommandArgCount()
{
    sal_uInt32 result = 0;

    pthread_mutex_lock (&(g_command_args.m_mutex));
    SAL_INFO_IF(
        g_command_args.m_nCount == 0, "sal.osl",
        "osl_getCommandArgCount w/o prior call to osl_setCommandArgs");
    if (g_command_args.m_nCount > 0)
        result = g_command_args.m_nCount - 1;
    pthread_mutex_unlock (&(g_command_args.m_mutex));

    return result;
}

/***************************************
 osl_getCommandArg().
 **************************************/
oslProcessError SAL_CALL osl_getCommandArg (sal_uInt32 nArg, rtl_uString ** strCommandArg)
{
    oslProcessError result = osl_Process_E_NotFound;

    pthread_mutex_lock (&(g_command_args.m_mutex));
    assert(g_command_args.m_nCount > 0);
    if (g_command_args.m_nCount > (nArg + 1))
    {
        rtl_uString_assign (strCommandArg, g_command_args.m_ppArgs[nArg + 1]);
        result = osl_Process_E_None;
    }
    pthread_mutex_unlock (&(g_command_args.m_mutex));

    return result;
}

/***************************************
 osl_setCommandArgs().
 **************************************/
void SAL_CALL osl_setCommandArgs (int argc, char ** argv)
{
    assert(argc > 0);
    pthread_mutex_lock (&(g_command_args.m_mutex));
    SAL_WARN_IF(g_command_args.m_nCount != 0, "sal.osl", "args already set");
    if (g_command_args.m_nCount == 0)
    {
        rtl_uString** ppArgs = static_cast<rtl_uString**>(rtl_allocateZeroMemory (argc * sizeof(rtl_uString*)));
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
                if (rtl_ustr_indexOfChar (rtl_uString_getStr(ppArgs[0]), '/') == -1)
                {
                    rtl_uString * pSearchPath = 0;
                    osl_getEnvironment (OUString("PATH").pData, &pSearchPath);
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

    return result;
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

    return result;
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

    return result;
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

    return result;
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
    return result;
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

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
