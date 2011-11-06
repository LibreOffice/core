/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
