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
import java.util.concurrent.TimeUnit;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.util.BluetoothOperator;
import org.libreoffice.impressremote.util.Intents;

class BluetoothServersFinder extends BroadcastReceiver implements ServersFinder, Runnable {
    private static final int SEARCH_DELAY_IN_SECONDS = 5;

    private final Context mContext;

    private final Map<String, Server> mServers;

    public BluetoothServersFinder(Context aContext) {
        mContext = aContext;

        mServers = new HashMap<String, Server>();
    }

    @Override
    public void startSearch() {
        if (!BluetoothOperator.isAvailable()) {
            return;
        }

        if (BluetoothOperator.getAdapter().isDiscovering()) {
            return;
        }

        setUpBluetoothActionsReceiver();

        if (!BluetoothOperator.getAdapter().isEnabled()) {
            return;
        }

        BluetoothOperator.getAdapter().startDiscovery();
    }

    private void setUpBluetoothActionsReceiver() {
        IntentFilter aBluetoothActionsFilter = new IntentFilter();
        aBluetoothActionsFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        aBluetoothActionsFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        aBluetoothActionsFilter.addAction(BluetoothDevice.ACTION_FOUND);

        mContext.registerReceiver(this, aBluetoothActionsFilter);
    }

    @Override
    public void onReceive(Context aContext, Intent aIntent) {
        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(aIntent.getAction())) {
            switch (aIntent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0)) {
                case BluetoothAdapter.STATE_ON:
                    BluetoothOperator.getAdapter().startDiscovery();
                    return;

                default:
                    return;
            }
        }

        if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(aIntent.getAction())) {
            startDiscoveryDelayed();
            return;
        }

        if (BluetoothDevice.ACTION_FOUND.equals(aIntent.getAction())) {
            BluetoothDevice aBluetoothDevice = aIntent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            addServer(aBluetoothDevice);
        }
    }

    private void startDiscoveryDelayed() {
        // Start discovery again after a small delay.
        // Check whether device is on in case the user manually
        // disabled Bluetooth.

        if (!BluetoothOperator.getAdapter().isEnabled()) {
            return;
        }

        Handler aDiscoveryHandler = new Handler();
        aDiscoveryHandler.postDelayed(this, TimeUnit.SECONDS.toMillis(SEARCH_DELAY_IN_SECONDS));
    }

    @Override
    public void run() {
        BluetoothOperator.getAdapter().startDiscovery();
    }
    private void addServer(BluetoothDevice aBluetoothDevice) {
        Server.Type aServerType = buildServerType(aBluetoothDevice);
        String aServerAddress = aBluetoothDevice.getAddress();
        String aServerName = aBluetoothDevice.getName();

        Server aServer = Server.newBluetoothInstance(aServerType, aServerAddress, aServerName);
        mServers.put(aServer.getAddress(), aServer);

        Intent bIntent = Intents.buildServersListChangedIntent();
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(bIntent);
    }

    private Server.Type buildServerType(BluetoothDevice aBluetoothDevice) {
        int aBluetoothClass = aBluetoothDevice.getBluetoothClass().getMajorDeviceClass();

        switch (aBluetoothClass) {
            case BluetoothClass.Device.Major.COMPUTER:
                return Server.Type.COMPUTER;

            case BluetoothClass.Device.Major.PHONE:
                return Server.Type.PHONE;

            default:
                return Server.Type.UNDEFINED;
        }
    }

    @Override
    public void stopSearch() {
        if (!BluetoothOperator.isAvailable()) {
            return;
        }

        try {
            mContext.unregisterReceiver(this);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }

        BluetoothOperator.getAdapter().cancelDiscovery();
    }

    @Override
    public List<Server> getServers() {
        return new ArrayList<Server>(mServers.values());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
