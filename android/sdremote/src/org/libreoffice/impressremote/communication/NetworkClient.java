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
import java.util.Random;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.Preferences;

public class NetworkClient extends Client {
    private Socket mSocket;

    private final String mPin;

    public NetworkClient(Server aServer, CommunicationService aCommunicationService, Receiver aReceiver) {
        super(aServer, aCommunicationService, aReceiver);

        mPin = loadPin();

        startPairingActivity();
        startPairing();
    }

    private String loadPin() {
        Context aContext = mCommunicationService.getApplicationContext();

        if (Preferences
            .doContain(aContext, Preferences.Locations.AUTHORIZED_REMOTES,
                mServer.getName())) {
            return Preferences
                .getString(aContext, Preferences.Locations.AUTHORIZED_REMOTES,
                    mServer.getName());
        }

        String aPin = generatePin();

        Preferences.set(aContext, Preferences.Locations.AUTHORIZED_REMOTES,
            mServer.getName(), aPin);

        return aPin;
    }

    private String generatePin() {
        return String.format("%04d", generatePinNumber());
    }

    private int generatePinNumber() {
        Random aRandomGenerator = new Random();

        int aMaximumPin = (int) Math.pow(10, Protocol.PIN_NUMBERS_COUNT) - 1;

        return aRandomGenerator.nextInt(aMaximumPin);
    }

    private void startPairingActivity() {
        Intent aPairingIntent = new Intent(
            CommunicationService.MSG_PAIRING_STARTED);
        aPairingIntent.putExtra("PIN", mPin);

        LocalBroadcastManager.getInstance(mCommunicationService)
            .sendBroadcast(aPairingIntent);
    }

    private void startPairing() {
        // TODO: get the proper name
        String aPhoneName = CommunicationService.getDeviceName();

        sendCommand(Protocol.Commands
            .prepareCommand(Protocol.Commands.PAIR, aPhoneName, mPin));
    }

    @Override
    protected void setUpServerConnection() {
        mSocket = buildServerConnection();
    }

    private Socket buildServerConnection() {
        try {
            return new Socket(mServer.getAddress(),
                Protocol.Ports.CLIENT_CONNECTION);
        } catch (UnknownHostException e) {
            throw new RuntimeException("Unable to connect to unknown host.");
        } catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException("Unable to connect to host.");
        }
    }

    @Override
    protected InputStream buildMessagesStream() {
        try {
            return mSocket.getInputStream();
        } catch (IOException e) {
            throw new RuntimeException("Unable to open messages stream.");
        }
    }

    @Override
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
            throw new RuntimeException("Unable to close network socket.");
        }
    }

    @Override
    public void validating() throws IOException {
        String aMessage = mMessagesReader.readLine();

        if (aMessage == null) {
            throw new RuntimeException(
                "End of stream reached before any data received.");
        }

        while (!aMessage.equals(Protocol.Messages.PAIRED)) {
            if (aMessage.equals(Protocol.Messages.VALIDATING)) {
                startPinValidation();

                while (mMessagesReader.readLine().length() != 0) {
                    // Read off empty lines
                }

                aMessage = mMessagesReader.readLine();
            } else {
                return;
            }
        }

        callSuccessfulPairing();

        while (mMessagesReader.readLine().length() != 0) {
            // Get rid of extra lines
        }

        startListening();
    }

    private void startPinValidation() {
        Intent aPairingIntent = new Intent(
            CommunicationService.STATUS_PAIRING_PINVALIDATION);
        aPairingIntent.putExtra("PIN", mPin);
        aPairingIntent.putExtra("SERVERNAME", mServer.getName());

        LocalBroadcastManager.getInstance(mCommunicationService)
            .sendBroadcast(aPairingIntent);
    }

    private void callSuccessfulPairing() {
        Intent aSuccessfulPairingIntent = new Intent(
            CommunicationService.MSG_PAIRING_SUCCESSFUL);

        LocalBroadcastManager.getInstance(mCommunicationService)
            .sendBroadcast(aSuccessfulPairingIntent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
