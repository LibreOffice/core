/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/system/XProxySettings.hpp>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#ifdef WNT

#include <stdio.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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


// int SAL_CALL main(int nArgc, char* Argv[], char* Env[]   )
int SAL_CALL main(int, char*, char* )
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
        Reference< XProxySettings > xProxySettings(
            g_xFactory->createInstance( OUString("com.sun.star.system.SystemProxySettings") ), UNO_QUERY );

        if ( !xProxySettings.is() )
        {
            OSL_FAIL( "Error creating SystemProxySettings Service" );
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
            g_xFactory->createInstance( OUString("com.sun.star.system.SOProxySettings") ), UNO_QUERY );

        if ( !xProxySettings.is() )
        {
            OSL_FAIL( "Error creating SystemProxySettings Service" );
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
        if (bProxyEnabled)
        {
            printf("Test ProxyEnabled true\n");
        }

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
