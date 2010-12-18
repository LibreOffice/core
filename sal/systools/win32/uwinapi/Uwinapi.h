/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#pragma once

#ifdef _UWINAPI_
#define _KERNEL32_
#define _USER32_
#define _SHELL32_
#endif

#include <windows.h>
#include <malloc.h>

#ifndef _UWINAPI_
EXTERN_C WINBASEAPI DWORD UWINAPI_dwFakedVersion;
#endif

EXTERN_C WINBASEAPI DWORD SetVersion( DWORD dwVersion );

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


EXTERN_C WINBASEAPI LPSTR WINAPI lstrchrA( LPCSTR lpString, CHAR c );
EXTERN_C WINBASEAPI LPWSTR WINAPI lstrchrW( LPCWSTR lpString, WCHAR c );
EXTERN_C WINBASEAPI LPSTR WINAPI lstrrchrA( LPCSTR lpString, CHAR c );
EXTERN_C WINBASEAPI LPWSTR WINAPI lstrrchrW( LPCWSTR lpString, WCHAR c );

#ifdef UNICODE
#define lstrrchr    lstrrchrW
#define lstrchr     lstrchrW
#else
#define lstrrchr    lstrrchrA
#define lstrchr     lstrchrA
#endif

#define IsValidHandle(Handle)   ((DWORD)(Handle) + 1 > 1)

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

#endif  // __cplusplus

#define STRBUF2WSTR( lpStr, cchSrcBuffer, cchDestBuffer ) \
    MultiByteToWideChar( CP_ACP, 0, lpStr##A, cchSrcBuffer, lpStr##W, cchDestBuffer )

#define STR2WSTR( lpStr, cchBuffer ) \
    STRBUF2WSTR( lpStr, -1, cchBuffer )

#define WSTR2STR( lpStr, cchBuffer ) \
    WideCharToMultiByte( CP_ACP, 0, lpStr##W, -1, lpStr##A, cchBuffer, NULL, NULL )

EXTERN_C void WINAPI ResolveThunk_WINDOWS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );
EXTERN_C void WINAPI ResolveThunk_TRYLOAD( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );
EXTERN_C void WINAPI ResolveThunk_ALLWAYS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure );




#define IMPLEMENT_THUNK( module, resolve, rettype, calltype, func, params ) \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr; \
EXTERN_C rettype calltype func##_##resolve params; \
static rettype calltype func##_##Failure params; \
static _declspec ( naked ) func##_Thunk() \
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











#define DEFINE_CUSTOM_THUNK( module, resolve, rettype, calltype, func, params ) \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr; \
static _declspec ( naked ) func##_Thunk() \
{ \
    ResolveThunk_##resolve( &module##_##func##_Ptr, #module ".dll", #func ); \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( naked ) rettype calltype func params \
{ \
    _asm    jmp [module##_##func##_Ptr] \
} \
EXTERN_C _declspec( dllexport ) FARPROC module##_##func##_Ptr = (FARPROC)func##_Thunk;
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
