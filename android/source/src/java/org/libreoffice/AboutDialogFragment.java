/*
 *
 *  * This file is part of the LibreOffice project.
 *  * This Source Code Form is subject to the terms of the Mozilla Public
 *  * License, v. 2.0. If a copy of the MPL was not distributed with this
 *  * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

package org.libreoffice;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.DialogFragment;
import android.view.View;
import android.widget.TextView;

import java.io.File;

public class AboutDialogFragment extends DialogFragment {

    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";


    @NonNull @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        @SuppressLint("InflateParams") //suppressed because the view will be placed in a dialog
        View messageView = getActivity().getLayoutInflater().inflate(R.layout.about, null, false);

        // When linking text, force to always use default color. This works
        // around a pressed color state bug.
        TextView textView = (TextView) messageView.findViewById(R.id.about_credits);
        int defaultColor = textView.getTextColors().getDefaultColor();
        textView.setTextColor(defaultColor);

        // Take care of placeholders in the version and vendor text views.
        TextView versionView = (TextView)messageView.findViewById(R.id.about_version);
        TextView vendorView = (TextView)messageView.findViewById(R.id.about_vendor);
        try
        {
            String versionName = getActivity().getPackageManager()
                    .getPackageInfo(getActivity().getPackageName(), 0).versionName;
            String[] tokens = versionName.split("/");
            if (tokens.length == 3)
            {
                String version = versionView.getText().toString();
                String vendor = vendorView.getText().toString();
                version = version.replace("$VERSION", tokens[0]);
                version = version.replace("$BUILDID", tokens[1]);
                vendor = vendor.replace("$VENDOR", tokens[2]);
                versionView.setText(version);
                vendorView.setText(vendor);
            }
            else
                throw new PackageManager.NameNotFoundException();
        }
        catch (PackageManager.NameNotFoundException e)
        {
            versionView.setText("");
            vendorView.setText("");
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder .setIcon(R.drawable.lo_icon)
                .setTitle(R.string.app_name)
                .setView(messageView)
                .setNegativeButton(R.string.about_license, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        loadFromAbout("/assets/license.txt");
                        dialog.dismiss();
                    }
                })
                .setPositiveButton(R.string.about_notice, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        loadFromAbout("/assets/notice.txt");
                        dialog.dismiss();
                    }
                })
                .setNeutralButton(R.string.about_moreinfo, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        loadFromAbout(DEFAULT_DOC_PATH);
                        dialog.dismiss();
                    }
                });

        return builder.create();
    }

    private void loadFromAbout(String input) {
        Intent i = new Intent(Intent.ACTION_VIEW, Uri.fromFile(new File(input)));
        String packageName = getActivity().getApplicationContext().getPackageName();
        ComponentName componentName = new ComponentName(packageName, LibreOfficeMainActivity.class.getName());
        i.setComponent(componentName);
        getActivity().startActivity(i);
    }
}
