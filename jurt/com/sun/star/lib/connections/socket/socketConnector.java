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

package com.sun.star.lib.connections.socket;

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
import java.net.UnknownHostException;

/**
 * A component that implements the <code>XConnector</code> interface.
 *
 * <p>The <code>socketConnector</code> is a specialized component that uses TCP
 * sockets for communication.  The <code>socketConnector</code> is generally
 * used by the <code>com.sun.star.connection.Connector</code> service.</p>
 *
 * @see com.sun.star.connection.XAcceptor
 * @see com.sun.star.connection.XConnection
 * @see com.sun.star.connection.XConnector
 * @see com.sun.star.comp.loader.JavaLoader
 *
 * @since UDK 1.0
 */
public final class socketConnector implements XConnector {
    /**
     * The name of the service.
     *
     * <p>The <code>JavaLoader</code> acceses this through reflection.</p>
     *
     * @see com.sun.star.comp.loader.JavaLoader
     */
    public static final String __serviceName
    = "com.sun.star.connection.socketConnector";

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
        return implName.equals(socketConnector.class.getName())
            ? FactoryHelper.getServiceFactory(socketConnector.class,
                                              __serviceName, multiFactory,
                                              regKey)
            : null;
    }

    /**
     * Connects via the described socket to a waiting server.
     *
     * <p>The connection description has the following format:
     * <code><var>type</var></code><!--
     *     -->*(<code><var>key</var>=<var>value</var></code>),
     * where <code><var>type</var></code> should be <code>socket</code>
     * (ignoring case).  Supported keys (ignoring case) currently are
     * <dl>
     * <dt><code>host</code>
     * <dd>The name or address of the server.  Must be present.
     * <dt><code>port</code>
     * <dd>The TCP port number of the server (defaults to <code>6001</code>).
     * <dt><code>tcpnodelay</code>
     * <dd>A flag (<code>0</code>/<code>1</code>) enabling or disabling Nagle's
     *     algorithm on the resulting connection.
     * </dl></p>
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
        if (connected) {
            throw new ConnectionSetupException("alread connected");
        }
        ConnectionDescriptor desc;
        try {
            desc = new ConnectionDescriptor(connectionDescription);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            throw new ConnectionSetupException(e.toString());
        }
        if (desc.getHost() == null) {
            throw new ConnectionSetupException("host parameter missing");
        }
        // Try all (IPv4 and IPv6) addresses, in case this client is on a
        // dual-stack host and the server process is an IPv4-only process, also
        // on a dual-stack host (see Stevens, Fenner, Rudoff: "Unix Network
        // Programming, Volume 1: The Sockets Networking API, 3rd Edition",
        // p. 359):
        InetAddress[] adr;
        try {
            adr = InetAddress.getAllByName(desc.getHost());
        } catch (UnknownHostException e) {
            throw new ConnectionSetupException(e.toString());
        }
        Socket socket = null;
        for (int i = 0; i < adr.length; ++i) {
            try {
                socket = new Socket(adr[i], desc.getPort());
                break;
            } catch (IOException e) {
                if (i == adr.length - 1) {
                    throw new NoConnectException(e.toString());
                }
            }
        }
        XConnection con;
        try {
            if (desc.getTcpNoDelay() != null) {
                socket.setTcpNoDelay(desc.getTcpNoDelay().booleanValue());
            }
            con = new SocketConnection(connectionDescription, socket);
        } catch (IOException e) {
            throw new NoConnectException(e.toString());
        }
        connected = true;
        return con;
    }

    private boolean connected = false;
}
