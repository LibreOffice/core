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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

public class BluetoothClient extends Client {
    // Standard UUID for the Serial Port Profile.
    // https://www.bluetooth.org/en-us/specification/assigned-numbers-overview/service-discovery
    private static final String STANDARD_SPP_UUID = "00001101-0000-1000-8000-00805F9B34FB";

    private final boolean mBluetoothWasEnabled;

    private BluetoothSocket mSocket;

    public BluetoothClient(Server aServer, CommunicationService aCommunicationService, Receiver aReceiver, boolean aBluetoothWasEnabled) {
        super(aServer, aCommunicationService, aReceiver);

        mBluetoothWasEnabled = aBluetoothWasEnabled;

        if (!mBluetoothWasEnabled) {
            BluetoothAdapter.getDefaultAdapter().enable();
        }
    }

    @Override
    protected void setUpServerConnection() {
        mSocket = buildServerConnection();
    }

    private BluetoothSocket buildServerConnection() {
        try {
            BluetoothDevice aBluetoothServer = BluetoothAdapter
                .getDefaultAdapter()
                .getRemoteDevice(mServer.getAddress());

            BluetoothAdapter.getDefaultAdapter().cancelDiscovery();

            BluetoothSocket aSocket = aBluetoothServer
                .createRfcommSocketToServiceRecord(
                    UUID.fromString(STANDARD_SPP_UUID));

            aSocket.connect();

            return aSocket;
        } catch (IOException e) {
            throw new RuntimeException("Unable to connect to Bluetooth host.");
        }
    }

    protected InputStream buildMessagesStream() {
        try {
            return mSocket.getInputStream();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open messages stream.");
        }
    }

    protected OutputStream buildCommandsStream() {
        try {
            return mSocket.getOutputStream();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open commands stream.");
        }
    }

    @Override
    public void closeConnection() {
        try {
            mSocket.close();
        } catch (IOException e) {
            throw new RuntimeException("Unable to close Bluetooth socket.");
        }
    }

    protected void onDisconnect() {
        if (!mBluetoothWasEnabled) {
            BluetoothAdapter.getDefaultAdapter().disable();
        }
    }

    @Override
    public void validating() throws IOException {
        String aMessage = mMessagesReader.readLine();

        if (!aMessage.equals(Protocol.Messages.PAIRED)) {
            return;
        }

        while (mMessagesReader.readLine().length() != 0) {
            // Get rid of extra lines
        }

        callSuccessfulPairing();

        startListening();
    }

    private void callSuccessfulPairing() {
        Intent aSuccessfulPairingIntent = new Intent(
            CommunicationService.MSG_PAIRING_SUCCESSFUL);

        LocalBroadcastManager.getInstance(mCommunicationService)
            .sendBroadcast(aSuccessfulPairingIntent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
