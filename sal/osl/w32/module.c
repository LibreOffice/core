/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 12:58:02 $
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

    nRtldMode = nRtldMode; /* avoid warnings */

    nError = osl_getSystemPathFromFileURL(strModuleName, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, strModuleName);

    hInstance = LoadLibraryW(Module->buffer);
    if (hInstance == NULL)
        hInstance = LoadLibraryExW(Module->buffer, NULL,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);

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
    /* casting from a function pointer to a data pointer is invalid
       be in this case unavoidable because the API has to stay
       compitable we need to keep this function which returns a
       void* by definition */
    return (void*)(osl_getFunctionSymbol(Module, strSymbolName));
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *strSymbolName )
{
    rtl_String *symbolName = NULL;
    oslGenericFunction address;

    OSL_ASSERT(Module);
    OSL_ASSERT(strSymbolName);

    rtl_uString2String(
        &symbolName,
        strSymbolName->buffer,
        strSymbolName->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS
    );

    address=(oslGenericFunction)GetProcAddress((HINSTANCE)Module, rtl_string_getStr(symbolName));
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

                if ( lpfnModule32First(hModuleSnap, &me32) )
                {
                    do
                    {
                        if ( (BYTE *)pv >= (BYTE *)me32.hModule && (BYTE *)pv < (BYTE *)me32.hModule + me32.modBaseSize )
                        {
                            rtl_uString *ustrSysPath = NULL;

                            rtl_string2UString( &ustrSysPath, me32.szExePath, strlen(me32.szExePath), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
                            OSL_ASSERT(ustrSysPath != NULL);
                            osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                            rtl_uString_release( ustrSysPath );

                            bSuccess = sal_True;
                        }

                    } while ( !bSuccess && lpfnModule32Next( hModuleSnap, &me32 ) );
                }


                // Do not forget to clean up the snapshot object.

                CloseHandle (hModuleSnap);
            }

        }
    }

    return  bSuccess;
}

/***************************************************************************************/
/* Implementation for Windows NT, 2K and XP (2K and XP could use the above method too) */
/***************************************************************************************/

#include <imagehlp.h>

typedef BOOL (WINAPI *SymInitialize_PROC)(
    HANDLE   hProcess,
    LPSTR    UserSearchPath,
    BOOL     fInvadeProcess
    );

typedef BOOL (WINAPI *SymCleanup_PROC)(
    HANDLE hProcess
    );

typedef BOOL (WINAPI *SymGetModuleInfo_PROC)(
    HANDLE              hProcess,
    DWORD               dwAddr,
    PIMAGEHLP_MODULE  ModuleInfo
    );

/* Seems that IMAGEHLP.DLL is always availiable on NT 4. But MSDN from Platform SDK says Win 2K is required. MSDN from VS 6.0a says
    it's O.K on NT 4 ???!!!
    BTW: We are using ANSI function because not all version of IMAGEHLP.DLL contain Unicode support
*/

static sal_Bool SAL_CALL _osl_addressGetModuleURL_NT4( void *pv, rtl_uString **pustrURL )
{
    sal_Bool    bSuccess    = sal_False;    /* Assume failure */

    /*  IMAGEHELP.DLL has a bug that it recursivly scans subdirectories of
        the root when calling SymInitialize(), so we preferr DBGHELP.DLL
        which exports the same symbols and is shipped with OOo */

    HMODULE     hModImageHelp = LoadLibrary( "DBGHELP.DLL" );

    if ( !hModImageHelp )
        hModImageHelp = LoadLibrary( "IMAGEHLP.DLL" );

    if ( hModImageHelp )
    {
        SymGetModuleInfo_PROC   lpfnSymGetModuleInfo;
        SymInitialize_PROC      lpfnSymInitialize;
        SymCleanup_PROC         lpfnSymCleanup;


        lpfnSymInitialize = (SymInitialize_PROC)GetProcAddress( hModImageHelp, "SymInitialize" );
        lpfnSymCleanup = (SymCleanup_PROC)GetProcAddress( hModImageHelp, "SymCleanup" );
        lpfnSymGetModuleInfo = (SymGetModuleInfo_PROC)GetProcAddress( hModImageHelp, "SymGetModuleInfo" );


        if ( lpfnSymInitialize && lpfnSymCleanup && lpfnSymGetModuleInfo )
        {
            IMAGEHLP_MODULE ModuleInfo;
            CHAR    szModuleFileName[MAX_PATH];
            LPCSTR  lpSearchPath = NULL;

            if ( GetModuleFileNameA( NULL, szModuleFileName, sizeof(szModuleFileName) ) )
            {
                char *pLastBkSlash = strrchr( szModuleFileName, '\\' );

                if (
                    pLastBkSlash &&
                    pLastBkSlash > szModuleFileName
                    && *(pLastBkSlash - 1) != ':'
                    && *(pLastBkSlash - 1) != '\\'
                    )
                {
                    *pLastBkSlash = 0;
                    lpSearchPath = szModuleFileName;
                }
            }

            lpfnSymInitialize( GetCurrentProcess(), lpSearchPath, TRUE );

            ZeroMemory( &ModuleInfo, sizeof(ModuleInfo) );
            ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

            bSuccess = (sal_Bool)(!!lpfnSymGetModuleInfo( GetCurrentProcess(), (DWORD)pv, &ModuleInfo ));

            if ( bSuccess )
            {
                /*  #99182 On localized (non-english) NT4 and XP (!!!) for some libraries the LoadedImageName member of ModuleInfo isn't filled. Because
                    other members ModuleName and ImageName do not contain the full path we can cast the Member
                    BaseOfImage to a HMODULE (on NT it's the same) and use GetModuleFileName to retrieve the full
                    path of the loaded image */

                if ( ModuleInfo.LoadedImageName[0] || GetModuleFileNameA( (HMODULE)ModuleInfo.BaseOfImage, ModuleInfo.LoadedImageName, sizeof(ModuleInfo.LoadedImageName) ) )
                {
                    rtl_uString *ustrSysPath = NULL;

                    rtl_string2UString( &ustrSysPath, ModuleInfo.LoadedImageName, strlen(ModuleInfo.LoadedImageName), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
                    OSL_ASSERT(ustrSysPath != NULL);
                    osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                    rtl_uString_release( ustrSysPath );
                }
                else
                    bSuccess = sal_False;
            }

            lpfnSymCleanup( GetCurrentProcess() );
        }

        FreeLibrary( hModImageHelp );
    }

    return bSuccess;
}


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

/* This version can fail because PSAPI.DLL is not always part of NT 4 despite MSDN Libary 6.0a say so */

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
    /* Use ..._NT first because ..._NT4 is much slower */
    if ( IS_NT )
        return _osl_addressGetModuleURL_NT( pv, pustrURL ) || _osl_addressGetModuleURL_NT4( pv, pustrURL );
    else
        return _osl_addressGetModuleURL_Windows( pv, pustrURL );
}

/*****************************************************************************/
/* osl_getModuleURLFromFunctionAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress( oslGenericFunction addr, rtl_uString ** ppLibraryUrl )
{
    /* casting a function pointer to a data pointer (void*) is
       not allowed according to the C/C++ standards. In this case
       it is unavoidable because we have to stay compatible we
       cannot remove any function. */
    return osl_getModuleURLFromAddress((void*)addr, ppLibraryUrl);
}


