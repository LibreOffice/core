/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.fragment;

import java.util.concurrent.TimeUnit;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.widget.TimePicker;

import com.actionbarsherlock.app.SherlockDialogFragment;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.util.Intents;

public class TimerEditingDialog extends SherlockDialogFragment implements TimePickerDialog.OnTimeSetListener, DialogInterface.OnClickListener, DialogInterface.OnShowListener, View.OnClickListener {
    public static final String TAG = "TIMER_EDITING";

    private static final boolean IS_24_HOUR_VIEW = true;

    private int mMinutes;

    public static TimerEditingDialog newInstance(int aMinutes) {
        TimerEditingDialog aDialog = new TimerEditingDialog();

        aDialog.setArguments(buildArguments(aMinutes));

        return aDialog;
    }

    private static Bundle buildArguments(int aMinutes) {
        Bundle aArguments = new Bundle();

        aArguments.putInt("MINUTES", aMinutes);

        return aArguments;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mMinutes = getArguments().getInt("MINUTES");
    }

    @Override
    public Dialog onCreateDialog(Bundle aSavedInstanceState) {
        TimePickerDialog aDialog = new TimePickerDialog(getActivity(), this,
            getHours(mMinutes), getMinutes(mMinutes), IS_24_HOUR_VIEW);

        aDialog.setTitle(R.string.title_timer);

        aDialog.setButton(DialogInterface.BUTTON_POSITIVE, getString(R.string.button_save), this);
        aDialog.setButton(DialogInterface.BUTTON_NEGATIVE, getString(R.string.button_cancel), this);
        aDialog.setButton(DialogInterface.BUTTON_NEUTRAL, getString(R.string.button_reset), this);

        aDialog.setOnShowListener(this);

        return aDialog;
    }

    private int getMinutes(int aMinutes) {
        return (int) (aMinutes - getHours(aMinutes) * TimeUnit.HOURS.toMinutes(1));
    }

    private int getHours(int aMinutes) {
        return (int) (aMinutes / TimeUnit.HOURS.toMinutes(1));
    }

    @Override
    public void onTimeSet(TimePicker aTimePicker, int aHour, int aMinute) {
        mMinutes = getMinutes(aHour, aMinute);
    }

    private int getMinutes(int aHours, int aMinutes) {
        return (int) (TimeUnit.HOURS.toMinutes(aHours) + aMinutes);
    }

    @Override
    public void onClick(DialogInterface aDialogInterface, int aButtonId) {
        getTimePickerDialog().onClick(aDialogInterface, aButtonId);

        switch (aButtonId) {
            case DialogInterface.BUTTON_NEGATIVE:
                resumeTimer();
                break;

            case DialogInterface.BUTTON_POSITIVE:
                changeTimer();
                break;

            default:
                break;
        }
    }

    private TimePickerDialog getTimePickerDialog() {
        return (TimePickerDialog) getDialog();
    }

    private void resumeTimer() {
        Intent aIntent = Intents.buildTimerResumedIntent();
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(aIntent);
    }

    private void changeTimer() {
        Intent aIntent = Intents.buildTimerChangedIntent(mMinutes);
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(aIntent);
    }

    @Override
    public void onShow(DialogInterface dialogInterface) {
        setUpNeutralButton();
    }

    private void setUpNeutralButton() {
        TimePickerDialog aDialog = (TimePickerDialog) getDialog();

        aDialog.getButton(DialogInterface.BUTTON_NEUTRAL).setOnClickListener(this);
    }

    @Override
    public void onClick(View aView) {
        // Requires the additional listener to not close the dialog.

        resetTime();
    }

    private void resetTime() {
        TimePickerDialog aDialog = (TimePickerDialog) getDialog();

        aDialog.updateTime(0, 0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
