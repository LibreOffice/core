/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.widget.TextView;

public class PairingActivity extends Activity {

    private TextView mPinText;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pairing);

        mPinText = (TextView) findViewById(R.id.pairing_pin);

        IntentFilter aFilter = new IntentFilter(
                        CommunicationService.MSG_PAIRING_STARTED);
        aFilter.addAction(CommunicationService.MSG_PAIRING_SUCCESSFUL);
        LocalBroadcastManager.getInstance(this).registerReceiver(mListener,
                        aFilter);

        //        mBluetoothContainer = findViewById(R.id.selector_container_bluetooth);
        //        mBluetoothList = (LinearLayout) findViewById(R.id.selector_list_bluetooth);
        //        mNetworkContainer = findViewById(R.id.selector_container_network);
        //        mNetworkList = (LinearLayout) findViewById(R.id.selector_list_network);
        //        mNoServerLabel = (TextView) findViewById(R.id.selector_label_none);
        //
        //        refreshLists();
    }

    private BroadcastReceiver mListener = new BroadcastReceiver() {

        @Override
        public void onReceive(Context aContext, Intent aIntent) {
            if (aIntent.getAction().equals(
                            CommunicationService.MSG_PAIRING_STARTED)) {
                String aPin = aIntent.getStringExtra("PIN");
                mPinText.setText(aPin);
                //                refreshLists();
            } else if (aIntent.getAction().equals(
                            CommunicationService.MSG_PAIRING_SUCCESSFUL)) {
                mPinText.setText("Paired!");
            }

        }
    };

}
