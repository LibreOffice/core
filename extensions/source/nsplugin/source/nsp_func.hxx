/*************************************************************************
 *
 *  $RCSfile: nsp_func.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-20 10:07:19 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __NSP_FUNC_HXX
#define __NSP_FUNC_HXX


// Some function diffrent from UNIX and Window.

/************************ In UNIX ************************/


#ifdef UNIX
// for chdir()
#define NSP_CHDIR   chdir
//for  vsnprintf()
#define NSP_vsnprintf    vsnprintf


//for pipe()
typedef int NSP_PIPE_FD;
#define NSP_Inherited_Pipe(fp) pipe(fd)
#define NSP_Close_Pipe(fp)    close(fp)
//for write(), read()
#define NSP_Write_Pipe(a, b, c, d) write(a, b, c)
#define NSP_Read_Pipe(a, b, c, d) read(a, b, c)

//for socket
#define NSP_CloseSocket close

//for sleep
#define NSP_Sleep(a) sleep(a)

//for mutex
#define NSP_Mute_Obj    pthread_mutex_t
#define NSP_New_Mute_Obj()    PTHREAD_MUTEX_INITIALIZER
#define NSP_Lock_Mute_Obj(a)  pthread_mutex_lock(&a)
#define NSP_Unlock_Mute_Obj(a)  pthread_mutex_unlock(&a)

#define STRNICMP(a, b ,c)  strncasecmp(a, b ,c)
#endif //end of UNIX

/************************ In WINWS ************************/

#ifdef WNT

#include <windows.h>
//for chdir()
#define NSP_CHDIR(a)   _chdir(a)
//for  vsnprintf()
#define NSP_vsnprintf    _vsnprintf

//for pipe()
typedef HANDLE NSP_PIPE_FD;
SECURITY_ATTRIBUTES  NSP_pipe_access = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
#define NSP_Inherited_Pipe(fd) (!CreatePipe(&fd[0], &fd[1], &NSP_pipe_access, 1024*10))
#define NSP_Close_Pipe(fp)    CloseHandle(fp)
//for write(), read()
#define NSP_Write_Pipe(a, b, c, d) WriteFile(a, b, c, d, NULL)
#define NSP_Read_Pipe(a, b, c, d) ReadFile(a, b, c, d, NULL)

//for execl()
#define NSP_CreateProcess(a, b ) CreateProcess(a, b, NULL, NULL, TRUE, 0, NULL, NULL, NULL, NULL)

//for socket
#define NSP_CloseSocket closesocket
//for sleep
#define NSP_Sleep(a) Sleep(a*1000)

//for mutex
#define NSP_Mute_Obj    HANDLE
#define NSP_New_Mute_Obj()    CreateMutex (NULL, FALSE, NULL)
#define NSP_Lock_Mute_Obj(a)  WaitForSingleObject(a, INFINITE)
#define NSP_Unlock_Mute_Obj(a)  ReleaseMutex(a)
#define STRNICMP(a, b ,c)  strnicmp(a, b ,c)

#endif //end of WNT


#endif //end of  __NSP_FUNC_HXX
