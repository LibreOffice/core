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
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import android.content.Intent;

/**
 * Generic Client for the remote control. To implement a Client for a specific
 * transport medium you must provide input and output streams (
 * <code>mInputStream</code> and <code>mOutputStream</code> before calling any
 * methods.
 *
 * @author Andrzej J.R. Hunt
 */
public abstract class Client {

    protected static final String CHARSET = "UTF-8";

    protected InputStream mInputStream;
    protected BufferedReader mReader;
    protected OutputStream mOutputStream;
    protected String mPin = "";

    private static Client latestInstance = null;

    public abstract void closeConnection();

    private Receiver mReceiver;

    protected Server mServer;

    protected CommunicationService mCommunicationService;

    protected Client(Server aServer, CommunicationService aCommunicationService) {
        mServer = aServer;
        mCommunicationService = aCommunicationService;
        latestInstance = this;
    }

    public void setReceiver(Receiver aReceiver) {
        mReceiver = aReceiver;
    }

    protected void startListening() {

        Thread t = new Thread() {
            public void run() {
                listen();
            };

        };
        t.start();
    }

    private final void listen() {
        try {
            while (true) {
                ArrayList<String> aList = new ArrayList<String>();
                String aTemp;
                // read until empty line
                while ((aTemp = mReader.readLine()) != null
                                && aTemp.length() != 0) {
                    aList.add(aTemp);
                }
                if (aTemp == null) {
                    mCommunicationService.connectTo(mServer);
                    Intent aIntent = new Intent(
                                    mCommunicationService
                                                    .getApplicationContext(),
                                    ReconnectionActivity.class);
                    aIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mCommunicationService.getApplicationContext()
                                    .startActivity(aIntent);
                    return;
                }
                mReceiver.parseCommand(aList);
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (IOException e1) {
            // TODO stream couldn't be opened.
            e1.printStackTrace();
        } finally {
            latestInstance = null;
            onDisconnect();
        }

    }

    public static String getPin() {
        if (latestInstance != null) {
            return latestInstance.mPin;
        } else {
            return "";
        }
    }

    /**
     * Send a valid command to the Server.
     */
    public void sendCommand(String command) {
        try {
            mOutputStream.write(command.getBytes(CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new Error("Specified network encoding [" + CHARSET
                            + " not available.");
        } catch (IOException e) {
            // I.e. connection closed. This will be dealt with by the listening
            // loop.
        }
    }

    /**
     * Called after the Client disconnects. Can be extended to allow for
     * cleaning up bluetooth properties etc.
     */
    protected void onDisconnect() {
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */