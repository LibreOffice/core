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
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Random;

import org.libreoffice.impressremote.Globals;

import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

/**
 * Standard Network client. Connects to a server using Sockets.
 */
public class NetworkClient extends Client {

    private static final int PORT = 1599;

    private Socket mSocket;
    private Intent mIntent;
    private String mPin;
    private Server mServer;

    public NetworkClient(Server aServer,
            CommunicationService aCommunicationService, Receiver aReceiver)
            throws UnknownHostException, IOException {
        super(aServer, aCommunicationService, aReceiver);
        mServer = aServer;
        mSocket = new Socket(mServer.getAddress(), PORT);
        mInputStream = mSocket.getInputStream();
        mReader = new BufferedReader(new InputStreamReader(mInputStream,
                CHARSET));
        mOutputStream = mSocket.getOutputStream();

        // Pairing.
        mPin = setupPin(mServer);
        mIntent = new Intent(CommunicationService.MSG_PAIRING_STARTED);
        mIntent.putExtra("PIN", mPin);
        LocalBroadcastManager.getInstance(mCommunicationService).sendBroadcast(
                mIntent);
        // Send out
        String aName = CommunicationService.getDeviceName(); // TODO: get the
                                                             // proper name
        sendCommand("LO_SERVER_CLIENT_PAIR\n" + aName + "\n" + mPin + "\n\n");
    }

    private String setupPin(Server aServer) {
        // Get settings
        SharedPreferences aPreferences = mCommunicationService
                .getSharedPreferences("sdremote_authorisedremotes",
                        android.content.Context.MODE_PRIVATE);
        if (aPreferences.contains(aServer.getName())) {
            return aPreferences.getString(aServer.getName(), "");
        } else {
            String aPin = generatePin();

            Editor aEdit = aPreferences.edit();
            aEdit.putString(aServer.getName(), aPin);
            aEdit.commit();

            return aPin;
        }

    }

    private String generatePin() {
        Random aRandom = new Random();
        String aPin = "" + (aRandom.nextInt(9000) + 1000);
        while (aPin.length() < 4) {
            aPin = "0" + aPin; // Add leading zeros if necessary
        }
        return aPin;
    }

    @Override
    public void closeConnection() {
        try {
            if (mSocket != null)
                mSocket.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    @Override
    public void validating() throws IOException {

        // Wait until we get the appropriate string back...
        String aTemp = mReader.readLine();

        if (aTemp == null) {
            throw new IOException(
                    "End of stream reached before any data received.");
        }

        while (!aTemp.equals("LO_SERVER_SERVER_PAIRED")) {
            if (aTemp.equals("LO_SERVER_VALIDATING_PIN")) {
                // Broadcast that we need a pin screen.
                mIntent = new Intent(
                        CommunicationService.STATUS_PAIRING_PINVALIDATION);
                mIntent.putExtra("PIN", mPin);
                mIntent.putExtra("SERVERNAME", mServer.getName());
                LocalBroadcastManager.getInstance(mCommunicationService)
                        .sendBroadcast(mIntent);
                while (mReader.readLine().length() != 0) {
                    // Read off empty lines
                }
                aTemp = mReader.readLine();
            } else {
                return;
            }
        }

        mIntent = new Intent(CommunicationService.MSG_PAIRING_SUCCESSFUL);
        LocalBroadcastManager.getInstance(mCommunicationService).sendBroadcast(
                mIntent);

        while (mReader.readLine().length() != 0) {
            // Get rid of extra lines
            Log.i(Globals.TAG, "NetworkClient: extra line");
        }
        Log.i(Globals.TAG, "NetworkClient: calling startListening");
        startListening();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
