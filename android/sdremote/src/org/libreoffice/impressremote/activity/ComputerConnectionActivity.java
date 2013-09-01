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
import android.support.v4.app.Fragment;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.fragment.ComputerConnectionFragment;
import org.libreoffice.impressremote.util.FragmentOperator;
import org.libreoffice.impressremote.util.Intents;
import org.libreoffice.impressremote.communication.Server;

public class ComputerConnectionActivity extends SherlockFragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpHomeButton();
        setUpTitle();
        setUpFragment();
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpTitle() {
        String aComputerName = extractReceivedComputer().getName();

        getSupportActionBar().setSubtitle(aComputerName);
    }

    private Server extractReceivedComputer() {
        return getIntent().getParcelableExtra(Intents.Extras.SERVER);
    }

    private void setUpFragment() {
        Server aComputer = extractReceivedComputer();
        Fragment aFragment = ComputerConnectionFragment.newInstance(aComputer);

        FragmentOperator.addFragment(this, aFragment);
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
