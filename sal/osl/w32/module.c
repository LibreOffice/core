/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hro $ $Date: 2001-09-03 10:48:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "system.h"
#include <tlhelp32.h>

#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <systools/win32/kernel9x.h>

/*
    under WIN32, we use the void* oslModule
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/
oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 nRtldMode )
{
    HINSTANCE hInstance;
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    rtl_uString* Module = NULL;
    oslModule ret = 0;
    oslFileError    nError;

    OSL_ASSERT(strModuleName);

    nError = osl_getSystemPathFromFileURL(strModuleName, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, strModuleName);

    hInstance = LoadLibraryW(Module->buffer);

    if (hInstance <= (HINSTANCE)HINSTANCE_ERROR)
        hInstance = 0;

    ret = (oslModule) hInstance;
    rtl_uString_release(Module);
    SetErrorMode(errorMode);

    return ret;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule Module)
{
    OSL_ASSERT(Module);
    FreeLibrary((HINSTANCE)Module);
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *strSymbolName)
{
    rtl_String *symbolName = NULL;
    void *address;

    OSL_ASSERT(Module);
    OSL_ASSERT(strSymbolName);

    rtl_uString2String(
        &symbolName,
        strSymbolName->buffer,
        strSymbolName->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS
    );

    address=(void*)GetProcAddress((HINSTANCE)Module, rtl_string_getStr(symbolName));
    rtl_string_release(symbolName);

    return address;
}


/*****************************************************************************/
/* osl_addressGetModuleURL */
/*****************************************************************************/

/*****************************************************************************/
/* Implementation for Windows 95, 98 and Me */
/*****************************************************************************/

/* Undefine because there is no explicit "A" definition */

#ifdef MODULEENTRY32
#undef MODULEENTRY32
#endif

#ifdef LPMODULEENTRY32
#undef LPMODULEENTRY32
#endif

typedef HANDLE (WINAPI *CreateToolhelp32Snapshot_PROC)( DWORD dwFlags, DWORD th32ProcessID );
typedef BOOL (WINAPI *Module32First_PROC)( HANDLE   hSnapshot, LPMODULEENTRY32 lpme32 );
typedef BOOL (WINAPI *Module32Next_PROC)( HANDLE    hSnapshot, LPMODULEENTRY32 lpme32 );

static sal_Bool SAL_CALL _osl_addressGetModuleURL_Windows( void *pv, rtl_uString **pustrURL )
{
    sal_Bool    bSuccess        = sal_False;    /* Assume failure */
    HMODULE     hModKernel32    = GetModuleHandleA( "KERNEL32.DLL" );

    if ( hModKernel32 )
    {
        CreateToolhelp32Snapshot_PROC   lpfnCreateToolhelp32Snapshot = (CreateToolhelp32Snapshot_PROC)GetProcAddress( hModKernel32, "CreateToolhelp32Snapshot" );
        Module32First_PROC              lpfnModule32First = (Module32First_PROC)GetProcAddress( hModKernel32, "Module32First" );
        Module32Next_PROC               lpfnModule32Next = (Module32Next_PROC)GetProcAddress( hModKernel32, "Module32Next" );

        if ( lpfnCreateToolhelp32Snapshot && lpfnModule32First && lpfnModule32Next )
        {
            HANDLE  hModuleSnap = NULL;
            BOOL    fSuccess    = FALSE;
            DWORD   dwProcessId = GetCurrentProcessId();

            // Take a snapshot of all modules in the specified process.

            hModuleSnap = lpfnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId );

            if ( INVALID_HANDLE_VALUE != hModuleSnap )
            {
                MODULEENTRY32   me32    = {0};

                // Fill the size of the structure before using it.

                me32.dwSize = sizeof(MODULEENTRY32);

                // Walk the module list of the process, and find the module of
                // interest. Then copy the information to the buffer pointed
                // to by lpMe32 so that it can be returned to the caller.

                if ( Module32First(hModuleSnap, &me32) )
                {
                    do
                    {
                        if ( (BYTE *)pv >= (BYTE *)me32.hModule && (BYTE *)pv < (BYTE *)me32.hModule + me32.modBaseSize )
                        {
                            rtl_uString *ustrSysPath = NULL;

                            rtl_string2UString( &ustrSysPath, me32.szExePath, strlen(me32.szExePath), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
                            osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                            rtl_uString_release( ustrSysPath );

                            bSuccess = sal_True;
                        }

                    } while ( !bSuccess && Module32Next( hModuleSnap, &me32 ) );
                }


                // Do not forget to clean up the snapshot object.

                CloseHandle (hModuleSnap);
            }

        }
    }

    return  bSuccess;
}

