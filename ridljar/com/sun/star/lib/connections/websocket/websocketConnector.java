/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.connections.websocket;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.connection.NoConnectException;
import com.sun.star.connection.XConnection;
import com.sun.star.connection.XConnector;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;

import java.io.IOException;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URISyntaxException;
import java.net.UnknownHostException;

/**
 * A component that implements the <code>XConnector</code> interface.
 *
 * <p>The <code>websocketConnector</code> is a specialized component that uses
 * websockets for communication.  The <code>websocketConnector</code> is generally
 * used by the <code>com.sun.star.connection.Connector</code> service.</p>
 *
 * @see com.sun.star.connection.XAcceptor
 * @see com.sun.star.connection.XConnection
 * @see com.sun.star.connection.XConnector
 * @see com.sun.star.comp.loader.JavaLoader
 */
public final class websocketConnector implements XConnector {
    /**
     * The name of the service.
     *
     * <p>The <code>JavaLoader</code> accesses this through reflection.</p>
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static final String __serviceName
        = "com.sun.star.connection.websocketConnector";

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
        return implName.equals(websocketConnector.class.getName())
               ? FactoryHelper.getServiceFactory(websocketConnector.class,
                       __serviceName, multiFactory,
                       regKey)
               : null;
    }

    /**
     * Connects via the described websocket to a waiting server.
     *
     * <p>The connection description has the following format:
     * <code><var>type</var></code><!--
     *     -->*(<code><var>key</var>=<var>value</var></code>),
     * where <code><var>type</var></code> should be <code>websocket</code>
     * (ignoring case).  Supported keys (ignoring case) currently are</p>
     * <dl>
     * <dt><code>url</code>
     * <dd>The URL the websocket server is listening on, starting with
     *     either ws:// or wss://
     * </dl>
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
        if (connected)
            throw new ConnectionSetupException("Already connected to the socket");

        ConnectionDescriptor desc;
        try {
            desc = new ConnectionDescriptor(connectionDescription);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new ConnectionSetupException(e);
        }

        WebsocketConnection websocket = null;
        try {
            websocket = new WebsocketConnection(connectionDescription, desc);
            connected = websocket.isOpen();
        } catch (IOException e) {
            throw new ConnectionSetupException(e);
        } catch (URISyntaxException e) {
            throw new ConnectionSetupException(e);
        } catch (InterruptedException e) {
            throw new ConnectionSetupException(e);
        }

        if (websocket == null || !connected)
            throw new ConnectionSetupException("Could not connect to the server. Is it up?");

        return websocket;
    }

    private boolean connected = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
