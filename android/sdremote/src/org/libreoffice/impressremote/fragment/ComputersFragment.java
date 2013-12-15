/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.fragment;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.ListFragment;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v4.view.MenuItemCompat;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.ViewAnimator;

import org.libreoffice.impressremote.adapter.ComputersAdapter;
import org.libreoffice.impressremote.util.Fragments;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;
import org.libreoffice.impressremote.util.SavedStates;

public class ComputersFragment extends ListFragment implements ServiceConnection, Runnable {
    private static final int SHOWING_PROGRESS_MESSAGE_DELAY_IN_SECONDS = 3;

    public static enum Type {
        WIFI, BLUETOOTH
    }

    private CommunicationService mCommunicationService;
    private BroadcastReceiver mIntentsReceiver;

    public static ComputersFragment newInstance(Type aType) {
        ComputersFragment aFragment = new ComputersFragment();

        aFragment.setArguments(buildArguments(aType));

        return aFragment;
    }

    private static Bundle buildArguments(Type aType) {
        Bundle aArguments = new Bundle();

        aArguments.putSerializable(Fragments.Arguments.TYPE, aType);

        return aArguments;
    }

    @Override
    public void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);

        setHasOptionsMenu(true);
    }

    @Override
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstanceState) {
        return aInflater.inflate(R.layout.fragment_computers_list, aContainer, false);
    }

    @Override
    public void onViewStateRestored(Bundle aSavedInstanceState) {
        super.onViewStateRestored(aSavedInstanceState);

        if (aSavedInstanceState != null
                && aSavedInstanceState.getBoolean(SavedStates.Keys.PROGRESS_MESSAGE)) {
            showProgressMessage();
            showLearnMoreMessage();
        }
    }

    private void showProgressMessage() {
        TextView aProgressMessageView = getProgressMessageView();
        Animation aFadeInAnimation = AnimationUtils.loadAnimation(getActivity(), android.R.anim.fade_in);

        aProgressMessageView.setText(getProgressMessage());

        aProgressMessageView.startAnimation(aFadeInAnimation);
        aProgressMessageView.setVisibility(View.VISIBLE);
    }

    private TextView getProgressMessageView() {
        return (TextView) getView().findViewById(R.id.text_progress_message);
    }

    private void showLearnMoreMessage() {
        TextView learnMoreView = (TextView) getView().findViewById(R.id.text_learn_more);
        Animation aFadeInAnimation = AnimationUtils.loadAnimation(getActivity(), android.R.anim.fade_in);

        learnMoreView.setMovementMethod(LinkMovementMethod.getInstance());

        learnMoreView.startAnimation(aFadeInAnimation);
        learnMoreView.setVisibility(View.VISIBLE);
    }

    private String getProgressMessage() {
        switch (getType()) {
            case WIFI:
                return getString(R.string.message_search_wifi);

            case BLUETOOTH:
                return getString(R.string.message_search_bluetooth);

            default:
                return "";
        }
    }

    private Type getType() {
        return (Type) getArguments().getSerializable(Fragments.Arguments.TYPE);
    }

    @Override
    public void onActivityCreated(Bundle aSavedInstanceState) {
        super.onActivityCreated(aSavedInstanceState);

        Intent aServiceIntent = Intents.buildCommunicationServiceIntent(getActivity());
        // use startService to not kill it on config changes like rotating the screen
        getActivity().startService(aServiceIntent);
        getActivity().bindService(aServiceIntent, this, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onServiceConnected(ComponentName aComponentName, IBinder aBinder) {
        CommunicationService.ServiceBinder aServiceBinder = (CommunicationService.ServiceBinder) aBinder;
        mCommunicationService = aServiceBinder.getService();

        startComputersSearch();
        loadComputers();
    }

    private void startComputersSearch() {
        mCommunicationService.startServersSearch();
    }

    private void loadComputers() {
        if (!isVisible()) {
            return;
        }

        List<Server> computerList = getComputers();
        if (computerList.isEmpty()) {
            setCurrentView(R.id.layout_progress);

            setUpProgressMessage();

            setListAdapter(null);
        } else {
            if (getComputersAdapter() == null) {
                setListAdapter(new ComputersAdapter(getActivity()));
            }

            getComputersAdapter().clear();
            getComputersAdapter().add(computerList);

            setCurrentView(android.R.id.list);
        }
    }

    private List<Server> getComputers() {
        List<Server> aComputers = new ArrayList<Server>();

        for (Server aComputer : mCommunicationService.getServers()) {
            if (isComputerSupportsRequiredType(aComputer)) {
                aComputers.add(aComputer);
            }
        }

        return aComputers;
    }

    private boolean isComputerSupportsRequiredType(Server aComputer) {
        switch (getType()) {
            case WIFI:
                return aComputer.getProtocol() == Server.Protocol.TCP;

            case BLUETOOTH:
                return aComputer.getProtocol() == Server.Protocol.BLUETOOTH;

            default:
                return false;
        }
    }

    private void setCurrentView(int aViewId) {
        ViewAnimator aViewAnimator = (ViewAnimator) getView().findViewById(R.id.view_animator);
        View aView = getView().findViewById(aViewId);

        if (aViewId == aViewAnimator.getCurrentView().getId()) {
            return;
        }

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aView));
    }

    private void setUpProgressMessage() {
        new Handler().postDelayed(this, TimeUnit.SECONDS.toMillis(SHOWING_PROGRESS_MESSAGE_DELAY_IN_SECONDS));
    }

    @Override
    public void run() {
        if (!isVisible()) {
            return;
        }

        if (!isShowingProgressMessageRequired()) {
            return;
        }

        showProgressMessage();
        showLearnMoreMessage();
    }

    private boolean isShowingProgressMessageRequired() {
        return getProgressMessageView().getVisibility() == View.INVISIBLE;
    }

    private ComputersAdapter getComputersAdapter() {
        return (ComputersAdapter) getListAdapter();
    }

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }

    @Override
    public void onStart() {
        super.onStart();

        registerIntentsReceiver();
        setUpContextMenu();

        if (mCommunicationService == null) {
            return;
        }

        startComputersSearch();
        loadComputers();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SERVERS_LIST_CHANGED);
        aIntentFilter.addAction(Intents.Actions.BT_DISCOVERY_CHANGED);

        getBroadcastManager().registerReceiver(mIntentsReceiver, aIntentFilter);
    }

    private static final class IntentsReceiver extends BroadcastReceiver {
        private final ComputersFragment mComputersFragment;

        public IntentsReceiver(ComputersFragment aComputersFragment) {
            mComputersFragment = aComputersFragment;
        }

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (Intents.Actions.SERVERS_LIST_CHANGED.equals(aIntent.getAction())) {
                mComputersFragment.loadComputers();
            } else if (Intents.Actions.BT_DISCOVERY_CHANGED.equals(aIntent.getAction())) {
                ActivityCompat.invalidateOptionsMenu(mComputersFragment.getActivity());
            }
        }
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    private void setUpContextMenu() {
        registerForContextMenu(getListView());
    }

    @Override
    public void onCreateContextMenu(ContextMenu aMenu, View aView, ContextMenu.ContextMenuInfo aMenuInfo) {
        super.onCreateContextMenu(aMenu, aView, aMenuInfo);

        getActivity().getMenuInflater().inflate(R.menu.menu_context_computers, aMenu);
    }

    @Override
    public boolean onContextItemSelected(android.view.MenuItem aMenuItem) {
        if (!getUserVisibleHint()) {
            // Wrong context menu could be dispatched.
            // Android’s issue #20065.
            return false;
        }

        int aComputerPosition = getListItemPosition(aMenuItem);
        Server aComputer = getComputersAdapter().getItem(aComputerPosition);

        switch (aMenuItem.getItemId()) {
            case R.id.menu_remove_computer:
                removeComputer(aComputer);
                return true;

            default:
                return super.onContextItemSelected(aMenuItem);
        }
    }

    private int getListItemPosition(android.view.MenuItem aMenuItem) {
        AdapterView.AdapterContextMenuInfo aMenuItemInfo = (AdapterView.AdapterContextMenuInfo) aMenuItem.getMenuInfo();

        return aMenuItemInfo.position;
    }

    private void removeComputer(Server aComputer) {
        mCommunicationService.removeServer(aComputer);

        Intent aIntent = Intents.buildServersListChangedIntent();
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(aIntent);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case R.id.menu_add_computer:
                callComputerCreationActivity();
                return true;

            case R.id.menu_start_discovery:
                if (BluetoothAdapter.getDefaultAdapter().startDiscovery()) {
                    MenuItemCompat.setActionView(aMenuItem, R.layout.progress);
                }
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void callComputerCreationActivity() {
        Intent aIntent = Intents.buildComputerCreationIntent(getActivity());
        startActivityForResult(aIntent, Intents.RequestCodes.CREATE_SERVER);
    }

    @Override
    public void onActivityResult(int aRequestCode, int aResultCode, Intent aIntent) {
        if (aRequestCode != Intents.RequestCodes.CREATE_SERVER) {
            return;
        }

        if (aResultCode != Activity.RESULT_OK) {
            return;
        }

        String aServerAddress = aIntent.getStringExtra(Intents.Extras.SERVER_ADDRESS);
        String aServerName = aIntent.getStringExtra(Intents.Extras.SERVER_NAME);

        addComputer(aServerAddress, aServerName);
        loadComputers();
    }

    private void addComputer(String aAddress, String aName) {
        mCommunicationService.addServer(aAddress, aName);

        Intent aIntent = Intents.buildServersListChangedIntent();
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(aIntent);
    }

    @Override
    public void onListItemClick(ListView aListView, View aView, int aPosition, long aId) {
        Server aComputer = getComputersAdapter().getItem(aPosition);

       startConnectionActivity(aComputer);
    }

    private void startConnectionActivity(Server aComputer) {
        Intent aIntent = Intents.buildComputerConnectionIntent(getActivity(), aComputer);
        startActivity(aIntent);
    }

    @Override
    public void onStop() {
        super.onStop();

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

        boolean aProgressMessageDisplayed = !TextUtils.isEmpty(getProgressMessageView().getText().toString());

        aOutState.putBoolean(SavedStates.Keys.PROGRESS_MESSAGE, aProgressMessageDisplayed);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(getActivity().isFinishing()) {
            getActivity().stopService(Intents.buildCommunicationServiceIntent(getActivity()));
        }
        getActivity().unbindService(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
