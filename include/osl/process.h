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


#ifndef INCLUDED_OSL_PROCESS_H
#define INCLUDED_OSL_PROCESS_H

#include "sal/config.h"

#include "osl/file.h"
#include "osl/security.h"
#include "osl/time.h"
#include "rtl/locale.h"
#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef sal_Int32 oslProcessOption;
#define     osl_Process_WAIT       0x0001    /* wait for completion */
#define     osl_Process_SEARCHPATH 0x0002    /* search path for executable */
#define     osl_Process_DETACHED   0x0004    /* run detached */
#define     osl_Process_NORMAL     0x0000    /* run in normal window */
#define     osl_Process_HIDDEN     0x0010    /* run hidden */
#define     osl_Process_MINIMIZED  0x0020    /* run in minimized window */
#define     osl_Process_MAXIMIZED  0x0040    /* run in maximized window */
#define     osl_Process_FULLSCREEN 0x0080    /* run in fullscreen window */

typedef sal_Int32 oslProcessData;

/* defines for osl_getProcessInfo , can be OR'ed */
#define     osl_Process_IDENTIFIER  0x0001   /* retrieves the process identifier   */
#define     osl_Process_EXITCODE    0x0002   /* retrieves exit code of the process */
#define     osl_Process_CPUTIMES    0x0004   /* retrieves used cpu time            */
#define     osl_Process_HEAPUSAGE   0x0008   /* retrieves the used size of heap    */

typedef sal_uInt32 oslProcessIdentifier;
typedef sal_uInt32 oslProcessExitCode;

