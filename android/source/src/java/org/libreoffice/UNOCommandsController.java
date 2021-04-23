/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.EditText;
import android.widget.Scroller;
import android.widget.TextView;

import org.json.JSONException;
import org.json.JSONObject;

import static org.libreoffice.SearchController.addProperty;

class UNOCommandsController implements View.OnClickListener {
    private final LibreOfficeMainActivity mActivity;
    private JSONObject mRootJSON = new JSONObject();


    UNOCommandsController(LibreOfficeMainActivity activity) {
        mActivity = activity;

        activity.findViewById(R.id.button_send_UNO_commands).setOnClickListener(this);
        activity.findViewById(R.id.button_send_UNO_commands_clear).setOnClickListener(this);
        activity.findViewById(R.id.button_send_UNO_commands_show).setOnClickListener(this);
        activity.findViewById(R.id.button_add_property).setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.button_send_UNO_commands) {
            String cmdText = ((EditText) mActivity.findViewById(R.id.UNO_commands_string)).getText().toString();
            LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:" + cmdText, mRootJSON.toString()));
        } else if (view.getId() == R.id.button_add_property) {
            String parentValue = ((EditText) mActivity.findViewById(R.id.UNO_commands_string_parent_value)).getText().toString();
            String type = ((EditText) mActivity.findViewById(R.id.UNO_commands_string_type)).getText().toString();
            String value = ((EditText) mActivity.findViewById(R.id.UNO_commands_string_value)).getText().toString();
            try {
                addProperty(mRootJSON, parentValue, type, value);
            } catch (JSONException e) {
                e.printStackTrace();
            }
            showCommandDialog();
        } else if (view.getId() == R.id.button_send_UNO_commands_clear) {
            mRootJSON = new JSONObject();
            ((EditText) mActivity.findViewById(R.id.UNO_commands_string_parent_value)).setText("");
            ((EditText) mActivity.findViewById(R.id.UNO_commands_string_type)).setText("");
            ((EditText) mActivity.findViewById(R.id.UNO_commands_string_value)).setText("");
            showCommandDialog();
        } else if (view.getId() == R.id.button_send_UNO_commands_show) {
            showCommandDialog();
        }
    }

    private void showCommandDialog() {
        try {
            AlertDialog dialog = new AlertDialog.Builder(mActivity)
                    .setTitle(R.string.current_uno_command)
                    .setMessage(mRootJSON.toString(2))
                    .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    })
                    .setIcon(android.R.drawable.ic_dialog_info)
                    .show();
            TextView textView = dialog.findViewById(android.R.id.message);
            if (textView != null) {
                textView.setScroller(new Scroller(mActivity));
                textView.setVerticalScrollBarEnabled(true);
                textView.setMovementMethod(new ScrollingMovementMethod());
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
}
