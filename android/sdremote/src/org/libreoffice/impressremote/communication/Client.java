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

import android.content.Context;

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

    public abstract void closeConnection();

    private Receiver mReceiver;

    protected Context mContext;

    public Client(Context aContext) {
        mContext = aContext;
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

    private void listen() {
        try {
            while (true) {
                ArrayList<String> aList = new ArrayList<String>();
                String aTemp;
                // read until empty line
                while ((aTemp = mReader.readLine()).length() != 0) {
                    aList.add(aTemp);
                }
                mReceiver.parseCommand(aList);
            }
        } catch (UnsupportedEncodingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e1) {
            // TODO stream couldn't be opened.
            e1.printStackTrace();
        }

    }

    public String getPin() {
        return mPin;
    }

    /**
     * Send a valid JSON string to the server.
     *
     * @param command
     *            Must be a valid JSON string.
     */
    public void sendCommand(String command) {
        try {
            mOutputStream.write(command.getBytes(CHARSET));
        } catch (UnsupportedEncodingException e) {
            throw new Error("Specified network encoding [" + CHARSET
                            + " not available.");
        } catch (IOException e) {
            // TODO Notify that stream has closed.
        }
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */