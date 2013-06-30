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
import java.net.Socket;
import java.net.UnknownHostException;

public class TcpServerConnection implements ServerConnection {
    private final Socket mServerConnection;

    public TcpServerConnection(Server aServer) {
        mServerConnection = buildServerConnection(aServer);
    }

    private Socket buildServerConnection(Server aServer) {
        try {
            String aServerAddress = aServer.getAddress();
            int aServerPort = Protocol.Ports.CLIENT_CONNECTION;

            return new Socket(aServerAddress, aServerPort);
        } catch (UnknownHostException e) {
            throw new RuntimeException("Unable to connect to unknown host.");
        } catch (IOException e) {
            throw new RuntimeException("Unable to connect to host.");
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

    @Override
    public void close() {
        try {
            mServerConnection.close();
        } catch (IOException e) {
            throw new RuntimeException("Unable to close server connection.");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
