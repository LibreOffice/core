/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestSmplMail.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:11:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _COM_SUN_STAR_SYSTEM_XSIMPLEMAILCLIENTSUPPLIER_HPP_
#include <com/sun/star/system/XSimpleMailClientSupplier.hpp>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

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

#include <stdio.h>
#include <windows.h>

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


int SAL_CALL main(int nArgc, char* Argv[], char* Env[]  )
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
