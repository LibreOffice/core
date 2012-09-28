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
package org.openoffice;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.configuration.backend.PropertyInfo;
import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XLayerContentDescriber;
import com.sun.star.configuration.backend.XLayerHandler;
import com.sun.star.configuration.backend.XSingleLayerStratum;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XTimeStamped;

public class JavaSystemBackend implements XSingleLayerStratum, XTypeProvider,
                                          XServiceInfo, XTimeStamped, XComponent {
    public final static String __serviceName = "com.sun.star.configuration.backend.PlatformBackend";
    public final static String __implName = "org.openoffice.JavaSystemBackend";
    public final static String testComponent = "org.openoffice.Office.Common";
    protected static XMultiServiceFactory msf = null;
    protected XLayer aLayer = null;

    /**
     * Get the implementation id.
     * @return An empty implementation id.
     * @see com.sun.star.lang.XTypeProvider
     */
    public byte[] getImplementationId() {
        return new byte[0];
    }

    /**
     * Function for reading the implementation name.
     *
     * @return the implementation name
     * @see com.sun.star.lang.XServiceInfo
     */
    public String getImplementationName() {
        return __implName;
    }

    public com.sun.star.configuration.backend.XLayer getLayer(String str,
                                                              String str1)
        throws com.sun.star.configuration.backend.BackendAccessException,
               com.sun.star.lang.IllegalArgumentException {
        if (aLayer == null) {
            System.out.println("JavaSystemBackend::getLayer() called for " +
                               str);
            aLayer = new CommonLayer();
        }

        return aLayer;
    }

    /**
     * Function for reading all supported services
     *
     * @return An aaray with all supported service names
     * @see com.sun.star.lang.XServiceInfo
     */
    public String[] getSupportedServiceNames() {
        String[] supServiceNames = { __serviceName };

        return supServiceNames;
    }

    public String getTimestamp() {
        //not really implemented
        return "2004-03-31";
    }

    /**
     * Get all implemented types of this class.
     * @return An array of implemented interface types.
     * @see com.sun.star.lang.XTypeProvider
     */
    public Type[] getTypes() {
        Type[] type = new Type[5];
        type[0] = new Type(XInterface.class);
        type[1] = new Type(XTypeProvider.class);
        type[2] = new Type(XSingleLayerStratum.class);
        type[3] = new Type(XServiceInfo.class);
        type[4] = new Type(XTimeStamped.class);

        return type;
    }

    public com.sun.star.configuration.backend.XUpdatableLayer getUpdatableLayer(String str)
        throws com.sun.star.configuration.backend.BackendAccessException,
               com.sun.star.lang.NoSupportException,
               com.sun.star.lang.IllegalArgumentException {
        throw new com.sun.star.lang.NoSupportException(
                "Cannot write to test backend", this);
    }

    /**
     * Does the implementation support this service?
     *
     * @param serviceName The name of the service in question
     * @return true, if service is supported, false otherwise
     * @see com.sun.star.lang.XServiceInfo
     */
    public boolean supportsService(String serviceName) {
        if (serviceName.equals(__serviceName)) {
            return true;
        }

        return false;
    }

    /**
    *
    * Gives a factory for creating the service.
    * This method is called by the <code>JavaLoader</code>
    * <p>
    * @return  returns a <code>XSingleServiceFactory</code> for creating the component
    * @param   implName     the name of the implementation for which a service is desired
    * @param   multiFactory the service manager to be used if needed
    * @param   regKey       the registryKey
    * @see                  com.sun.star.comp.loader.JavaLoader
    */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                            XMultiServiceFactory multiFactory,
                                                            XRegistryKey regKey) {
        XSingleServiceFactory xSingleServiceFactory = null;

        msf = multiFactory;

        if (implName.equals(JavaSystemBackend.class.getName())) {
            xSingleServiceFactory = FactoryHelper.getServiceFactory(
                                            JavaSystemBackend.class,
                                            __serviceName, multiFactory,
                                            regKey);
        }

        return xSingleServiceFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        boolean success = FactoryHelper.writeRegistryServiceInfo(__implName,
                                                                 __serviceName,
                                                                 regKey);

        if (success) {
            try {
                String keyName = "/" + __implName +
                                 "/DATA/SupportedComponents";
                XRegistryKey newKey = regKey.createKey(keyName);

                String[] supportedComponents = { testComponent };
                newKey.setAsciiListValue(supportedComponents);
            } catch (Exception ex) {
                success = false; // prevent startup loop
                System.out.println("can't register component");
            }
        }

        return success;
    }

    public void addEventListener(com.sun.star.lang.XEventListener xEventListener) {
    }

    public void dispose() {
        System.out.println("JavaSystemBackend::dispose() called");
    }

    public void removeEventListener(com.sun.star.lang.XEventListener xEventListener) {
    }

    protected class CommonLayer implements XLayer, XTimeStamped {
        public void readData(XLayerHandler xLayerHandler)
                      throws com.sun.star.lang.NullPointerException,
                             com.sun.star.lang.WrappedTargetException,
                             com.sun.star.configuration.backend.MalformedDataException {
            System.out.println("CommonLayer is read");

            if (xLayerHandler == null) {
                throw new com.sun.star.lang.NullPointerException(
                        "null is not a legal LayerHandler");
            }

            XLayerContentDescriber xLayerContentDescriber = null;

            try {
                xLayerContentDescriber = UnoRuntime.queryInterface(
                                                 XLayerContentDescriber.class,
                                                 msf.createInstance(
                                                         "com.sun.star.comp.configuration.backend.LayerDescriber"));
            } catch (com.sun.star.uno.Exception e) {
                throw new com.sun.star.lang.NullPointerException(
                        "exception while creating LayerDesccriber");
            }

            if (xLayerContentDescriber == null) {
                throw new com.sun.star.lang.NullPointerException(
                        "created LayerDescriber isn't valid");
            }

            PropertyInfo[] pInfo = new PropertyInfo[1];
            pInfo[0] = new PropertyInfo();
            pInfo[0].Name = "org.openoffice.Office.Common/Undo/Steps";
            pInfo[0].Value = new Integer(12);
            pInfo[0].Type = "int";
            pInfo[0].Protected = false;
            xLayerContentDescriber.describeLayer(xLayerHandler, pInfo);
        }

        public String getTimestamp() {
            //not really implemented
            return "2004-03-31";
        }
    }
}
