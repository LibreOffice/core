/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.view.View;
import android.widget.EditText;

class UNOCommandsController implements View.OnClickListener {
    private LibreOfficeMainActivity mActivity;


    UNOCommandsController(LibreOfficeMainActivity activity) {
        mActivity = activity;

        activity.findViewById(R.id.button_send_UNO_commands).setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        String cmdText = ((EditText) mActivity.findViewById(R.id.UNO_commands_string)).getText().toString();
        LOKitShell.sendEvent(new LOEvent(LOEvent.UNO_COMMAND, ".uno:"+cmdText));
    }
}