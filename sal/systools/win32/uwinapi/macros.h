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

#define _UWINAPI_
#include <systools/win32/uwinapi.h>

#ifndef _INC_MALLOC
#   include <malloc.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4740)
#endif

#ifndef _INC_TCHAR
#   ifdef UNICODE
#       define _UNICODE
#   endif
#   include <TCHAR.H>
#endif

// Globally disable "warning C4100: unreferenced formal parameter" caused by
// IMPLEMENT_THUNK:
#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

/* Version macros */

#define MAKE_VER_WIN32( major, minor, build, isWindows ) \
((DWORD)MAKELONG( MAKEWORD( major, minor ), (build) | ( isWindows ? 0x8000 : 0 ) ))

#define MAKE_VER_WIN32_NT( major, minor, build ) \
    MAKE_VER_WIN32( major, minor, build, FALSE )

#define MAKE_VER_WIN32_WINDOWS( major, minor, build ) \
    MAKE_VER_WIN32( major, minor, build, TRUE )

#define VER_WIN32_WINDOWS_95    MAKE_VER_WIN32_WINDOWS( 4, 0, 0 )
#define VER_WIN32_WINDOWS_98    MAKE_VER_WIN32_WINDOWS( 4, 10, 0 )
#define VER_WIN32_WINDOWS_ME    MAKE_VER_WIN32_WINDOWS( 4, 90, 0 )
#define VER_WIN32_NT_NT4        MAKE_VER_WIN32_NT( 4, 0, 0 )
#define VER_WIN32_NT_2000       MAKE_VER_WIN32_NT( 5, 0, 0 )
#define VER_WIN32_NT_XP         MAKE_VER_WIN32_NT( 5, 1, 0 )


#ifdef __cplusplus

#define _AUTO_WSTR2STR( lpStrA, lpStrW ) \
LPSTR   lpStrA; \
if ( lpStrW ) \
{ \
    int cNeeded = WideCharToMultiByte( CP_ACP, 0, lpStrW, -1, NULL, 0, NULL, NULL ); \
    lpStrA = (LPSTR)_alloca( cNeeded * sizeof(CHAR) ); \
    WideCharToMultiByte( CP_ACP, 0, lpStrW, -1, lpStrA, cNeeded, NULL, NULL ); \
} \
else \
    lpStrA = NULL;


#define AUTO_WSTR2STR( lpStr ) \
    _AUTO_WSTR2STR( lpStr##A, lpStr##W )

#define AUTO_STR( lpStr, cchBuffer ) \
LPSTR   lpStr##A = lpStr##W ? (LPSTR)_alloca( (cchBuffer) * sizeof(CHAR) ) : NULL;

#endif  /* __cplusplus */


#define STRBUF2WSTR( lpStr, cchSrcBuffer, cchDestBuffer ) \
    MultiByteToWideChar( CP_ACP, 0, lpStr##A, cchSrcBuffer, lpStr##W, (int) cchDestBuffer )

#define STR2WSTR( lpStr, cchBuffer ) \
    STRBUF2WSTR( lpStr, -1, cchBuffer )

#define WSTR2STR( lpStr, cchBuffer ) \
    WideCharToMultiByte( CP_ACP, 0, lpStr##W, -1, lpStr##A, cchBuffer, NULL, NULL )

EXTERN_C void WINAPI ResolveThunk_WINDOWS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );
EXTERN_C void WINAPI ResolveThunk_TRYLOAD( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );
EXTERN_C void WINAPI ResolveThunk_ALLWAYS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );




#ifdef __MINGW32__
#define IMPLEMENT_THUNK( module, resolve, rettype, calltype, func, params ) \
static void func##_Thunk(); \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk; \
EXTERN_C rettype calltype func params \
{ \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
} \
EXTERN_C rettype calltype func##_##resolve params; \
static rettype calltype func##_##Failure params; \
static void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func, (FARPROC)func##_##resolve, (FARPROC)func##_##Failure ); \
    asm("   movl    %ebp, %esp"); \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
} \
static rettype calltype func##_##Failure params \
{ \
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED ); \
    return (rettype)0; \
} \
EXTERN_C rettype calltype func##_##resolve params
#else
#define IMPLEMENT_THUNK( module, resolve, rettype, calltype, func, params ) \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr; \
EXTERN_C rettype calltype func##_##resolve params; \
static rettype calltype func##_##Failure params; \
static _declspec ( naked ) void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func, (FARPROC)func##_##resolve, (FARPROC)func##_##Failure ); \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( naked ) rettype calltype func params \
{ \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk; \
static rettype calltype func##_##Failure params \
{ \
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED ); \
    return (rettype)0; \
} \
EXTERN_C rettype calltype func##_##resolve params
#endif



#ifdef __MINGW32__
#define DEFINE_CUSTOM_THUNK( module, resolve, rettype, calltype, func, params ) \
static void func##_Thunk(); \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk; \
static void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func ); \
    asm("   movl    %ebp, %esp"); \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
} \
EXTERN_C rettype calltype func params \
{ \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
}
#else
#define DEFINE_CUSTOM_THUNK( module, resolve, rettype, calltype, func, params ) \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr; \
static _declspec ( naked ) void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func ); \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( naked ) rettype calltype func params \
{ \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk;
#endif


#ifdef __MINGW32__
#define DEFINE_DEFAULT_THUNK( module, resolve, rettype, calltype, func, params ) \
static void func##_Thunk(); \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk; \
static rettype calltype func##_##Failure params; \
static _declspec ( naked ) void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func, NULL, (FARPROC)func##_##Failure ); \
    asm("   movl    %ebp, %esp"); \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
} \
EXTERN_C _declspec( naked ) rettype calltype func params \
{ \
    asm("   popl    %ebp"); \
    asm("   jmp *(%0)"::"m"(module##_##func##_Ptr)); \
} \
static rettype calltype func##_##Failure params \
{ \
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED ); \
    return (rettype)0; \
}
#else
#define DEFINE_DEFAULT_THUNK( module, resolve, rettype, calltype, func, params ) \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr; \
static rettype calltype func##_##Failure params; \
static _declspec ( naked ) void func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func, NULL, (FARPROC)func##_##Failure ); \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( naked ) rettype calltype func params \
{ \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk; \
static rettype calltype func##_##Failure params \
{ \
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED ); \
    return (rettype)0; \
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
