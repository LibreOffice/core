/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import android.os.Bundle;
import android.support.v4.app.Fragment;

import com.actionbarsherlock.app.ActionBar;
import com.actionbarsherlock.app.SherlockFragmentActivity;

public class ComputersActivity extends SherlockFragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpTabs();
    }

    private void setUpTabs() {
        ActionBar aActionBar = getSupportActionBar();

        aActionBar.addTab(buildBluetoothServersTab());
        aActionBar.addTab(buildWiFiServersTab());
    }

    private ActionBar.Tab buildBluetoothServersTab() {
        ComputersFragment aFragment = ComputersFragment.newInstance(
            ComputersFragment.Type.BLUETOOTH);

        return buildActionBarTab(aFragment, R.string.title_bluetooth);
    }

    private ActionBar.Tab buildActionBarTab(Fragment aFragment, int aTitleResourceId) {
        ActionBar.Tab aTab = getSupportActionBar().newTab();

        aTab.setTabListener(new ActionBarTabListener(aFragment));
        aTab.setText(aTitleResourceId);

        return aTab;
    }

    private ActionBar.Tab buildWiFiServersTab() {
        ComputersFragment aFragment = ComputersFragment.newInstance(
            ComputersFragment.Type.WIFI);

        return buildActionBarTab(aFragment, R.string.title_wifi);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
