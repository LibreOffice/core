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
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.ViewAnimator;

import com.actionbarsherlock.app.SherlockListFragment;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.adapter.ComputersAdapter;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.communication.CommunicationService;
import org.libreoffice.impressremote.communication.Server;

public class ComputersFragment extends SherlockListFragment implements ServiceConnection, Runnable {
    private static final int SHOWING_PROGRESS_MESSAGE_DELAY_IN_SECONDS = 10;

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
    public View onCreateView(LayoutInflater aInflater, ViewGroup aContainer, Bundle aSavedInstanceState) {
        return aInflater.inflate(R.layout.fragment_computers_list, aContainer, false);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mType = (Type) getArguments().getSerializable("TYPE");

        setUpActionBar();
    }

    private void setUpActionBar() {
        setHasOptionsMenu(true);
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

        mCommunicationService.startSearch();

        loadComputers();
    }

    private void loadComputers() {
        if (!isServiceBound()) {
            return;
        }

        if (getComputers().isEmpty()) {
            hideComputersList();
            setUpProgressMessage();
            tearDownComputersAdapter();
        }
        else {
            setUpComputersAdapter();
            fillComputersAdapter();
            showComputersList();
        }
    }

    private boolean isServiceBound() {
        return mCommunicationService != null;
    }

    private void hideComputersList() {
        ViewAnimator aViewAnimator = getViewAnimator();
        ViewGroup aProgressBarLayout = getProgressBarLayout();

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aProgressBarLayout));
    }

    private ViewAnimator getViewAnimator() {
        return (ViewAnimator) getView().findViewById(R.id.view_animator);
    }

    private ViewGroup getProgressBarLayout() {
        return (ViewGroup) getView().findViewById(R.id.container_progress);
    }

    private void setUpProgressMessage() {
        new Handler().postDelayed(this, TimeUnit.SECONDS.toMillis(SHOWING_PROGRESS_MESSAGE_DELAY_IN_SECONDS));
    }

    @Override
    public void run() {
        if (!isAdded()) {
            return;
        }

        if (isShowingProgressMessageRequired()) {
            showProgressMessage();
        }
    }

    private boolean isShowingProgressMessageRequired() {
        return getProgressMessageView().getVisibility() == View.INVISIBLE;
    }

    private TextView getProgressMessageView() {
        return (TextView) getView().findViewById(R.id.text_progress_message);
    }

    private void showProgressMessage() {
        TextView aProgressMessageView = getProgressMessageView();
        Animation aFadeInAnimation = AnimationUtils.loadAnimation(getActivity(), android.R.anim.fade_in);

        aProgressMessageView.setText(getProgressMessage());

        aProgressMessageView.startAnimation(aFadeInAnimation);
        aProgressMessageView.setVisibility(View.VISIBLE);
    }

    private String getProgressMessage() {
        switch (mType) {
            case WIFI:
                return getString(R.string.message_search_wifi);

            case BLUETOOTH:
                return getString(R.string.message_search_bluetooth);

            default:
                return "";
        }
    }

    private void tearDownComputersAdapter() {
        getComputesList().setAdapter(null);
    }

    private ListView getComputesList() {
        return (ListView) getView().findViewById(android.R.id.list);
    }

    private void showComputersList() {
        ViewAnimator aViewAnimator = getViewAnimator();
        ListView aComputersList= getComputesList();

        aViewAnimator.setDisplayedChild(aViewAnimator.indexOfChild(aComputersList));
    }

    private void setUpComputersAdapter() {
        if (isComputersAdapterExist()) {
            return;
        }

        setListAdapter(new ComputersAdapter(getActivity()));
    }

    private boolean isComputersAdapterExist() {
        return getComputersAdapter() != null;
    }

    private ComputersAdapter getComputersAdapter() {
        return (ComputersAdapter) getListAdapter();
    }

    private void fillComputersAdapter() {
        getComputersAdapter().clear();
        getComputersAdapter().add(getComputers());
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

    @Override
    public void onServiceDisconnected(ComponentName aComponentName) {
        mCommunicationService = null;
    }

    @Override
    public void onResume() {
        super.onResume();

        registerIntentsReceiver();

        loadComputers();
    }

    private void registerIntentsReceiver() {
        mIntentsReceiver = new IntentsReceiver(this);
        IntentFilter aIntentFilter = buildIntentsReceiverFilter();

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
            }
        }
    }

    private IntentFilter buildIntentsReceiverFilter() {
        IntentFilter aIntentFilter = new IntentFilter();
        aIntentFilter.addAction(Intents.Actions.SERVERS_LIST_CHANGED);

        return aIntentFilter;
    }

    private LocalBroadcastManager getBroadcastManager() {
        Context aContext = getActivity().getApplicationContext();

        return LocalBroadcastManager.getInstance(aContext);
    }

    @Override
    public void onPause() {
        super.onPause();

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
    public void onListItemClick(ListView aListView, View aView, int aPosition, long aId) {
        Server aComputer = getComputersAdapter().getItem(aPosition);

        Intent aIntent = Intents.buildComputerConnectionIntent(getActivity(), aComputer);
        startActivity(aIntent);
    }

    @Override
    public void onStart() {
        super.onStart();

        setUpContextMenu();
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