typedef enum {
    osl_Process_E_None,                 /* no error */
    osl_Process_E_NotFound,             /* image not found */
    osl_Process_E_TimedOut,             /* timeout occurred */
    osl_Process_E_NoPermission,         /* permission denied */
    osl_Process_E_Unknown,              /* unknown error */
    osl_Process_E_InvalidError,         /* unmapped error */
    osl_Process_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslProcessError;

#ifdef _WIN32
#   pragma pack(push, 8)
#endif

typedef struct {
    sal_uInt32              Size;
    oslProcessData          Fields;
    oslProcessIdentifier    Ident;
    oslProcessExitCode      Code;
    TimeValue               UserTime;
    TimeValue               SystemTime;
    sal_uInt32              HeapUsage;
} oslProcessInfo;

#if defined( _WIN32)
#   pragma pack(pop)
#endif

/** Process handle

    @see osl_executeProcess
    @see osl_terminateProcess
    @see osl_freeProcessHandle
    @see osl_getProcessInfo
    @see osl_joinProcess
*/
typedef void* oslProcess;

/** Execute a process.

    Executes the program image provided in strImageName in a new process.

    @param[in] ustrImageName
    The file URL of the executable to be started.
    Can be NULL in this case the file URL of the executable must be the first element
    in ustrArguments.

    @param[in] ustrArguments
    An array of argument strings. Can be NULL if strImageName is not NULL.
    If strImageName is NULL it is expected that the first element contains
    the file URL of the executable to start.

    @param[in] nArguments
    The number of arguments provided. If this number is 0 strArguments will be ignored.

    @param[in] Options
    A combination of int-constants to describe the mode of execution.

    @param[in] Security
    The user and his rights for which the process is started. May be NULL in which case
    the process will be started in the context of the current user.

    @param[in] ustrDirectory
    The file URL of the working directory of the new process. If the specified directory
    does not exist or is inaccessible the working directory of the newly created process
    is undefined. If this parameter is NULL or the caller provides an empty string the
    new process will have the same current working directory as the calling process.

    @param[in] ustrEnvironments
    An array of strings describing environment variables that should be merged into the
    environment of the new process. Each string has to be in the form "variable=value".
    This parameter can be NULL in which case the new process gets the same environment
    as the parent process.

    @param[in] nEnvironmentVars
    The number of environment variables to set.

    @param[out] pProcess
    Pointer to a oslProcess variable, which receives the handle of the newly created process.
    This parameter must not be NULL.

    @retval osl_Process_E_None on success
    @retval osl_Process_E_NotFound if the specified executable could not be found</dd>
    @retval osl_Process_E_InvalidError if invalid parameters will be detected</dd>
    @retval osl_Process_E_Unknown if arbitrary other errors occur</dd>

    @see oslProcessOption
    @see osl_executeProcess_WithRedirectedIO
    @see osl_freeProcessHandle
    @see osl_loginUser
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_executeProcess(
    rtl_uString* ustrImageName,
    rtl_uString* ustrArguments[],
    sal_uInt32  nArguments,
    oslProcessOption Options,
    oslSecurity Security,
    rtl_uString* ustrDirectory,
    rtl_uString* ustrEnvironments[],
    sal_uInt32 nEnvironmentVars,
    oslProcess* pProcess);


/** Execute a process and redirect child process standard IO.

    @param[in] strImageName
    The file URL of the executable to be started.
    Can be NULL in this case the file URL of the executable must be the first element
    in ustrArguments.

    @param[in] ustrArguments
    An array of argument strings. Can be NULL if strImageName is not NULL.
    If strImageName is NULL it is expected that the first element contains
    the file URL of the executable to start.

    @param[in] nArguments
    The number of arguments provided. If this number is 0 strArguments will be ignored.

    @param[in] Options
    A combination of int-constants to describe the mode of execution.

    @param[in] Security
    The user and his rights for which the process is started. May be NULL in which case
    the process will be started in the context of the current user.

    @param[in] ustrDirectory
    The file URL of the working directory of the new process. If the specified directory
    does not exist or is inaccessible the working directory of the newly created process
    is undefined. If this parameter is NULL or the caller provides an empty string the
    new process will have the same current working directory as the calling process.

    @param[in] ustrEnvironments
    An array of strings describing environment variables that should be merged into the
    environment of the new process. Each string has to be in the form "variable=value".
    This parameter can be NULL in which case the new process gets the same environment
    as the parent process.

    @param[in] nEnvironmentVars
    The number of environment variables to set.

    @param[out] pProcess
    Pointer to a oslProcess variable, which receives the handle of the newly created process.
    This parameter must not be NULL.

    @param[out] pChildInputWrite
    Pointer to a oslFileHandle variable that receives the handle which can be used to write
    to the child process standard input device. The returned handle is not random accessible.
    The handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @param[out] pChildOutputRead
    Pointer to a oslFileHandle variable that receives the handle which can be used to read from
    the child process standard output device. The returned handle is not random accessible.
    The Handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @param[out] pChildErrorRead
    Pointer to a oslFileHandle variable that receives the handle which can be used to read from
    the child process standard error device. The returned handle is not random accessible.
    The Handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @retval osl_Process_E_None on success
    @retval osl_Process_E_NotFound if the specified executable could not be found
    @retval osl_Process_E_InvalidError if invalid parameters will be detected
    @retval osl_Process_E_Unknown if arbitrary other errors occur

    @see oslProcessOption
    @see osl_executeProcess
    @see osl_freeProcessHandle
    @see osl_loginUser
    @see osl_closeFile
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
    rtl_uString* strImageName,
    rtl_uString* ustrArguments[],
    sal_uInt32 nArguments,
    oslProcessOption Options,
    oslSecurity Security,
    rtl_uString* ustrDirectory,
    rtl_uString* ustrEnvironments[],
    sal_uInt32 nEnvironmentVars,
    oslProcess* pProcess,
    oslFileHandle* pChildInputWrite,
    oslFileHandle* pChildOutputRead,
    oslFileHandle* pChildErrorRead);

/** Terminate a process

    @param[in] Process the handle of the process to be terminated

    @see osl_executeProcess
    @see osl_getProcess
    @see osl_joinProcess
 */
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_terminateProcess(
        oslProcess Process);


/** @deprecated

    Retrieve the process handle of a process identifier

    @param[in] Ident a process identifier

    @return the process handle on success, NULL in all other cases
 */
SAL_DLLPUBLIC oslProcess SAL_CALL osl_getProcess(
        oslProcessIdentifier Ident) SAL_COLD;


/** Free the specified process-handle.

    @param[in] Process
*/
SAL_DLLPUBLIC void SAL_CALL osl_freeProcessHandle(
        oslProcess Process);


/** Wait for completion of the specified childprocess.
    @param[in] Process

    @retval ols_Process_E_None

    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_joinProcess(
        oslProcess Process);

/** Wait with a timeout for the completion of the specified child
    process.

    @param[in] Process A process identifier.
    @param[in] pTimeout A timeout value or NULL for infinite waiting.
        The unit of resolution is second.

    @retval osl_Process_E_None on success
    @retval osl_Process_E_TimedOut waiting for the child process timed out
    @retval osl_Process_E_Unknown an error occurred or the parameter are invalid

    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_joinProcessWithTimeout(
        oslProcess Process, const TimeValue* pTimeout);

/** Retrieves information about a Process
    @param[in] Process  the process handle of the process
    @param[in] Fields   the information which is to be retrieved
                        this can be one or more of
                        osl_Process_IDENTIFIER
                        osl_Process_EXITCODE
                        osl_Process_CPUTIMES
                        osl_Process_HEAPUSAGE
    @param[out] pInfo   a pointer to a valid oslProcessInfo structure.
                        the Size field has to be initialized with the size
                        of the oslProcessInfo structure.
                        on success the Field member holds the (or'ed)
                        retrieved valid information fields.
    @retval osl_Process_E_None on success
    @retval osl_Process_E_Unknown on failure
 */
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessInfo(
        oslProcess Process, oslProcessData Fields, oslProcessInfo* pInfo);

/** Get the filename of the executable.
    @param[out] strFile the string that receives the executable file path.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess

    Ideally this will return the true executable file path as a file:
    URL, but actually in case something else happens to have been
    passed as argv[0] to osl_setCommandArgs(), it will return that
    either as a file URL, or as such in case it doesn't look like an
    absolute pathname.
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getExecutableFile(
        rtl_uString **strFile);

/** @return the number of commandline arguments passed to the main-function of
    this process
    @see osl_getCommandArg
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getCommandArgCount(void);

/** Get the nArg-th command-line argument passed to the main-function of this process.
    @param[in] nArg  The number of the argument to return.
    @param[out] strCommandArg The string receives the nArg-th command-line argument.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getCommandArg(
        sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Set the command-line arguments as passed to the main-function of this process.

    Deprecated: This function is only for internal use. Passing the args from main will
    only work for Unix, on Windows there's no effect, the full command line will automatically
    be taken. This is due to Windows 9x/ME limitation that don't allow UTF-16 wmain to provide
    a osl_setCommandArgsU( int argc, sal_Unicode **argv );

    @param[in] argc  The number of elements in the argv array.
    @param[in] argv  The array of command-line arguments.
    @see osl_getExecutableFile
    @see osl_getCommandArgCount
    @see osl_getCommandArg
*/
SAL_DLLPUBLIC void SAL_CALL osl_setCommandArgs (int argc, char **argv);

/** Get the value of one environment variable.
    @param[in] strVar  denotes the name of the variable to get.
    @param[out] strValue string that receives the value of environment variable.
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getEnvironment(
        rtl_uString *strVar, rtl_uString **strValue);

/** Set the value of one environment variable.
    @param[in] strVar  denotes the name of the variable to set.
    @param[in] strValue  string of the new value of environment variable.

    @since UDK 3.2.13
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_setEnvironment(
        rtl_uString *strVar, rtl_uString *strValue);

/** Unsets the value of one environment variable.
    @param[in] strVar  denotes the name of the variable to unset.

    @since UDK 3.2.13
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_clearEnvironment(
        rtl_uString *strVar);

/** Get the working directory of the current process as a file URL.

    The file URL is encoded as common for the OSL file API.
    @param[out] pustrWorkingDir string that receives the working directory file URL.
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessWorkingDir(
        rtl_uString **pustrWorkingDir );

/** Get the locale the process is currently running in.

    @param[out] ppLocale a pointer that receives the currently selected locale structure
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessLocale(
        rtl_Locale ** ppLocale );

/** Change the locale of the process.

    @param[in] pLocale  a pointer to the locale to be set

    @deprecated  LibreOffice itself does not use this, and client code should
        not have good use for it either.  It may eventually be removed.
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_setProcessLocale(
        rtl_Locale * pLocale );

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_PROCESS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
