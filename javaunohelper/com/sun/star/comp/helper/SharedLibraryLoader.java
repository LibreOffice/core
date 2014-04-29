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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;

/**
 * @deprecated use class Bootstrap bootstrapping a native UNO installation
 *             and use the shared library loader service.
 *
 * The <code>SharedLibraryLoader</code> class provides the functionality of the <code>com.sun.star.loader.SharedLibrary</code>
 * service.
 * <p>
 * @see         com.sun.star.loader.SharedLibrary
 * @see         com.sun.star.comp.servicemanager.ServiceManager
 * @see         com.sun.star.lang.ServiceManager
 */
public class SharedLibraryLoader {
    /**
     * The default library which contains the SharedLibraryLoader component
     */
    public static final String DEFAULT_LIBRARY = "shlibloader.uno";

    /**
     * The default implementation name
     */
    public static final String DEFAULT_IMPLEMENTATION = "com.sun.star.comp.stoc.DLLComponentLoader";

    static {
        System.loadLibrary("juh");
    }

    private static native boolean component_writeInfo(
            String libName, XMultiServiceFactory smgr, XRegistryKey regKey,
            ClassLoader loader );

    private static native Object component_getFactory(
            String libName, String implName, XMultiServiceFactory smgr,
            XRegistryKey regKey, ClassLoader loader );

    /**
     * Supplies the ServiceFactory of the default SharedLibraryLoader.
     * The defaults are "shlibloader.uno"
     * for the library and "com.sun.star.comp.stoc.DLLComponentLoader"
     * for the component name.
     * <p>
     * @return  the factory for the "com.sun.star.comp.stoc.DLLComponentLoader" component.
     * @param   smgr    the ServiceManager
     * @param   regKey  the root registry key
     * @see     com.sun.star.loader.SharedLibrary
     * @see     com.sun.star.lang.ServiceManager
     * @see     com.sun.star.registry.RegistryKey
     */
    public static XSingleServiceFactory getServiceFactory(
                XMultiServiceFactory smgr,
                XRegistryKey regKey )
    {
        return UnoRuntime.queryInterface(
                    XSingleServiceFactory.class,
                    component_getFactory(
                        DEFAULT_LIBRARY, DEFAULT_IMPLEMENTATION, smgr, regKey,
                        SharedLibraryLoader.class.getClassLoader() ) );
    }

    /**
     * Loads and returns a specific factory for a given library and implementation name.
     * <p>
     * @return  the factory of the component
     * @param   libName the name of the shared library
     * @param   impName the implementation name of the component
     * @param   smgr    the ServiceManager
     * @param   regKey  the root registry key
     * @see     com.sun.star.loader.SharedLibrary
     * @see     com.sun.star.lang.ServiceManager
     * @see     com.sun.star.registry.RegistryKey
     */
    public static XSingleServiceFactory getServiceFactory(
                String libName,
                String impName,
                XMultiServiceFactory smgr,
                XRegistryKey regKey )
    {
        return UnoRuntime.queryInterface(
                    XSingleServiceFactory.class,
                    component_getFactory(
                        libName, impName, smgr, regKey,
                        SharedLibraryLoader.class.getClassLoader() ) );
    }

    /**
     * Registers the SharedLibraryLoader under a RegistryKey.
     * <p>
     * @return  true if the registration was successful - otherwise false
     * @param   smgr    the ServiceManager
     * @param   regKey  the root key under that the component should be registered
     * @see     com.sun.star.loader.SharedLibrary
     * @see     com.sun.star.lang.ServiceManager
     * @see     com.sun.star.registry.RegistryKey
     */
    public static boolean writeRegistryServiceInfo(
                com.sun.star.lang.XMultiServiceFactory smgr,
                com.sun.star.registry.XRegistryKey regKey )
    {
        return component_writeInfo(
            DEFAULT_LIBRARY, smgr, regKey,
            SharedLibraryLoader.class.getClassLoader() );
    }

    /**
     * Registers the SharedLibraryLoader under a RegistryKey.
     * <p>
     * @return  true if the registration was successful - otherwise false
     * @param   libName name of the shared library
     * @param   smgr    the ServiceManager
     * @param   regKey  the root key under that the component should be registered
     * @see     com.sun.star.loader.SharedLibrary
     * @see     com.sun.star.lang.ServiceManager
     * @see     com.sun.star.registry.RegistryKey
     */
    public static boolean writeRegistryServiceInfo(
                String libName,
                com.sun.star.lang.XMultiServiceFactory smgr,
                com.sun.star.registry.XRegistryKey regKey )

            throws  com.sun.star.registry.InvalidRegistryException,
                    com.sun.star.uno.RuntimeException
    {
        return component_writeInfo(
            libName, smgr, regKey, SharedLibraryLoader.class.getClassLoader() );
    }
}

