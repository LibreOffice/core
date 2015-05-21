package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.view.View;
import android.widget.TextView;

import java.io.File;

/**
 * The about dialog.
 */
public class LOAbout {

    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";
    private final Activity mActivity;

    private boolean mNewActivity;

    public LOAbout(Activity activity, boolean newActivity) {
        mActivity = activity;
        mNewActivity = newActivity;
    }

    private void loadFromAbout(String input) {
        if (mNewActivity) {
            Intent i = new Intent(Intent.ACTION_VIEW, Uri.fromFile(new File(input)));
            String packageName = mActivity.getApplicationContext().getPackageName();
            ComponentName componentName = new ComponentName(packageName, LibreOfficeMainActivity.class.getName());
            i.setComponent(componentName);
            mActivity.startActivity(i);
        } else {
            LOKitShell.sendCloseEvent();
            LOKitShell.sendLoadEvent(input);
        }
    }

    public void showAbout() {
        // Inflate the about message contents
        View messageView = mActivity.getLayoutInflater().inflate(R.layout.about, null, false);

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
            String versionName = mActivity.getPackageManager().getPackageInfo(mActivity.getPackageName(), 0).versionName;
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
                throw new NameNotFoundException();
        }
        catch (NameNotFoundException e)
        {
            versionView.setText("");
            vendorView.setText("");
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
        builder.setIcon(R.drawable.lo_icon);
        builder.setTitle(R.string.app_name);
        builder.setView(messageView);

        builder.setNegativeButton(R.string.about_license, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                loadFromAbout("/assets/license.txt");
                dialog.dismiss();
            }
        });

        builder.setPositiveButton(R.string.about_notice, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                loadFromAbout("/assets/notice.txt");
                dialog.dismiss();
            }
        });

        builder.setNeutralButton(R.string.about_moreinfo, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                loadFromAbout(DEFAULT_DOC_PATH);
                dialog.dismiss();
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
