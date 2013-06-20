/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.communication.Server.Protocol;

public class BluetoothFinder extends BroadcastReceiver {
    // TODO: add removal of cached items
    private final Context mContext;

    private final Map<String, Server> mServers;

    public BluetoothFinder(Context aContext) {
        mContext = aContext;

        mServers = new HashMap<String, Server>();
    }

    public void startSearch() {
        if (!isBluetoothAvailable()) {
            return;
        }

        BluetoothAdapter.getDefaultAdapter().startDiscovery();

        registerSearchResultsReceiver();
    }

    private boolean isBluetoothAvailable() {
        return BluetoothAdapter.getDefaultAdapter() != null;
    }

    private void registerSearchResultsReceiver() {
        IntentFilter aIntentFilter = new IntentFilter(
            BluetoothDevice.ACTION_FOUND);
        aIntentFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        aIntentFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);

        mContext.registerReceiver(this, aIntentFilter);
    }

    public void stopSearch() {
        if (!isBluetoothAvailable()) {
            return;
        }

        BluetoothAdapter.getDefaultAdapter().cancelDiscovery();

        unregisterSearchResultsReceiver();
    }

    private void unregisterSearchResultsReceiver() {
        mContext.unregisterReceiver(this);
    }

    public Collection<Server> getServers() {
        return mServers.values();
    }

    @Override
    public void onReceive(Context aContext, Intent aIntent) {
        if (aIntent.getAction().equals(BluetoothDevice.ACTION_FOUND)) {
            BluetoothDevice aBluetoothDevice = (BluetoothDevice) aIntent
                .getExtras().get(BluetoothDevice.EXTRA_DEVICE);

            if (aBluetoothDevice == null) {
                return;
            }

            createServer(aBluetoothDevice);

            callUpdatingServersList();

            return;
        }

        if (aIntent.getAction()
            .equals(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)) {
            startDiscoveryDelayed();

            return;
        }

        if (aIntent.getAction()
            .equals(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)) {
            startDiscoveryDelayed();
        }
    }

    private void createServer(BluetoothDevice aBluetoothDevice) {
        String aServerAddress = aBluetoothDevice.getAddress();
        String aServerName = aBluetoothDevice.getName();

        Server aServer = new Server(Protocol.BLUETOOTH, aServerAddress,
            aServerName, System.currentTimeMillis());
        mServers.put(aServerAddress, aServer);
    }

    private void callUpdatingServersList() {
        Intent aServersListChangedIntent = new Intent(
            CommunicationService.MSG_SERVERLIST_CHANGED);

        LocalBroadcastManager.getInstance(mContext)
            .sendBroadcast(aServersListChangedIntent);
    }

    private void startDiscoveryDelayed() {
        // Start discovery again after a small delay.
        // but check whether device is on in case the user manually
        // disabled bluetooth

        if (!BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            return;
        }

        Handler aHandler = new Handler();
        aHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                // Looping, huh?
            }
        }, 1000 * 15);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
