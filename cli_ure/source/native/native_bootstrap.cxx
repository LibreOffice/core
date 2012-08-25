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

// Use UNICODE Windows and C API.
#define _UNICODE
#define UNICODE

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include "uno/environment.hxx"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>

#include "native_share.h"

#include "rtl/bootstrap.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include <delayimp.h>
#include <stdio.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace cli_ure {
    WCHAR * resolveLink(WCHAR * path);
}

#define INSTALL_PATH L"Software\\LibreOffice\\UNO\\InstallPath"
#define URE_LINK L"\\ure-link"
#define URE_BIN L"\\bin"
#define UNO_PATH L"UNO_PATH"

namespace
{

 /*
 * Gets the installation path from the Windows Registry for the specified
 * registry key.
 *
 * @param hroot       open handle to predefined root registry key
 * @param subKeyName  name of the subkey to open
 *
 * @return the installation path or NULL, if no installation was found or
 *         if an error occurred
 */
WCHAR* getPathFromRegistryKey( HKEY hroot, LPCWSTR subKeyName )
{
    HKEY hkey;
    DWORD type;
    TCHAR* data = NULL;
    DWORD size;

    /* open the specified registry key */
    if ( RegOpenKeyEx( hroot, subKeyName, 0, KEY_READ, &hkey ) != ERROR_SUCCESS )
    {
        return NULL;
    }

    /* find the type and size of the default value */
    if ( RegQueryValueEx( hkey, NULL, NULL, &type, NULL, &size) != ERROR_SUCCESS )
    {
        RegCloseKey( hkey );
        return NULL;
    }

    /* get memory to hold the default value */
    data = new WCHAR[size];

    /* read the default value */
    if ( RegQueryValueEx( hkey, NULL, NULL, &type, (LPBYTE) data, &size ) != ERROR_SUCCESS )
    {
        RegCloseKey( hkey );
        return NULL;
    }

    /* release registry key handle */
    RegCloseKey( hkey );

    return data;
}

/* If the path does not end with '\' the las segment will be removed.
    path: C:\a\b
    ->    C:\a
    @param io_path
        in/out parameter. The string is not reallocated. Simply a '\0'
        will be inserted to shorten the string.
*/
void oneDirUp(LPTSTR io_path)
{
    WCHAR * pEnd = io_path + lstrlen(io_path) - 1;
    while (pEnd > io_path //prevent crashing if provided string does not contain a backslash
        && *pEnd != L'\\')
        pEnd --;
    *pEnd = L'\0';
}


/* Returns the path to the program folder of the brand layer,
    for example c:/LibreOffice 3/program
   This path is either obtained from the environment variable UNO_PATH
   or the registry item
   "Software\\LibreOffice\\UNO\\InstallPath"
   either in HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
   The return value must be freed with delete[]
*/
WCHAR * getInstallPath()
{
    WCHAR * szInstallPath = NULL;

    DWORD  cChars = GetEnvironmentVariable(UNO_PATH, NULL, 0);
    if (cChars > 0)
    {
        szInstallPath = new WCHAR[cChars];
        cChars = GetEnvironmentVariable(UNO_PATH, szInstallPath, cChars);
        //If PATH is not set then it is no error
        if (cChars == 0)
        {
            delete[] szInstallPath;
            return NULL;
        }
    }

    if (! szInstallPath)
    {
        szInstallPath = getPathFromRegistryKey( HKEY_CURRENT_USER, INSTALL_PATH );
        if ( szInstallPath == NULL )
        {
            /* read the key's default value from HKEY_LOCAL_MACHINE */
            szInstallPath = getPathFromRegistryKey( HKEY_LOCAL_MACHINE, INSTALL_PATH );
        }
    }
    return szInstallPath;
}

/* Returns the path to the URE/bin path, where cppuhelper lib resides.
    The returned string must be freed with delete[]
*/
WCHAR* getUnoPath()
{
    WCHAR * szLinkPath = NULL;
    WCHAR * szUrePath = NULL;
    WCHAR * szUreBin = NULL; //the return value

    WCHAR * szInstallPath = getInstallPath();
    if (szInstallPath)
    {
        oneDirUp(szInstallPath);

        //build the path to the ure-link file
        szUrePath = new WCHAR[MAX_PATH];
        szUrePath[0] = L'\0';
        lstrcat(szUrePath, szInstallPath);
        lstrcat(szUrePath, URE_LINK);

        //get the path to the actual Ure folder
        if (cli_ure::resolveLink(szUrePath))
        {
            //build the path to the URE/bin directory
            szUreBin = new WCHAR[lstrlen(szUrePath) + lstrlen(URE_BIN) + 1];
            szUreBin[0] = L'\0';
            lstrcat(szUreBin, szUrePath);
            lstrcat(szUreBin, URE_BIN);
        }
    }
#if OSL_DEBUG_LEVEL >=2
    if (szUreBin)
    {
        fwprintf(stdout,L"[cli_cppuhelper]: Path to URE libraries:\n %s \n", szUreBin);
    }
    else
    {
        fwprintf(stdout,L"[cli_cppuhelper]: Failed to determine location of URE.\n");
    }
#endif
    delete[] szInstallPath;
    delete[] szLinkPath;
    delete[] szUrePath;
    return szUreBin;
}


/*We extend the path to contain the Ure/bin folder,
  so that components can use osl_loadModule with arguments, such as
  "reg3.dll". That is, the arguments are only the library names.
*/
void extendPath(LPCWSTR szUreBinPath)
{
    if (!szUreBinPath)
        return;

    WCHAR * sEnvPath = NULL;
    DWORD  cChars = GetEnvironmentVariable(L"PATH", sEnvPath, 0);
    if (cChars > 0)
    {
        sEnvPath = new WCHAR[cChars];
        cChars = GetEnvironmentVariable(L"PATH", sEnvPath, cChars);
        //If PATH is not set then it is no error
        if (cChars == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)
        {
            delete[] sEnvPath;
            return;
        }
    }
    //prepare the new PATH. Add the Ure/bin directory at the front.
    //note also adding ';'
    WCHAR * sNewPath = new WCHAR[lstrlen(sEnvPath) + lstrlen(szUreBinPath) + 2];
    sNewPath[0] = L'\0';
    lstrcat(sNewPath, szUreBinPath);
    if (lstrlen(sEnvPath))
    {
        lstrcat(sNewPath, L";");
        lstrcat(sNewPath, sEnvPath);
    }
    BOOL bSet = SetEnvironmentVariable(L"PATH", sNewPath);

    delete[] sEnvPath;
    delete[] sNewPath;
}


HMODULE loadFromPath(LPCWSTR sLibName)
{
    if (sLibName == NULL)
        return NULL;

    WCHAR * szUreBinPath =  getUnoPath();
    if (!szUreBinPath)
        return NULL;

    extendPath(szUreBinPath);

    WCHAR*  szFullPath = new WCHAR[lstrlen(sLibName) + lstrlen(szUreBinPath) + 2];
    szFullPath[0] = L'\0';
    lstrcat(szFullPath, szUreBinPath);
    lstrcat(szFullPath, L"\\");
    lstrcat(szFullPath, sLibName);
    HMODULE handle = LoadLibraryEx(szFullPath, NULL,
        LOAD_WITH_ALTERED_SEARCH_PATH);

    delete[] szFullPath;
    delete[] szUreBinPath;
    return handle;
}

/*Hook for delayed loading of libraries which this library is linked with.
    This is a failure hook. That is, it is only called when the loading of
    a library failed. It will be called when loading of cppuhelper failed.
    Because we extend the PATH to the URE/bin folder while this function is
    executed (see extendPath), all other libraries are found.
*/
extern "C" FARPROC WINAPI delayLoadHook(
    unsigned        dliNotify,
    PDelayLoadInfo  pdli
    )
{
    if (dliNotify == dliFailLoadLib)
    {
        LPWSTR szLibName = NULL;
         //Convert the ansi file name to wchar_t*
        int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pdli->szDll, -1, NULL, 0);
        if (size > 0)
        {
            szLibName = new WCHAR[size];
            if (! MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pdli->szDll, -1, szLibName, size))
            {
                return 0;
            }
        }
        HANDLE h = loadFromPath(szLibName);
        delete[] szLibName;
        return (FARPROC) h;
    }
    return 0;
}
}

