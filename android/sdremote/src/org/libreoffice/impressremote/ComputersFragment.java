/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import java.util.ArrayList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import com.actionbarsherlock.app.SherlockListFragment;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;

public class ComputersFragment extends SherlockListFragment implements ServiceConnection {
    public static enum Type {
        WIFI, BLUETOOTH
    }

    private Type mType;

    private CommunicationService mCommunicationService;
    private BroadcastReceiver mIntentsReceiver;

    public static ComputersFragment newInstance(Type aType) {
        ComputersFragment aFragment = new ComputersFragment();

        aFragment.setArguments(buildArguments(aType));

        return aFragment;
    }

    private static Bundle buildArguments(Type aType) {
        Bundle aArguments = new Bundle();

        aArguments.putSerializable("TYPE", aType);

        return aArguments;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mType = (Type) getArguments().getSerializable("TYPE");
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        bindService();
    }

    private void bindService() {
        Intent aServiceIntent = new Intent(getActivity(), CommunicationService.class);

        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.CBinder aServiceBinder = (CommunicationService.CBinder) aBinder;

        mCommunicationService = aServiceBinder.getService();

        mCommunicationService.startSearch();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        unbindService();
    }

    private void unbindService() {
        if (!isServiceBound()) {
            return;
        }

        getActivity().unbindService(this);
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }

    @Override
    public void onResume() {
        super.onResume();

        registerIntentsReceiver();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = buildIntentsReceiverFilter();

        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(mIntentsReceiver, aIntentFilter);
    }

    private static final class IntentsReceiver extends BroadcastReceiver {
        private final ComputersFragment mComputersFragment;

        public IntentsReceiver(ComputersFragment aComputersFragment) {
            mComputersFragment = aComputersFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (CommunicationService.MSG_SERVERLIST_CHANGED.equals(aIntent.getAction())) {
                mComputersFragment.loadComputers();
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(CommunicationService.MSG_SERVERLIST_CHANGED);

        return aIntentFilter;
    }

    public void loadComputers() {
        if (!isServiceBound()) {
            return;
        }

        if (getComputers().isEmpty()) {
            return;
        }

        if (!isComputersAdapterExist()) {
            setUpComputersAdapter();
        }

        getComputersAdapter().clear();
        getComputersAdapter().add(getComputers());
    }

    private boolean isComputersAdapterExist() {
        return getComputersAdapter() != null;
    }

    private ComputersAdapter getComputersAdapter() {
        return (ComputersAdapter) getListAdapter();
    }

    private void setUpComputersAdapter() {
        setListAdapter(new ComputersAdapter(getActivity()));
    }

    private List<Server> getComputers() {
        List<Server> aComputers = new ArrayList<Server>();

        for (Server aServer : mCommunicationService.getServers()) {
            if (isComputerSupportsRequiredType(aServer)) {
                aComputers.add(aServer);
            }
        }

        return aComputers;
    }

    private boolean isComputerSupportsRequiredType(Server aServer) {
        switch (mType) {
            case WIFI:
                return aServer.getProtocol() == Server.Protocol.TCP;

            case BLUETOOTH:
                return aServer.getProtocol() == Server.Protocol.BLUETOOTH;

            default:
                return false;
        }
    }

    @Override
    public void onPause() {
        super.onPause();

        unregisterIntentsReceiver();
    }

    private void unregisterIntentsReceiver() {
        try {
            getActivity().unregisterReceiver(mIntentsReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
