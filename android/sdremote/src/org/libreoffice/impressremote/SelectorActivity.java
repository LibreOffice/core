/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;
import org.libreoffice.impressremote.communication.Server.Protocol;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.LinearLayout;
import android.widget.TextView;

public class SelectorActivity extends Activity {

    private CommunicationService mCommunicationService;

    private View mBluetoothContainer;
    private LinearLayout mBluetoothList;
    private View mNetworkContainer;
    private LinearLayout mNetworkList;
    private TextView mNoServerLabel;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_selector);

        IntentFilter aFilter = new IntentFilter(
                        CommunicationService.MSG_SERVERLIST_CHANGED);
        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        mBluetoothContainer = findViewById(R.id.selector_container_bluetooth);
        mBluetoothList = (LinearLayout) findViewById(R.id.selector_list_bluetooth);
        mNetworkContainer = findViewById(R.id.selector_container_network);
        mNetworkList = (LinearLayout) findViewById(R.id.selector_list_network);
        mNoServerLabel = (TextView) findViewById(R.id.selector_label_none);

        refreshLists();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(mListener);
    }

    @Override
    public void onBackPressed() {
        mCommunicationService.stopFindingServers();
        Intent aIntent = new Intent(this, CommunicationService.class);
        stopService(aIntent);
        super.onBackPressed();
    }

    @Override
    protected void onResume() {
        super.onResume();
        doBindService();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        if (mCommunicationService != null) {
            mCommunicationService.stopFindingServers();
        }
        doUnbindService();
    }

    void doBindService() {
        Intent aIntent = new Intent(this, CommunicationService.class);
        startService(aIntent);
        bindService(aIntent, mConnection, Context.BIND_IMPORTANT);
    }

    void doUnbindService() {
        unbindService(mConnection);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();
            mCommunicationService.startFindingServers();
        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (aIntent.getAction().equals(
                            CommunicationService.MSG_SERVERLIST_CHANGED)) {
                refreshLists();
            }

        }
    };

    private HashMap<Server, View> mBluetoothServers = new HashMap<Server, View>();
    private HashMap<Server, View> mNetworkServers = new HashMap<Server, View>();

    private void refreshLists() {
        if (mCommunicationService != null) {

            Server[] aServers = mCommunicationService.getServers();

            // Bluetooth -- Remove old
            for (Entry<Server, View> aEntry : mBluetoothServers.entrySet()) {
                if (!Arrays.asList(aServers).contains(aEntry.getKey())) {
                    mBluetoothServers.remove(aEntry.getKey());
                    mBluetoothList.removeView(aEntry.getValue());
                }
            }
            // Network -- Remove old
            for (Entry<Server, View> aEntry : mNetworkServers.entrySet()) {
                if (!Arrays.asList(aServers).contains(aEntry.getKey())) {
                    mNetworkServers.remove(aEntry.getKey());
                    mNetworkList.removeView(aEntry.getValue());
                }
            }
            // Add all new
            for (Server aServer : aServers) {
                boolean aIsBluetooth = (aServer.getProtocol() == Protocol.BLUETOOTH);
                HashMap<Server, View> aMap = aIsBluetooth ? mBluetoothServers
                                : mNetworkServers;
                LinearLayout aLayout = aIsBluetooth ? mBluetoothList
                                : mNetworkList;

                if (!aMap.containsValue(aServer)) {
                    View aView = getLayoutInflater()
                                    .inflate(R.layout.activity_selector_sublayout_server,
                                                    null);

                    TextView aText = (TextView) aView
                                    .findViewById(R.id.selector_sub_label);
                    aText.setOnClickListener(mClickListener);
                    aText.setText(aServer.getName());
                    aLayout.addView(aView);
                    aMap.put(aServer, aText);
                }

            }
        }
        // Hide as necessary

        mBluetoothContainer
                        .setVisibility((mBluetoothServers.size() != 0) ? View.VISIBLE
                                        : View.GONE);
        mNetworkContainer
                        .setVisibility((mNetworkServers.size() != 0) ? View.VISIBLE
                                        : View.GONE);

        mNoServerLabel.setVisibility(((mBluetoothServers.size() == 0) && (mNetworkServers
                        .size() == 0)) ? View.VISIBLE : View.GONE);
    }

    private OnClickListener mClickListener = new OnClickListener() {

        @Override
        public void onClick(View aView) {
            mCommunicationService.stopFindingServers();

            Server aDesiredServer = null;

            if (mBluetoothServers.containsValue(aView)) {
                for (Entry<Server, View> aEntry : mBluetoothServers.entrySet()) {
                    if (aEntry.getValue() == aView) {
                        aDesiredServer = aEntry.getKey();
                    }
                }
            } else if (mNetworkServers.containsValue(aView)) {
                for (Entry<Server, View> aEntry : mNetworkServers.entrySet()) {
                    if (aEntry.getValue() == aView) {
                        aDesiredServer = aEntry.getKey();
                    }
                }
            }
            if (aDesiredServer != null) {
                mCommunicationService.connectTo(aDesiredServer);
                Intent aIntent = new Intent(SelectorActivity.this,
                                PairingActivity.class);
                startActivity(aIntent);
            }

        }
    };

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */