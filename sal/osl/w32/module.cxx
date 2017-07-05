/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "system.h"
#include <tlhelp32.h>

#include "file_url.hxx"
#include "path_helper.hxx"

#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <vector>

/*
    under WIN32, we use the void* oslModule
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/
oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 /*nRtldMode*/ )
{
    HMODULE h;
#if OSL_DEBUG_LEVEL < 2
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif
    rtl_uString* Module = nullptr;
    oslModule ret = nullptr;
    oslFileError    nError;

    SAL_INFO( "sal.osl", "osl_loadModule: " << OUString(strModuleName->buffer, wcslen(SAL_W(strModuleName->buffer))) );
    assert(strModuleName);

    nError = osl_getSystemPathFromFileURL(strModuleName, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, strModuleName);

    h = LoadLibraryW(reinterpret_cast<LPCWSTR>(Module->buffer));

    if (h == nullptr)
        h = LoadLibraryExW(reinterpret_cast<LPCWSTR>(Module->buffer), nullptr,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);

    //In case of long path names (\\?\c:\...) try to shorten the filename.
    //LoadLibrary cannot handle file names which exceed 260 letters.
    //In case the path is to long, the function will fail. However, the error
    //code can be different. For example, it returned  ERROR_FILENAME_EXCED_RANGE
    //on Windows XP and ERROR_INSUFFICIENT_BUFFER on Windows 7 (64bit)
    if (h == nullptr && Module->length > 260)
    {
        std::vector<WCHAR> vec(Module->length + 1);
        DWORD len = GetShortPathNameW(reinterpret_cast<LPCWSTR>(Module->buffer),
                                      reinterpret_cast<LPWSTR>(&vec[0]), Module->length + 1);
        if (len )
        {
            h = LoadLibraryW(reinterpret_cast<LPWSTR>(&vec[0]));

            if (h == nullptr)
                h = LoadLibraryExW(reinterpret_cast<LPWSTR>(&vec[0]), nullptr,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);
        }
    }

    ret = static_cast<oslModule>(h);
    rtl_uString_release(Module);
#if OSL_DEBUG_LEVEL < 2
    SetErrorMode(errorMode);
#endif

    return ret;
}

/*****************************************************************************/
/* osl_loadModuleAscii */
/*****************************************************************************/
oslModule SAL_CALL osl_loadModuleAscii(const sal_Char *pModuleName, sal_Int32 )
{
    HMODULE h;
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    oslModule ret = nullptr;

    SAL_INFO( "sal.osl", "osl_loadModule: " << pModuleName );
    assert(pModuleName);

    h = LoadLibrary(pModuleName);
    if (h == nullptr)
        h = LoadLibraryEx(pModuleName, nullptr,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);

    ret = static_cast<oslModule>(h);
    SetErrorMode(errorMode);

    return ret;
}

oslModule osl_loadModuleRelativeAscii(
    oslGenericFunction, char const * relativePath, sal_Int32 mode)
{
    return osl_loadModuleAscii(relativePath, mode); //TODO: FIXME
}

/*****************************************************************************/
/* osl_getModuleHandle */
/*****************************************************************************/

sal_Bool SAL_CALL
osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult)
{
    LPCWSTR pName = pModuleName ? reinterpret_cast<LPCWSTR>(pModuleName->buffer) : nullptr;
    HMODULE h = GetModuleHandleW(pName);
    if( h )
    {
        *pResult = static_cast<oslModule>(h);
        return true;
    }

    return false;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule Module)
{
    FreeLibrary(static_cast<HMODULE>(Module));
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *strSymbolName)
{
    /* casting from a function pointer to a data pointer is invalid
       be in this case unavoidable because the API has to stay
       compatible. We need to keep this function which returns a
       void* by definition */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4054)
#endif
    return reinterpret_cast<void*>(osl_getFunctionSymbol(Module, strSymbolName));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *strSymbolName )
{
    rtl_String *symbolName = nullptr;
    oslGenericFunction address;

    assert(Module);
    assert(strSymbolName);

    rtl_uString2String(
        &symbolName,
        strSymbolName->buffer,
        strSymbolName->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS
    );

    address=osl_getAsciiFunctionSymbol(Module, rtl_string_getStr(symbolName));
    rtl_string_release(symbolName);

    return address;
}

/*****************************************************************************/
/* osl_getAsciiFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getAsciiFunctionSymbol( oslModule Module, const sal_Char *pSymbol )
{
    oslGenericFunction fncAddr = nullptr;

    if( pSymbol )
        fncAddr=reinterpret_cast<oslGenericFunction>(GetProcAddress(static_cast<HMODULE>(Module), pSymbol));

    return fncAddr;
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

/***************************************************************************************/
/* Implementation for Windows NT, 2K and XP (2K and XP could use the above method too) */
/***************************************************************************************/

#ifdef _MSC_VER
#pragma warning(push,1) /* disable warnings within system headers */
#endif
#include <imagehlp.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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

/* Seems that IMAGEHLP.DLL is always available on NT 4. But MSDN from Platform SDK says Win 2K is required. MSDN from VS 6.0a says
    it's O.K on NT 4 ???!!!
    BTW: We are using ANSI function because not all version of IMAGEHLP.DLL contain Unicode support
*/

