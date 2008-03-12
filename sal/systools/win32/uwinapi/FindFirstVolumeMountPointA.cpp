/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FindFirstVolumeMountPointA.cpp,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:37:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "macros.h"
DEFINE_DEFAULT_THUNK( kernel32, TRYLOAD, HANDLE, WINAPI, FindFirstVolumeMountPointA, (LPCSTR lpszRootPathName, LPSTR lpszVolumeMountPoint, DWORD cchBufferLength) )
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