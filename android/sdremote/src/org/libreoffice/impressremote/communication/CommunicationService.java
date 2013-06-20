/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.Preferences;
import org.libreoffice.impressremote.communication.Server.Protocol;

public class CommunicationService extends Service implements Runnable {
    public static enum State {
        DISCONNECTED, SEARCHING, CONNECTING, CONNECTED
    }

    public static final String MSG_SLIDESHOW_STARTED = "SLIDESHOW_STARTED";
    public static final String MSG_SLIDE_CHANGED = "SLIDE_CHANGED";
    public static final String MSG_SLIDE_PREVIEW = "SLIDE_PREVIEW";
    public static final String MSG_SLIDE_NOTES = "SLIDE_NOTES";

    public static final String MSG_SERVERLIST_CHANGED = "SERVERLIST_CHANGED";
    public static final String MSG_PAIRING_STARTED = "PAIRING_STARTED";
    public static final String MSG_PAIRING_SUCCESSFUL = "PAIRING_SUCCESSFUL";

    public static final String STATUS_CONNECTED_SLIDESHOW_RUNNING = "STATUS_CONNECTED_SLIDESHOW_RUNNING";
    public static final String STATUS_CONNECTED_NOSLIDESHOW = "STATUS_CONNECTED_NOSLIDESHOW";

    public static final String STATUS_PAIRING_PINVALIDATION = "STATUS_PAIRING_PINVALIDATION";
    public static final String STATUS_CONNECTION_FAILED = "STATUS_CONNECTION_FAILED";

    /**
     * Used to protect all writes to mState, mStateDesired, and mServerDesired.
     */
    private final Object mConnectionVariableMutex = new Object();

    private State mState = State.DISCONNECTED;
    private State mStateDesired = State.DISCONNECTED;

    private Server mServerDesired = null;

    private boolean mBluetoothPreviouslyEnabled;

    private final IBinder mBinder = new CBinder();

    private Transmitter mTransmitter;

    private Client mClient;

    private final Receiver mReceiver = new Receiver(this);

    private final ServerFinder mNetworkFinder = new ServerFinder(this);
    private final BluetoothFinder mBluetoothFinder = new BluetoothFinder(this);

    private Thread mThread = null;

    /**
     * Key to use with getSharedPreferences to obtain a Map of stored servers.
     * The keys are the ip/hostnames, the values are the friendly names.
     */
    private final Map<String, Server> mManualServers = new HashMap<String, Server>();

    /**
     * Get the publicly visible device name -- generally the bluetooth name,
     * however for bluetoothless devices the device model name is used.
     *
     * @return The device name.
     */
    public static String getDeviceName() {
        if (BluetoothAdapter.getDefaultAdapter() == null) {
            return Build.MODEL;
        }

        if (BluetoothAdapter.getDefaultAdapter().getName() == null) {
            return Build.MODEL;
        }

        return BluetoothAdapter.getDefaultAdapter().getName();
    }

    public String getPairingDeviceName() {
        return Client.getName();
    }

    @Override
    public void run() {
        synchronized (this) {
            while (true) {
                // Condition
                try {
                    wait();
                } catch (InterruptedException e) {
                    // We have finished
                    return;
                }

                // Work
                synchronized (mConnectionVariableMutex) {
                    if ((mStateDesired == State.CONNECTED) && (mState == State.CONNECTED)) {
                        closeConnection();
                    }

                    if ((mStateDesired == State.DISCONNECTED) && (mState == State.CONNECTED)) {
                        closeConnection();
                    }

                    if (mStateDesired == State.CONNECTED) {
                        mState = State.CONNECTING;

                        openConnection();
                    }
                }
            }
        }
    }

    private void closeConnection() {
        mClient.closeConnection();
        mClient = null;

        mState = State.DISCONNECTED;
    }

    private void openConnection() {
        try {
            mClient = buildClient();
            mClient.validating();

            mTransmitter = new Transmitter(mClient);

            mState = State.CONNECTED;
        } catch (IOException e) {
            connectionFailed();
        }
    }

