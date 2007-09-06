/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DllMain.cpp,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 13:49:52 $
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
        return DisableThreadLibraryCalls( hModule );
    default:
        return TRUE;
    }

}





