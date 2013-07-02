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

public class CommunicationService extends Service implements Runnable, MessagesListener {
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

    private final ServersFinder mTcpServersFinder = new TcpServersFinder(this);
    private final ServersFinder mBluetoothServersFinder = new BluetoothServersFinder(
        this);

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
        return getDeviceName();
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

    private ServerConnection mServerConnection;

    private MessagesReceiver mMessagesReceiver;
    private CommandsTransmitter mCommandsTransmitter;

    private void closeConnection() {
        mServerConnection.close();

        mState = State.DISCONNECTED;
    }

    private void openConnection() {
        mServerConnection = buildServerConnection();

        mMessagesReceiver = new MessagesReceiver(mServerConnection, this);
        mCommandsTransmitter = new CommandsTransmitter(mServerConnection);

        pairWithServer();

        mState = State.CONNECTED;
    }

    private ServerConnection buildServerConnection() {
        switch (mServerDesired.getProtocol()) {
            case TCP:
                return new TcpServerConnection(mServerDesired);

            case BLUETOOTH:
                return new BluetoothServerConnection(mServerDesired);

            default:
                throw new RuntimeException("Unknown desired protocol.");
        }
    }

    private void pairWithServer() {
        if (mServerDesired.getProtocol() == Server.Protocol.BLUETOOTH) {
            return;
        }

        mCommandsTransmitter.pair(getDeviceName(), loadPin());

        startPairingActivity();
    }

    private void startPairingActivity() {
        Intent aPairingIntent = new Intent(MSG_PAIRING_STARTED);
        aPairingIntent.putExtra("PIN", loadPin());

        LocalBroadcastManager.getInstance(this).sendBroadcast(aPairingIntent);
    }

    private String loadPin() {
        if (Preferences.doContain(this,
            Preferences.Locations.AUTHORIZED_REMOTES,
            mServerDesired.getAddress())) {
            return Preferences
                .getString(this, Preferences.Locations.AUTHORIZED_REMOTES,
                    mServerDesired.getAddress());
        }

        String aPin = Protocol.Pin.generate();

        Preferences.set(this, Preferences.Locations.AUTHORIZED_REMOTES,
            mServerDesired.getAddress(), aPin);

        return aPin;
    }

    private void connectionFailed() {
        mState = State.DISCONNECTED;
        Intent aIntent = new Intent(
            CommunicationService.STATUS_CONNECTION_FAILED);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    public void startSearch() {
        mTcpServersFinder.startSearch();

        BluetoothAdapter aAdapter = BluetoothAdapter.getDefaultAdapter();
        if (aAdapter != null) {
            mBluetoothPreviouslyEnabled = aAdapter.isEnabled();

            if (!mBluetoothPreviouslyEnabled) {
                aAdapter.enable();
            }

            mBluetoothServersFinder.startSearch();
        }
    }

    public void stopSearch() {
        mTcpServersFinder.stopSearch();
        mBluetoothServersFinder.stopSearch();
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
                mTcpServersFinder.stopSearch();
                mBluetoothServersFinder.stopSearch();
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

    public CommandsTransmitter getTransmitter() {
        return mCommandsTransmitter;
    }

    public List<Server> getServers() {
        List<Server> aServers = new ArrayList<Server>();

        aServers.addAll(mTcpServersFinder.getServers());
        aServers.addAll(mBluetoothServersFinder.getServers());
        aServers.addAll(mManualServers.values());

        return aServers;
    }

    public SlideShow getSlideShow() {
        return mSlideShow;
    }

    void loadServersFromPreferences() {
        SharedPreferences aPref = getSharedPreferences(
            Preferences.Locations.STORED_SERVERS,
            MODE_PRIVATE);

        @SuppressWarnings("unchecked")
        Map<String, String> aStoredMap = (Map<String, String>) aPref.getAll();

        for (Entry<String, String> aServerEntry : aStoredMap.entrySet()) {
            mManualServers.put(aServerEntry.getKey(), new Server(
                Server.Protocol.TCP, aServerEntry.getKey(),
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
        mManualServers
            .put(aAddress, new Server(Server.Protocol.TCP, aAddress,
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

    @Override
    public void onPinValidation() {
        startPinValidation();
    }

    private void startPinValidation() {
        Intent aPairingIntent = new Intent(STATUS_PAIRING_PINVALIDATION);
        aPairingIntent.putExtra("PIN", loadPin());
        aPairingIntent.putExtra("SERVERNAME", mServerDesired.getName());

        LocalBroadcastManager.getInstance(this).sendBroadcast(aPairingIntent);
    }

    @Override
    public void onSuccessfulPairing() {
        callSuccessfulPairing();
    }

    private void callSuccessfulPairing() {
        Intent aSuccessfulPairingIntent = new Intent(MSG_PAIRING_SUCCESSFUL);

        LocalBroadcastManager.getInstance(this).sendBroadcast(
            aSuccessfulPairingIntent);
    }

    private SlideShow mSlideShow;

    @Override
    public void onSlideShowStart(int aSlidesCount, int aCurrentSlideIndex) {
        mSlideShow = new SlideShow();
        mSlideShow.setSlidesCount(aSlidesCount);
        mSlideShow.setCurrentSlideIndex(aCurrentSlideIndex);

        Intent aStatusConnectedSlideShowRunningIntent = new Intent(
            STATUS_CONNECTED_SLIDESHOW_RUNNING);
        Intent aSlideChangedIntent = new Intent(MSG_SLIDE_CHANGED);
        aSlideChangedIntent.putExtra("slide_number", aCurrentSlideIndex);

        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aStatusConnectedSlideShowRunningIntent);
        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aSlideChangedIntent);
    }

    @Override
    public void onSlideShowFinish() {
        mSlideShow = new SlideShow();

        Intent aStatusConnectedNoSlideShowIntent = new Intent(
            STATUS_CONNECTED_NOSLIDESHOW);

        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aStatusConnectedNoSlideShowIntent);
    }

    @Override
    public void onSlideChanged(int aCurrentSlideIndex) {
        mSlideShow.setCurrentSlideIndex(aCurrentSlideIndex);

        Intent aSlideChangedIntent = new Intent(MSG_SLIDE_CHANGED);
        aSlideChangedIntent.putExtra("slide_number", aCurrentSlideIndex);

        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aSlideChangedIntent);
    }

    @Override
    public void onSlidePreview(int aSlideIndex, byte[] aPreview) {
        mSlideShow.setSlidePreview(aSlideIndex, aPreview);

        Intent aSlidePreviewChangedIntent = new Intent(MSG_SLIDE_PREVIEW);
        aSlidePreviewChangedIntent.putExtra("slide_number", aSlideIndex);

        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aSlidePreviewChangedIntent);
    }

    @Override
    public void onSlideNotes(int aSlideIndex, String aNotes) {
        mSlideShow.setSlideNotes(aSlideIndex, aNotes);

        Intent aSlideNotesChangedIntent = new Intent(MSG_SLIDE_NOTES);
        aSlideNotesChangedIntent.putExtra("slide_number", aSlideIndex);

        LocalBroadcastManager.getInstance(this)
            .sendBroadcast(aSlideNotesChangedIntent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
