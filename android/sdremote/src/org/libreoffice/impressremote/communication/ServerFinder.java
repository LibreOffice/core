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
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

public class ServerFinder implements Runnable {
    private final Context mContext;

    private DatagramSocket mSocket = null;
    private Thread mListenerThread;
    private boolean mFinishRequested;

    private final Map<String, Server> mServers;

    public ServerFinder(Context aContext) {
        mContext = aContext;

        mSocket = null;
        mListenerThread = null;
        mFinishRequested = false;

        mServers = new HashMap<String, Server>();
    }

    public void startSearch() {
        if (mSocket != null) {
            return;
        }

        mFinishRequested = false;

        if (mListenerThread == null) {
            mListenerThread = new Thread(this);
        }

        mListenerThread.start();
    }

    @Override
    public void run() {
        addLocalServerForEmulator();

        long aStartSearchTime = 0;

        setUpSearchSocket();

        while (!mFinishRequested) {
            if (System
                .currentTimeMillis() - aStartSearchTime > 1000 * 15) {
                sendSearchCommand();

                aStartSearchTime = System.currentTimeMillis();

                removeStaleServers();
            }

            listenForServer();
        }
    }

    /**
     * Check whether we are on an emulator and add it's host to the list of
     * servers if so (although we do not know whether libo is running on
     * the host).
     */
    private void addLocalServerForEmulator() {
        if (!isLocalServerForEmulatorReachable()) {
            return;
        }

        Server aServer = new Server(Server.Protocol.NETWORK,
            Protocol.Addresses.SERVER_LOCAL_FOR_EMULATOR, "Android Emulator",
            0);

        mServers.put(aServer.getAddress(), aServer);

        callUpdatingServersList();
    }

    private boolean isLocalServerForEmulatorReachable() {
        try {
            InetAddress aLocalServerAddress = InetAddress
                .getByName(Protocol.Addresses.SERVER_LOCAL_FOR_EMULATOR);

            return aLocalServerAddress.isReachable(100);
        } catch (UnknownHostException e) {
            return false;
        } catch (IOException e) {
            return false;
        }
    }

    private void callUpdatingServersList() {
        Intent aIntent = new Intent(
            CommunicationService.MSG_SERVERLIST_CHANGED);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
    }

    private void setUpSearchSocket() {
        try {
            mSocket = new DatagramSocket();
            mSocket.setSoTimeout(1000 * 10);
        } catch (SocketException e) {
            throw new RuntimeException("Unable to open search socket.");
        }
    }

    private void sendSearchCommand() {
        try {
            mSocket.send(buildSearchPacket());
        } catch (IOException e) {
            throw new RuntimeException("Unable to send search packet.");
        }
    }

    private DatagramPacket buildSearchPacket() {
        try {
            String aSearchCommand = Protocol.Commands
                .prepareCommand(Protocol.Commands.SEARCH_SERVERS);

            DatagramPacket aSearchPacket = new DatagramPacket(
                aSearchCommand.getBytes(), aSearchCommand.length());
            aSearchPacket.setAddress(
                InetAddress.getByName(Protocol.Addresses.SERVER_SEARCH));
            aSearchPacket.setPort(Protocol.Ports.SERVER_SEARCH);

            return aSearchPacket;
        } catch (UnknownHostException e) {
            throw new RuntimeException("Unable to find address to search.");
        }
    }

    private void removeStaleServers() {
        for (Server aServer : mServers.values()) {
            if (aServer.mNoTimeout) {
                continue;
            }

            if (System.currentTimeMillis()
                - aServer
                .getTimeDiscovered() > 60 * 1000) {
                mServers
                    .remove(aServer.getAddress());
                callUpdatingServersList();
            }
        }
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
                    aCommand = new String(aPacket.getData(), 0, i,
                        Protocol.CHARSET);
                    break;
                }
            }
            if (i == aBuffer.length || !"LOREMOTE_ADVERTISE".equals(aCommand)) {
                return;
            }
            for (int j = i + 1; j < aBuffer.length; j++) {
                if (aPacket.getData()[j] == '\n') {
                    aName = new String(aPacket.getData(), i + 1, j - (i + 1),
                        Protocol.CHARSET);
                    break;
                }
            }
            if (aName == null) {
                return;
            }
            Server aServer = new Server(Server.Protocol.NETWORK, aPacket
                .getAddress().getHostAddress(), aName,
                System.currentTimeMillis());
            mServers.put(aServer.getAddress(), aServer);

            callUpdatingServersList();
        } catch (SocketTimeoutException e) {
            // Ignore -- we want to timeout to enable checking whether we
            // should stop listening periodically
        } catch (IOException e) {
        }

    }

    public void stopSearch() {
        if (mListenerThread == null) {
            return;
        }

        mFinishRequested = true;
        mListenerThread = null;
    }

    public Collection<Server> getServers() {
        return mServers.values();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
