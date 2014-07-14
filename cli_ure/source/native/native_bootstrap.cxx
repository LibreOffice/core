/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

// INSTALL_PATH value needs to correspond to the Windows registry subkey
// in main\scp2\source\ooo\registryitem_ooo.scp
#define INSTALL_PATH L"Software\\OpenOffice\\UNO\\InstallPath"
#define INSTALL_PATH_64 L"Software\\Wow6432Node\\OpenOffice\\UNO\\InstallPath"
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

/* Returns the path to the program folder of the brand layer,
    for example c:/openoffice.org 3/program
   This path is either obtained from the environment variable UNO_PATH
   or the registry item
   "Software\\OpenOffice\\UNO\\InstallPath"
   either in HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
   The return value must be freed with delete[]
*/
WCHAR * getInstallPath()
{
    WCHAR * szInstallPath = NULL;

    DWORD  cChars = GetEnvironmentVariable(UNO_PATH, NULL, 0);
    if (cChars > 0)
    {
        szInstallPath = new WCHAR[cChars+1];
        cChars = GetEnvironmentVariable(UNO_PATH, szInstallPath, cChars+1);
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
            /* read the key's default value from HKEY_LOCAL_USER */
            szInstallPath = getPathFromRegistryKey( HKEY_CURRENT_USER, INSTALL_PATH_64 );
        }
        if ( szInstallPath == NULL )
        {
            /* read the key's default value from HKEY_LOCAL_MACHINE */
            szInstallPath = getPathFromRegistryKey( HKEY_LOCAL_MACHINE, INSTALL_PATH );
        }
        if ( szInstallPath == NULL )
        {
            /* read the key's default value from HKEY_LOCAL_MACHINE */
            szInstallPath = getPathFromRegistryKey( HKEY_LOCAL_MACHINE, INSTALL_PATH_64 );
        }
    }
    return szInstallPath;
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

    WCHAR * szUreBinPath =  getInstallPath();
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
    registry keys HKEY_CURRENT_USER\Software\OpenOffice\UNO\InstallPath
    and HKEY_LOCAL_MACHINE\Software\OpenOffice\UNO\InstallPath are examined.
    The default value contain the path to the office prgoram dir. No seaparate URE
    anymore.
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
