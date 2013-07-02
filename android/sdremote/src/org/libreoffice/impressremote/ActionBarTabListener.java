/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;

import com.actionbarsherlock.app.ActionBar;

public class ActionBarTabListener implements ActionBar.TabListener {
    private final Fragment mTabFragment;

    public ActionBarTabListener(Fragment aTabFragment) {
        mTabFragment = aTabFragment;
    }

    @Override
    public void onTabSelected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
        if (mTabFragment.isDetached()) {
            aFragmentTransaction.attach(mTabFragment);
        }

        aFragmentTransaction.replace(android.R.id.content, mTabFragment);
    }

    @Override
    public void onTabUnselected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
        if (mTabFragment.isDetached()) {
            return;
        }

        aFragmentTransaction.detach(mTabFragment);
    }

    @Override
    public void onTabReselected(ActionBar.Tab aTab, FragmentTransaction aFragmentTransaction) {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
