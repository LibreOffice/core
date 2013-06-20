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

import org.libreoffice.impressremote.Globals;
import org.libreoffice.impressremote.communication.Server.Protocol;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;
import android.preference.PreferenceManager;
import android.support.v4.content.LocalBroadcastManager;

public class CommunicationService extends Service implements Runnable {

    public static enum State {
        DISCONNECTED, SEARCHING, CONNECTING, CONNECTED
    }

    /**
     * Get the publicly visible device name -- generally the bluetooth name,
     * however for bluetoothless devices the device model name is used.
     *
     * @return The device name.
     */
    public static String getDeviceName() {
        BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
        if (aAdapter != null) {
            String aName = aAdapter.getName();
            if (aName != null)
                return aName;
        }
        return Build.MODEL;
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

    public String getPairingDeviceName() {
        return Client.getName();
    }

    private State mStateDesired = State.DISCONNECTED;

    private Server mServerDesired = null;

    @Override
    public void run() {
        Log.i(Globals.TAG, "CommunicationService.run()");
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
                Log.i(Globals.TAG, "CommunicationService.run: at \"Work\"");
                synchronized (mConnectionVariableMutex) {
                    if ((mStateDesired == State.CONNECTED && mState == State.CONNECTED)
                                    || (mStateDesired == State.DISCONNECTED && mState == State.CONNECTED)) {
                        mClient.closeConnection();
                        mClient = null;
                        mState = State.DISCONNECTED;
                    }
                    if (mStateDesired == State.CONNECTED) {
                        mState = State.CONNECTING;
                        try {
                            switch (mServerDesired.getProtocol()) {
                            case NETWORK:
                                mClient = new NetworkClient(mServerDesired,
                                                this, mReceiver);
                                mClient.validating();
                                break;
                            case BLUETOOTH:
                                mClient = new BluetoothClient(mServerDesired,
                                                this, mReceiver,
                                                mBluetoothPreviouslyEnabled);
                                mClient.validating();
                                break;
                            }
                            mTransmitter = new Transmitter(mClient);
                            mState = State.CONNECTED;
                        } catch (IOException e) {
                            Log.i(Globals.TAG, "CommunicationService.run: " + e);
                            connextionFailed();
                        }
                    }
                }
                Log.i(Globals.TAG, "CommunicationService.finished work");
            }
        }
    }

    private void connextionFailed() {
        mClient = null;
        mState = State.DISCONNECTED;
        Intent aIntent = new Intent(
                CommunicationService.STATUS_CONNECTION_FAILED);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    private boolean mBluetoothPreviouslyEnabled;

    public void startSearching() {
        Log.i(Globals.TAG, "CommunicationService.startSearching()");
        SharedPreferences aPref = PreferenceManager.getDefaultSharedPreferences(this);
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

    public void stopSearching() {
        Log.i(Globals.TAG, "CommunicationService.stopSearching()");
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
        Log.i(Globals.TAG, "CommunicationService.connectTo(" + aServer + ")");
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
        Log.d(Globals.TAG, "Service Disconnected");
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

    /**
     * Notify the UI that the service has connected to a server AND a slideshow
     * is running.
     * In this case the PresentationActivity should be started.
     */
    public static final String STATUS_CONNECTED_SLIDESHOW_RUNNING = "STATUS_CONNECTED_SLIDESHOW_RUNNING";
    /**
     * Notify the UI that the service has connected to a server AND no slideshow
     * is running.
     * In this case the StartPresentationActivity should be started.
     */
    public static final String STATUS_CONNECTED_NOSLIDESHOW = "STATUS_CONNECTED_NOSLIDESHOW";

    public static final String STATUS_PAIRING_PINVALIDATION = "STATUS_PAIRING_PINVALIDATION";

    public static final String STATUS_CONNECTION_FAILED = "STATUS_CONNECTION_FAILED";

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

    public List<Server> getServers() {
        ArrayList<Server> aServers = new ArrayList<Server>();
        aServers.addAll(mNetworkFinder.getServers());
        aServers.addAll(mBluetoothFinder.getServers());
        aServers.addAll(mManualServers.values());
        return aServers;
    }

    public SlideShow getSlideShow() {
        return mReceiver.getSlideShow();
    }

    public boolean isSlideShowRunning() {
        return mReceiver.isSlideShowRunning();
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

    public void removeServer(Server aServer) {
        mManualServers.remove(aServer.getAddress());

        SharedPreferences aPref = getSharedPreferences(SERVERSTORAGE_KEY,
                        MODE_PRIVATE);
        Editor aEditor = aPref.edit();
        aEditor.remove(aServer.getAddress());
        aEditor.apply();

    }

    public Client getClient() {
        return mClient;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
