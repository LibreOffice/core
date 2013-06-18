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
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.text.TextUtils;

public abstract class Client implements Runnable {
    protected final BufferedReader mMessagesReader;
    protected final OutputStream mCommandsStream;

    protected String mPin = "";
    protected String mName = "";

    private static Client latestInstance = null;

    protected final Server mServer;
    protected final CommunicationService mCommunicationService;
    protected final Receiver mReceiver;

    protected Client(Server aServer, CommunicationService aCommunicationService, Receiver aReceiver) {
        mServer = aServer;
        mName = aServer.getName();
        mCommunicationService = aCommunicationService;
        mReceiver = aReceiver;
        latestInstance = this;

        setUpServerConnection();

        mMessagesReader = buildMessagesReader(buildMessagesStream());
        mCommandsStream = buildCommandsStream();
    }

    protected abstract void setUpServerConnection();

    private BufferedReader buildMessagesReader(InputStream aMessagesStream) {
        try {
            return new BufferedReader(
                new InputStreamReader(aMessagesStream, Protocol.CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("Unable to create messages reader.");
        }
    }

    protected abstract InputStream buildMessagesStream();

    protected abstract OutputStream buildCommandsStream();

    public static String getPin() {
        if (latestInstance == null) {
            return "";
        }

        return latestInstance.mName;
    }

    public static String getName() {
        if (latestInstance == null) {
            return "";
        }

        return latestInstance.mName;
    }

    protected void startListening() {
        Thread aListeningThread = new Thread(this);

        aListeningThread.start();
    }

    @Override
    public void run() {
        listen();
    }

    private void listen() {
        try {
            while (true) {
                List<String> aMessage = readMessage();

                if (aMessage == null) {
                    return;
                }

                mReceiver.parseCommand(aMessage);
            }
        } catch (IOException e) {
            // TODO: stream couldn't be opened
            e.printStackTrace();
        } finally {
            onDisconnect();
        }
    }

    private List<String> readMessage() throws IOException {
        List<String> aMessage = new ArrayList<String>();

        String aMessageParameter = mMessagesReader.readLine();

        while ((aMessageParameter != null) && (!TextUtils
            .isEmpty(aMessageParameter))) {
            aMessage.add(aMessageParameter);

            aMessageParameter = mMessagesReader.readLine();
        }

        if (aMessageParameter == null) {
            startReconnection();

            return null;
        }

        return aMessage;
    }

    private void startReconnection() {
        Intent aReconnectionIntent = new Intent(
            mCommunicationService.getApplicationContext(),
            ReconnectionActivity.class);
        aReconnectionIntent.putExtra("server", mServer);
        aReconnectionIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        mCommunicationService.getApplicationContext()
            .startActivity(aReconnectionIntent);
    }

    /**
     * Called after the Client disconnects. Can be extended to allow for
     * cleaning up bluetooth properties etc.
     */
    protected void onDisconnect() {
    }

    /**
     * Send a valid command to the Server.
     */
    public void sendCommand(String aCommand) {
        try {
            mCommandsStream.write(aCommand.getBytes(Protocol.CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException("UTF-8 must be used for commands.");
        } catch (IOException e) {
            // I.e. connection closed. This will be dealt with by the listening
            // loop.
        }
    }

    public abstract void closeConnection();

    public abstract void validating() throws IOException;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
