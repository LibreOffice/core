/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegistryServiceFactory_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:44:22 $
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

import com.sun.star.comp.loader.JavaLoader;

import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.container.XSet;
import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XElementAccess;

import com.sun.star.lang.XComponent;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XInitialization;

import com.sun.star.loader.XImplementationLoader;

import com.sun.star.registry.XSimpleRegistry;

public class RegistryServiceFactory_Test {

    static public boolean test(String applicat) throws java.lang.Exception {
        boolean passed = false;
        System.err.println();
        System.out.println("*******************************************************************");
        System.err.println("RegistryServiceFactory - doing tests...");
        System.err.println();

        try {
            XMultiServiceFactory msf = RegistryServiceFactory.create( applicat );
            String services[] = msf.getAvailableServiceNames();
            System.out.println("Available services are:");
            System.err.println();
            if (services.length == 0)
                System.out.println("No services avialable!");

            else
                for ( int i=0; i<services.length; i++ )
                    System.out.println(services[i]);

            passed = true;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        System.err.println();
        System.err.println("RegistryServiceFactory test passed? " + passed);
        System.out.println("*******************************************************************");
        System.err.println();
        return passed;
    }

    private static void usage() {
        System.out.println();
        System.out.println("usage:");
        System.out.println("java com.sun.star.comp.helper.RegistryServiceFactory [rdb-file]");
        System.out.println("example:");
        System.out.println("java com.sun.star.comp.helper.RegistryServiceFactory c:\\applicat.rdb");
        System.exit( -1 );
    }

    static public void main(String args[]) throws java.lang.Exception {
        if ( args.length != 1 )
            usage();
        System.exit( test(args[0]) == true ? 0: -1 );
    }
}

