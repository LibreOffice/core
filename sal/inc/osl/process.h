/*************************************************************************
 *
 *  $RCSfile: process.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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
#define     osl_Process_IDENTIFIER  0x0001
#define     osl_Process_EXITCODE    0x0002
#define     osl_Process_CPUTIMES    0x0004
#define     osl_Process_HEAPUSAGE   0x0008

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
    sal_uInt32               Size;
    oslProcessData       Fields;
    oslProcessIdentifier Ident;
    oslProcessExitCode   Code;
    TimeValue            UserTime;
    TimeValue            SystemTime;
    sal_uInt32               HeapUsage;
} oslProcessInfo;

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
#endif

/** Process handle
    @see osl_executeProcess
    @see osl_freeProcessHandle
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

oslProcessError SAL_CALL osl_terminateProcess(oslProcess Process);

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

/** Receive a single io-resource inherited by a parent process using the Netsape
    portable runtime.
    NOTE: Currently only pipes are supported.
    @param pResource [out] the buffer to be filled.
    @param name [in] the name identifying the requested handle.
*/
oslProcessError SAL_CALL osl_getIOResource(oslIOResource *pResource, const char * name);

sal_Bool SAL_CALL osl_sendResourcePipe(oslPipe Pipe, oslSocket Socket);

oslSocket SAL_CALL osl_receiveResourcePipe(oslPipe Pipe);

#ifdef __cplusplus
}
#endif

#endif    /* _OSL_PROCESS_H_ */

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.13  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.12  2000/07/31 17:41:51  mfe
*    send and receive resource dummy implemented
*
*    Revision 1.11  2000/07/21 16:15:41  mfe
*    IO resource funcs added
*
*    Revision 1.10  2000/06/18 12:18:57  obr
*    enable inheritance of nspr file handles
*
*    Revision 1.9  2000/05/17 14:30:14  mfe
*    comments upgraded
*
*    Revision 1.8  2000/03/31 17:02:04  rs
*    UNICODE-Changes
*
*    Revision 1.7  2000/03/27 09:17:50  hro
*    UNICODE New osl_getThreadTextEncoding
*
*    Revision 1.6  2000/03/20 15:23:15  obr
*    Unicode API changes
*
*    Revision 1.5  2000/03/17 13:41:25  obr
*    Fixed parameter/docu
*
*    Revision 1.4  2000/03/17 13:05:34  obr
*    osl_getDefaulttextEncoding added.
*
*    Revision 1.3  2000/03/17 12:03:56  obr
*    Unicode API changes
*
*    Revision 1.2  1999/10/27 15:02:08  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:19  jsc
*    verschoben aus osl
*
*    Revision 1.18  1999/08/05 11:14:38  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.17  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.16  1998/07/20 17:17:33  rh
*    #53072, #53073 Bugfixes for 5.0
*
*    Revision 1.15  1998/03/13 15:07:31  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*    Revision 1.14  1998/02/16 19:34:51  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.13  1997/11/28 08:55:50  fm
*    osl_PTSOCKETSEMAPHORE added
*
*    Revision 1.12  1997/09/22 16:36:15  rh
*    terminate added
*
*    Revision 1.11  1997/07/31 15:28:39  ts
*    *** empty log message ***
*
*    Revision 1.10  1997/07/25 10:01:07  fm
*    *** empty log message ***
*
*    Revision 1.9  1997/07/22 14:29:29  rh
*    process added
*
*    Revision 1.8  1997/07/18 08:45:13  ts
*    *** empty log message ***
*
*    Revision 1.7  1997/07/17 19:01:44  ts
*    *** empty log message ***
*
*    Revision 1.6  1997/07/17 11:20:54  ts
*    *** empty log message ***
*
*    Revision 1.5  1997/07/17 11:02:27  rh
*    Header adapted and profile added
*
*    Revision 1.4  1997/07/15 19:01:58  rh
*    system.h inserted
*
*    Revision 1.3  1997/07/14 09:09:11  rh
*    Adaptions for killable sleeps
*
*    Revision 1.2  1997/07/11 10:14:28  ts
*    *** empty log message ***
*
*    Revision 1.1  1997/07/11 07:33:02  rh
*    added
*
*************************************************************************/
