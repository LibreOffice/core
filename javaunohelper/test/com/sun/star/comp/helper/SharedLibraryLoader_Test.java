/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    private static final String NATIVE_SERVICE_MANAGER_LIB_NAME = "servicemgr.uno";
    private static final String NATIVE_REGISTRY_IMP_NAME = "com.sun.star.comp.stoc.SimpleRegistry";
    private static final String NATIVE_REGISTRY_LIB_NAME = "simplereg.uno";

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
        System.out.println(sharedLibraryLoaderFactory == null? "failed" : "successful");
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

        sharedLibraryLoader = UnoRuntime.queryInterface(
                XImplementationLoader.class, sharedLibraryLoaderFactory.createInstance() );

        System.out.print("Test - ");
        System.out.println(sharedLibraryLoader == null? "failed" : "successful");
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
            UnoRuntime.queryInterface( XSingleServiceFactory.class,
                        sharedLibraryLoader.activate(NATIVE_SERVICE_MANAGER_IMP_NAME, null, NATIVE_SERVICE_MANAGER_LIB_NAME, null));

        System.err.println("- instantiate the native ServiceManger");
        nativeServiceManager = UnoRuntime.queryInterface( XMultiServiceFactory.class, aSMgrFac.createInstance() );

        System.out.print("Test - ");
        System.out.println(nativeServiceManager == null? "failed" : "successful");

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
            UnoRuntime.queryInterface( XSingleServiceFactory.class,
                        sharedLibraryLoader.activate(NATIVE_REGISTRY_IMP_NAME, null, NATIVE_REGISTRY_LIB_NAME, null)
            );
        System.err.println("- instantiate the Registry");
        simpleRegistry =
            UnoRuntime.queryInterface( XSimpleRegistry.class, aRegFac.createInstance() );
        System.out.print("Test - ");
        System.out.println(simpleRegistry == null? "failed" : "successful");
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
        System.out.println( result==false ? "failed" : "successful");
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

