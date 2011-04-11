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
 * @see com.sun.star.connections.XAcceptor
 * @see com.sun.star.connections.XConnection
 * @see com.sun.star.connections.XConnector
 * @see com.sun.star.loader.JavaLoader
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
     * @see com.sun.star.connections.XAcceptor
     * @see com.sun.star.connections.XConnection
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
