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
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import org.libreoffice.impressremote.util.BluetoothOperator;
import org.libreoffice.impressremote.util.Intents;

public class CommunicationService extends Service implements Runnable, MessagesListener, Timer.TimerListener {
    public static final class ServiceBinder extends Binder {
        private final CommunicationService mCommunicationService;

        public ServiceBinder(CommunicationService aCommunicationService) {
            mCommunicationService = aCommunicationService;
        }

        public CommunicationService getService() {
            return mCommunicationService;
        }
    }

    private IBinder mBinder;

    private ServersManager mServersManager;

    private BluetoothOperator.State mBluetoothState;

    private Timer mTimer;
    private SlideShow mSlideShow;

    private Server mServer;
    private ServerConnection mServerConnection;

    private MessagesReceiver mMessagesReceiver;
    private CommandsTransmitter mCommandsTransmitter;

    @Override
    public void onCreate() {
        mBinder = new ServiceBinder(this);

        mServersManager = new ServersManager(this);

        saveBluetoothState();
        enableBluetooth();

        mTimer = new Timer(this);
        mSlideShow = new SlideShow(mTimer);
    }

    private void saveBluetoothState() {
        mBluetoothState = BluetoothOperator.getState();
    }

    private void enableBluetooth() {
        BluetoothOperator.enable();
    }

    @Override
    public IBinder onBind(Intent aIntent) {
        return mBinder;
    }

    public void startServersSearch() {
        mServersManager.startServersSearch();
    }

    public void stopServersSearch() {
        mServersManager.stopServersSearch();
    }

    public void addServer(String aAddress, String aName) {
        mServersManager.addTcpServer(aAddress, aName);
    }

    public void removeServer(Server aServer) {
        mServersManager.removeServer(aServer);
    }

    public List<Server> getServers() {
        return mServersManager.getServers();
    }

    public void connectServer(Server aServer) {
        mServer = aServer;

        Thread aConnectionThread = new Thread(this);
        aConnectionThread.start();
    }

    @Override
    public void run() {
        try {
            disconnectServer();
            connectServer();
        }
        catch (RuntimeException e) {
            sendConnectionFailedMessage();
        }
    }

    private void connectServer() {
        mServerConnection = buildServerConnection();
        mServerConnection.open();

        mMessagesReceiver = new MessagesReceiver(mServerConnection, this);
        mCommandsTransmitter = new CommandsTransmitter(mServerConnection);

        if (PairingProvider.isPairingNecessary(mServer)) {
            pair();
        }
    }

    private ServerConnection buildServerConnection() {
        switch (mServer.getProtocol()) {
            case TCP:
                return new TcpServerConnection(mServer);

            case BLUETOOTH:
                return new BluetoothServerConnection(mServer);

            default:
                throw new RuntimeException("Unknown desired protocol.");
        }
    }

    private void pair() {
        String aPairingDeviceName = PairingProvider.getPairingDeviceName(this);
        String aPairingPin = PairingProvider.getPairingPin(this, mServer);

        mCommandsTransmitter.pair(aPairingDeviceName, aPairingPin);
    }

    private void sendConnectionFailedMessage() {
        Intent aIntent = Intents.buildConnectionFailedIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    public void disconnectServer() {
        if (!isServerConnectionAvailable()) {
            return;
        }

        mServerConnection.close();
    }

    private boolean isServerConnectionAvailable() {
        return mServerConnection != null;
    }

    public CommandsTransmitter getTransmitter() {
        return mCommandsTransmitter;
    }

    public SlideShow getSlideShow() {
        return mSlideShow;
    }

    @Override
    public void onPinValidation() {
        String aPin = PairingProvider.getPairingPin(this, mServer);

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
        mSlideShow = new SlideShow(mTimer);
        mSlideShow.setSlidesCount(aSlidesCount);

        Intent aIntent = Intents.buildSlideShowRunningIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);

        onSlideChanged(aCurrentSlideIndex);
    }

    @Override
    public void onSlideShowFinish() {
        mSlideShow = new SlideShow(mTimer);

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
        mSlideShow.setSlidePreviewBytes(aSlideIndex, aPreview);

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
    public void onTimerUpdated() {
        Intent aIntent = Intents.buildTimerUpdatedIntent();
        LocalBroadcastManager.getInstance(this).sendBroadcast(aIntent);
    }

    @Override
    public void onDestroy() {
        stopServersSearch();
        disconnectServer();

        restoreBluetoothState();
    }

    private void restoreBluetoothState() {
        if (!BluetoothOperator.isStateValid(mBluetoothState)) {
            return;
        }

        if (mBluetoothState.wasBluetoothEnabled()) {
            return;
        }

        disableBluetooth();
    }

    private void disableBluetooth() {
        BluetoothOperator.disable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
