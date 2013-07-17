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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.communication.Server.Protocol;

public class BluetoothServersFinder extends BroadcastReceiver implements ServersFinder, Runnable {
    private static final int SEARCH_DELAY_IN_MILLISECONDS = 1000 * 10;

    private final Context mContext;

    private final Map<String, Server> mServers;

    public BluetoothServersFinder(Context aContext) {
        mContext = aContext;

        mServers = new HashMap<String, Server>();
    }

    @Override
    public void startSearch() {
        if (!isBluetoothAvailable()) {
            return;
        }

        if (BluetoothAdapter.getDefaultAdapter().isDiscovering()) {
            return;
        }

        setUpSearchResultsReceiver();

        BluetoothAdapter.getDefaultAdapter().startDiscovery();
    }

    private boolean isBluetoothAvailable() {
        return BluetoothAdapter.getDefaultAdapter() != null;
    }

    private void setUpSearchResultsReceiver() {
        IntentFilter aSearchResultsFilter = new IntentFilter(
            BluetoothDevice.ACTION_FOUND);
        aSearchResultsFilter
            .addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);

        mContext.registerReceiver(this, aSearchResultsFilter);
    }

    @Override
    public void onReceive(Context aContext, Intent aIntent) {
        if (BluetoothDevice.ACTION_FOUND.equals(aIntent.getAction())) {
            BluetoothDevice aBluetoothDevice = aIntent
                .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            addServer(buildServer(aBluetoothDevice));

            callUpdatingServersList();

            return;
        }

        if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED
            .equals(aIntent.getAction())) {

            startDiscoveryDelayed();
        }
    }

    private void addServer(Server aServer) {
        mServers.put(aServer.getAddress(), aServer);
    }

    private Server buildServer(BluetoothDevice aBluetoothDevice) {
        String aServerAddress = aBluetoothDevice.getAddress();
        String aServerName = aBluetoothDevice.getName();

        return new Server(Protocol.BLUETOOTH, aServerAddress, aServerName);
    }

    private void callUpdatingServersList() {
        Intent aIntent = Intents.buildServersListChangedIntent();
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(aIntent);
    }

    private void startDiscoveryDelayed() {
        // Start discovery again after a small delay.
        // but check whether device is on in case the user manually
        // disabled bluetooth

        if (!BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            return;
        }

        Handler aHandler = new Handler();
        aHandler.postDelayed(this, SEARCH_DELAY_IN_MILLISECONDS);
    }

    @Override
    public void run() {
        BluetoothAdapter.getDefaultAdapter().startDiscovery();
    }

    @Override
    public void stopSearch() {
        if (!isBluetoothAvailable()) {
            return;
        }

        tearDownSearchResultsReceiver();

        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
    }

    private void tearDownSearchResultsReceiver() {
        try {
            mContext.unregisterReceiver(this);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }
    }

    @Override
    public List<Server> getServers() {
        return new ArrayList<Server>(mServers.values());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
