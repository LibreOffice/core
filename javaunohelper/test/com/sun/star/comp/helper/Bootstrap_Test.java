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

import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.Map;

public class Bootstrap_Test {

    private static final Logger logger = Logger.getLogger(Bootstrap_Test.class.getName());

    public static boolean test( String ini_file, Map<String,String> bootstrap_parameters )
        throws java.lang.Exception
    {
        boolean passed = false;
        logger.log(Level.INFO, "Bootstrap - doing tests...");

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
            logger.log(Level.FINE, "Available services are:");
            if (services.length == 0)
                logger.log(Level.FINE, "No services available!");

            else
                for ( int i=0; i<services.length; i++ )
                    logger.log(Level.FINE, services[i]);

            XComponent xComp = UnoRuntime.queryInterface(
                XComponent.class, xContext );
            xComp.dispose();

            passed = true;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        logger.log(Level.INFO, "Bootstrap test passed? " + passed);
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

    public static void main(String args[]) throws java.lang.Exception {
        if ( args.length == 0 )
            usage();

        java.util.HashMap<String,String> bootstrap_parameters = new java.util.HashMap<String,String>();
        for ( int nPos = 1; nPos < args.length; ++nPos ) {
            if (args[nPos].contains("=")) {
                String bootstrap_parameter[] = args[nPos].split("=",2);
                if (bootstrap_parameter[0].length() > 0) {
                    bootstrap_parameters.put( bootstrap_parameter[0], bootstrap_parameter[1] );
                } else {
                    System.out.println();
                    System.out.println("The 1st argument in a bootstrap parameter is the key of a HashMap element and can't be null : '" + args[nPos] + "'");
                    usage();
                }
            } else {
                System.out.println();
                System.out.println("Missing '=' in bootstrap parameter : '" + args[nPos] + "'");
                usage();
            }
        }

        System.exit( test(args[0], bootstrap_parameters) ? 0: -1 );
    }
}

