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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <osl/file.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <stdio.h>

#include "..\FOPServiceInfo.hxx"

#define _WIN32_DCOM

#include <windows.h>


//  namesapces


using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star::ui::dialogs;
using namespace std                     ;

#define RDB_SYSPATH "D:\\Projects\\gsl\\sysui\\wntmsci7\\bin\\applicat.rdb"


//  global variables


Reference< XMultiServiceFactory >   g_xFactory;


//  main


int SAL_CALL main(int /*nArgc*/, char* /*Argv[]*/, char* /*Env[]*/  )
{
    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    printf("Starting test of FolderPicker Service\n");

    //CreateDeepDirectory( );


    // get the global service-manager


    // Get global factory for uno services.
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( RDB_SYSPATH ) );

    // Print a message if an error occurred.
    if ( g_xFactory.is() == sal_False )
    {
        OSL_FAIL("Can't create RegistryServiceFactory");
        return(-1);
    }

    printf("Creating RegistryServiceFactory successful\n");


    // try to get an Interface to a XFilePicker Service


    Reference< XFolderPicker2 > xFolderPicker( g_xFactory->createInstance( FOLDER_PICKER_SERVICE_NAME ), UNO_QUERY );

    if ( xFolderPicker.is() == sal_False )
    {
        OSL_FAIL( "Error creating FolderPicker Service" );
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
    catch( css::uno::Exception& )
    {
        MessageBox( NULL, "Exception caught!", "Error", MB_OK );
    }


    // shutdown


    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occurred.
    if ( xComponent.is() == sal_False )
    {
        OSL_FAIL("Error shuting down");
    }

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();

    printf("Test successful\n");

    CoUninitialize( );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
