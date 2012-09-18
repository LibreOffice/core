/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import java.text.MessageFormat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.widget.TextView;

import com.actionbarsherlock.app.SherlockActivity;

public class PairingActivity extends SherlockActivity {
    private ActivityChangeBroadcastProcessor mBroadcastProcessor;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_pairing);

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);

        IntentFilter aFilter = new IntentFilter();

        mBroadcastProcessor = new ActivityChangeBroadcastProcessor(this);
        mBroadcastProcessor.addToFilter(aFilter);

        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        String aPin = getIntent().getStringExtra("PIN");
        String aServerName = getIntent().getStringExtra("SERVERNAME");

        ((TextView) findViewById(R.id.pairing_pin)).setText(aPin);
        ((TextView) findViewById(R.id.pairing_instruction2_deviceName))
                        .setText(MessageFormat
                                        .format(getResources()
                                                        .getString(R.string.pairing_instructions_2_deviceName),
                                                        aServerName));

        getSupportActionBar().setTitle(aServerName);
    }

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            mBroadcastProcessor.onReceive(aContext, aIntent);
        }
    };

}
