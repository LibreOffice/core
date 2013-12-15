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
import java.util.Set;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.util.Intents;

class BluetoothServersFinder extends BroadcastReceiver implements ServersFinder {
    private static final BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();

    private final Context mContext;

    private final Map<String, Server> mServers;

    public BluetoothServersFinder(Context aContext) {
        mContext = aContext;

        mServers = new HashMap<String, Server>();
    }

    @Override
    public void startSearch() {
        if (btAdapter == null) {
            return;
        }
        IntentFilter aBluetoothActionsFilter = new IntentFilter();
        aBluetoothActionsFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        aBluetoothActionsFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        aBluetoothActionsFilter.addAction(BluetoothDevice.ACTION_FOUND);

        mContext.registerReceiver(this, aBluetoothActionsFilter);
        Set<BluetoothDevice> pairedDevices = btAdapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                addServer(device);
            }
        } else {
            if (btAdapter.isDiscovering()) {
                return;
            }
            btAdapter.startDiscovery();
        }
    }

    @Override
    public void onReceive(Context aContext, Intent aIntent) {
        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(aIntent.getAction())) {
            switch (aIntent.getIntExtra(BluetoothAdapter.EXTRA_STATE, 0)) {
                case BluetoothAdapter.STATE_ON:
                    startSearch();
                    return;

                default:
                    return;
            }
        }

        if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(aIntent.getAction())) {
            LocalBroadcastManager.getInstance(mContext)
                .sendBroadcast(new Intent(Intents.Actions.BT_DISCOVERY_CHANGED));
            return;
        }

        if (BluetoothDevice.ACTION_FOUND.equals(aIntent.getAction())) {
            BluetoothDevice aBluetoothDevice = aIntent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            addServer(aBluetoothDevice);
        }
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
        if (btAdapter == null) {
            return;
        }

        try {
            mContext.unregisterReceiver(this);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }

        btAdapter.cancelDiscovery();
    }

    @Override
    public List<Server> getServers() {
        return new ArrayList<Server>(mServers.values());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
