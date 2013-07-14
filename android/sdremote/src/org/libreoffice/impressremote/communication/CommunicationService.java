/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.List;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.Intents;

public class CommunicationService extends Service implements Runnable, MessagesListener {
    public static enum State {
        DISCONNECTED, SEARCHING, CONNECTING, CONNECTED
    }

    /**
     * Used to protect all writes to mState, mStateDesired, and mServerDesired.
     */
    private final Object mConnectionVariableMutex = new Object();

    private State mState;
    private State mStateDesired;

    private Server mServerDesired;

    private IBinder mBinder;

    private ServersManager mServersManager;

    private ServerConnection mServerConnection;

    private MessagesReceiver mMessagesReceiver;
    private CommandsTransmitter mCommandsTransmitter;

    private SlideShow mSlideShow;

    private Thread mThread;

    @Override
    public void onCreate() {
        mState = State.DISCONNECTED;
        mStateDesired = State.DISCONNECTED;

        mServerDesired = null;

        mBinder = new CBinder();

        mServersManager = new ServersManager(this);

        mThread = new Thread(this);
        mThread.start();
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

                        try {
                            openConnection();
                        }
                        catch (RuntimeException e) {
                            connectionFailed();
                        }
                    }
                }
            }
        }
    }

    private void closeConnection() {
        mServerConnection.close();

        mState = State.DISCONNECTED;
    }

    private void openConnection() {
        mServerConnection = buildServerConnection();

        mMessagesReceiver = new MessagesReceiver(mServerConnection, this);
        mCommandsTransmitter = new CommandsTransmitter(mServerConnection);

        if (PairingProvider.isPairingNecessary(mServerDesired)) {
            pair();
        }

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

    private void pair() {
        String aPairingDeviceName = PairingProvider.getPairingDeviceName(this);
        String aPairingPin = PairingProvider.getPairingPin(this, mServerDesired);

        mCommandsTransmitter.pair(aPairingDeviceName, aPairingPin);
    }

    private void connectionFailed() {
        mState = State.DISCONNECTED;

        Intent aIntent = Intents.buildConnectionFailedIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    public void startSearch() {
        mState = State.SEARCHING;

        if (BluetoothAdapter.getDefaultAdapter() != null) {
            BluetoothAdapter.getDefaultAdapter().enable();
        }

        mServersManager.startServersSearch();
    }

    public void stopSearch() {
        mServersManager.stopServersSearch();

        if (BluetoothAdapter.getDefaultAdapter() != null) {
            BluetoothAdapter.getDefaultAdapter().disable();
        }
    }

    public List<Server> getServers() {
        return mServersManager.getServers();
    }

    public void connectTo(Server aServer) {
        synchronized (mConnectionVariableMutex) {
            if (mState == State.SEARCHING) {
                mServersManager.stopServersSearch();
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

    public CommandsTransmitter getTransmitter() {
        return mCommandsTransmitter;
    }

    public SlideShow getSlideShow() {
        return mSlideShow;
    }

    @Deprecated
    public void addServer(String aAddress, String aName, boolean aRemember) {
        mServersManager.addTcpServer(aAddress, aName);
    }

    public void addServer(String aAddress, String aName) {
        mServersManager.addTcpServer(aAddress, aName);
    }

    public void removeServer(Server aServer) {
        mServersManager.removeServer(aServer);
    }

    @Override
    public void onPinValidation() {
        String aPin = PairingProvider.getPairingPin(this, mServerDesired);

        Intent aIntent = Intents.buildPairingValidationIntent(aPin);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onSuccessfulPairing() {
        Intent aIntent = Intents.buildPairingSuccessfulIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onSlideShowStart(int aSlidesCount, int aCurrentSlideIndex) {
        mSlideShow = new SlideShow();
        mSlideShow.setSlidesCount(aSlidesCount);

        Intent aIntent = Intents.buildSlideShowRunningIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);

        onSlideChanged(aCurrentSlideIndex);
    }

    @Override
    public void onSlideShowFinish() {
        mSlideShow = new SlideShow();

        Intent aIntent = Intents.buildSlideShowStoppedIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onSlideChanged(int aCurrentSlideIndex) {
        mSlideShow.setCurrentSlideIndex(aCurrentSlideIndex);

        Intent aIntent = Intents.buildSlideChangedIntent(aCurrentSlideIndex);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onSlidePreview(int aSlideIndex, byte[] aPreview) {
        mSlideShow.setSlidePreview(aSlideIndex, aPreview);

        Intent aIntent = Intents.buildSlidePreviewIntent(aSlideIndex);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onSlideNotes(int aSlideIndex, String aNotes) {
        mSlideShow.setSlideNotes(aSlideIndex, aNotes);

        Intent aIntent = Intents.buildSlideNotesIntent(aSlideIndex);
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onDestroy() {
        stopSearch();

        mThread.interrupt();
        mThread = null;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
