/*************************************************************************
 *
 *  $RCSfile: Test_fops.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:17:00 $
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


//-----------------------------------------------------------
//  interface includes
//-----------------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

//--------------------------------------------------------------
//  other includes
//--------------------------------------------------------------

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UI_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_FILEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
