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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

public class AboutDialogBuilder extends AlertDialog.Builder {

    private TextView mVersionLabel;

    public AboutDialogBuilder(Context aContext) {
        super(aContext);

        LayoutInflater aInflater = (LayoutInflater) aContext
                        .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View aLayout = aInflater.inflate(R.layout.dialog_about, null);

        setView(aLayout);

        //        setTitle(R.string.about);
        setPositiveButton(
                        aContext.getResources().getString(R.string.about_close),
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                dialog.dismiss();
                            }
                        });

        mVersionLabel = (TextView) aLayout.findViewById(R.id.about_version);

        try {
            PackageInfo aInfo = aContext.getPackageManager().getPackageInfo(
                            aContext.getPackageName(), 0);
            String aVersionString = MessageFormat.format(
                            aContext.getResources().getString(
                                            R.string.about_versionstring),
                            aInfo.versionName, aInfo.versionCode);
            mVersionLabel.setText(aVersionString);
        } catch (NameNotFoundException e) {
            // Cannot logically happen
        }

    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */