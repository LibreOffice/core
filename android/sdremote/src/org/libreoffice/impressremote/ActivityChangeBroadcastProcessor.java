/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

/**
 * This class is used to centralise the processing of Broadcasts concerning
 * presentation/connection status changes which result in a change of activity.
 *
 * I.e. this will switch to the correct activity and correctly set up the
 * activity backstack when switching activity.
 *
 * To use create this on activity startup, and pass messages from your
 * BroadcastReceiver's onReceive.
 *
 */
public class ActivityChangeBroadcastProcessor {

    private Activity mActivity;

    public ActivityChangeBroadcastProcessor(Activity aActivity) {
        mActivity = aActivity;
    }

    public void addToFilter(IntentFilter aFilter) {
        aFilter.addAction(Intents.Actions.SLIDE_SHOW_STOPPED);
        aFilter.addAction(Intents.Actions.SLIDE_SHOW_RUNNING);
        aFilter.addAction(Intents.Actions.PAIRING_VALIDATION);
    }

    public void onReceive(Context aContext, Intent aIntent) {
        if (aIntent.getAction().equals(
                        Intents.Actions.SLIDE_SHOW_STOPPED)) {
            Intent nIntent = new Intent(mActivity,
                            StartPresentationActivity.class);
            nIntent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            mActivity.startActivity(nIntent);
        } else if (aIntent
                        .getAction()
                        .equals(Intents.Actions.SLIDE_SHOW_RUNNING)) {
            Intent nIntent = new Intent(mActivity, PresentationActivity.class);
            nIntent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
            mActivity.startActivity(nIntent);
        } else if (aIntent.getAction().equals(
                Intents.Actions.PAIRING_VALIDATION)) {
            Intent nIntent = new Intent(mActivity, PairingActivity.class);
            nIntent.putExtras(aIntent.getExtras()); // Pass on pin and other info.
            mActivity.startActivity(nIntent);
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
