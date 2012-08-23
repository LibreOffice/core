/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
import java.util.Map;
import java.util.Map.Entry;

import org.libreoffice.impressremote.communication.Server.Protocol;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Binder;
import android.os.IBinder;

public class CommunicationService extends Service implements Runnable {

    public enum State {
        DISCONNECTED, SEARCHING, CONNECTING, CONNECTED
    };

    /**
     * Get the publicly visible device name -- generally the bluetooth name,
     * however for bluetoothless devices the device model name is used.
     *
     * @return The device name.
     */
    public static String getDeviceName() {
        BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
        if (aAdapter != null) {
            return aAdapter.getName();
        } else {
            return android.os.Build.MODEL;
        }
    }

    /**
     * Used to protect all writes to mState, mStateDesired, and mServerDesired.
     */
    private Object mConnectionVariableMutex = new Object();

    private State mState = State.DISCONNECTED;

    public State getState() {
        return mState;
    }

    public String getPairingPin() {
        return Client.getPin();
    }

    private State mStateDesired = State.DISCONNECTED;

    private Server mServerDesired = null;

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
                    if ((mStateDesired == State.CONNECTED && mState == State.CONNECTED)
                                    || (mStateDesired == State.DISCONNECTED && mState == State.CONNECTED)) {
                        mClient.closeConnection();
                        mState = State.DISCONNECTED;
                    }
                    if (mStateDesired == State.CONNECTED) {
                        mState = State.CONNECTING;
                        switch (mServerDesired.getProtocol()) {
                        case NETWORK:
                            mClient = new NetworkClient(
                                            mServerDesired.getAddress(), this);
                            break;
                        case BLUETOOTH:
                            mClient = new BluetoothClient(
                                            mServerDesired.getAddress(), this);
                            break;
                        }
                        mTransmitter = new Transmitter(mClient);
                        mClient.setReceiver(mReceiver);
                        mState = State.CONNECTED;
                    }
                }
            }
        }

    }

    public void startSearching() {
        mNetworkFinder.startFinding();
        mBluetoothFinder.startFinding();
    }

    public void stopSearching() {
        mNetworkFinder.stopFinding();
        mBluetoothFinder.stopFinding();
    }

    public void connectTo(Server aServer) {
        synchronized (mConnectionVariableMutex) {
            if (mState == State.SEARCHING) {
                mNetworkFinder.stopFinding();
                mBluetoothFinder.stopFinding();
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

    /**
     * Return the service to clients.
     */
    public class CBinder extends Binder {
        public CommunicationService getService() {
            return CommunicationService.this;
        }
    }

    private final IBinder mBinder = new CBinder();

    public static final String MSG_SLIDESHOW_STARTED = "SLIDESHOW_STARTED";
    public static final String MSG_SLIDE_CHANGED = "SLIDE_CHANGED";
    public static final String MSG_SLIDE_PREVIEW = "SLIDE_PREVIEW";
    public static final String MSG_SLIDE_NOTES = "SLIDE_NOTES";

    public static final String MSG_SERVERLIST_CHANGED = "SERVERLIST_CHANGED";
    public static final String MSG_PAIRING_STARTED = "PAIRING_STARTED";
    public static final String MSG_PAIRING_SUCCESSFUL = "PAIRING_SUCCESSFUL";

    private Transmitter mTransmitter;

    private Client mClient;

    private Receiver mReceiver = new Receiver(this);

    private ServerFinder mNetworkFinder = new ServerFinder(this);
    private BluetoothFinder mBluetoothFinder = new BluetoothFinder(this);

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        return mBinder;
    }

    private Thread mThread = null;

    @Override
    public void onCreate() {
        // TODO Create a notification (if configured).
        loadServersFromPreferences();

        mThread = new Thread(this);
        mThread.start();
    }

    @Override
    public void onDestroy() {
        // TODO Destroy the notification (as necessary).
        mManualServers.clear();

        mThread.interrupt();
        mThread = null;
    }

    public Transmitter getTransmitter() {
        return mTransmitter;
    }

    public Server[] getServers() {
        ArrayList<Server> aServers = new ArrayList<Server>();
        aServers.addAll(mNetworkFinder.getServerList());
        aServers.addAll(mBluetoothFinder.getServerList());
        aServers.addAll(mManualServers.values());
        return aServers.toArray(new Server[aServers.size()]);
    }

    public SlideShow getSlideShow() {
        return mReceiver.getSlideShow();
    }

    public boolean isSlideShowRunning() {
        return mReceiver.isSlideShowRunning();
    }

    /**
     * Manually add a new (network) server to the list of servers.
     * @param aAddress
     * @param aRemember
     */
    public void addServer(String aAddress, String aName, boolean aRemember) {
        for (String aServer : mManualServers.keySet()) {
            if (aServer.equals(aAddress))
                return;
        }
        mManualServers.put(aAddress, new Server(Protocol.NETWORK, aAddress,
                        aName, 0));
        if (aRemember) {
            SharedPreferences aPref = getSharedPreferences(SERVERSTORAGE_KEY,
                            MODE_PRIVATE);
            Editor aEditor = aPref.edit();
            aEditor.putString(aAddress, aName);
            aEditor.apply();
        }
    }

    /**
     * Key to use with getSharedPreferences to obtain a Map of stored servers.
     * The keys are the ip/hostnames, the values are the friendly names.
     */
    private static final String SERVERSTORAGE_KEY = "sdremote_storedServers";
    private HashMap<String, Server> mManualServers = new HashMap<String, Server>();

    void loadServersFromPreferences() {
        SharedPreferences aPref = getSharedPreferences(SERVERSTORAGE_KEY,
                        MODE_PRIVATE);

        Map<String, String> aStoredMap = (Map<String, String>) aPref.getAll();

        for (Entry<String, String> aServerEntry : aStoredMap.entrySet()) {
            mManualServers.put(aServerEntry.getKey(), new Server(
                            Protocol.NETWORK, aServerEntry.getKey(),
                            aServerEntry.getValue(), 0));
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */