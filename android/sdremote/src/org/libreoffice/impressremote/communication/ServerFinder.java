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
import java.io.UnsupportedEncodingException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.Collection;
import java.util.HashMap;

import org.libreoffice.impressremote.Globals;
import org.libreoffice.impressremote.communication.Server.Protocol;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

public class ServerFinder {

    private Context mContext;

    private static final int PORT = 1598;
    private static final String GROUPADDRESS = "239.0.0.1";

    private static final String CHARSET = "UTF-8";

    private static final long SEARCH_INTERVAL = 1000 * 15;

    private DatagramSocket mSocket = null;

    private Thread mListenerThread = null;

    private boolean mFinishRequested = false;

    private HashMap<String, Server> mServerList = new HashMap<String, Server>();

    public ServerFinder(Context aContext) {
        mContext = aContext;
    }

    private void listenForServer() {
        byte[] aBuffer = new byte[500];
        DatagramPacket aPacket = new DatagramPacket(aBuffer, aBuffer.length);

        try {
            String aCommand = null;
            String aName = null;
            mSocket.receive(aPacket);
            int i;
            for (i = 0; i < aBuffer.length; i++) {
                if (aPacket.getData()[i] == '\n') {
                    aCommand = new String(aPacket.getData(), 0, i, CHARSET);
                    break;
                }
            }
            if (i == aBuffer.length || !"LOREMOTE_ADVERTISE".equals(aCommand)) {
                return;
            }
            for (int j = i + 1; j < aBuffer.length; j++) {
                if (aPacket.getData()[j] == '\n') {
                    aName = new String(aPacket.getData(), i + 1, j - (i + 1),
                                    CHARSET);
                    break;
                }
            }
            if (aName == null) {
                return;
            }
            Server aServer = new Server(Server.Protocol.NETWORK, aPacket
                            .getAddress().getHostAddress(), aName,
                            System.currentTimeMillis());
            mServerList.put(aServer.getAddress(), aServer);
            Log.i(Globals.TAG, "ServerFinder.listenForServer: contains " + aName);

            notifyActivity();
        } catch (java.net.SocketTimeoutException e) {
            // Ignore -- we want to timeout to enable checking whether we
            // should stop listening periodically
        } catch (IOException e) {
        }

    }

    public void startFinding() {
        if (mSocket != null)
            return;

        mFinishRequested = false;

        if (mListenerThread == null) {
            mListenerThread = new Thread() {
                @Override
                public void run() {
                    checkAndAddEmulator();
                    long aTime = 0;
                    try {
                        mSocket = new DatagramSocket();
                        mSocket.setSoTimeout(1000 * 10);
                        while (!mFinishRequested) {
                            if (System.currentTimeMillis() - aTime > SEARCH_INTERVAL) {
                                String aString = "LOREMOTE_SEARCH\n";
                                DatagramPacket aPacket = new DatagramPacket(
                                                aString.getBytes(CHARSET),
                                                aString.length(),
                                                InetAddress.getByName(GROUPADDRESS),
                                                PORT);
                                mSocket.send(aPacket);
                                aTime = System.currentTimeMillis();
                                // Remove stale servers
                                for (Server aServer : mServerList.values()) {
                                    if (!aServer.mNoTimeout
                                                    && System.currentTimeMillis()
                                                                    - aServer.getTimeDiscovered() > 60 * 1000) {
                                        mServerList.remove(aServer.getAddress());
                                        notifyActivity();

                                    }
                                }
                            }

                            listenForServer();
                        }
                    } catch (SocketException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (UnsupportedEncodingException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (IOException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                }
            };
            mListenerThread.start();
        }

    }

    public void stopFinding() {
        if (mListenerThread != null) {
            mFinishRequested = true;
            mListenerThread = null;
        }
    }

    /**
     * Check whether we are on an emulator and add it's host to the list of
     * servers if so (although we do not know whether libo is running on
     * the host).
     */
    private void checkAndAddEmulator() {
        try {
            if (InetAddress.getByName("10.0.2.2").isReachable(100)) {
                Log.i(Globals.TAG, "ServerFinder.checkAndAddEmulator: NulledNot, whatever that is supposed to mean");
                Server aServer = new Server(Protocol.NETWORK, "10.0.2.2",
                                "Android Emulator Host", 0);
                aServer.mNoTimeout = true;
                mServerList.put(aServer.getAddress(), aServer);
                notifyActivity();
            }
        } catch (IOException e) {
            // Probably means we can't connect -- i.e. no emulator host
        }
    }

    /**
     * Notify the activity that the server list has changed.
     */
    private void notifyActivity() {
        Intent aIntent = new Intent(CommunicationService.MSG_SERVERLIST_CHANGED);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
    }

    public Collection<Server> getServerList() {
        return mServerList.values();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
