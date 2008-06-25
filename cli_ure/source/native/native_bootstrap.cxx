/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: native_bootstrap.cxx,v $
 * $Revision: 1.14 $
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

// We are using the Windows UNICODE API
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

#define OFFICE_LOCATION_REGISTRY_KEY L"Software\\OpenOffice.org\\Layer\\URE\\1"
#define UREINSTALLLOCATION L"UREINSTALLLOCATION"
#define URE_BIN L"\\bin"

namespace
{


//Returns the path to the URE/bin folder.
//The caller must free the returned string with delete[]
wchar_t * getUnoPath()
{
    wchar_t *  theUnoPath = NULL;
    bool failed = false;
    HKEY    hKey = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,OFFICE_LOCATION_REGISTRY_KEY,
        0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, OFFICE_LOCATION_REGISTRY_KEY,
            0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
#if OSL_DEBUG_LEVEL >= 2
            fprintf(stderr, "cli_cppuhelper: Office not properly installed. "
                "Could not open registry keys.");
#endif
            failed = true;
        }
    }
    if (! failed)
    {
        DWORD   dwType = 0;
        DWORD   dwLen = 0;
        wchar_t *arData = NULL;
        //get the length for the path to office
        if (RegQueryValueEx(hKey, UREINSTALLLOCATION, NULL, &dwType, NULL,
            &dwLen) == ERROR_SUCCESS)
        {
            arData = new  wchar_t[dwLen];
            arData[0] = '\0';
            if (RegQueryValueEx(hKey, UREINSTALLLOCATION, NULL, &dwType, (LPBYTE) arData,
                & dwLen) == ERROR_SUCCESS)
            {
                int test = lstrlen(URE_BIN);
                //attach the bin directory to the URE path
                int sizePath = lstrlen(arData) + lstrlen(URE_BIN) + 1;
                theUnoPath = new wchar_t[sizePath];
                 theUnoPath[0] = '\0';
                 lstrcat(theUnoPath, arData);
                 lstrcat(theUnoPath, URE_BIN);
                delete[] arData;
#if OSL_DEBUG_LEVEL >=2
                fprintf(stdout,"[cli_cppuhelper]: Using path %S to load office libraries.", theUnoPath);
#endif
            }
        }
        RegCloseKey(hKey);
    }
    return theUnoPath;
}


//Returns the path to the Ure/bin directory and expands the PATH by inserting the
// ure/bin path at the front.
wchar_t const * getUreBinPathAndSetPath()
{
    static wchar_t * theBinPath = NULL;

    if (theBinPath)
        return theBinPath;

    wchar_t * unoPath = getUnoPath();
    if (!unoPath)
        return NULL;

    //We extend the path to contain the program directory of the office,
    //so that components can use osl_loadModule with arguments, such as
    //"reg3.dll". That is, the arguments are only the library names.

    wchar_t * sEnvPath = NULL;
    DWORD  cChars = GetEnvironmentVariable(L"PATH", sEnvPath, 0);
    if (cChars > 0)
    {
        sEnvPath = new wchar_t[cChars];
        cChars = GetEnvironmentVariable(L"PATH", sEnvPath, cChars);
        //If PATH is not set then it is no error
        if (cChars == 0 && GetLastError() != ERROR_ENVVAR_NOT_FOUND)
        {
            delete[] sEnvPath;
            return NULL;
        }
    }
    //prepare the new PATH. Add the Ure/bin directory at the front
    wchar_t * sNewPath = new wchar_t[lstrlen(sEnvPath) + lstrlen(unoPath) + 2];
    sNewPath[0] = '\0';
    lstrcat(sNewPath, unoPath);
    if (lstrlen(sEnvPath))
    {
        lstrcat(sNewPath, L";");
        lstrcat(sNewPath, sEnvPath);
    }

    BOOL bSet = SetEnvironmentVariable(L"PATH", sNewPath);

    theBinPath = unoPath;
    delete[] sEnvPath;
    delete[] sNewPath;

    return theBinPath;
}

