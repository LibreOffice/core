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
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.ViewAnimator;

import com.actionbarsherlock.app.SherlockFragment;
import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuInflater;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.util.Fragments;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;
import org.libreoffice.impressremote.util.SavedStates;

public class ComputerConnectionFragment extends SherlockFragment implements ServiceConnection {
    private Server mComputer;

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

        mComputer = getArguments().getParcelable(Fragments.Arguments.COMPUTER);

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

        if (aSavedInstanceState == null) {
            return;
        }

        loadLayout(aSavedInstanceState);
        loadPin(aSavedInstanceState);
        loadErrorMessage(aSavedInstanceState);
    }

    private void loadLayout(Bundle aSavedInstanceState) {
        int aLayoutIndex = aSavedInstanceState.getInt(SavedStates.Keys.LAYOUT_INDEX);

        getViewAnimator().setDisplayedChild(aLayoutIndex);
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
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        bindService();
    }

    private void bindService() {
        Intent aServiceIntent = Intents.buildCommunicationServiceIntent(getActivity());
        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.CBinder aServiceBinder = (CommunicationService.CBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        connectToComputer();
    }

    private void connectToComputer() {
        if (!isServiceBound()) {
            return;
        }

        if (!isComputerConnectionRequired()) {
            return;
        }

        mCommunicationService.connectTo(mComputer);
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }

    private boolean isComputerConnectionRequired() {
        return getViewAnimator().getDisplayedChild() == getViewAnimator().indexOfChild(getProgressBar());
    }

    private ProgressBar getProgressBar() {
        return (ProgressBar) getView().findViewById(R.id.progress_bar);
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

        showPinValidationLayout();
    }

    private void showPinValidationLayout() {
        ViewAnimator aViewAnimator = getViewAnimator();
        LinearLayout aValidationLayout = getPinValidationLayout();

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aValidationLayout));
    }

    private LinearLayout getPinValidationLayout() {
        return (LinearLayout) getView().findViewById(R.id.layout_pin_validation);
    }

    private void setUpPresentation() {
        Intent aIntent = Intents.buildSlideShowIntent(getActivity());
        startActivity(aIntent);

        getActivity().finish();
    }

    private void setUpErrorMessage() {
        TextView aSecondaryMessageTextView = getSecondaryErrorMessageTextView();
        aSecondaryMessageTextView.setText(buildSecondaryErrorMessage());

        showErrorMessageLayout();
    }

    private String buildSecondaryErrorMessage() {
        switch (mComputer.getProtocol()) {
            case BLUETOOTH:
                return getString(R.string.message_impress_pairing_check);

            case TCP:
                return getString(R.string.message_impress_wifi_enabling);

            default:
                return "";
        }
    }

    private void showErrorMessageLayout() {
        ViewAnimator aViewAnimator = getViewAnimator();
        LinearLayout aMessageLayout = getErrorMessageLayout();

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aMessageLayout));
    }

    private LinearLayout getErrorMessageLayout() {
        return (LinearLayout) getView().findViewById(R.id.layout_error_message);
    }

    private void refreshActionBarMenu() {
        getSherlockActivity().supportInvalidateOptionsMenu();
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

        return getViewAnimator().getCurrentView().getId() == R.id.layout_error_message;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_reconnect:
                showProgressBar();
                connectToComputer();
                refreshActionBarMenu();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void showProgressBar() {
        ViewAnimator aViewAnimator = getViewAnimator();
        ProgressBar aProgressBar = getProgressBar();

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aProgressBar));
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

        saveLayout(aOutState);
        savePin(aOutState);
        saveErrorMessage(aOutState);
    }

    private void saveLayout(Bundle aOutState) {
        int aLayoutIndex = getViewAnimator().getDisplayedChild();

        aOutState.putInt(SavedStates.Keys.LAYOUT_INDEX, aLayoutIndex);
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

        unbindService();
    }

    private void unbindService() {
        if (!isServiceBound()) {
            return;
        }

        getActivity().unbindService(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