ExternC
PfnDliHook   __pfnDliFailureHook2 = delayLoadHook;

namespace uno
{
namespace util
{

/** Bootstrapping native UNO.

    Bootstrapping requires the existence of many libraries which are contained
    in an URE installation. To find and load these libraries the Windows
    registry keys HKEY_CURRENT_USER\Software\LibreOffice\Layer\URE\1
    and HKEY_LOCAL_MACHINE\Software\LibreOffice\Layer\URE\1 are examined.
    These contain a named value UREINSTALLLOCATION which holds a path to the URE
    installation folder.
*/
public ref class Bootstrap sealed
{
    inline Bootstrap() {}

public:

    /** Bootstraps the initial component context from a native UNO installation.

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
        defaultBootstrap_InitialComponentContext();

    /** Bootstraps the initial component context from a native UNO installation.

        @param ini_file
               a file URL of an ini file, e.g. uno.ini/unorc. (The ini file must
               reside next to the cppuhelper library)
        @param bootstrap_parameters
               bootstrap parameters (maybe null)

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
        defaultBootstrap_InitialComponentContext(
            ::System::String ^ ini_file,
            ::System::Collections::IDictionaryEnumerator ^
              bootstrap_parameters );

    /** Bootstraps the initial component context from a native UNO installation.

    @see cppuhelper/bootstrap.hxx:bootstrap()
     */
    static ::unoidl::com::sun::star::uno::XComponentContext ^
    bootstrap();
};

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext ^
Bootstrap::defaultBootstrap_InitialComponentContext(
    ::System::String ^ ini_file,
    ::System::Collections::IDictionaryEnumerator ^ bootstrap_parameters )
{
    if (nullptr != bootstrap_parameters)
    {
        bootstrap_parameters->Reset();
        while (bootstrap_parameters->MoveNext())
        {
            OUString key(
                String_to_ustring( safe_cast< ::System::String ^ >(
                                       bootstrap_parameters->Key ) ) );
            OUString value(
                String_to_ustring( safe_cast< ::System::String ^ >(
                                       bootstrap_parameters->Value ) ) );

            ::rtl::Bootstrap::set( key, value );
        }
    }

    // bootstrap native uno
    Reference< XComponentContext > xContext;
    if (nullptr == ini_file)
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext();
    }
    else
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext(
            String_to_ustring( safe_cast< ::System::String ^ >( ini_file ) ) );
    }

    return safe_cast< ::unoidl::com::sun::star::uno::XComponentContext ^ >(
        to_cli( xContext ) );
}

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext ^
Bootstrap::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext( nullptr, nullptr );
}

::unoidl::com::sun::star::uno::XComponentContext ^ Bootstrap::bootstrap()
{
    Reference<XComponentContext> xContext = ::cppu::bootstrap();
    return safe_cast< ::unoidl::com::sun::star::uno::XComponentContext ^ >(
        to_cli( xContext ) );

}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
