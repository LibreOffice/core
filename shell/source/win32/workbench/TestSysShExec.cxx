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

#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <cppuhelper/servicefactory.hxx>

#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <iostream>

#include <windows.h>


//  namespaces


using namespace ::cppu;
using namespace ::css::uno;
using namespace ::css::lang;
using namespace ::css::system;

constexpr OUStringLiteral RDB_SYSPATH = u"D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\applicat.rdb";


//  global variables


Reference< XMultiServiceFactory >   g_xFactory;


//  main


// int SAL_CALL main(int nArgc, char* Argv[], char* Env[]   )
int SAL_CALL main(int nArgc, char* Argv[], char*    )
{

    // get the global service-manager


    if ( nArgc < 4 )
        return 0;

    // Get global factory for uno services.
    OUString rdbName ( RDB_SYSPATH  );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occurred.
    if ( !g_xFactory  )
    {
        OSL_FAIL("Can't create RegistryServiceFactory");
        return -1;
    }

    std::cout << "Creating RegistryServiceFactory successful" << std::endl;


    // try to get an Interface to a XFilePicker Service


    Reference< XSystemShellExecute > xSysShExec(
        g_xFactory->createInstance("com.sun.star.system.SystemShellExecute"), UNO_QUERY );

    if ( !xSysShExec )
    {
        OSL_FAIL( "Error creating SystemShellExecute Service" );
        return -1;
    }

    //"c:\\winnt\\notepad.exe"
    OUString cmd( Argv[1] );
    OUString param( Argv[2] ); //c:\\winnt\\iis5.log

    try
    {
        xSysShExec->execute( cmd, param, atoi( Argv[3] ) );
    }
    catch( SystemShellExecuteException&  )
    {
        OSL_FAIL( "Error executing system command" );
    }
    catch( IllegalArgumentException& )
    {
        OSL_FAIL( "Invalid parameter" );
    }


    // shutdown


    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    // Print a message if an error occurred.
    if ( !xComponent )
    {
        OSL_FAIL("Error shutting down");
    }

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();

    std::cout << "Test successful" << std::endl;

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
