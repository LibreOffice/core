/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidapp;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.text.Html;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;

import org.libreoffice.androidlib.LOActivity;

public class AboutDialogFragment extends DialogFragment {

    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";


    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        @SuppressLint("InflateParams") //suppressed because the view will be placed in a dialog
        View messageView = getActivity().getLayoutInflater().inflate(R.layout.about, null, false);

        // When linking text, force to always use default color. This works
        // around a pressed color state bug.
        TextView textView = messageView.findViewById(R.id.about_credits);
        int defaultColor = textView.getTextColors().getDefaultColor();
        textView.setTextColor(defaultColor);
        textView.setText(getResources().getString(R.string.info_url));

        // Take care of placeholders in the version text view.
        TextView versionView = messageView.findViewById(R.id.about_version);
        try
        {
            String versionName = getActivity().getPackageManager()
                    .getPackageInfo(getActivity().getPackageName(), 0).versionName;
            String versionHash = getString(R.string.version_hash);

            if (!versionHash.isEmpty() && !versionName.isEmpty())
            {
                String version = String.format(versionView.getText().toString().replace("\n", "<br/>"),
                        versionName, "<a href=\"https://github.com/CollaboraOnline/online/commits/" + versionHash + "\">" + versionHash + "</a>");
                @SuppressWarnings("deprecation") // since 24 with additional option parameter
                Spanned versionString = Html.fromHtml(version);
                versionView.setText(versionString);
                versionView.setMovementMethod(LinkMovementMethod.getInstance());
            }
            else
                throw new PackageManager.NameNotFoundException();
        }
        catch (PackageManager.NameNotFoundException e)
        {
            versionView.setText("");
        }

        // Take care of some placeholders
        TextView descriptionView = messageView.findViewById(R.id.about_description);
        String description = descriptionView.getText().toString();
        description = description.replace("$APP_NAME", getResources().getString(R.string.app_name));
        descriptionView.setText(description);

        TextView vendorView = messageView.findViewById(R.id.about_vendor);
        String vendor = vendorView.getText().toString();
        vendor = vendor.replace("$VENDOR", getResources().getString(R.string.vendor));
        vendorView.setText(vendor);

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        int brandIcon = this.getResources().getIdentifier("ic_launcher_brand", "drawable",
                this.getActivity().getPackageName());
        builder .setIcon(BuildConfig.APP_HAS_BRANDING? brandIcon: R.drawable.lo_icon)
                .setTitle(R.string.app_name)
                .setView(messageView)
                .setNegativeButton(R.string.about_license, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(getContext(), ShowHTMLActivity.class);
                        intent.putExtra("path", "license.html");
                        startActivity(intent);
                        dialog.dismiss();                    }
                })
                .setPositiveButton(R.string.about_notice, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(getContext(), ShowHTMLActivity.class);
                        intent.putExtra("path", "notice.txt");
                        startActivity(intent);
                        dialog.dismiss();
                    }
                })
                /* FIXME Fix lauching the example document, this currently crashes.
                .setNeutralButton(R.string.about_moreinfo, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(getContext(), LOActivity.class);
                        intent.putExtra("URI", "file:///android_asset/example.odt");
                        startActivity(intent);
                        dialog.dismiss();
                    }
                })*/;

        return builder.create();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
