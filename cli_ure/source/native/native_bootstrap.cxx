/*************************************************************************
 *
 *  $RCSfile: native_bootstrap.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 13:05:55 $
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

#include "native_share.h"

#include "rtl/bootstrap.hxx"
#include "cppuhelper/bootstrap.hxx"
#include <windows.h>
#include <delayimp.h>
#include <stdio.h>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define OFFICE_LOCATION_REGISTRY_KEY "Software\\OpenOffice.org\\UNO\\InstallPath"


#pragma unmanaged

namespace
{
char* getLibraryPath()
{
    static char* sPath;
    //do not use oslMutex here. That would cause to load sal and we would
    //run in a loop with delayLoadHook
    if (sPath == NULL)
    {
        bool failed = false;
        HKEY    hKey = 0;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,OFFICE_LOCATION_REGISTRY_KEY,
                          0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, OFFICE_LOCATION_REGISTRY_KEY,
                              0, KEY_READ, &hKey) != ERROR_SUCCESS)
            {
                OSL_ASSERT(0);
                fprintf(stderr, "cli_cppuhelper: Office not properly installed. "
                        "Could not open registry keys.");
                failed = true;
            }
        }
        if (failed)
            return NULL;

        DWORD   dwType = 0;
        DWORD   dwLen = 0;
        char *arData = NULL;
        //get the length for the path to office
        if (RegQueryValueExA(hKey, NULL, NULL, &dwType, NULL,
                                &dwLen) == ERROR_SUCCESS)
        {
            arData = new  char[dwLen];
            if (RegQueryValueExA(hKey, NULL, NULL, &dwType, (LPBYTE) arData,
                                 & dwLen) == ERROR_SUCCESS)
            {
                sPath = strdup(arData);
#if OSL_DEBUG_LEVEL >=2
                fprintf(stdout,"[cli_cppuhelper]: Using path %s to load office libraries.", sPath);
#endif
            }
            delete [] arData;
        }
        RegCloseKey(hKey);

        //We extend the path to contain the program directory of the office,
        //so that components can use osl_loadModule with arguments, such as
        //"reg3.dll". That is, the arguments are only the library names.
        char * sEnvPath = getenv("PATH");
        char * buff = NULL;
        if (sEnvPath)
            buff = new char[sizeof("PATH=") + strlen(sEnvPath) + strlen(sPath) + 2];
        else
            buff = new char[sizeof("PATH=") + strlen(sPath) + 2];
        buff[0] = 0;
        strcat(buff, "PATH=");
        strcat(buff, sPath);
        if (sEnvPath)
        {
            strcat(buff, ";");
            strcat(buff, sEnvPath);
        }
        putenv(buff);
    }

    return  sPath;
}


//Hook for delayed loading of libraries which this library is linked with.
extern "C"  FARPROC WINAPI delayLoadHook(
    unsigned        dliNotify,
    PDelayLoadInfo  pdli
    )
{
    if (dliNotify == dliFailLoadLib)
    {
        char* sPath = getLibraryPath();
        int lenPath = strlen(sPath);
        //create string to contain the full path to cppuhelper
        int lenLib = strlen(pdli->szDll);
        char*  sFullPath = new char[lenLib + lenPath + 2];
        sFullPath[0] = 0;
        sFullPath = strcat(sFullPath, sPath);
        sFullPath = strcat(sFullPath, "\\");
        sFullPath = strcat(sFullPath, pdli->szDll);

        HMODULE handle = LoadLibraryExA(sFullPath, NULL,
                                        LOAD_WITH_ALTERED_SEARCH_PATH);
        delete[] sFullPath;
        return (FARPROC) handle;
    }
    return 0;
}
}

ExternC
PfnDliHook   __pfnDliFailureHook2 = delayLoadHook;
#pragma managed

namespace uno
{
namespace util
{

/** Bootstrapping native UNO.

    Bootstrapping requires the existence of many libraries which are contained
    in an office installation. To find and load these libraries the Windows
    registry keys HKEY_CURRENT_USER\Software\OpenOffice.org\UNO\InstallPath
    and HKEY_LOCAL_MACHINE\Software\OpenOffice.org\UNO\InstallPath are examined.
    These contain the paths to the program folder of the office which was
    installed most recently. Please note that the office's setup either
    writes the key in HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE dependent on
    whether the user chooses a user installation or an installation for all
    users.
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
