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
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import android.text.Html;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.widget.TextView;

public class AboutDialogFragment extends DialogFragment {

    @NonNull @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {

        @SuppressLint("InflateParams") //suppressed because the view will be placed in a dialog
        View messageView = getActivity().getLayoutInflater().inflate(R.layout.about, null, false);

        // When linking text, force to always use default color. This works
        // around a pressed color state bug.
        TextView textView = messageView.findViewById(R.id.about_credits);
        int defaultColor = textView.getTextColors().getDefaultColor();
        textView.setTextColor(defaultColor);

        // Take care of placeholders and set text in version and vendor text views.
        try
        {
            String versionName = getActivity().getPackageManager()
                    .getPackageInfo(getActivity().getPackageName(), 0).versionName;
            String version = String.format(getString(R.string.app_version), versionName, BuildConfig.BUILD_ID_SHORT);
            @SuppressWarnings("deprecation") // since 24 with additional option parameter
            Spanned versionString = Html.fromHtml(version);
            TextView versionView = messageView.findViewById(R.id.about_version);
            versionView.setText(versionString);
            versionView.setMovementMethod(LinkMovementMethod.getInstance());
            TextView vendorView = messageView.findViewById(R.id.about_vendor);
            String vendor = getString(R.string.app_vendor).replace("$VENDOR", BuildConfig.VENDOR);
            vendorView.setText(vendor);
        }
        catch (PackageManager.NameNotFoundException e)
        {
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder .setIcon(R.mipmap.ic_launcher)
                .setTitle(R.string.app_name)
                .setView(messageView)
                .setNegativeButton(R.string.about_license, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        loadFromAbout(R.raw.license);
                        dialog.dismiss();
                    }
                })
                .setPositiveButton(R.string.about_notice, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        loadFromAbout(R.raw.notice);
                        dialog.dismiss();
                    }
                });

        // when privacy policy URL is set (via '--with-privacy-policy-url=<url>' autogen option),
        // add button to open that URL
        final String privacyUrl = BuildConfig.PRIVACY_POLICY_URL;
        if (!privacyUrl.isEmpty() && !privacyUrl.equals("undefined")) {
            builder.setNeutralButton(R.string.about_privacy_policy, (DialogInterface dialog, int id) -> {
                Intent openPrivacyUrlIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(privacyUrl));
                startActivity(openPrivacyUrlIntent);
                dialog.dismiss();
            });
        }

        return builder.create();
    }

    private void loadFromAbout(int resourceId) {
        Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse("android.resource://" + BuildConfig.APPLICATION_ID + "/" + resourceId));
        String packageName = getActivity().getApplicationContext().getPackageName();
        ComponentName componentName = new ComponentName(packageName, LibreOfficeMainActivity.class.getName());
        i.setComponent(componentName);
        getActivity().startActivity(i);
    }
}
