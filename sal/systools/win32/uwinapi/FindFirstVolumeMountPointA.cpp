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


#include "macros.h"
#ifdef __MINGW32__
DEFINE_DEFAULT_THUNK( kernel32, TRYLOAD, HANDLE, WINAPI, FindFirstVolumeMountPointA, (LPSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength) )
#else
DEFINE_DEFAULT_THUNK( kernel32, TRYLOAD, HANDLE, WINAPI, FindFirstVolumeMountPointA, (LPCSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength) )
#endif
/*
extern "C" _declspec( dllexport ) FARPROC kernel32_FindFirstVolumeMountPointA_Ptr;
static HANDLE __stdcall FindFirstVolumeMountPointA_Failure (LPSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength);
static _declspec ( naked ) void FindFirstVolumeMountPointA_Thunk()
{
     ResolveThunk_TRYLOAD( &kernel32_FindFirstVolumeMountPointA_Ptr, "kernel32" ".dll", "FindFirstVolumeMountPointA", 0, (FARPROC)FindFirstVolumeMountPointA_Failure );
     _asm jmp [kernel32_FindFirstVolumeMountPointA_Ptr] }

//extern "C" _declspec( naked ) HANDLE __stdcall
//extern "C" HANDLE WINAPI
extern "C" _declspec( naked ) HANDLE __stdcall FindFirstVolumeMountPointA (LPCSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength)
{
    _asm jmp [kernel32_FindFirstVolumeMountPointA_Ptr]
}

extern "C" _declspec( dllexport ) FARPROC kernel32_FindFirstVolumeMountPointA_Ptr = (FARPROC)FindFirstVolumeMountPointA_Thunk;
static HANDLE __stdcall FindFirstVolumeMountPointA_Failure (LPSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength)
{ SetLastError( 120L ); return (HANDLE)0; }
*/
