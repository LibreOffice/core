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


#ifndef _OSL_PROCESS_H_
#define _OSL_PROCESS_H_

#include "sal/config.h"

#include "osl/file.h"
#include "osl/pipe.h"
#include "osl/security.h"
#include "osl/socket.h"
#include "osl/time.h"
#include "rtl/locale.h"
#include "rtl/textenc.h"
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
    osl_Process_E_TimedOut,             /* timout occurred */
    osl_Process_E_NoPermission,         /* permission denied */
    osl_Process_E_Unknown,              /* unknown error */
    osl_Process_E_InvalidError,         /* unmapped error */
    osl_Process_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslProcessError;

typedef enum {
    osl_Process_TypeNone,       /* no descriptor */
    osl_Process_TypeSocket,     /* socket */
    osl_Process_TypeFile,       /* file   */
    osl_Process_TypePipe,       /* pipe   */
    osl_Process_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
} oslDescriptorType;

typedef sal_Int32 oslDescriptorFlag;
#define osl_Process_DFNONE       0x0000
#define osl_Process_DFWAIT       0x0001

#ifdef SAL_W32
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

#if defined( SAL_W32)
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

    @param ustrImageName
    [in] The file URL of the executable to be started.
    Can be NULL in this case the file URL of the executable must be the first element
    in ustrArguments.

    @param ustrArguments
    [in] An array of argument strings. Can be NULL if strImageName is not NULL.
    If strImageName is NULL it is expected that the first element contains
    the file URL of the executable to start.

    @param nArguments
    [in] The number of arguments provided. If this number is 0 strArguments will be ignored.

    @param Options
    [in] A combination of int-constants to describe the mode of execution.

    @param Security
    [in] The user and his rights for which the process is started. May be NULL in which case
    the process will be started in the context of the current user.

    @param ustrDirectory
    [in] The file URL of the working directory of the new proces. If the specified directory
    does not exist or is inaccessible the working directory of the newly created process
    is undefined. If this parameter is NULL or the caller provides an empty string the
    new process will have the same current working directory as the calling process.

    @param ustrEnvironments
    [in] An array of strings describing environment variables that should be merged into the
    environment of the new process. Each string has to be in the form "variable=value".
    This parameter can be NULL in which case the new process gets the same environment
    as the parent process.

    @param nEnvironmentVars
    [in] The number of environment variables to set.

    @param pProcess
    [out] Pointer to a oslProcess variable, wich receives the handle of the newly created process.
    This parameter must not be NULL.

    @return
    <dl>
    <dt>osl_Process_E_None</dt>
    <dd>on success</dd>
    <dt>osl_Process_E_NotFound</dt>
    <dd>if the specified executable could not be found</dd>
    <dt>osl_Process_E_InvalidError</dt>
    <dd>if invalid parameters will be detected</dd>
    <dt>osl_Process_E_Unknown</dt>
    <dd>if arbitrary other errors occur</dd>
    </dl>

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

    @param strImageName
    [in] The file URL of the executable to be started.
    Can be NULL in this case the file URL of the executable must be the first element
    in ustrArguments.

    @param ustrArguments
    [in] An array of argument strings. Can be NULL if strImageName is not NULL.
    If strImageName is NULL it is expected that the first element contains
    the file URL of the executable to start.

    @param nArguments
    [in] The number of arguments provided. If this number is 0 strArguments will be ignored.

    @param Options
    [in] A combination of int-constants to describe the mode of execution.

    @param Security
    [in] The user and his rights for which the process is started. May be NULL in which case
    the process will be started in the context of the current user.

    @param ustrDirectory
    [in] The file URL of the working directory of the new proces. If the specified directory
    does not exist or is inaccessible the working directory of the newly created process
    is undefined. If this parameter is NULL or the caller provides an empty string the
    new process will have the same current working directory as the calling process.

    @param ustrEnvironments
    [in] An array of strings describing environment variables that should be merged into the
    environment of the new process. Each string has to be in the form "variable=value".
    This parameter can be NULL in which case the new process gets the same environment
    as the parent process.

    @param nEnvironmentVars
    [in] The number of environment variables to set.

    @param pProcess
    [out] Pointer to a oslProcess variable, wich receives the handle of the newly created process.
    This parameter must not be NULL.

    @param pChildInputWrite
    [in] Pointer to a oslFileHandle variable that receives the handle which can be used to write
    to the child process standard input device. The returned handle is not random accessible.
    The handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @param pChildOutputRead
    [in] Pointer to a oslFileHandle variable that receives the handle which can be used to read from
    the child process standard output device. The returned handle is not random accessible.
    The Handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @param pChildErrorRead
    [in] Pointer to a oslFileHandle variable that receives the handle which can be used to read from
    the child process standard error device. The returned handle is not random accessible.
    The Handle has to be closed with osl_closeFile if no longer used. This parameter can be NULL.

    @return
    <dl>
    <dt>osl_Process_E_None</dt>
    <dd>on success</dd>
    <dt>osl_Process_E_NotFound</dt>
    <dd>if the specified executable could not be found</dd>
    <dt>osl_Process_E_InvalidError</dt>
    <dd>if invalid parameters will be detected</dd>
    <dt>osl_Process_E_Unknown</dt>
    <dd>if arbitrary other errors occur</dd>
    </dl>

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
    @param Process [in] the handle of the process to be terminated

    @see osl_executeProcess
    @see osl_getProcess
    @see osl_joinProcess
 */
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_terminateProcess(
        oslProcess Process);


