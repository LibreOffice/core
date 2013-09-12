/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

class TcpServerConnection implements ServerConnection {
    private final Server mServer;
    private final Socket mServerConnection;

    public TcpServerConnection(Server aServer) {
        mServer = aServer;
        mServerConnection = buildServerConnection();
    }

    private Socket buildServerConnection() {
        return new Socket();
    }

    @Override
    public void open() {
        try {
            mServerConnection.connect(buildServerAddress());
        } catch (IOException e) {
            throw new RuntimeException("Unable to open server connection.");
        }
    }

    private SocketAddress buildServerAddress() {
        String aServerAddress = mServer.getAddress();
        int aServerPort = Protocol.Ports.CLIENT_CONNECTION;

        return new InetSocketAddress(aServerAddress, aServerPort);
    }

    @Override
    public void close() {
        try {
            mServerConnection.close();
        } catch (IOException e) {
            throw new RuntimeException("Unable to close server connection.");
        }
    }

    @Override
    public InputStream buildMessagesStream() {
        try {
            return mServerConnection.getInputStream();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open messages stream.");
        }
    }

    @Override
    public OutputStream buildCommandsStream() {
        try {
            return mServerConnection.getOutputStream();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open commands stream.");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
