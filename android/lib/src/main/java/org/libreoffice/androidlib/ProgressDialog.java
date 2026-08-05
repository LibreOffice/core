/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.app.Activity;
import android.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

/**
 * Class to handle progress when loading, saving, or handling other time intensive operations.
 *
 * The dialog and its views belong to the main thread. Every method here hops onto that thread
 * first, so a call from any thread is safe.
 */
public class ProgressDialog {
    /** For the inflater. */
    Activity mActivity;

    /** The underlying dialog. */
    AlertDialog mProgressDialog = null;

    /** Various elements of the dialog. */
    TextView mTextView;
    ProgressBar mIndeterminateProgress;
    ProgressBar mDeterminateProgress;

    /** Current progress of the determinate progress bar. */
    int mProgress = 0;

    public ProgressDialog(Activity activity) {
        mActivity = activity;
    }

    /** Set up the dialog if it does not exist; but don't show it yet. */
    private void create() {
        if (mProgressDialog != null)
            return;

        LayoutInflater inflater = mActivity.getLayoutInflater();

        View loadingView = inflater.inflate(R.layout.lolib_dialog_loading, null);

        mTextView = loadingView.findViewById(R.id.progress_dialog_text);
        mIndeterminateProgress = loadingView.findViewById(R.id.progress_indeterminate);
        mDeterminateProgress = loadingView.findViewById(R.id.progress_determinate);

        mProgressDialog = new AlertDialog.Builder(mActivity)
            .setView(loadingView)
            .setCancelable(false)
            .create();
    }

    /** Set the progress to indeterminate state. */
    public void indeterminate(int messageId) {
        mActivity.runOnUiThread(() -> {
            create();

            mIndeterminateProgress.setVisibility(View.VISIBLE);
            mDeterminateProgress.setVisibility(View.INVISIBLE);
            mTextView.setText(mActivity.getText(messageId));

            mProgressDialog.show();
        });
    }

    /** Set the progress to indeterminate state. */
    public void determinate(int messageId) {
        mActivity.runOnUiThread(() -> {
            create();

            mIndeterminateProgress.setVisibility(View.INVISIBLE);
            mDeterminateProgress.setVisibility(View.VISIBLE);
            mTextView.setText(mActivity.getText(messageId));

            mProgress = 0;
            mDeterminateProgress.setProgress(mProgress);

            mProgressDialog.show();
        });
    }

    /** Replace the message shown above the progress bar. */
    public void message(String message) {
        mActivity.runOnUiThread(() -> {
            if (mProgressDialog == null)
                return;

            mTextView.setText(message);
        });
    }

    /** Update the progress value. */
    public void determinateProgress(int progress) {
        mActivity.runOnUiThread(() -> {
            if (mProgressDialog == null)
                return;

            if (mProgress > progress)
                return;

            mProgress = progress;
            mDeterminateProgress.setProgress(mProgress);
        });
    }

    /** Kill the dialog. */
    public void dismiss() {
        mActivity.runOnUiThread(() -> {
            if (mProgressDialog == null)
                return;

            mProgressDialog.dismiss();
            mProgressDialog = null;
        });
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
