package org.libreoffice;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
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

import java.util.ArrayList;
import java.util.List;

public abstract class LOAbout extends Activity {

    private static final String DEFAULT_DOC_PATH = "/assets/example.odt";

    protected void showAbout() {
        // Inflate the about message contents
        View messageView = getLayoutInflater().inflate(R.layout.about, null, false);

        // When linking text, force to always use default color. This works
        // around a pressed color state bug.
        TextView textView = (TextView) messageView.findViewById(R.id.about_credits);
        int defaultColor = textView.getTextColors().getDefaultColor();
        textView.setTextColor(defaultColor);

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setIcon(R.drawable.lo_icon);
        builder.setTitle(R.string.app_name);
        builder.setView(messageView);

        builder.setNegativeButton(R.string.about_license, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                LOKitShell.sendEvent(LOEventFactory.close());
                LOKitShell.sendEvent(LOEventFactory.load("/assets/license.txt"));
                dialog.dismiss();
            }
        });

        builder.setPositiveButton(R.string.about_notice, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                LOKitShell.sendEvent(LOEventFactory.close());
                LOKitShell.sendEvent(LOEventFactory.load("/assets/notice.txt"));
                dialog.dismiss();
            }
        });

        builder.setNeutralButton(R.string.about_moreinfo, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int id) {
                LOKitShell.sendEvent(LOEventFactory.close());
                LOKitShell.sendEvent(LOEventFactory.load(DEFAULT_DOC_PATH));
                dialog.dismiss();
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
