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

#include "native_share.h"

#include "rtl/bootstrap.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include <memory>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <delayimp.h>

#define INSTALL_PATH L"Software\\LibreOffice\\UNO\\InstallPath"
#define UNO_PATH L"UNO_PATH"

namespace {

/* Gets the installation path from the Windows Registry for the specified
   registry key.

   @param hroot       open handle to predefined root registry key
   @param subKeyName  name of the subkey to open

   @return the installation path or nullptr, if no installation was found or
           if an error occurred
*/
WCHAR* getPathFromRegistryKey( HKEY hroot, LPCWSTR subKeyName )
{
    // open the specified registry key
    HKEY hkey;
    if ( RegOpenKeyExW( hroot, subKeyName, 0, KEY_READ, &hkey ) != ERROR_SUCCESS )
        return nullptr;

    struct CloseKeyGuard {
        HKEY m_hkey;
        ~CloseKeyGuard() { RegCloseKey( m_hkey ); }
    } aCloseKeyGuard{hkey};

    // find the type and size of the default value
    DWORD type;
    DWORD size;
    if ( RegQueryValueExW( hkey, nullptr, nullptr, &type, nullptr, &size ) != ERROR_SUCCESS )
        return nullptr;

    // get memory to hold the default value
    std::unique_ptr<WCHAR[]> data(new WCHAR[size]);

    // read the default value
    if ( RegQueryValueExW( hkey, nullptr, nullptr, &type, reinterpret_cast<LPBYTE>(data.get()), &size ) != ERROR_SUCCESS )
        return nullptr;

    return data.release();
}

/* Returns the path to the program folder of the brand layer,
   for example C:/Program Files/LibreOffice/program
   This path is either obtained from the environment variable UNO_PATH
   or the registry item "Software\\LibreOffice\\UNO\\InstallPath"
   either in HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
   The return value must be freed with delete[]
*/
WCHAR* getInstallPath()
{
    std::unique_ptr<WCHAR[]> szInstallPath;

    DWORD cChars = GetEnvironmentVariableW(UNO_PATH, nullptr, 0);
    if (cChars > 0)
    {
        szInstallPath.reset(new WCHAR[cChars]);
        cChars = GetEnvironmentVariableW(UNO_PATH, szInstallPath.get(), cChars);
        // If PATH is not set then it is no error
        if (cChars == 0)
            return nullptr;
    }

    if (! szInstallPath)
    {
        szInstallPath.reset(getPathFromRegistryKey( HKEY_CURRENT_USER, INSTALL_PATH ));
        if (! szInstallPath)
        {
            // read the key's default value from HKEY_LOCAL_MACHINE
            szInstallPath.reset(getPathFromRegistryKey( HKEY_LOCAL_MACHINE, INSTALL_PATH ));
        }
    }
    return szInstallPath.release();
}

/* We extend the path to contain the install folder,
   so that components can use osl_loadModule with arguments, such as
   "reg3.dll". That is, the arguments are only the library names.
*/
void extendPath(LPCWSTR szPath)
{
    if (!szPath)
        return;

    std::unique_ptr<WCHAR[]> sEnvPath;
    DWORD cChars = GetEnvironmentVariableW(L"PATH", nullptr, 0);
    if (cChars > 0)
    {
        sEnvPath.reset(new WCHAR[cChars]);
        cChars = GetEnvironmentVariableW(L"PATH", sEnvPath.get(), cChars);
        // If PATH is not set then it is no error
        if (cChars == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)
            return;
    }
    // Prepare the new PATH. Add the directory at the front.
    // Note also adding ';'
    std::unique_ptr<WCHAR[]> sNewPath(new WCHAR[lstrlenW(sEnvPath.get()) + lstrlenW(szPath) + 2]);
    lstrcpyW(sNewPath.get(), szPath);
    if (lstrlenW(sEnvPath.get()))
    {
        lstrcatW(sNewPath.get(), L";");
        lstrcatW(sNewPath.get(), sEnvPath.get());
    }
    SetEnvironmentVariableW(L"PATH", sNewPath.get());
}

HMODULE loadFromPath(LPCSTR sLibName)
{
    if (!sLibName)
        return nullptr;

    // Convert the ansi file name to wchar_t*
    int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sLibName, -1, nullptr, 0);
    if (size == 0)
        return nullptr;
    std::unique_ptr<WCHAR[]> wsLibName(new WCHAR[size]);
    if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sLibName, -1, wsLibName.get(), size))
        return nullptr;

    std::unique_ptr<WCHAR[]> szPath(getInstallPath());
    if (!szPath)
        return nullptr;

    extendPath(szPath.get());

    std::unique_ptr<WCHAR[]> szFullPath(new WCHAR[lstrlenW(wsLibName.get()) + lstrlenW(szPath.get()) + 2]);
    lstrcpyW(szFullPath.get(), szPath.get());
    lstrcatW(szFullPath.get(), L"\\");
    lstrcatW(szFullPath.get(), wsLibName.get());
    HMODULE handle = LoadLibraryW(szFullPath.get());
    return handle;
}

/* Hook for delayed loading of libraries which this library is linked with.
   This is a failure hook. That is, it is only called when the loading of
   a library failed. It will be called when loading of cppuhelper failed.
   Because we extend the PATH to the install folder while this function is
   executed (see extendPath), all other libraries are found.
*/
extern "C" FARPROC WINAPI delayLoadHook(
    unsigned int   dliNotify,
    PDelayLoadInfo pdli
    )
{
    if (dliNotify == dliFailLoadLib)
    {
        HANDLE h = loadFromPath(pdli->szDll);
        return reinterpret_cast<FARPROC>(h);
    }
    return nullptr;
}
}

ExternC
const PfnDliHook __pfnDliFailureHook2 = delayLoadHook;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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
