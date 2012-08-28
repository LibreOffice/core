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
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Random;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.support.v4.content.LocalBroadcastManager;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 */
public class NetworkClient extends Client {

    private static final int PORT = 1599;

    private Socket mSocket;

    public NetworkClient(Server aServer, Context aContext) {
        super(aContext);
        try {
            mSocket = new Socket(aServer.getAddress(), PORT);
            mInputStream = mSocket.getInputStream();
            mReader = new BufferedReader(new InputStreamReader(mInputStream,
                            CHARSET));
            mOutputStream = mSocket.getOutputStream();
            // Pairing.
            String aPin = setupPin(aServer);
            Intent aIntent = new Intent(
                            CommunicationService.MSG_PAIRING_STARTED);
            aIntent.putExtra("PIN", aPin);
            mPin = aPin;
            LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
            // Send out
            String aName = CommunicationService.getDeviceName(); // TODO: get the proper name
            sendCommand("LO_SERVER_CLIENT_PAIR\n" + aName + "\n" + aPin
                            + "\n\n");

            // Wait until we get the appropriate string back...
            System.out.println("SF:waiting");
            String aTemp = mReader.readLine();
            System.out.println("SF:waited");
            if (!aTemp.equals("LO_SERVER_SERVER_PAIRED")) {
                return;
            } else {
                aIntent = new Intent(
                                CommunicationService.MSG_PAIRING_SUCCESSFUL);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aIntent);
            }
            while (mReader.readLine().length() != 0) {
                // Get rid of extra lines
                System.out.println("SF: empty line");
            }
            System.out.println("SD: empty");
            startListening();
        } catch (UnknownHostException e) {
            // TODO Tell the user we have a problem
            e.printStackTrace();
        } catch (IOException e) {
            // TODO As above
            e.printStackTrace();
        }

    }

    private String setupPin(Server aServer) {
        // Get settings
        SharedPreferences aPreferences = mContext.getSharedPreferences(
                        "sdremote_authorisedremotes",
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

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */