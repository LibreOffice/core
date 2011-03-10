/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package com.sun.star.comp.urlresolver;


import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XUnoUrlResolver;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.NoConnectException;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.UnoRuntime;


/**
 * This component gives a factory for an <code>UnoUrlResolver</code> service.
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2008-04-11 11:12:25 $
 * @author      Kay Ramme
 * @see         com.sun.star.brige.XBrideFactory
 * @see         com.sun.star.connection.Connector
 * @since       UDK1.0
 */
public class UrlResolver {
    static private final boolean DEBUG = false;


    static public class _UrlResolver implements XUnoUrlResolver {
        static private final String __serviceName = "com.sun.star.bridge.UnoUrlResolver";

        private XMultiServiceFactory _xMultiServiceFactory;

        public _UrlResolver(XMultiServiceFactory xMultiServiceFactory) {
            _xMultiServiceFactory = xMultiServiceFactory;
        }

        public Object resolve(/*IN*/String dcp) throws NoConnectException, ConnectionSetupException, IllegalArgumentException, com.sun.star.uno.RuntimeException {
            String conDcp  ;
            String protDcp  ;
            String rootOid  ;

            if(dcp.indexOf(';') == -1) {// use old style
                conDcp = dcp;
                protDcp = "iiop";
                rootOid = "classic_uno";
            }
            else { // new style
                int index = dcp.indexOf(':');
                String url = dcp.substring(0, index).trim();
                dcp = dcp.substring(index + 1).trim();

                index = dcp.indexOf(';');
                conDcp = dcp.substring(0, index).trim();
                dcp = dcp.substring(index + 1).trim();

                index = dcp.indexOf(';');
                protDcp = dcp.substring(0, index).trim();
                dcp = dcp.substring(index + 1).trim();

                rootOid = dcp.trim().trim();
            }

            Object rootObject  ;
            XBridgeFactory xBridgeFactory ;
            try {
                xBridgeFactory = UnoRuntime.queryInterface(XBridgeFactory.class,
                                                                          _xMultiServiceFactory.createInstance("com.sun.star.bridge.BridgeFactory"));
            } catch (com.sun.star.uno.Exception e) {
                throw new com.sun.star.uno.RuntimeException(e.getMessage());
            }
            XBridge xBridge = xBridgeFactory.getBridge(conDcp + ";" + protDcp);

            if(xBridge == null) {
                Object connector ;
                try {
                    connector = _xMultiServiceFactory.createInstance("com.sun.star.connection.Connector");
                } catch (com.sun.star.uno.Exception e) {
                        throw new com.sun.star.uno.RuntimeException(e.getMessage());
                }

                XConnector connector_xConnector = UnoRuntime.queryInterface(XConnector.class, connector);

                // connect to the server
                XConnection xConnection = connector_xConnector.connect(conDcp);
                try {
                    xBridge = xBridgeFactory.createBridge(conDcp + ";" + protDcp, protDcp, xConnection, null);
                } catch (com.sun.star.bridge.BridgeExistsException e) {
                    throw new com.sun.star.uno.RuntimeException(e.getMessage());
                }
            }
            rootObject = xBridge.getInstance(rootOid);
            return rootObject;
        }
    }


    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be uses if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                            XMultiServiceFactory multiFactory,
                                                            XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

          if (implName.equals(UrlResolver.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(_UrlResolver.class,
                                                                    _UrlResolver.__serviceName,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }
}
