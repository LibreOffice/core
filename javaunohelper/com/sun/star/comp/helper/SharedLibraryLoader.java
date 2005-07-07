/*************************************************************************
 *
 *  $RCSfile: SharedLibraryLoader.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 10:57:25 $
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
        return (XSingleServiceFactory) UnoRuntime.queryInterface(
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
        return (XSingleServiceFactory) UnoRuntime.queryInterface(
                    XSingleServiceFactory.class,
                    component_getFactory(
                        libName, impName, smgr, regKey,
                        SharedLibraryLoader.class.getClassLoader() ) );
    }

    /**
     * Registers the SharedLibraryLoader under a RegistryKey.
     * <p>
     * @return  true if the registration was successfull - otherwise false
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
     * @return  true if the registration was successfull - otherwise false
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

