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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.lang.*;

public class StorageFunctionality  {

    public static void main( String args[] )
    {
        // connect to the office
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

        XMultiServiceFactory xMSF = null;

        // create connection(s) and get multiservicefactory
        try {
            xMSF = connect( sConnectionString );

            if ( xMSF == null )
            {
                System.out.println( "Error: Couldn't get MSF!" );
                return;
            }
        } catch( Exception e ) {
            System.out.println( "Error: Couldn't get MSF, exception: " + e );
            return;
        }

        XSingleServiceFactory xStorageFactory = null;
        try
        {
            Object oStorageFactory = xMSF.createInstance( "com.sun.star.embed.StorageFactory" );
            xStorageFactory = (XSingleServiceFactory)UnoRuntime.queryInterface( XSingleServiceFactory.class,
                                                                                oStorageFactory );

            if ( xStorageFactory == null )
            {
                System.out.println( "Error: Can not get storage factory!" );
                return;
            }
        }
        catch ( Exception e )
        {
            System.out.println( "Error: Can't get storage factory, exception: " + e + "!" );
            return;
        }

        boolean bTestsPassed = true;

        final int nNumTests = 9;
        StorageTest pTests[] = new StorageTest[nNumTests];
        pTests[0] = (StorageTest) new Test01( xMSF, xStorageFactory );
        pTests[1] = (StorageTest) new Test02( xMSF, xStorageFactory );
        pTests[2] = (StorageTest) new Test03( xMSF, xStorageFactory );
        pTests[3] = (StorageTest) new Test04( xMSF, xStorageFactory );
        pTests[4] = (StorageTest) new Test05( xMSF, xStorageFactory );
        pTests[5] = (StorageTest) new Test06( xMSF, xStorageFactory );
        pTests[6] = (StorageTest) new Test07( xMSF, xStorageFactory );
        pTests[7] = (StorageTest) new Test08( xMSF, xStorageFactory );
        pTests[8] = (StorageTest) new Test09( xMSF, xStorageFactory );

           System.out.println( "\nstart testing\n" );

        for ( int nInd = 0; nInd < nNumTests; nInd++ )
        {
            String sTestName = "Test" + ( ( nInd < 9 ) ? "0" : "" ) + ( nInd + 1 );

               System.out.println( "======= Storage test " + sTestName + " started!" );
            if ( pTests[nInd].test() )
                   System.out.println( "======= Storage test " + sTestName + " passed!" );
            else
            {
                   System.out.println( "======= Storage test " + sTestName + " failed!" );
                bTestsPassed = false;
            }
        }

        if ( bTestsPassed )
            System.out.println( "\ntesting passed" );
        else
            System.out.println( "\ntesting failed" );

        System.out.println( "done" );

        System.exit( 0 );
    }


    public static XMultiServiceFactory connect( String sConnectStr )
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException,
    Exception
    {
        // Get component context
        XComponentContext xComponentContext =
        com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
        null );

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager =
        xComponentContext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  oUrlResolver  = xLocalServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", xComponentContext );
        XUnoUrlResolver xUrlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(
            XUnoUrlResolver.class, oUrlResolver );

        Object oInitialObject = xUrlResolver.resolve( sConnectStr );
        XNamingService xName = (XNamingService)UnoRuntime.queryInterface(
            XNamingService.class, oInitialObject );

        XMultiServiceFactory xMSF = null;
        if( xName != null ) {
            System.err.println( "got the remote naming service !" );
            Object oMSF = xName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, oMSF );
        }
        else
            System.out.println( "Error: Can't get XNamingService interface from url resolver!" );

        return xMSF;
    }

}

