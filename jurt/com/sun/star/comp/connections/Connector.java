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
 * @see com.sun.star.connection.XAcceptor
 * @see com.sun.star.connection.XConnection
 * @see com.sun.star.connection.XConnector
 * @see com.sun.star.comp.loader.JavaLoader
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
     * @see com.sun.star.connection.XAcceptor
     * @see com.sun.star.connection.XConnection
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
