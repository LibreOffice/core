/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Bootstrap_Test.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.comp.helper;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;


public class Bootstrap_Test {

    static public boolean test( String ini_file, java.util.Hashtable bootstrap_parameters )
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

            XMultiServiceFactory msf = (XMultiServiceFactory)UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xContext.getServiceManager() );
            String services[] = msf.getAvailableServiceNames();
            System.out.println("Available services are:");
            System.err.println();
            if (services.length == 0)
                System.out.println("No services avialable!");

            else
                for ( int i=0; i<services.length; i++ )
                    System.out.println(services[i]);

            XComponent xComp = (XComponent)UnoRuntime.queryInterface(
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

        java.util.Hashtable bootstrap_parameters = new java.util.Hashtable();
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

