/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.activity;

import android.content.ContentResolver;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.view.MenuItem;
import android.webkit.WebView;

import org.libreoffice.impressremote.R;

public class LicensesActivity extends ActionBarActivity {
    private static final String SCHEME = ContentResolver.SCHEME_FILE;
    private static final String AUTHORITY = "android_asset";
    private static final String PATH = "licenses.html";

    @Override
    protected void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);
        setContentView(R.layout.activity_licenses);

        setUpHomeButton();
        setUpContent();
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpContent() {
        getLicensesView().loadUrl(buildLicensesUri());
    }

    private WebView getLicensesView() {
        return (WebView) findViewById(R.id.view_licenses);
    }

    private String buildLicensesUri() {
        return String.format("%s:///%s/%s", SCHEME, AUTHORITY, PATH);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case android.R.id.home:
                navigateUp();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void navigateUp() {
        finish();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
