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
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <osl/file.hxx>

//--------------------------------------------------------------
//  namesapces
//--------------------------------------------------------------

using namespace ::rtl                   ;
using namespace ::cppu                  ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::lang  ;
using namespace std                     ;
using namespace com::sun::star::system;

#define RDB_SYSPATH "D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\applicat.rdb"

//--------------------------------------------------------------
//  global variables
//--------------------------------------------------------------

Reference< XMultiServiceFactory >   g_xFactory;

//--------------------------------------------------------------
//  main
//--------------------------------------------------------------


// int SAL_CALL main(int nArgc, char* Argv[], char* pEnv[]  )
// make Warning free, leave out typename
int SAL_CALL main(int , char*, char* )
{
    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------

    // Get global factory for uno services.
    OUString rdbName = OUString( RDB_SYSPATH  );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occurred.
    if ( g_xFactory.is() == sal_False )
    {
        OSL_FAIL("Can't create RegistryServiceFactory");
        return(-1);
    }

    printf("Creating RegistryServiceFactory successful\n");

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    try
    {
        Reference< XSimpleMailClientSupplier > xSmplMailClientSuppl(
            g_xFactory->createInstance( OUString("com.sun.star.system.SimpleSystemMail") ), UNO_QUERY );

        if ( !xSmplMailClientSuppl.is() )
        {
            OSL_FAIL( "Error creating SimpleSystemMail Service" );
            return(-1);
        }

        Reference< XSimpleMailClient > xSmplMailClient(
            xSmplMailClientSuppl->querySimpleMailClient( ) );

        if ( xSmplMailClient.is( ) )
        {
            Reference< XSimpleMailMessage > xSmplMailMsg(
                xSmplMailClient->createSimpleMailMessage( ) );

            if ( xSmplMailMsg.is( ) )
            {
                xSmplMailMsg->setRecipient( OUString("tino.rachui@germany.sun.com") );
                xSmplMailMsg->setOriginator( OUString("tino.rachui@germany.sun.com") );

                Sequence< OUString > ccRecips( 1 );
                ccRecips[0] = OUString("tino.rachui@germany.sun.com");

                xSmplMailMsg->setCcRecipient( ccRecips );

                Sequence< OUString > bccRecips( 1 );
                bccRecips[0] = OUString("tino.rachui@germany.sun.com");

                xSmplMailMsg->setBccRecipient( bccRecips );

                xSmplMailMsg->setSubject( OUString("Mapi Test") );

                Sequence< OUString > attachements( 2 );

                OUString aFile("D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\testprx.exe");
                OUString aFileURL;

                osl::FileBase::getFileURLFromSystemPath( aFile, aFileURL );
                attachements[0] = aFileURL;

                aFile = OUString("D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\testsyssh.exe");
                osl::FileBase::getFileURLFromSystemPath( aFile, aFileURL );

                attachements[1] = aFile;

                xSmplMailMsg->setAttachement( attachements );

                xSmplMailClient->sendSimpleMailMessage( xSmplMailMsg, 0 );
            }
        }
    }
    catch( Exception& )
    {
    }

    //--------------------------------------------------
    // shutdown
    //--------------------------------------------------

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
    g_xFactory = Reference< XMultiServiceFactory >();

    printf("Test successful\n");

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