/** @deprecated
    Retrieve the process handle of a process identifier
    @param Ident [in] a process identifier

    @return the process handle on success, NULL in all other cases
 */
SAL_DLLPUBLIC oslProcess SAL_CALL osl_getProcess(
        oslProcessIdentifier Ident);


/** Free the specified proces-handle.
    @param Process [in]
*/
SAL_DLLPUBLIC void SAL_CALL osl_freeProcessHandle(
        oslProcess Process);


/** Wait for completation of the specified childprocess.
    @param Process [in]
    @return ols_Process_E_None
    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_joinProcess(
        oslProcess Process);

/** Wait with a timeout for the completion of the specified child
    process.

    @param Process [in]
    A process identifier.

    @param pTimeout [in]
    A timeout value or NULL for infinite waiting.
    The unit of resolution is second.

    @return
    osl_Process_E_None on success
    osl_Process_E_TimedOut waiting for the child process timed out
    osl_Process_E_Unknown an error occurred or the parameter are invalid

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
    @param[out] pInfo   a pointer to a vaid oslProcessInfo structure.
                        the Size field has to be initialized with the size
                        of the oslProcessInfo structure.
                        on success the the Field member holds the (or'ed)
                        retrieved valid information fields.
    @return osl_Process_E_None on success, osl_Process_E_Unknown on failure.
 */
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessInfo(
        oslProcess Process, oslProcessData Fields, oslProcessInfo* pInfo);

/** Get the filename of the executable.
    @param strFile [out] the string that receives the executable file path.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getExecutableFile(
        rtl_uString **strFile);

/** @return the number of commandline arguments passed to the main-function of
    this process
    @see osl_getCommandArg
*/
SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getCommandArgCount(void);

/** Get the nArg-th command-line argument passed to the main-function of this process.
    @param nArg [in] The number of the argument to return.
    @param strCommandArg [out] The string receives the nArg-th command-line argument.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getCommandArg(
        sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Set the command-line arguments as passed to the main-function of this process.

    Depricated: This function is only for internal use. Passing the args from main will
    only work for Unix, on Windows there's no effect, the full command line will automtically
    be taken. This is due to Windows 9x/ME limitation that don't allow UTF-16 wmain to provide
    a osl_setCommandArgsU( int argc, sal_Unicode **argv );

    @param argc [in] The number of elements in the argv array.
    @param argv [in] The array of command-line arguments.
    @see osl_getExecutableFile
    @see osl_getCommandArgCount
    @see osl_getCommandArg
*/
SAL_DLLPUBLIC void SAL_CALL osl_setCommandArgs (int argc, char **argv);

/** Get the value of one enviroment variable.
    @param strVar [in] denotes the name of the variable to get.
    @param strValue [out] string that receives the value of environment variable.
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getEnvironment(
        rtl_uString *strVar, rtl_uString **strValue);

/** Set the value of one enviroment variable.
    @param strVar [in] denotes the name of the variable to set.
    @param strValue [in] string of the new value of environment variable.

    @since UDK 3.2.13
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_setEnvironment(
        rtl_uString *strVar, rtl_uString *strValue);

/** Unsets the value of one enviroment variable.
    @param strVar [in] denotes the name of the variable to unset.

    @since UDK 3.2.13
*/
SAL_DLLPUBLIC oslProcessError SAL_CALL osl_clearEnvironment(
        rtl_uString *strVar);

/** Get the working directory of the current process as a file URL.

    The file URL is encoded as common for the OSL file API.
    @param  pustrWorkingDir [out] string that receives the working directory file URL.
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessWorkingDir(
        rtl_uString **pustrWorkingDir );

/** Get the locale the process is currently running in.

    The unix implementation caches the value it returns, so if you have to change the locale
    your are running in, you will have to use osl_setProcessLocale therefor.

    @param  ppLocale [out] a pointer that receives the currently selected locale structure
    @see osl_setProcessLocale
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_getProcessLocale(
        rtl_Locale ** ppLocale );

/** Change the locale of the process.

    @param  pLocale [in] a pointer to the locale to be set
    @see osl_getProcessLocale
*/

SAL_DLLPUBLIC oslProcessError SAL_CALL osl_setProcessLocale(
        rtl_Locale * pLocale );


SAL_DLLPUBLIC sal_Bool SAL_CALL osl_sendResourcePipe(
        oslPipe Pipe, oslSocket Socket );

SAL_DLLPUBLIC oslSocket SAL_CALL osl_receiveResourcePipe(
        oslPipe Pipe );

#ifdef __cplusplus
}
#endif

#endif    /* _OSL_PROCESS_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
