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
#ifndef __NSP_FUNC_HXX
#define __NSP_FUNC_HXX


// Some function different from UNIX and Window.

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
static SECURITY_ATTRIBUTES  NSP_pipe_access = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
