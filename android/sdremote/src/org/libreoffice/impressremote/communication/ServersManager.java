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
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;

import java.util.List;
import java.util.Map;
import java.util.Set;

import android.content.Context;

import org.libreoffice.impressremote.util.Preferences;

class ServersManager implements Comparator<Server> {
    private final ServersFinder mBluetoothServersFinder;
    private final ServersFinder mTcpServersFinder;

    private final Set<Server> mBlacklistedServers;
    private final Preferences mSavedServersPreferences;

    public ServersManager(Context aContext) {
        mBluetoothServersFinder = new BluetoothServersFinder(aContext);
        mTcpServersFinder = new TcpServersFinder(aContext);

        mBlacklistedServers = new HashSet<Server>();
        mSavedServersPreferences = Preferences.getSavedServersInstance(aContext);
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

        aServers = filterBlacklistedServers(aServers);

        Collections.sort(aServers, this);

        return aServers;
    }

    private List<Server> getManualAddedTcpServers() {
        Map<String, ?> aServersEntries = mSavedServersPreferences.getAll();

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

    @Override
    public int compare(Server aFirstServer, Server aSecondServer) {
        String aFirstServerName = aFirstServer.getName();
        String aSecondServerName = aSecondServer.getName();

        return aFirstServerName.compareToIgnoreCase(aSecondServerName);
    }

    public void addTcpServer(String aAddress, String aName) {
        mSavedServersPreferences.set(aAddress, aName);
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
        mSavedServersPreferences.remove(aServer.getAddress());
    }

    private void blacklistServer(Server aServer) {
        mBlacklistedServers.add(aServer);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
