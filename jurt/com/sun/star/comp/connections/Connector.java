/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Connector.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.comp.connections;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.NoConnectException;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;

/**
 * A component that implements the <code>XConnector</code> interface.
 *
 * <p>The <code>Connector</code> is a general component, that uses less general
 * components (like <code>com.sun.star.connection.socketConnector</code>) to
 * implement its functionality.</p>
 *
 * @see com.sun.star.connections.XAcceptor
 * @see com.sun.star.connections.XConnection
 * @see com.sun.star.connections.XConnector
 * @see com.sun.star.loader.JavaLoader
 *
 * @since UDK 1.0
 */
public class Connector implements XConnector {
    /**
     * The name of the service.
     *
     * <p>The <code>JavaLoader</code> acceses this through reflection.</p>
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static final String __serviceName
    = "com.sun.star.connection.Connector";

    /**
     * Returns a factory for creating the service.
     *
     * <p>This method is called by the <code>JavaLoader</code>.</p>
     *
     * @param implName the name of the implementation for which a service is
     *     requested.
     * @param multiFactory the service manager to be used (if needed).
     * @param regKey the registry key.
     * @return an <code>XSingleServiceFactory</code> for creating the component.
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(
        String implName, XMultiServiceFactory multiFactory, XRegistryKey regKey)
    {
        return implName.equals(Connector.class.getName())
            ? FactoryHelper.getServiceFactory(Connector.class, __serviceName,
                                              multiFactory, regKey)
            : null;
    }

    /**
     * Writes the service information into the given registry key.
     *
     * <p>This method is called by the <code>JavaLoader</code>.</p>
     *
     * @param regKey the registry key.
     * @return <code>true</code> if the operation succeeded.
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(Connector.class.getName(),
                                                      __serviceName, regKey);
    }

    /**
     * Constructs a new <code>Connector</code> that uses the given service
     * factory to create a specific <code>XConnector</code>.
     *
     * @param serviceFactory the service factory to use.
     */
    public Connector(XMultiServiceFactory serviceFactory) {
        this.serviceFactory = serviceFactory;
    }

    /**
     * Connects via the given connection type to a waiting server.
     *
     * <p>The connection description has the following format:
     * <code><var>type</var></code><!--
     *     -->*(<code><var>key</var>=<var>value</var></code>).
     * The specific <code>XConnector</code> implementation is instantiated
     * through the service factory as
     * <code>com.sun.star.connection.<var>type</var>Connector</code> (with
     * <code><var>type</var></code> in lower case).</p>
     *
     * @param connectionDescription the description of the connection.
     * @return an <code>XConnection</code> to the server.
     *
     * @see com.sun.star.connections.XAcceptor
     * @see com.sun.star.connections.XConnection
     */
    public synchronized XConnection connect(String connectionDescription)
        throws NoConnectException, ConnectionSetupException
    {
        if (DEBUG) {
            System.err.println("##### " + getClass().getName() + ".connect("
                               + connectionDescription + ")");
        }
        if (connected) {
            throw new ConnectionSetupException("already connected");
        }
        XConnection con
            = ((XConnector) Implementation.getConnectionService(
                   serviceFactory, connectionDescription, XConnector.class,
                   "Connector")).connect(connectionDescription);
        connected = true;
        return con;
    }

    private static final boolean DEBUG = false;

    private final XMultiServiceFactory serviceFactory;

    private boolean connected = false;
}
