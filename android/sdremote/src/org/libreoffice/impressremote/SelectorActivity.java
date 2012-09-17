/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import java.text.MessageFormat;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map.Entry;

import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;
import org.libreoffice.impressremote.communication.Server.Protocol;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockActivity;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuInflater;
import com.actionbarsherlock.view.MenuItem;

public class SelectorActivity extends SherlockActivity {

    private CommunicationService mCommunicationService;

    private View mBluetoothContainer;
    private LinearLayout mBluetoothList;
    private View mNetworkContainer;
    private LinearLayout mNetworkList;
    private TextView mNoServerLabel;
    private ActivityChangeBroadcastProcessor mBroadcastProcessor;

    ProgressDialog mProgressDialog = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_selector);

        IntentFilter aFilter = new IntentFilter(
                        CommunicationService.MSG_SERVERLIST_CHANGED);

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);
        mBroadcastProcessor.addToFilter(aFilter);

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
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getSupportMenuInflater();
        inflater.inflate(R.menu.selector_activity, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.menu_selector_addServer:

            AlertDialog.Builder builder;
            AlertDialog alertDialog;

            LayoutInflater inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);
            final View layout = inflater.inflate(R.layout.dialog_addserver,
                            null);

            builder = new AlertDialog.Builder(this);
            builder.setView(layout);
            builder.setTitle(R.string.addserver);
            builder.setPositiveButton(R.string.addserver_add,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog,
                                                int which) {
                                    EditText aAddressEntry = (EditText) layout
                                                    .findViewById(R.id.addserver_addressentry);
                                    EditText aNameEntry = (EditText) layout
                                                    .findViewById(R.id.addserver_nameentry);
                                    CheckBox aRememberServer = (CheckBox) layout
                                                    .findViewById(R.id.addserver_remember);
                                    mCommunicationService
                                                    .addServer(aAddressEntry
                                                                    .getText()
                                                                    .toString(),
                                                                    aNameEntry.getText()
                                                                                    .toString(),
                                                                    aRememberServer.isChecked());
                                    refreshLists();
                                }
                            });
            builder.setNegativeButton(R.string.addserver_cancel, null);
            alertDialog = builder.create();
            alertDialog.show();

            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onBackPressed() {
        if (mCommunicationService != null)
            mCommunicationService.stopSearching();
        Intent aIntent = new Intent(this, CommunicationService.class);
        stopService(aIntent);
        super.onBackPressed();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mNetworkList.removeAllViews();
        mBluetoothList.removeAllViews();
        mNetworkServers.clear();
        mBluetoothServers.clear();
        doBindService();
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        if (mCommunicationService != null) {
            mCommunicationService.stopSearching();
        }
        doUnbindService();
        if (mProgressDialog != null) {
            mProgressDialog.dismiss();
        }
    }

    void doBindService() {
        Intent aIntent = new Intent(this, CommunicationService.class);
        startService(aIntent);
        bindService(aIntent, mConnection, Context.BIND_IMPORTANT);
    }

    void doUnbindService() {
        unbindService(mConnection);
        mCommunicationService = null;
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();
            mCommunicationService.startSearching();
            refreshLists();
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
                return;
            }
            mBroadcastProcessor.onReceive(aContext, aIntent);

        }
    };

    private HashMap<Server, View> mBluetoothServers = new HashMap<Server, View>();
    private HashMap<Server, View> mNetworkServers = new HashMap<Server, View>();

    private void deleteServer(View aView) {
        for (Entry<Server, View> aEntry : mNetworkServers.entrySet()) {
            System.out.println(aEntry.getKey().getName());
            System.out.println(aView);
            System.out.println(aEntry.getValue());
            if (aEntry.getValue() == aView
                            .findViewById(R.id.selector_sub_label)
                            || aEntry.getValue().findViewById(
                                            R.id.selector_sub_label) == aView
                                            .findViewById(R.id.selector_sub_label))
                mCommunicationService.removeServer(aEntry.getKey());
        }
    }

    private void refreshLists() {
        if (mCommunicationService != null) {

            Server[] aServers = mCommunicationService.getServers();

            // Bluetooth -- Remove old
            for (Entry<Server, View> aEntry : mBluetoothServers.entrySet()) {
                if (!Arrays.asList(aServers).contains(aEntry.getKey())) {
                    mBluetoothServers.remove(aEntry.getKey());
                    mBluetoothList.removeView((View) aEntry.getValue()
                                    .getParent());
                }
            }
            // Network -- Remove old
            for (Entry<Server, View> aEntry : mNetworkServers.entrySet()) {
                if (!Arrays.asList(aServers).contains(aEntry.getKey())) {
                    mNetworkServers.remove(aEntry.getKey());
                    mNetworkList.removeView((View) aEntry.getValue()
                                    .getParent());
                }
            }
            // Add all new
            for (Server aServer : aServers) {
                boolean aIsBluetooth = (aServer.getProtocol() == Protocol.BLUETOOTH);
                HashMap<Server, View> aMap = aIsBluetooth ? mBluetoothServers
                                : mNetworkServers;
                LinearLayout aLayout = aIsBluetooth ? mBluetoothList
                                : mNetworkList;

                if (!aMap.containsKey(aServer)) {
                    View aView = getLayoutInflater()
                                    .inflate(R.layout.activity_selector_sublayout_server,
                                                    null);

                    TextView aText = (TextView) aView
                                    .findViewById(R.id.selector_sub_label);
                    aText.setOnClickListener(mClickListener);
                    aText.setText(aServer.getName());
                    aLayout.addView(aView);
                    aMap.put(aServer, aText);

                    //                    registerForContextMenu(aView);
                    registerForContextMenu(aText);
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
            mCommunicationService.stopSearching();

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
                // Connect Service and wait for broadcast
                String aFormat = getResources().getString(
                                R.string.selector_dialog_connecting);
                String aDialogText = MessageFormat.format(aFormat,
                                aDesiredServer.getName());

                mProgressDialog = ProgressDialog.show(SelectorActivity.this,
                                "", aDialogText, true);
                //                Intent aIntent = new Intent(SelectorActivity.this,
                //                                PairingActivity.class);
                //                startActivity(aIntent);
            }

        }
    };

    View aLastSelected = null;

    public void onCreateContextMenu(android.view.ContextMenu menu, View v,
                    android.view.ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        aLastSelected = v;
        android.view.MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.selector_contextmenu, menu);

    }

    @Override
    public boolean onContextItemSelected(android.view.MenuItem item) {
        switch (item.getItemId()) {
        case R.id.selector_context_delete:
            deleteServer(aLastSelected);
            refreshLists();
            return true;
        }
        return super.onContextItemSelected(item);
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
