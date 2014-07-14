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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"


//-----------------------------------------------------------
//  interface includes
//-----------------------------------------------------------
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <osl/file.hxx>

//--------------------------------------------------------------
//  other includes
//--------------------------------------------------------------
#include <cppuhelper/servicefactory.hxx>

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif
#include <sal/types.h>
#include <osl/diagnose.h>

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#include <cppuhelper/implbase1.hxx>

#include <stdio.h>

#ifndef _FPSERVICEINFO_HXX_
#include "..\FOPServiceInfo.hxx"
#endif

#include <osl/file.hxx>

#define _WIN32_DCOM

#include <windows.h>

//--------------------------------------------------------------
//  namesapces
//--------------------------------------------------------------

using namespace ::rtl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::ui::dialogs;
using namespace std                     ;

//--------------------------------------------------------------
//  defines
//--------------------------------------------------------------

#define RDB_SYSPATH "D:\\Projects\\gsl\\sysui\\wntmsci7\\bin\\applicat.rdb"

//--------------------------------------------------------------
//  global variables
//--------------------------------------------------------------

Reference< XMultiServiceFactory >   g_xFactory;

/*
void CreateDeepDirectory( )
{
    // create a deep directory

    OUString aPathURL( L"file:///d|/Deep" );
    OUString normalizedPath;

    OSL_ASSERT( ::osl::FileBase::E_None == \
        ::osl::FileBase::getNormalizedPathFromFileURL( aPathURL, normalizedPath ) );

    while( ::osl::FileBase::E_None == osl::Directory::create( normalizedPath ) )
    {
        aPathURL += L"/Deep";
        OSL_ASSERT( ::osl::FileBase::E_None == \
            ::osl::FileBase::getNormalizedPathFromFileURL( aPathURL, normalizedPath ) );
    }

}
*/

//--------------------------------------------------------------
//  main
//--------------------------------------------------------------


int SAL_CALL main(int /*nArgc*/, char* /*Argv[]*/, char* /*Env[]*/  )
{
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    printf("Starting test of FolderPicker Service\n");

    //CreateDeepDirectory( );

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------

    // Get global factory for uno services.
    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occurred.
    if ( g_xFactory.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    printf("Creating RegistryServiceFactory successful\n");

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    Reference< XFolderPicker > xFolderPicker;

    xFolderPicker = Reference< XFolderPicker >(
        g_xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM ( FOLDER_PICKER_SERVICE_NAME ) ) ), UNO_QUERY );

    if ( xFolderPicker.is() == sal_False )
    {
        OSL_ENSURE( sal_False, "Error creating FolderPicker Service" );
        return(-1);
    }

    try
    {
        xFolderPicker->setDisplayDirectory( L"file:///C|" );
        xFolderPicker->setTitle( L"FolderBrowse Dialog" );
        xFolderPicker->execute( );

        OUString rootDir = xFolderPicker->getDisplayDirectory( );
        OUString selectedDir = xFolderPicker->getDirectory( );

        xFolderPicker->setDisplayDirectory( selectedDir );
        xFolderPicker->execute( );

        rootDir = xFolderPicker->getDisplayDirectory( );
        selectedDir = xFolderPicker->getDirectory( );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        MessageBox( NULL, "Exception caught!", "Error", MB_OK );
    }

    //--------------------------------------------------
    // shutdown
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occurred.
    if ( xComponent.is() == sal_False )
    {
        OSL_ENSURE(sal_False, "Error shuting down");
    }

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    printf("Test successful\n");

    CoUninitialize( );

    return 0;
}
