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

#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <malloc.h>
#define _MBCS
#include <tchar.h>


HMODULE     UWINAPI_BaseAddress = NULL;
const CHAR  szUnicowsModuleName[] = "UNICOWS.DLL";

static HMODULE WINAPI _LoadUnicowsLibrary(VOID)
{
    CHAR        szModulePath[MAX_PATH];
    HMODULE     hModuleUnicows = NULL;

    // First search in the same directory as UWINAPI.DLL was loaded from. This is because
    // UWINAPI.DLL not always resides in the same directory as the actual application.

    if ( UWINAPI_BaseAddress && GetModuleFileNameA( UWINAPI_BaseAddress, szModulePath, MAX_PATH ) )
    {
        char    *lpLastBkSlash = _tcsrchr( szModulePath, '\\' );

        if ( lpLastBkSlash )
        {
            size_t  nParentDirSize = (size_t) (_tcsinc( lpLastBkSlash ) - szModulePath);
            LPSTR   lpUnicowsModulePath = (LPTSTR)_alloca( nParentDirSize + sizeof(szUnicowsModuleName) );

            if ( lpUnicowsModulePath )
            {
                _tcsncpy( lpUnicowsModulePath, szModulePath, nParentDirSize );
                _tcscpy( lpUnicowsModulePath + nParentDirSize, szUnicowsModuleName );

                hModuleUnicows = LoadLibraryA( lpUnicowsModulePath );
            }
        }
    }

    // Search at the common places

    if ( !hModuleUnicows )
        hModuleUnicows = LoadLibraryA(szUnicowsModuleName);

    return hModuleUnicows;
}

static HMODULE WINAPI LoadUnicowsLibrary(VOID)
{
    HMODULE hModuleUnicows;
    int     idMsg = IDOK;

    do
    {
        hModuleUnicows = _LoadUnicowsLibrary();

        if ( !hModuleUnicows )
        {
            LPVOID lpMsgBuf;

            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                ERROR_DLL_NOT_FOUND /* GetLastError() */,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPSTR)&lpMsgBuf,
                0,
                NULL
            );
            // Process any inserts in lpMsgBuf.
            CHAR szModuleFileName[MAX_PATH];

            GetModuleFileNameA( NULL, szModuleFileName, sizeof(szModuleFileName) );
            LPSTR   lpMessage = (LPSTR)_alloca( strlen( (LPCSTR)lpMsgBuf ) + sizeof(szUnicowsModuleName) + 1 );
            strcpy( lpMessage, (LPCSTR)lpMsgBuf );
            strcat( lpMessage, "\n" );
            strcat( lpMessage, szUnicowsModuleName );
            // Free the buffer.
            LocalFree( lpMsgBuf );
            // Display the string.
            idMsg = MessageBoxA( NULL, lpMessage,
                szModuleFileName, MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TASKMODAL );

            if ( IDABORT == idMsg )
                TerminateProcess( GetCurrentProcess(), 255 );
        }
    } while ( !hModuleUnicows && IDRETRY == idMsg );

    return hModuleUnicows;
}

extern "C" {
FARPROC _PfnLoadUnicows = (FARPROC)LoadUnicowsLibrary;
}

#ifdef __MINGW32__

extern "C" {

typedef void (*func_ptr) (void);
extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];

static void do_startup(void);
static void do_cleanup(void);

HMODULE hModuleUnicowsDLL;

void
__do_global_dtors (void)
{
  static func_ptr *p = __DTOR_LIST__ + 1;

  /*
   * Call each destructor in the destructor list until a null pointer
   * is encountered.
   */
  while (*p)
    {
      (*(p)) ();
      p++;
    }
}

void
__do_global_ctors (void)
{
  unsigned long nptrs = (unsigned long) __CTOR_LIST__[0];
  unsigned i;

  /*
   * If the first entry in the constructor list is -1 then the list
   * is terminated with a null entry. Otherwise the first entry was
   * the number of pointers in the list.
   */
  if (nptrs == static_cast<unsigned long>(-1))
    {
      for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++)
    ;
    }

  /*
   * Go through the list backwards calling constructors.
   */
  for (i = nptrs; i >= 1; i--)
    {
      __CTOR_LIST__[i] ();
    }

  /*
   * Register the destructors for processing on exit.
   */
  atexit (__do_global_dtors);
}

static int initialized = 0;

void
__main (void)
{
  if (!initialized)
    {
      initialized = 1;
      do_startup();
      __do_global_ctors ();
    }
}

static void do_startup( void )
{
    if (((LONG)GetVersion()&0x800000ff) == 0x80000004)
        {
        hModuleUnicowsDLL = LoadUnicowsLibrary();
        if (hModuleUnicowsDLL)
            atexit(do_cleanup);
    }
}

void do_cleanup( void )
{
    FreeLibrary(hModuleUnicowsDLL);
}
}

#endif

extern "C" BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID )
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        UWINAPI_BaseAddress = hModule;
#ifdef __MINGW32__
        return TRUE;
#else
        return DisableThreadLibraryCalls( hModule );
#endif
    default:
        return TRUE;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
