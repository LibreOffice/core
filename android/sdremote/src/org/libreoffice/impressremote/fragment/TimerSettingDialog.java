package org.libreoffice.impressremote.fragment;

import java.util.concurrent.TimeUnit;

import android.app.Dialog;
import android.app.TimePickerDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.widget.TimePicker;

import com.actionbarsherlock.app.SherlockDialogFragment;
import org.libreoffice.impressremote.R;
import org.libreoffice.impressremote.util.Intents;

public class TimerSettingDialog extends SherlockDialogFragment implements TimePickerDialog.OnTimeSetListener, DialogInterface.OnClickListener {
    public static final String TAG = "TIMER_SETTING";

    private static final int INITIAL_HOUR = 0;
    private static final int INITIAL_MINUTE = 15;

    private static final boolean IS_24_HOUR_VIEW = true;

    private int mMinutes;

    public static TimerSettingDialog newInstance() {
        return new TimerSettingDialog();
    }

    @Override
    public Dialog onCreateDialog(Bundle aSavedInstanceState) {
        TimePickerDialog aDialog = new TimePickerDialog(getActivity(), this,
            INITIAL_HOUR, INITIAL_MINUTE, IS_24_HOUR_VIEW);

        aDialog.setTitle(R.string.title_timer);

        aDialog.setButton(DialogInterface.BUTTON_POSITIVE, getString(R.string.button_start), this);
        aDialog.setButton(DialogInterface.BUTTON_NEGATIVE, getString(R.string.button_cancel), this);

        return aDialog;
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

        if (aButtonId != DialogInterface.BUTTON_POSITIVE) {
            return;
        }

        startTimer();
    }

    private TimePickerDialog getTimePickerDialog() {
        return (TimePickerDialog) getDialog();
    }

    private void startTimer() {
        Intent aIntent = Intents.buildTimerStartedIntent(mMinutes);
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(aIntent);
    }
}
