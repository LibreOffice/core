/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 */
public class BluetoothClient extends Client {

    private boolean mBluetoothWasEnabled;
    private BluetoothAdapter mAdapter;

    public BluetoothClient(Server aServer,
                    CommunicationService aCommunicationService) {
        super(aServer, aCommunicationService);
        try {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
            mBluetoothWasEnabled = mAdapter.isEnabled();
            if (!mBluetoothWasEnabled) {
                mAdapter.enable();
            }

            BluetoothDevice aDevice = mAdapter.getRemoteDevice(aServer
                            .getAddress());
            mAdapter.cancelDiscovery();
            BluetoothSocket aSocket = aDevice
                            .createRfcommSocketToServiceRecord(UUID
                                            .fromString("00001101-0000-1000-8000-00805F9B34FB"));
            aSocket.connect();
            //            mSocket = aSocket;
            System.out.println("Connected");

            mInputStream = aSocket.getInputStream();
            mReader = new BufferedReader(new InputStreamReader(mInputStream,
                            CHARSET));
            mOutputStream = aSocket.getOutputStream();

            //            mOutputStream.write(20);
            //            mOutputStream.write(20);
            //            mOutputStream.write(20);
            //            mOutputStream.flush();
            //            System.out.println("reading");
            //            while (true) {
            //                System.out.println(mInputStream.read());
            //            }
            String aTemp = mReader.readLine();
            System.out.println("SF:waited");
            if (!aTemp.equals("LO_SERVER_SERVER_PAIRED")) {
                return;
            }
            while (mReader.readLine().length() != 0) {
                // Get rid of extra lines
            }
            Intent aIntent = new Intent(
                            CommunicationService.MSG_PAIRING_SUCCESSFUL);
            LocalBroadcastManager.getInstance(mCommunicationService)
                            .sendBroadcast(aIntent);
            startListening();
            // Pairing.
            //            Random aRandom = new Random();
            //            String aPin = "" + (aRandom.nextInt(9000) + 1000);
            //            while (aPin.length() < 4) {
            //                aPin = "0" + aPin; // Add leading zeros if necessary
            //            }
            //            Intent aIntent = new Intent(
            //                            CommunicationService.MSG_PAIRING_STARTED);
            //            aIntent.putExtra("PIN", aPin);
            //            mPin = aPin;
            //            LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
            //            // Send out
            //            String aName = CommunicationService.getDeviceName(); // TODO: get the proper name
            //            sendCommand("LO_SERVER_CLIENT_PAIR\n" + aName + "\n" + aPin
            //                            + "\n\n");
            //
            //            // Wait until we get the appropriate string back...
            //            System.out.println("SF:waiting");
            //            String aTemp = mReader.readLine();
            //            System.out.println("SF:waited");
            //            if (!aTemp.equals("LO_SERVER_SERVER_PAIRED")) {
            //                return;
            //            } else {
            //
            //            }
            //            while (mReader.readLine().length() != 0) {
            //                // Get rid of extra lines
            //                System.out.println("SF: empty line");
            //            }
            //            System.out.println("SD: empty");
            //            startListening();

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    @Override
    public void closeConnection() {
        //        try {
        //            if (mSocket != null)
        //                mSocket.close();
        //        } catch (IOException e) {
        //            // TODO Auto-generated catch block
        //            e.printStackTrace();
        //        }
    }

    protected void onDisconnect() {
        if (!mBluetoothWasEnabled) {
            mAdapter.disable();
        }
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */