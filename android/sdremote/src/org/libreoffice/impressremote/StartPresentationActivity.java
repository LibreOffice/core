/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.view.View.OnClickListener;

import com.actionbarsherlock.app.SherlockActivity;

public class StartPresentationActivity extends SherlockActivity {
    private CommunicationService mCommunicationService = null;
    private ActivityChangeBroadcastProcessor mBroadcastProcessor;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_startpresentation);
        bindService(new Intent(this, CommunicationService.class), mConnection,
                        Context.BIND_IMPORTANT);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        IntentFilter aFilter = new IntentFilter(
                        Intents.Actions.SLIDE_SHOW_STARTED);

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);
        mBroadcastProcessor.addToFilter(aFilter);

        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        findViewById(R.id.startpresentation_button).setOnClickListener(
                        mClickListener);
    }

    @Override
    public void onBackPressed() {
        Intent aIntent = new Intent(this, SelectorActivity.class);
        aIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(aIntent);
        mCommunicationService.disconnect();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbindService(mConnection);
        LocalBroadcastManager.getInstance(this).unregisterReceiver(mListener);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName aClassName,
                        IBinder aService) {
            mCommunicationService = ((CommunicationService.CBinder) aService)
                            .getService();

        }

        @Override
        public void onServiceDisconnected(ComponentName aClassName) {
            mCommunicationService = null;
        }
    };

    private OnClickListener mClickListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            if (mCommunicationService != null) {
                mCommunicationService.getTransmitter().startPresentation();
            }
        }
    };

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            mBroadcastProcessor.onReceive(aContext, aIntent);
        }
    };

    @Override
    public boolean onOptionsItemSelected(
            com.actionbarsherlock.view.MenuItem item) {
        switch (item.getItemId()) {
        case android.R.id.home:
            onBackPressed();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