/*****************************************************************************/
/* Implementation for Windows 95, 98 and Me */
/*****************************************************************************/

typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

typedef BOOL (WINAPI *EnumProcessModules_PROC)(
  HANDLE hProcess,      // handle to the process
  HMODULE * lphModule,  // array to receive the module handles
  DWORD cb,             // size of the array
  LPDWORD lpcbNeeded    // receives the number of bytes returned
);

typedef BOOL (WINAPI *GetModuleInformation_PROC)(
  HANDLE hProcess,         // handle to the process
  HMODULE hModule,         // handle to the module
  LPMODULEINFO lpmodinfo,  // structure that receives information
  DWORD cb                 // size of the structure
);

#define bufsizeof(buffer) (sizeof(buffer) / sizeof((buffer)[0]))

static sal_Bool SAL_CALL _osl_addressGetModuleURL_NT( void *pv, rtl_uString **pustrURL )
{
    sal_Bool    bSuccess    = sal_False;    /* Assume failure */
    HMODULE     hModPsapi = LoadLibrary( "PSAPI.DLL" );

    if ( hModPsapi )
    {
        EnumProcessModules_PROC     lpfnEnumProcessModules      = (EnumProcessModules_PROC)GetProcAddress( hModPsapi, "EnumProcessModules" );
        GetModuleInformation_PROC   lpfnGetModuleInformation    = (GetModuleInformation_PROC)GetProcAddress( hModPsapi, "GetModuleInformation" );

        if ( lpfnEnumProcessModules && lpfnGetModuleInformation )
        {
            DWORD       cbNeeded = 0;
            HMODULE     *lpModules = NULL;
            DWORD       nModules = 0;
            UINT        iModule = 0;
            MODULEINFO  modinfo;

            lpfnEnumProcessModules( GetCurrentProcess(), NULL, 0, &cbNeeded );

            lpModules = (HMODULE *)_alloca( cbNeeded );
            lpfnEnumProcessModules( GetCurrentProcess(), lpModules, cbNeeded, &cbNeeded );

            nModules = cbNeeded / sizeof(HMODULE);

            for ( iModule = 0; !bSuccess && iModule < nModules; iModule++ )
            {
                lpfnGetModuleInformation( GetCurrentProcess(), lpModules[iModule], &modinfo, sizeof(modinfo) );

                if ( (BYTE *)pv >= (BYTE *)modinfo.lpBaseOfDll && (BYTE *)pv < (BYTE *)modinfo.lpBaseOfDll + modinfo.SizeOfImage )
                {
                    WCHAR   szBuffer[MAX_PATH];
                    rtl_uString *ustrSysPath = NULL;

                    GetModuleFileNameW( lpModules[iModule], szBuffer, bufsizeof(szBuffer) );

                    rtl_uString_newFromStr( &ustrSysPath, szBuffer );
                    osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                    rtl_uString_release( ustrSysPath );

                    bSuccess = sal_True;
                }
            }
        }

        FreeLibrary( hModPsapi );
    }

    return bSuccess;
}

/*****************************************************************************/
/* Dispatcher for osl_osl_addressGetModuleURL */
/*****************************************************************************/

sal_Bool SAL_CALL osl_getModuleURLFromAddress( void *pv, rtl_uString **pustrURL )
{
    if ( IS_NT )
        return _osl_addressGetModuleURL_NT( pv, pustrURL );
    else
        return _osl_addressGetModuleURL_Windows( pv, pustrURL );
}


