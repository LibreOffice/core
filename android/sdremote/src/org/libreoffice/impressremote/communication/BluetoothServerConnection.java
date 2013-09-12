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
import java.util.UUID;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import org.libreoffice.impressremote.util.BluetoothOperator;

class BluetoothServerConnection implements ServerConnection {
    // Standard UUID for the Serial Port Profile.
    // https://www.bluetooth.org/en-us/specification/assigned-numbers-overview/service-discovery
    private static final String STANDARD_SPP_UUID = "00001101-0000-1000-8000-00805F9B34FB";

    private final BluetoothSocket mServerConnection;

    public BluetoothServerConnection(Server aServer) {
        mServerConnection = buildServerConnection(aServer);
    }

    private BluetoothSocket buildServerConnection(Server aServer) {
        try {
            BluetoothDevice aBluetoothServer = BluetoothOperator.getAdapter()
                .getRemoteDevice(aServer.getAddress());

            return aBluetoothServer.createRfcommSocketToServiceRecord(
                UUID.fromString(STANDARD_SPP_UUID));
        } catch (IOException e) {
            throw new RuntimeException("Unable to create server connection.");
        }
    }

    @Override
    public void open() {
        try {
            mServerConnection.connect();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open server connection.");
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
