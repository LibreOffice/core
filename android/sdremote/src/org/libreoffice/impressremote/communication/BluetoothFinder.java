/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import org.libreoffice.impressremote.communication.Server.Protocol;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.support.v4.content.LocalBroadcastManager;

public class BluetoothFinder {

    // TODO: add removal of cached items
    private Context mContext;

    BluetoothAdapter mAdapter;

    public BluetoothFinder(Context aContext) {
        mContext = aContext;
        mAdapter = BluetoothAdapter.getDefaultAdapter();

    }

    public void startFinding() {
        if (mAdapter == null) {
            return; // No bluetooth adapter found (emulator, special devices)
        }
        IntentFilter aFilter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        aFilter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        aFilter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, aFilter);
        mAdapter.startDiscovery();
    }

    public void stopFinding() {
        if (mAdapter == null) {
            return; // No bluetooth adapter found (emulator, special devices)
        }
        mAdapter.cancelDiscovery();
        try {
            mContext.unregisterReceiver(mReceiver);
        } catch (IllegalArgumentException e) {
            // The receiver wasn't registered
        }
    }

    private HashMap<String, Server> mServerList = new HashMap<String, Server>();

    public Collection<Server> getServerList() {
        return mServerList.values();
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent aIntent) {
            if (aIntent.getAction().equals(BluetoothDevice.ACTION_FOUND)) {
                System.out.println("Found");
                BluetoothDevice aDevice = (BluetoothDevice) aIntent.getExtras()
                                .get(BluetoothDevice.EXTRA_DEVICE);
                Server aServer = new Server(Protocol.BLUETOOTH,
                                aDevice.getAddress(), aDevice.getName(),
                                System.currentTimeMillis());
                mServerList.put(aServer.getAddress(), aServer);
                Intent aNIntent = new Intent(
                                CommunicationService.MSG_SERVERLIST_CHANGED);
                LocalBroadcastManager.getInstance(mContext).sendBroadcast(
                                aNIntent);
            } else if (aIntent.getAction().equals(
                            BluetoothAdapter.ACTION_DISCOVERY_FINISHED)
                            || aIntent.getAction()
                                            .equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                // Start discovery again after a small delay.
                // but check whether device is on incase the user manually
                // disabled bluetooth
                if (mAdapter.isEnabled()) {
                    Handler aHandler = new Handler();
                    aHandler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            System.out.println("Looping");

                        }
                    }, 1000 * 15);
                }
            }

        }

    };
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */