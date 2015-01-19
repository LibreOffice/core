package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.ComponentName;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.widget.DrawerLayout;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.content.Intent;
import android.net.Uri;
import android.content.pm.PackageManager.NameNotFoundException;

import java.util.ArrayList;
import java.util.List;
import java.io.File;

public abstract class LOAbout extends Activity {

    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";

    private boolean mNewActivity;

    public LOAbout(boolean newActivity) {
        super();
        mNewActivity = newActivity;
    }

    private void loadFromAbout(String input) {
        if (mNewActivity) {
            Intent i = new Intent(Intent.ACTION_VIEW, Uri.fromFile(new File(input)));
            i.setComponent(new ComponentName(
                        "org.libreoffice",
                        "org.libreoffice.LibreOfficeMainActivity"));
            startActivity(i);
        } else {
            LOKitShell.sendEvent(LOEventFactory.close());
            LOKitShell.sendEvent(LOEventFactory.load(input));
        }
    }

    protected void showAbout() {
        // Inflate the about message contents
        View messageView = getLayoutInflater().inflate(R.layout.about, null, false);

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
            String versionName = getPackageManager().getPackageInfo(getPackageName(), 0).versionName;
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


        AlertDialog.Builder builder = new AlertDialog.Builder(this);
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
