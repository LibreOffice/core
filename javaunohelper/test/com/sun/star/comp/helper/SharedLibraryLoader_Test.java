/*************************************************************************
 *
 *  $RCSfile: SharedLibraryLoader_Test.java,v $
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


public class SharedLibraryLoader_Test {

    private static final String NATIVE_SERVICE_MANAGER_IMP_NAME = "com.sun.star.comp.stoc.OServiceManager";
    private static final String NATIVE_SERVICE_MANAGER_LIB_NAME = "smgr";
    private static final String NATIVE_REGISTRY_IMP_NAME = "com.sun.star.comp.stoc.SimpleRegistry";
    private static final String NATIVE_REGISTRY_LIB_NAME = "simreg";

    private static XMultiServiceFactory     nativeServiceManager        = null;
    private static XSingleServiceFactory    sharedLibraryLoaderFactory  = null;
    private static XImplementationLoader    sharedLibraryLoader         = null;
    private static XSimpleRegistry          simpleRegistry              = null;

    static public boolean test_getSharedLibraryLoaderFactory()
            throws java.lang.Exception
    {
        sharedLibraryLoaderFactory = null;
        System.out.println("*******************************************************************");
        System.out.println("Test: <<< get SharedLibraryLoader factory >>>");
        sharedLibraryLoaderFactory = SharedLibraryLoader.getServiceFactory(null, null);

        System.out.print("Test - ");
        System.out.println(sharedLibraryLoaderFactory == null? "failed" : "successfull");
        System.out.println("*******************************************************************");
        System.out.println();

        return sharedLibraryLoaderFactory != null;
    }

    static public boolean test_instantiateSharedLibraryLoader()
            throws java.lang.Exception
    {
        sharedLibraryLoader = null;
        System.out.println("*******************************************************************");
        System.out.println("Test: <<< instantiate SharedLibraryLoader >>>");
        if ( sharedLibraryLoaderFactory == null )
            if ( ! test_getSharedLibraryLoaderFactory() )
                return false;

        sharedLibraryLoader = (XImplementationLoader) UnoRuntime.queryInterface(
                XImplementationLoader.class, sharedLibraryLoaderFactory.createInstance() );

        System.out.print("Test - ");
        System.out.println(sharedLibraryLoader == null? "failed" : "successfull");
        System.out.println("*******************************************************************");
        System.out.println();

        return sharedLibraryLoader != null;
    }

    static public boolean test_loadNativeServiceManager()
            throws java.lang.Exception
    {
        nativeServiceManager = null;

        System.out.println("*******************************************************************");
        System.out.println("Test: <<< load native ServiceManager >>>");
        if ( sharedLibraryLoader == null )
            if ( ! test_instantiateSharedLibraryLoader() )
                return false;

        System.err.println("- get the native ServiceManger factory");
        XSingleServiceFactory aSMgrFac =
            (XSingleServiceFactory) UnoRuntime.queryInterface( XSingleServiceFactory.class,
                        sharedLibraryLoader.activate(NATIVE_SERVICE_MANAGER_IMP_NAME, null, NATIVE_SERVICE_MANAGER_LIB_NAME, null));

        System.err.println("- instantiate the native ServiceManger");
        nativeServiceManager =  (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, aSMgrFac.createInstance() );

        System.out.print("Test - ");
        System.out.println(nativeServiceManager == null? "failed" : "successfull");

        System.out.println("*******************************************************************");
        System.out.println();
        return nativeServiceManager != null;
    }

    static public boolean test_loadNativeSimpleRegistry()
            throws java.lang.Exception
    {
        boolean result = false;
        System.out.println("*******************************************************************");
        System.out.println("Test: <<< load native SimpleRegistry >>>");
        if ( sharedLibraryLoader == null )
            if ( ! test_instantiateSharedLibraryLoader() )
                return false;

        System.err.println("- get factory of the Registry");
        XSingleServiceFactory aRegFac =
            (XSingleServiceFactory) UnoRuntime.queryInterface( XSingleServiceFactory.class,
                        sharedLibraryLoader.activate(NATIVE_REGISTRY_IMP_NAME, null, NATIVE_REGISTRY_LIB_NAME, null)
            );
        System.err.println("- instantiate the Registry");
        simpleRegistry =
            (XSimpleRegistry) UnoRuntime.queryInterface( XSimpleRegistry.class, aRegFac.createInstance() );
        System.out.print("Test - ");
        System.out.println(simpleRegistry == null? "failed" : "successfull");
        System.out.println("*******************************************************************");
        System.err.println();
        return true;
    }

    static public boolean test_registerSharedLibraryLoader()
            throws java.lang.Exception
    {
        boolean result = true;
        System.out.println("*******************************************************************");
        System.out.println("Test: <<< register SharedLibraryLoader at the Registry >>>");

        if ( simpleRegistry == null )
            if ( ! test_loadNativeSimpleRegistry() )
                return false;

        com.sun.star.registry.XRegistryKey regKey = simpleRegistry.getRootKey();
        result = SharedLibraryLoader.writeRegistryServiceInfo( null,  regKey );

        System.out.print("Test - ");
        System.out.println( result==false ? "failed" : "successfull");
        System.out.println("*******************************************************************");
        System.out.println();
        return result;
    }

    static public boolean test() throws java.lang.Exception {
        boolean passed = true;

        System.err.println("SharedLibraryLoader - doing tests...");
        passed = test_getSharedLibraryLoaderFactory() && passed;
        passed = test_instantiateSharedLibraryLoader() && passed;
        passed = test_loadNativeServiceManager() && passed;
        passed = test_loadNativeSimpleRegistry() && passed;
        //passed = test_registerSharedLibraryLoader() && passed;

        System.err.println("SharedLibraryLoader test passed? " + passed);

        return passed;
    }

    static public void main(String args[]) throws java.lang.Exception {
        System.exit( test() == true ? 0: -1 );
    }
}

