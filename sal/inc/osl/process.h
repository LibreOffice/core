/*************************************************************************
 *
 *  $RCSfile: process.h,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:39:59 $
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


#ifndef _OSL_PROCESS_H_
#define _OSL_PROCESS_H_

#ifndef _RTL_USTRING_H
#   include <rtl/ustring.h>
#endif
#ifndef _RTL_TEXTENC_H
#   include <rtl/textenc.h>
#endif
#ifndef _RTL_LOCALE_H
#   include <rtl/locale.h>
#endif

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _OSL_FILE_H_
#   include <osl/file.h>
#endif

#ifndef _OSL_PIPE_H_
#include <osl/pipe.h>
#endif

#ifndef _OSL_SOCKET_H_
#   include <osl/socket.h>
#endif
#ifndef _OSL_SECURITY_H_
#   include <osl/security.h>
#endif

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
    osl_Process_E_TimedOut,             /* timout occured */
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
#elif defined(SAL_OS2)
#   pragma pack(1)
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

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
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

    @param ustrEnviroments
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
oslProcessError SAL_CALL osl_executeProcess(
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

    @param ustrEnviroments
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
oslProcessError SAL_CALL osl_executeProcess_WithRedirectedIO(
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
oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process);


/** @deprecated
    Retrieve the process handle of a process identifier
    @param Ident [in] a process identifier

    @return the process handle on success, NULL in all other cases
 */
oslProcess SAL_CALL osl_getProcess(oslProcessIdentifier Ident);


/** Free the specified proces-handle.
    @param Process [in]
*/
void SAL_CALL osl_freeProcessHandle(oslProcess Process);


/** Wait for completation of the specified childprocess.
    @param Process [in]
    @return ols_Process_E_None
    @see osl_executeProcess
*/
oslProcessError SAL_CALL osl_joinProcess(oslProcess Process);

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
    osl_Process_E_Unknown an error occured or the parameter are invalid

    @see osl_executeProcess
*/
oslProcessError SAL_CALL osl_joinProcessWithTimeout(oslProcess Process, const TimeValue* pTimeout);

/** Retrieves information about a Process
    @param Process [in] the process handle of the process
    @param Field   [in] the information which is to be retrieved
                        this can be one or more of
                        osl_Process_IDENTIFIER
                        osl_Process_EXITCODE
                        osl_Process_CPUTIMES
                        osl_Process_HEAPUSAGE
    @param pInfo  [out] a pointer to a vaid oslProcessInfo structure.
                        the Size field has to be initialized with the size
                        of the oslProcessInfo structure.
                        on success the the Field member holds the (or'ed)
                        retrieved valid information fields.
    @return osl_Process_E_None on success, osl_Process_E_Unknown on failure.
 */
oslProcessError SAL_CALL osl_getProcessInfo(oslProcess Process, oslProcessData Fields,
                                   oslProcessInfo* pInfo);

/** Get the filename of the executable.
    @param strFile [out] the string that receives the executable file path.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess
*/
oslProcessError SAL_CALL osl_getExecutableFile(rtl_uString **strFile);

/** @return the number of commandline arguments passed to the main-function of
    this process
    @see osl_getCommandArg
*/
sal_uInt32 SAL_CALL osl_getCommandArgCount();

/** Get the nArg-th command-line argument passed to the main-function of this process.
    @param nArg [in] The number of the argument to return.
    @param strCommandArg [out] The string receives the nArg-th command-line argument.
    @return osl_Process_E_None or does not return.
    @see osl_executeProcess
*/
oslProcessError SAL_CALL osl_getCommandArg(sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Get the value of one enviroment variable.
    @param strVar [in] denotes the name of the variable to get.
    @param strValue [out] string that receives the value of environment variable.
*/
oslProcessError SAL_CALL osl_getEnvironment(rtl_uString *strVar, rtl_uString **strValue);

/** Get the working directory of the current process as a file URL.

    The file URL is encoded as common for the OSL file API.
    @param  pustrWorkingDir [out] string that receives the working directory file URL.
*/

oslProcessError SAL_CALL osl_getProcessWorkingDir( rtl_uString **pustrWorkingDir );

/** Get the locale the process is currently running in.

    The unix implementation caches the value it returns, so if you have to change the locale
    your are running in, you will have to use osl_setProcessLocale therefor.

    @param  ppLocale [out] a pointer that receives the currently selected locale structure
    @see osl_setProcessLocale
*/

oslProcessError SAL_CALL osl_getProcessLocale( rtl_Locale ** ppLocale );

/** Change the locale of the process.

    @param  pLocale [in] a pointer to the locale to be set
    @see osl_getProcessLocale
*/

oslProcessError SAL_CALL osl_setProcessLocale( rtl_Locale * pLocale );


sal_Bool SAL_CALL osl_sendResourcePipe(oslPipe Pipe, oslSocket Socket);

oslSocket SAL_CALL osl_receiveResourcePipe(oslPipe Pipe);

#ifdef __cplusplus
}
#endif

#endif    /* _OSL_PROCESS_H_ */

