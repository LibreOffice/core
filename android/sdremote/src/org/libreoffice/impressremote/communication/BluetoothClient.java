/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.UUID;

import org.libreoffice.impressremote.Globals;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

/**
 * Standard Network client. Connects to a server using Sockets.
 */
public class BluetoothClient extends Client {

    private boolean mBluetoothWasEnabled;
    private BluetoothAdapter mAdapter;
    private BluetoothSocket mSocket;

    public BluetoothClient(Server aServer,
                    CommunicationService aCommunicationService,
                    Receiver aReceiver, boolean aBluetoothWasEnabled)
                    throws IOException {
        super(aServer, aCommunicationService, aReceiver);

        Log.i(Globals.TAG, "BluetoothClient(" + aServer + ")");

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mBluetoothWasEnabled = aBluetoothWasEnabled;
        if (!mBluetoothWasEnabled) {
            mAdapter.enable();
        }

        BluetoothDevice aDevice = mAdapter
                        .getRemoteDevice(aServer.getAddress());
        mAdapter.cancelDiscovery();

        // This is the "standard UUID for the Serial Port Profile".
        // I.e. the 16-bit SerialPort UUID 0x1101 inserted into the
        // Bluetooth BASE_UUID. See
        // https://www.bluetooth.org/Technical/AssignedNumbers/service_discovery.htm
        mSocket = aDevice.createRfcommSocketToServiceRecord(UUID
                        .fromString("00001101-0000-1000-8000-00805F9B34FB"));

        mSocket.connect();
        Log.i(Globals.TAG, "BluetoothClient: connected");
    }

    @Override
    public void closeConnection() {
        try {
            if (mSocket != null)
                mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    protected void onDisconnect() {
        if (!mBluetoothWasEnabled) {
            mAdapter.disable();
        }
    }

    @Override
    public void validating() throws IOException {
        // TODO Auto-generated method stub

        mInputStream = mSocket.getInputStream();
        mReader = new BufferedReader(new InputStreamReader(mInputStream,
                        CHARSET));

        mOutputStream = mSocket.getOutputStream();

        String aTemp = mReader.readLine();
        Log.i(Globals.TAG, "BluetoothClient: got line " + aTemp);
        if (!aTemp.equals("LO_SERVER_SERVER_PAIRED")) {
            return;
        }
        while (mReader.readLine().length() != 0) {
            // Get rid of extra lines
        }
        Intent aIntent = new Intent(CommunicationService.MSG_PAIRING_SUCCESSFUL);
        LocalBroadcastManager.getInstance(mCommunicationService).sendBroadcast(
                        aIntent);
        startListening();

    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
