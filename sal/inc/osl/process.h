/*************************************************************************
 *
 *  $RCSfile: process.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:48:09 $
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

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
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

typedef union {
    oslSocket Socket;
    oslFileHandle File;
} oslResourceDescriptor;

/** */
typedef struct {
    oslDescriptorType     Type;
    oslDescriptorFlag     Flags;
    oslResourceDescriptor Descriptor;
} oslIOResource;

typedef struct {
    sal_uInt32           Size;
    oslProcessData       Fields;
    oslProcessIdentifier Ident;
    oslProcessExitCode   Code;
    TimeValue            UserTime;
    TimeValue            SystemTime;
    sal_uInt32           HeapUsage;
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
    @param strImageName [in] denotes the name of the executable to be started.
    @param strArguments [in] is an array of argument strings.
    @param nArgument [in] the number of arguments provided.
    @param Options [in] is a combination of int-constants to describe the mode of execution.
    @param Security [in] describes a the user and his rights for wich the process is started.
    @param strDirectory [in] denotes the name of the startup directory.
    @param strEnviroments [in] is an array of strings wich describes the enviroment to set.
    Each string has the form "variable=value".
    @param nEnvironmentVars [in] the number of environment vars to set.
    @param pResource [in] is a NULL terminated array of resources to transmit to the client process.
    @param pProcess [out] points to a oslProcess variable, in wich the processhandle is returned.
    @return osl_Process_E_None if the executable could be started, otherwise an error-code.
    @see osl_freeProcessHandle
    @see osl_getIOResources
    @see osl_loginUser
*/
oslProcessError SAL_CALL osl_executeProcess(rtl_uString *strImageName,
                                            rtl_uString *strArguments[],
                                            sal_uInt32   nArguments,
                                            oslProcessOption Options,
                                            oslSecurity Security,
                                            rtl_uString *strWorkDir,
                                            rtl_uString *strEnvironment[],
                                            sal_uInt32   nEnvironmentVars,
                                            oslIOResource* pResources,
                                            oslProcess *pProcess);


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

/** Receive io-resources like file descriptors or sockets from the parent process.
    @param Resources [out] is the buffer in wich the resources are returned.
    @param Max [in] is the size of this buffer.
    @return osl_Process_E_None if the variable exists, otherwise an errorcode.
    @see osl_executeProcess (provide the ioresources)
*/
oslProcessError SAL_CALL osl_getIOResources(oslIOResource Resources[], sal_uInt32 Max);

sal_Bool SAL_CALL osl_sendResourcePipe(oslPipe Pipe, oslSocket Socket);

oslSocket SAL_CALL osl_receiveResourcePipe(oslPipe Pipe);

#ifdef __cplusplus
}
#endif

#endif    /* _OSL_PROCESS_H_ */

