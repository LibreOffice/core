/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import org.libreoffice.impressremote.ActivityChangeBroadcastProcessor;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.SelectorActivity;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import com.actionbarsherlock.app.SherlockActivity;
import com.actionbarsherlock.view.MenuItem;

public class ReconnectionActivity extends SherlockActivity {

    private ActivityChangeBroadcastProcessor mBroadcastProcessor;
    private CommunicationService mCommunicationService;

    // private TextView mCountDownTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_reconnect);
        // mCountDownTextView = (TextView) findViewById(R.id.countDownTV);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        bindService(new Intent(this, CommunicationService.class), mConnection,
                Context.BIND_IMPORTANT);

        final Server desiredServer = getIntent().getParcelableExtra("server");

        IntentFilter aFilter = new IntentFilter();

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);
        mBroadcastProcessor.addToFilter(aFilter);

        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                aFilter);

        getSupportActionBar().setTitle(desiredServer.getName());

        // TODO Connection to desired server
        // Create a countdown clock for 10 seconds, then double the delay
        // with every failure. Until it reaches 1min. Like Gmail retry

    }

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            mBroadcastProcessor.onReceive(aContext, aIntent);
        }
    };

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

    @Override
    public void onBackPressed() {
        Intent aIntent = new Intent(this, SelectorActivity.class);
        aIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(aIntent);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
        case android.R.id.home:
            onBackPressed();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
