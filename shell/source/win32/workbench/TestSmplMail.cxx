/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestSmplMail.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_shell.hxx"


//-----------------------------------------------------------
//  interface includes
//-----------------------------------------------------------
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

//--------------------------------------------------------------
//  defines
//--------------------------------------------------------------

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

    try
    {
        Reference< XSimpleMailClientSupplier > xSmplMailClientSuppl(
            g_xFactory->createInstance( OUString::createFromAscii( "com.sun.star.system.SimpleSystemMail" ) ), UNO_QUERY );

        if ( !xSmplMailClientSuppl.is() )
        {
            OSL_ENSURE( sal_False, "Error creating SimpleSystemMail Service" );
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
                xSmplMailMsg->setRecipient( OUString::createFromAscii("tino.rachui@germany.sun.com") );
                xSmplMailMsg->setOriginator( OUString::createFromAscii( "tino.rachui@germany.sun.com" ) );

                Sequence< OUString > ccRecips( 1 );
                ccRecips[0] = OUString::createFromAscii( "tino.rachui@germany.sun.com" );

                xSmplMailMsg->setCcRecipient( ccRecips );

                Sequence< OUString > bccRecips( 1 );
                bccRecips[0] = OUString::createFromAscii( "tino.rachui@germany.sun.com" );

                xSmplMailMsg->setBccRecipient( bccRecips );

                xSmplMailMsg->setSubject( OUString::createFromAscii( "Mapi Test" ) );

                Sequence< OUString > attachements( 2 );

                OUString aFile = OUString::createFromAscii( "D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\testprx.exe" );
                OUString aFileURL;

                osl::FileBase::getFileURLFromSystemPath( aFile, aFileURL );
                attachements[0] = aFileURL;

                aFile = OUString::createFromAscii( "D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\testsyssh.exe" );
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

    return 0;
}
