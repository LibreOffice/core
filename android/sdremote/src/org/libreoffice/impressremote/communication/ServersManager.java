/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.ArrayList;
import java.util.HashSet;

import java.util.List;
import java.util.Map;
import java.util.Set;

import android.content.Context;

import org.libreoffice.impressremote.util.Preferences;

class ServersManager {
    private final Context mContext;

    private final ServersFinder mBluetoothServersFinder;
    private final ServersFinder mTcpServersFinder;

    private final Set<Server> mBlacklistedServers;

    public ServersManager(Context aContext) {
        mContext = aContext;

        mBluetoothServersFinder = new BluetoothServersFinder(mContext);
        mTcpServersFinder = new TcpServersFinder(mContext);

        mBlacklistedServers = new HashSet<Server>();
    }

    public void startServersSearch() {
        mBluetoothServersFinder.startSearch();
        mTcpServersFinder.startSearch();
    }

    public void stopServersSearch() {
        mBluetoothServersFinder.stopSearch();
        mTcpServersFinder.stopSearch();
    }

    public List<Server> getServers() {
        List<Server> aServers = new ArrayList<Server>();

        aServers.addAll(mBluetoothServersFinder.getServers());
        aServers.addAll(mTcpServersFinder.getServers());
        aServers.addAll(getManualAddedTcpServers());

        return filterBlacklistedServers(aServers);
    }

    private List<Server> getManualAddedTcpServers() {
        Map<String, ?> aServersEntries = Preferences
            .getAll(mContext, Preferences.Locations.STORED_SERVERS);

        return buildTcpServers(aServersEntries);
    }

    private List<Server> buildTcpServers(Map<String, ?> aServersEntries) {
        List<Server> aServers = new ArrayList<Server>();

        for (String aServerAddress : aServersEntries.keySet()) {
            String aServerName = (String) aServersEntries.get(aServerAddress);

            aServers.add(Server.newTcpInstance(aServerAddress, aServerName));
        }

        return aServers;
    }

    private List<Server> filterBlacklistedServers(List<Server> aServers) {
        List<Server> aFilteredServers = new ArrayList<Server>();

        for (Server aServer : aServers) {
            if (mBlacklistedServers.contains(aServer)) {
                continue;
            }

            aFilteredServers.add(aServer);
        }

        return aFilteredServers;
    }

    public void addTcpServer(String aAddress, String aName) {
        Preferences.set(mContext, Preferences.Locations.STORED_SERVERS,
            aAddress, aName);
    }

    public void removeServer(Server aServer) {
        if (getServersAddresses(getManualAddedTcpServers()).contains(aServer.getAddress())) {
            removeManualAddedServer(aServer);

            return;
        }

        blacklistServer(aServer);
    }

    private List<String> getServersAddresses(List<Server> aServers) {
        List<String> aServersAddresses = new ArrayList<String>();

        for (Server aServer : aServers) {
            aServersAddresses.add(aServer.getAddress());
        }

        return aServersAddresses;
    }

    private void removeManualAddedServer(Server aServer) {
        Preferences.remove(mContext, Preferences.Locations.STORED_SERVERS,
            aServer.getAddress());
    }

    private void blacklistServer(Server aServer) {
        mBlacklistedServers.add(aServer);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
