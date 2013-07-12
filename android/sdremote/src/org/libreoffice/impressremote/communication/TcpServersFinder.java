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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;

public class TcpServersFinder implements ServersFinder, Runnable {
    private static final int SEARCH_DELAY_IN_SECONDS = 10;
    private static final int BLOCKING_TIMEOUT_IN_MILLISECONDS = 1000 * 10;

    private static final int SEARCH_RESULT_BUFFER_SIZE = 1024;

    private final Context mContext;

    private final Map<String, Server> mServers;

    private DatagramSocket mSearchSocket;
    private ScheduledExecutorService mSearchService;

    public TcpServersFinder(Context aContext) {
        mContext = aContext;

        mServers = new HashMap<String, Server>();

        mSearchService = Executors.newSingleThreadScheduledExecutor();
    }

    @Override
    public void startSearch() {
        stopSearch();

        mSearchService = Executors.newSingleThreadScheduledExecutor();
        mSearchService.scheduleAtFixedRate(this, 0, SEARCH_DELAY_IN_SECONDS, TimeUnit.SECONDS);
    }

    @Override
    public void run() {
        setUpSearchSocket();

        sendSearchCommand();

        listenForSearchResults();

        tearDownSearchSocket();
    }

    private void setUpSearchSocket() {
        try {
            mSearchSocket = new DatagramSocket();
            mSearchSocket.setSoTimeout(BLOCKING_TIMEOUT_IN_MILLISECONDS);
        } catch (SocketException e) {
            throw new RuntimeException("Unable to open search socket.");
        }
    }

    private void sendSearchCommand() {
        try {
            mSearchSocket.send(buildSearchPacket());
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

    private void listenForSearchResults() {
        DatagramPacket aSearchResultPacket = buildSearchResultPacket();

        String aSearchResult = receiveSearchResult(aSearchResultPacket);

        if (TextUtils.isEmpty(aSearchResult)) {
            return;
        }

        Scanner aSearchResultScanner = new Scanner(aSearchResult);

        String aMessage = aSearchResultScanner.nextLine();

        if (!Protocol.Messages.ADVERTISE.equals(aMessage)) {
            return;
        }

        String aFoundServerHostname = aSearchResultScanner.nextLine();

        Server aFoundServer = new Server(Server.Protocol.TCP,
            aSearchResultPacket.getAddress().getHostAddress(),
            aFoundServerHostname);

        addServer(aFoundServer);

        callUpdatingServersList();
    }

    private DatagramPacket buildSearchResultPacket() {
        byte[] aSearchResultBuffer = new byte[SEARCH_RESULT_BUFFER_SIZE];

        return new DatagramPacket(
            aSearchResultBuffer, aSearchResultBuffer.length);
    }

    private String receiveSearchResult(DatagramPacket aSearchResultPacket) {
        try {
            mSearchSocket.receive(aSearchResultPacket);

            return new String(aSearchResultPacket.getData(), Protocol.CHARSET);
        } catch (SocketTimeoutException e) {
            return "";
        } catch (IOException e) {
            throw new RuntimeException("Unable to receive search result.");
        }
    }

    private void addServer(Server aServer) {
        mServers.put(aServer.getAddress(), aServer);
    }

    private void callUpdatingServersList() {
        Intent aServersListUpdatedIntent = new Intent(
            CommunicationService.MSG_SERVERLIST_CHANGED);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aServersListUpdatedIntent);
    }

    private void tearDownSearchSocket() {
        mSearchSocket.close();
    }

    @Override
    public void stopSearch() {
        mSearchService.shutdown();
    }

    @Override
    public List<Server> getServers() {
        return new ArrayList<Server>(mServers.values());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