    private Client buildClient() {
        switch (mServerDesired.getProtocol()) {
            case NETWORK:
                return new NetworkClient(mServerDesired, this, mReceiver);

            case BLUETOOTH:
                return new BluetoothClient(mServerDesired, this, mReceiver,
                    mBluetoothPreviouslyEnabled);

            default:
                throw new RuntimeException("Unknown desired protocol.");
        }
    }

    private void connectionFailed() {
        mClient = null;
        mState = State.DISCONNECTED;
        Intent aIntent = new Intent(
            CommunicationService.STATUS_CONNECTION_FAILED);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    public void startSearch() {
        SharedPreferences aPref = PreferenceManager
            .getDefaultSharedPreferences(this);
        boolean bEnableWifi = aPref.getBoolean("option_enablewifi", false);
        if (bEnableWifi)
            mNetworkFinder.startSearch();
        BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
        if (aAdapter != null) {
            mBluetoothPreviouslyEnabled = aAdapter.isEnabled();
            if (!mBluetoothPreviouslyEnabled)
                aAdapter.enable();
            mBluetoothFinder.startSearch();
        }
    }

    public void stopSearch() {
        mNetworkFinder.stopSearch();
        mBluetoothFinder.stopSearch();
        BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
        if (aAdapter != null) {
            if (!mBluetoothPreviouslyEnabled) {
                aAdapter.disable();
            }
        }
    }

    public void connectTo(Server aServer) {
        synchronized (mConnectionVariableMutex) {
            if (mState == State.SEARCHING) {
                mNetworkFinder.stopSearch();
                mBluetoothFinder.stopSearch();
                mState = State.DISCONNECTED;
            }
            mServerDesired = aServer;
            mStateDesired = State.CONNECTED;
            synchronized (this) {
                notify();
            }

        }
        // TODO: connect
    }

    public void disconnect() {
        synchronized (mConnectionVariableMutex) {
            mStateDesired = State.DISCONNECTED;
            synchronized (this) {
                notify();
            }
        }
    }

    public class CBinder extends Binder {
        public CommunicationService getService() {
            return CommunicationService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onCreate() {
        loadServersFromPreferences();

        mThread = new Thread(this);
        mThread.start();
    }

    @Override
    public void onDestroy() {
        mManualServers.clear();

        mThread.interrupt();
        mThread = null;
    }

    public Transmitter getTransmitter() {
        return mTransmitter;
    }

    public List<Server> getServers() {
        List<Server> aServers = new ArrayList<Server>();

        aServers.addAll(mNetworkFinder.getServers());
        aServers.addAll(mBluetoothFinder.getServers());
        aServers.addAll(mManualServers.values());

        return aServers;
    }

    public SlideShow getSlideShow() {
        return mReceiver.getSlideShow();
    }

    void loadServersFromPreferences() {
        SharedPreferences aPref = getSharedPreferences(
            Preferences.Locations.STORED_SERVERS,
            MODE_PRIVATE);

        @SuppressWarnings("unchecked")
        Map<String, String> aStoredMap = (Map<String, String>) aPref.getAll();

        for (Entry<String, String> aServerEntry : aStoredMap.entrySet()) {
            mManualServers.put(aServerEntry.getKey(), new Server(
                Protocol.NETWORK, aServerEntry.getKey(),
                aServerEntry.getValue(), 0));
        }
    }

    /**
     * Manually add a new (network) server to the list of servers.
     */
    public void addServer(String aAddress, String aName, boolean aRemember) {
        for (String aServer : mManualServers.keySet()) {
            if (aServer.equals(aAddress))
                return;
        }
        mManualServers.put(aAddress, new Server(Protocol.NETWORK, aAddress,
            aName, 0));
        if (aRemember) {

            Preferences
                .set(this, Preferences.Locations.STORED_SERVERS, aAddress,
                    aName);
        }
    }

    public void removeServer(Server aServer) {
        mManualServers.remove(aServer.getAddress());

        Preferences.remove(this, Preferences.Locations.STORED_SERVERS,
            aServer.getAddress());
    }

    public Client getClient() {
        return mClient;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