static bool SAL_CALL osl_addressGetModuleURL_NT4_( void *pv, rtl_uString **pustrURL )
{
    bool    bSuccess    = false;    /* Assume failure */

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

        lpfnSymInitialize = reinterpret_cast<SymInitialize_PROC>(GetProcAddress( hModImageHelp, "SymInitialize" ));
        lpfnSymCleanup = reinterpret_cast<SymCleanup_PROC>(GetProcAddress( hModImageHelp, "SymCleanup" ));
        lpfnSymGetModuleInfo = reinterpret_cast<SymGetModuleInfo_PROC>(GetProcAddress( hModImageHelp, "SymGetModuleInfo" ));

        if ( lpfnSymInitialize && lpfnSymCleanup && lpfnSymGetModuleInfo )
        {
            IMAGEHLP_MODULE ModuleInfo;
            ::osl::LongPathBuffer< sal_Char > aModuleFileName( MAX_LONG_PATH );
            LPSTR   lpSearchPath = nullptr;

            if ( GetModuleFileNameA( nullptr, aModuleFileName, aModuleFileName.getBufSizeInSymbols() ) )
            {
                char *pLastBkSlash = strrchr( aModuleFileName, '\\' );

                if (
                    pLastBkSlash &&
                    pLastBkSlash > static_cast<sal_Char*>(aModuleFileName)
                    && *(pLastBkSlash - 1) != ':'
                    && *(pLastBkSlash - 1) != '\\'
                    )
                {
                    *pLastBkSlash = 0;
                    lpSearchPath = aModuleFileName;
                }
            }

            lpfnSymInitialize( GetCurrentProcess(), lpSearchPath, TRUE );

            ZeroMemory( &ModuleInfo, sizeof(ModuleInfo) );
            ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);

            bSuccess = !!lpfnSymGetModuleInfo( GetCurrentProcess(), reinterpret_cast<DWORD_PTR>(pv), &ModuleInfo );

            if ( bSuccess )
            {
                /*  #99182 On localized (non-english) NT4 and XP (!!!) for some libraries the LoadedImageName member of ModuleInfo isn't filled. Because
                    other members ModuleName and ImageName do not contain the full path we can cast the Member
                    BaseOfImage to a HMODULE (on NT it's the same) and use GetModuleFileName to retrieve the full
                    path of the loaded image */

                if ( ModuleInfo.LoadedImageName[0] || GetModuleFileNameA( reinterpret_cast<HMODULE>(ModuleInfo.BaseOfImage), ModuleInfo.LoadedImageName, sizeof(ModuleInfo.LoadedImageName) ) )
                {
                    rtl_uString *ustrSysPath = nullptr;

                    rtl_string2UString( &ustrSysPath, ModuleInfo.LoadedImageName, strlen(ModuleInfo.LoadedImageName), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
                    assert(ustrSysPath);
                    osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                    rtl_uString_release( ustrSysPath );
                }
                else
                    bSuccess = false;
            }

            lpfnSymCleanup( GetCurrentProcess() );
        }

        FreeLibrary( hModImageHelp );
    }

    return bSuccess;
}

typedef struct MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} *LPMODULEINFO;

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

/* This version can fail because PSAPI.DLL is not always part of NT 4 despite MSDN Library 6.0a say so */

static bool SAL_CALL osl_addressGetModuleURL_NT_( void *pv, rtl_uString **pustrURL )
{
    bool    bSuccess    = false;    /* Assume failure */
    static HMODULE      hModPsapi = nullptr;

    if ( !hModPsapi )
        hModPsapi = LoadLibrary( "PSAPI.DLL" );

    if ( hModPsapi )
    {
        EnumProcessModules_PROC     lpfnEnumProcessModules      = reinterpret_cast<EnumProcessModules_PROC>(GetProcAddress( hModPsapi, "EnumProcessModules" ));
        GetModuleInformation_PROC   lpfnGetModuleInformation    = reinterpret_cast<GetModuleInformation_PROC>(GetProcAddress( hModPsapi, "GetModuleInformation" ));

        if ( lpfnEnumProcessModules && lpfnGetModuleInformation )
        {
            DWORD       cbNeeded = 0;
            HMODULE     *lpModules = nullptr;
            DWORD       nModules = 0;
            UINT        iModule = 0;
            MODULEINFO  modinfo;

            lpfnEnumProcessModules( GetCurrentProcess(), nullptr, 0, &cbNeeded );

            lpModules = static_cast<HMODULE *>(_alloca( cbNeeded ));
            lpfnEnumProcessModules( GetCurrentProcess(), lpModules, cbNeeded, &cbNeeded );

            nModules = cbNeeded / sizeof(HMODULE);

            for ( iModule = 0; !bSuccess && iModule < nModules; iModule++ )
            {
                lpfnGetModuleInformation( GetCurrentProcess(), lpModules[iModule], &modinfo, sizeof(modinfo) );

                if ( static_cast<BYTE *>(pv) >= static_cast<BYTE *>(modinfo.lpBaseOfDll) && static_cast<BYTE *>(pv) < static_cast<BYTE *>(modinfo.lpBaseOfDll) + modinfo.SizeOfImage )
                {
                    ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
                    rtl_uString *ustrSysPath = nullptr;

                    GetModuleFileNameW( lpModules[iModule], ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer), aBuffer.getBufSizeInSymbols() );

                    rtl_uString_newFromStr( &ustrSysPath, aBuffer );
                    osl_getFileURLFromSystemPath( ustrSysPath, pustrURL );
                    rtl_uString_release( ustrSysPath );

                    bSuccess = true;
                }
            }
        }

    }

    return bSuccess;
}

/*****************************************************************************/
/* Dispatcher for osl_osl_addressGetModuleURL */
/*****************************************************************************/

sal_Bool SAL_CALL osl_getModuleURLFromAddress( void *pv, rtl_uString **pustrURL )
{
    /* Use ..._NT first because ..._NT4 is much slower */
    return osl_addressGetModuleURL_NT_( pv, pustrURL ) || osl_addressGetModuleURL_NT4_( pv, pustrURL );
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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4054)
#endif
    return osl_getModuleURLFromAddress(reinterpret_cast<void*>(addr), ppLibraryUrl);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