HMODULE loadFromPath(wchar_t const * sLibName)
{
    if (sLibName == NULL)
        return NULL;

    wchar_t const * binPath =  getUreBinPathAndSetPath();
    if (!binPath)
        return NULL;


    wchar_t*  sFullPath = new wchar_t[lstrlen(sLibName) + lstrlen(binPath) + 2];
    sFullPath[0] = '\0';
    sFullPath = lstrcat(sFullPath, binPath);
    sFullPath = lstrcat(sFullPath, L"\\");
    sFullPath = lstrcat(sFullPath, sLibName);
    HMODULE handle = LoadLibraryEx(sFullPath, NULL,
        LOAD_WITH_ALTERED_SEARCH_PATH);
    delete[] sFullPath;
    return handle;

}

//Hook for delayed loading of libraries which this library is linked with.
extern "C"  FARPROC WINAPI delayLoadHook(
    unsigned        dliNotify,
    PDelayLoadInfo  pdli
    )
{
    if (dliNotify == dliFailLoadLib)
    {
        //Convert the ansi file name to wchar_t*
        int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pdli->szDll, -1, NULL, 0);
        if (size > 0)
        {
            wchar_t * buf = new wchar_t[size];
            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pdli->szDll, -1, buf, size))
            {
                HMODULE handle = NULL;
                return (FARPROC) loadFromPath(buf);
            }
        }
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
    registry keys HKEY_CURRENT_USER\Software\OpenOffice.org\Layer\URE\1
    and HKEY_LOCAL_MACHINE\Software\OpenOffice.org\Layer\URE\1 are examined.
    These contain a named value UREINSTALLLOCATION which holds a path to the URE
    installation folder.
*/
public __sealed __gc class Bootstrap
{
    inline Bootstrap() {}

public:

    /** Bootstraps the initial component context from a native UNO installation.

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext *
        defaultBootstrap_InitialComponentContext();

    /** Bootstraps the initial component context from a native UNO installation.

        @param ini_file
               a file URL of an ini file, e.g. uno.ini/unorc. (The ini file must
               reside next to the cppuhelper library)
        @param bootstrap_parameters
               bootstrap parameters (maybe null)

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext *
        defaultBootstrap_InitialComponentContext(
            ::System::String * ini_file,
            ::System::Collections::IDictionaryEnumerator *
              bootstrap_parameters );

    /** Bootstraps the initial component context from a native UNO installation.

    @see cppuhelper/bootstrap.hxx:bootstrap()
     */
    static ::unoidl::com::sun::star::uno::XComponentContext *
    bootstrap();
};

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext *
Bootstrap::defaultBootstrap_InitialComponentContext(
    ::System::String * ini_file,
    ::System::Collections::IDictionaryEnumerator * bootstrap_parameters )
{
    if (0 != bootstrap_parameters)
    {
        bootstrap_parameters->Reset();
        while (bootstrap_parameters->MoveNext())
        {
            OUString key(
                String_to_ustring( __try_cast< ::System::String * >(
                                       bootstrap_parameters->get_Key() ) ) );
            OUString value(
                String_to_ustring( __try_cast< ::System::String * >(
                                       bootstrap_parameters->get_Value() ) ) );

            ::rtl::Bootstrap::set( key, value );
        }
    }

    // bootstrap native uno
    Reference< XComponentContext > xContext;
    if (0 == ini_file)
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext();
    }
    else
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext(
            String_to_ustring( __try_cast< ::System::String * >( ini_file ) ) );
    }

    return __try_cast< ::unoidl::com::sun::star::uno::XComponentContext * >(
        to_cli( xContext ) );
}

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext *
Bootstrap::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext( 0, 0 );
}

::unoidl::com::sun::star::uno::XComponentContext * Bootstrap::bootstrap()
{
    Reference<XComponentContext> xContext = ::cppu::bootstrap();
    return __try_cast< ::unoidl::com::sun::star::uno::XComponentContext * >(
        to_cli( xContext ) );

}

}
}
