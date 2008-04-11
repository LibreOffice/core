/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestExtension.java,v $
 *
 * $Revision: 1.3 $
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
 *************************************************************************/
package com.sun.star.comp.smoketest;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getComponentFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class TestExtension {
    /** This class implements the component. At least the interfaces XServiceInfo,
     * XTypeProvider, and XInitialization should be provided by the service.
     */
    public static class _TestExtension extends WeakBase
        implements XServiceInfo {
        /** The service name, that must be used to get an instance of this service.
         */
        static private final String __serviceName =
        "com.sun.star.comp.smoketest.TestExtension";

        /** The initial component contextr, that gives access to
         * the service manager, supported singletons, ...
         * It's often later used
         */
        private XComponentContext m_cmpCtx;

        /** The service manager, that gives access to all registered services.
         * It's often later used
         */
        private XMultiComponentFactory m_xMCF;

        /** The constructor of the inner class has a XMultiServiceFactory parameter.
         * @param xmultiservicefactoryInitialization A special service factory
         * could be introduced while initializing.
         */
        public _TestExtension(XComponentContext xCompContext) {
            try {
                m_cmpCtx = xCompContext;
                m_xMCF = m_cmpCtx.getServiceManager();
            }
            catch( Exception e ) {
                e.printStackTrace();
            }
        }

        /** This method returns an array of all supported service names.
         * @return Array of supported service names.
         */
        public String[] getSupportedServiceNames() {
            return getServiceNames();
        }

        /** This method is a simple helper function to used in the
         * static component initialisation functions as well as in
         * getSupportedServiceNames.
         */
        public static String[] getServiceNames() {
            String[] sSupportedServiceNames = { __serviceName };
            return sSupportedServiceNames;
        }

        /** This method returns true, if the given service will be
         * supported by the component.
         * @param sServiceName Service name.
         * @return True, if the given service name will be supported.
         */
        public boolean supportsService( String sServiceName ) {
            return sServiceName.equals( __serviceName );
        }

        /** Return the class name of the component.
         * @return Class name of the component.
         */
        public String getImplementationName() {
            return  _TestExtension.class.getName();
        }
    }


    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( _TestExtension.class.getName() ) )
            xFactory = Factory.createComponentFactory(_TestExtension.class,
                                             _TestExtension.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return Factory.writeRegistryServiceInfo(_TestExtension.class.getName(),
                                                _TestExtension.getServiceNames(),
                                                regKey);
    }
        /** This method is a member of the interface for initializing an object
         * directly after its creation.
         * @param object This array of arbitrary objects will be passed to the
         * component after its creation.
         * @throws Exception Every exception will not be handled, but will be
         * passed to the caller.
         */
        public void initialize( Object[] object )
            throws com.sun.star.uno.Exception {
            /* The component describes what arguments its expected and in which
             * order!At this point you can read the objects and can intialize
             * your component using these objects.
             */
        }

}
