/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.fragment;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.ViewAnimator;

import org.libreoffice.impressremote.util.Fragments;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;
import org.libreoffice.impressremote.util.SavedStates;

public class ComputerConnectionFragment extends Fragment implements ServiceConnection {
    public static enum Result {
        CONNECTED, NOT_CONNECTED
    }

    private Result mResult = Result.NOT_CONNECTED;

    private CommunicationService mCommunicationService;
    private BroadcastReceiver mIntentsReceiver;

    public static ComputerConnectionFragment newInstance(Server aComputer) {
        ComputerConnectionFragment aFragment = new ComputerConnectionFragment();

        aFragment.setArguments(buildArguments(aComputer));

        return aFragment;
    }

    private static Bundle buildArguments(Server aComputer) {
        Bundle aArguments = new Bundle();

        aArguments.putParcelable(Fragments.Arguments.COMPUTER, aComputer);

        return aArguments;
    }

    @Override
    public void onCreate(Bundle aSavedInstance) {
        super.onCreate(aSavedInstance);

        setUpActionBarMenu();
    }

    private void setUpActionBarMenu() {
        setHasOptionsMenu(true);
    }

    @Override
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstance) {
        return aInflater.inflate(R.layout.fragment_computer_connection, aContainer, false);
    }

    @Override
    public void onViewStateRestored(Bundle aSavedInstanceState) {
        super.onViewStateRestored(aSavedInstanceState);

        if (!isSavedInstanceValid(aSavedInstanceState)) {
            return;
        }

        loadCurrentView(aSavedInstanceState);
        loadPin(aSavedInstanceState);
        loadErrorMessage(aSavedInstanceState);
    }

    private boolean isSavedInstanceValid(Bundle aSavedInstanceState) {
        return aSavedInstanceState != null;
    }

    private void loadCurrentView(Bundle aSavedInstanceState) {
        int aCurrentViewId = aSavedInstanceState.getInt(SavedStates.Keys.CURRENT_VIEW_ID);

        setCurrentView(aCurrentViewId);
    }

    private void setCurrentView(int aViewId) {
        ViewAnimator aViewAnimator = getViewAnimator();
        View aView = getView().findViewById(aViewId);

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aView));
    }

    private ViewAnimator getViewAnimator() {
        return (ViewAnimator) getView().findViewById(R.id.view_animator);
    }

    private void loadPin(Bundle aSavedInstanceState) {
        String aPin = aSavedInstanceState.getString(SavedStates.Keys.PIN);

        getPinTextView().setText(aPin);
    }

    private TextView getPinTextView() {
        return (TextView) getView().findViewById(R.id.text_pin);
    }

    private void loadErrorMessage(Bundle aSavedInstanceState) {
        String aErrorMessage = aSavedInstanceState.getString(SavedStates.Keys.ERROR_MESSAGE);

        getSecondaryErrorMessageTextView().setText(aErrorMessage);
    }

    private TextView getSecondaryErrorMessageTextView() {
        return (TextView) getView().findViewById(R.id.text_secondary_error_message);
    }

    @Override
    public void onActivityCreated(Bundle aSavedInstanceState) {
        super.onActivityCreated(aSavedInstanceState);

        bindService();
    }

    private void bindService() {
        Intent aServiceIntent = Intents.buildCommunicationServiceIntent(getActivity());
        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.ServiceBinder aServiceBinder = (CommunicationService.ServiceBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        connectComputer();
    }

    private void connectComputer() {
        if (!isComputerConnectionRequired()) {
            return;
        }

        mCommunicationService.connectServer(getComputer());
    }

    private boolean isComputerConnectionRequired() {
        return getViewAnimator().getCurrentView().getId() == R.id.progress_bar;
    }

    private Server getComputer() {
        return getArguments().getParcelable(Fragments.Arguments.COMPUTER);
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }

    @Override
    public void onStart() {
        super.onStart();

        registerIntentsReceiver();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = buildIntentsReceiverFilter();

        getBroadcastManager().registerReceiver(mIntentsReceiver, aIntentFilter);
    }

    private static class IntentsReceiver extends BroadcastReceiver {
        private final ComputerConnectionFragment mComputerConnectionFragment;

        public IntentsReceiver(ComputerConnectionFragment aComputerConnectionFragment) {
            mComputerConnectionFragment = aComputerConnectionFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.PAIRING_VALIDATION.equals(aIntent.getAction())) {
                String aPin = aIntent.getStringExtra(Intents.Extras.PIN);

                mComputerConnectionFragment.setUpPinValidationInstructions(aPin);
                mComputerConnectionFragment.refreshActionBarMenu();

                return;
            }

            if (Intents.Actions.PAIRING_SUCCESSFUL.equals(aIntent.getAction())) {
                mComputerConnectionFragment.setUpPresentation();
                mComputerConnectionFragment.refreshActionBarMenu();

                return;
            }

            if (Intents.Actions.CONNECTION_FAILED.equals(aIntent.getAction())) {
                mComputerConnectionFragment.setUpErrorMessage();
                mComputerConnectionFragment.refreshActionBarMenu();
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.PAIRING_VALIDATION);
        aIntentFilter.addAction(Intents.Actions.PAIRING_SUCCESSFUL);
        aIntentFilter.addAction(Intents.Actions.CONNECTION_FAILED);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    private void setUpPinValidationInstructions(String aPin) {
        getPinTextView().setText(aPin);

        setCurrentView(R.id.layout_pin_validation);
    }

    private void setUpPresentation() {
        mResult = Result.CONNECTED;

        Intent aIntent = Intents.buildSlideShowIntent(getActivity());
        startActivity(aIntent);

        getActivity().finish();
    }

    private void setUpErrorMessage() {
        TextView aSecondaryMessageTextView = getSecondaryErrorMessageTextView();
        aSecondaryMessageTextView.setText(buildSecondaryErrorMessage());

        setCurrentView(R.id.layout_error_message);
    }

    private String buildSecondaryErrorMessage() {
        switch (getComputer().getProtocol()) {
            case BLUETOOTH:
                return getString(R.string.message_impress_pairing_check);

            case TCP:
                return getString(R.string.message_impress_wifi_enabling);

            default:
                return "";
        }
    }

    private void refreshActionBarMenu() {
        getActivity().supportInvalidateOptionsMenu();
    }

    @Override
    public void onCreateOptionsMenu(Menu aMenu, MenuInflater aMenuInflater) {
        if (!shouldActionBarMenuBeDisplayed()) {
            aMenu.clear();
            return;
        }

        aMenuInflater.inflate(R.menu.menu_action_bar_computer_connection, aMenu);
    }

    private boolean shouldActionBarMenuBeDisplayed() {
        if (getView() == null) {
            return false;
        }

        return getCurrentViewId() == R.id.layout_error_message;
    }

    private int getCurrentViewId() {
        return getViewAnimator().getCurrentView().getId();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_reconnect:
                setCurrentView(R.id.progress_bar);
                connectComputer();
                refreshActionBarMenu();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    @Override
    public void onStop() {
        super.onStop();

        unregisterIntentsReceiver();
    }

    private void unregisterIntentsReceiver() {
        try {
            getBroadcastManager().unregisterReceiver(mIntentsReceiver);
        } catch (IllegalArgumentException e) {
            // Receiver not registered.
            // Fixed in Honeycomb: Android’s issue #6191.
        }
    }

    @Override
    public void onSaveInstanceState(Bundle aOutState) {
        super.onSaveInstanceState(aOutState);

        saveCurrentView(aOutState);
        savePin(aOutState);
        saveErrorMessage(aOutState);
    }

    private void saveCurrentView(Bundle aOutState) {
        int aCurrentViewId = getCurrentViewId();

        aOutState.putInt(SavedStates.Keys.CURRENT_VIEW_ID, aCurrentViewId);
    }

    private void savePin(Bundle aOutState) {
        String aPin = getPinTextView().getText().toString();

        aOutState.putString(SavedStates.Keys.PIN, aPin);
    }

    private void saveErrorMessage(Bundle aOutState) {
        String aErrorMessage = getSecondaryErrorMessageTextView().getText().toString();

        aOutState.putString(SavedStates.Keys.ERROR_MESSAGE, aErrorMessage);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        disconnectComputer();

        unbindService();
    }

    private void disconnectComputer() {
        if (!isDisconnectRequired()) {
            return;
        }

        mCommunicationService.disconnectServer();
    }

    private boolean isDisconnectRequired() {
        return mResult == Result.NOT_CONNECTED;
    }

    private void unbindService() {
        getActivity().unbindService(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
