/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestProxySet.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:11:24 $
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

#ifndef _COM_SUN_STAR_SYSTEM_XPROXYSETTINGS_HPP_
#include <com/sun/star/system/XProxySettings.hpp>
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

#ifdef WNT

#include <stdio.h>
#include <windows.h>

#endif

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

#ifdef WNT

#define RDB_SYSPATH "D:\\Projects\\gsl\\shell\\wntmsci7\\bin\\applicat.rdb"

#endif

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
        Reference< XProxySettings > xProxySettings(
            g_xFactory->createInstance( OUString::createFromAscii( "com.sun.star.system.SystemProxySettings" ) ), UNO_QUERY );

        if ( !xProxySettings.is() )
        {
            OSL_ENSURE( sal_False, "Error creating SystemProxySettings Service" );
            return(-1);
        }

        printf( "Creating instance of SystemProxySettings successful\n" );

        OUString ftpProxy    = xProxySettings->getFtpProxyAddress( );
        OUString httpProxy   = xProxySettings->getHttpProxyAddress( );
        OUString httpsProxy  = xProxySettings->getHttpsProxyAddress( );
        OUString socksProxy  = xProxySettings->getSocksProxyAddress( );
        OUString gopherProxy = xProxySettings->getGopherProxyAddress( );

        OUString ftpProxyPort    = xProxySettings->getFtpProxyPort( );
        OUString httpProxyPort   = xProxySettings->getHttpProxyPort( );
        OUString httpsProxyPort  = xProxySettings->getHttpsProxyPort( );
        OUString socksProxyPort  = xProxySettings->getSocksProxyPort( );
        OUString gopherProxyPort = xProxySettings->getGopherProxyPort( );

        OUString proxyBypass = xProxySettings->getProxyBypassAddress( );

        printf( "Test of SystemProxySettings successful\n" );

        xProxySettings = Reference< XProxySettings >(
            g_xFactory->createInstance( OUString::createFromAscii( "com.sun.star.system.SOProxySettings" ) ), UNO_QUERY );

        if ( !xProxySettings.is() )
        {
            OSL_ENSURE( sal_False, "Error creating SystemProxySettings Service" );
            return(-1);
        }

        printf( "Creating instance of SOProxySettings successful\n" );

        ftpProxy    = xProxySettings->getFtpProxyAddress( );
        httpProxy   = xProxySettings->getHttpProxyAddress( );
        httpsProxy  = xProxySettings->getHttpsProxyAddress( );
        socksProxy  = xProxySettings->getSocksProxyAddress( );
        gopherProxy = xProxySettings->getGopherProxyAddress( );

        ftpProxyPort    = xProxySettings->getFtpProxyPort( );
        httpProxyPort   = xProxySettings->getHttpProxyPort( );
        httpsProxyPort  = xProxySettings->getHttpsProxyPort( );
        socksProxyPort  = xProxySettings->getSocksProxyPort( );
        gopherProxyPort = xProxySettings->getGopherProxyPort( );

        proxyBypass = xProxySettings->getProxyBypassAddress( );

        sal_Bool bProxyEnabled = xProxySettings->isProxyEnabled( );

        printf( "Test of SOProxySettings successful\n" );
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
