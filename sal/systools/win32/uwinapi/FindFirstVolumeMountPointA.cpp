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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
