/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.activity;

import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.view.MenuItem;

import org.libreoffice.impressremote.R;

public class SettingsActivity extends PreferenceActivity {
    @Override
    protected void onCreate(Bundle aSavedInstanceState) {
        super.onCreate(aSavedInstanceState);

        setUpPreferences();
    }

    private void setUpPreferences() {
        // This action is deprecated
        // but we still need to target pre-Honeycomb devices.

        // TODO: try to use a fragment depending on platform version

        addPreferencesFromResource(R.xml.preferences);
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
