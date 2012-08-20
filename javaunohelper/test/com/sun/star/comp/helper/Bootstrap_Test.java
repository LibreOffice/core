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

package com.sun.star.comp.helper;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;


public class Bootstrap_Test {

    static public boolean test( String ini_file, java.util.Hashtable<String,String> bootstrap_parameters )
        throws java.lang.Exception
    {
        boolean passed = false;
        System.err.println();
        System.out.println("*******************************************************************");
        System.err.println("Bootstrap - doing tests...");
        System.err.println();

        try {
            XComponentContext xContext =
                com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext(
                    ini_file, bootstrap_parameters );

            if (AnyConverter.isVoid(
                    xContext.getValueByName(
                        "/singletons/com.sun.star.reflection.theTypeDescriptionManager" ) ))
            {
                throw new Exception(
                    "no /singletons/com.sun.star.reflection.theTypeDescriptionManager!" );
            }

            XMultiServiceFactory msf = UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xContext.getServiceManager() );
            String services[] = msf.getAvailableServiceNames();
            System.out.println("Available services are:");
            System.err.println();
            if (services.length == 0)
                System.out.println("No services avialable!");

            else
                for ( int i=0; i<services.length; i++ )
                    System.out.println(services[i]);

            XComponent xComp = UnoRuntime.queryInterface(
                XComponent.class, xContext );
            xComp.dispose();

            passed = true;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        System.err.println();
        System.err.println("Bootstrap test passed? " + passed);
        System.out.println("*******************************************************************");
        System.err.println();
        return passed;
    }

    private static void usage() {
        System.out.println();
        System.out.println("usage:");
        System.out.println("java com.sun.star.comp.helper.Bootstrap_Test ini-file name=value ...");
        System.out.println("example:");
        System.out.println("java com.sun.star.comp.helper.Bootstrap_Test file:///c:/ooo10/program/uno.ini SYSBINDIR=file:///c:/ooo10/program");
        System.exit( -1 );
    }

    static public void main(String args[]) throws java.lang.Exception {
        if ( args.length == 0 )
            usage();

        java.util.Hashtable<String,String> bootstrap_parameters = new java.util.Hashtable<String,String>();
        for ( int nPos = 1; nPos < args.length; ++nPos )
        {
            String arg = args[ nPos ];
            int n = arg.indexOf( '=' );
            if (n > 0)
            {
                bootstrap_parameters.put( arg.substring( 0, n ), arg.substring( n +1 ) );
            }
        }

        System.exit( test(args[0], bootstrap_parameters) == true ? 0: -1 );
    }
}

