/*************************************************************************
 *
 *  $RCSfile: UrlResolver.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:52 $
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

package com.sun.star.comp.urlresolver;


import java.util.Enumeration;


import com.sun.star.bridge.BridgeExistsException;
import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XInstanceProvider;
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

import com.sun.star.uno.MappingException;
import com.sun.star.uno.UnoRuntime;


/**
 * This component gives a factory for an <code>UnoUrlResolver</code> service.
 * <p>
 * @version     $Revision: 1.1.1.1 $ $ $Date: 2000-09-18 15:27:52 $
 * @author      Kay Ramme
 * @see         com.sun.star.brige.XBrideFactory
 * @see         com.sun.star.connection.Connector
 * @since       UDK1.0
 */
public class UrlResolver {
    static private final boolean DEBUG = true;


    static public class _UrlResolver implements XUnoUrlResolver {
        static private final String __serviceName = "com.sun.star.bridge.UnoUrlResolver";

        private XMultiServiceFactory _xMultiServiceFactory;

        public _UrlResolver(XMultiServiceFactory xMultiServiceFactory) {
            _xMultiServiceFactory = xMultiServiceFactory;
        }

        public Object resolve(/*IN*/String dcp) throws NoConnectException, ConnectionSetupException, IllegalArgumentException, com.sun.star.uno.RuntimeException {
            String conDcp = null;
            String protDcp = null;
            String rootOid = null;

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

            Object rootObject = null;

            try {
                XBridgeFactory xBridgeFactory = (XBridgeFactory)UnoRuntime.queryInterface(XBridgeFactory.class,
                                                                                          _xMultiServiceFactory.createInstance("com.sun.star.bridge.BridgeFactory"));

                XBridge xBridge = xBridgeFactory.getBridge(conDcp + ";" + protDcp);

                if(xBridge == null) {
                    Object connector = _xMultiServiceFactory.createInstance("com.sun.star.connection.Connector");

                    XConnector connector_xConnector = (XConnector)UnoRuntime.queryInterface(XConnector.class, connector);

                    // connect to the server
                    XConnection xConnection = connector_xConnector.connect(conDcp);

                    xBridge = xBridgeFactory.createBridge(conDcp + ";" + protDcp, protDcp, xConnection, null);
                }
                rootObject = xBridge.getInstance(rootOid);
            }
            catch(Exception exception) {
                if(DEBUG) {
                    System.err.println("##### " + getClass().getName() + ".resolve - exception occurred:" + exception);
                    exception.printStackTrace();
                }
                throw new com.sun.star.uno.RuntimeException(exception.getMessage());
            }

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

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(_UrlResolver.class.getName(), _UrlResolver.__serviceName, regKey);
    }

}

