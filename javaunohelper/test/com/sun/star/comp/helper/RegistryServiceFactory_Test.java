/*************************************************************************
 *
 *  $RCSfile: RegistryServiceFactory_Test.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:31:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

