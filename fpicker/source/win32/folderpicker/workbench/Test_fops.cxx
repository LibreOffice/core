/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Test_fops.cxx,v $
 * $Revision: 1.4 $
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
#include <rtl/ustring>
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


int SAL_CALL main(int nArgc, char* Argv[], char* Env[]  )
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

    // Print a message if an error occured.
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
    catch(...)
    {
        MessageBox( NULL, "Exception caught!", "Error", MB_OK );
    }

    //--------------------------------------------------
    // shutdown
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occured.
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
