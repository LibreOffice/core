/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Bootstrap_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:43:47 $
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

