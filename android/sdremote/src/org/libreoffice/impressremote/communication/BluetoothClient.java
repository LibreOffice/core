/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.net.Socket;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 */
public class BluetoothClient extends Client {

    private static final int PORT = 5;

    private Socket mSocket;

    public BluetoothClient(String bluetoothAddress, Context aContext) {
        super(aContext);
        try {
            BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
            BluetoothDevice aDevice = aAdapter
                            .getRemoteDevice(bluetoothAddress);
            BluetoothSocket aSocket = aDevice
                            .createRfcommSocketToServiceRecord(UUID
                                            .fromString("00001101-0000-1000-8000-00805f9b34fb"));
            aSocket.connect();

        } catch (Exception e) {
            e.printStackTrace();
        }
        //            BluetoothSocket aSocket = new BluetoothClient(bluetoothAddress, aContext)
        //                                    .createRfcommSocketToServiceRecord(UUID
        //                                                    .fromString("00001101-0000-1000-8000-00805F9B34F
        //        }
        //        try {
        //            mSocket = new Socket(ipAddress, PORT);
        //            mInputStream = mSocket.getInputStream();
        //            mReader = new BufferedReader(new InputStreamReader(mInputStream,
        //                            CHARSET));
        //            mOutputStream = mSocket.getOutputStream();
        //            // Pairing.
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
        //                aIntent = new Intent(
        //                                CommunicationService.MSG_PAIRING_SUCCESSFUL);
        //                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
        //                                aIntent);
        //            }
        //            while (mReader.readLine().length() != 0) {
        //                // Get rid of extra lines
        //                System.out.println("SF: empty line");
        //            }
        //            System.out.println("SD: empty");
        //            startListening();
        //        } catch (UnknownHostException e) {
        //            // TODO Tell the user we have a problem
        //            e.printStackTrace();
        //        } catch (IOException e) {
        //            // TODO As above
        //            e.printStackTrace();
        //        }

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

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */