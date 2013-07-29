/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.util;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;

public final class FragmentOperator {
    private FragmentOperator() {
    }

    public static void addFragment(FragmentActivity aActivity, Fragment aFragment) {
        if (isFragmentAdded(aActivity)) {
            return;
        }

        FragmentTransaction aFragmentTransaction = beginFragmentTransaction(aActivity);

        aFragmentTransaction.add(android.R.id.content, aFragment);

        aFragmentTransaction.commit();
    }

    private static boolean isFragmentAdded(FragmentActivity aActivity) {
        FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();

        return aFragmentManager.findFragmentById(android.R.id.content) != null;
    }

    private static FragmentTransaction beginFragmentTransaction(FragmentActivity aActivity) {
        FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();

        return aFragmentManager.beginTransaction();
    }

    public static void replaceFragmentAnimated(FragmentActivity aActivity, Fragment aFragment) {
        FragmentTransaction aFragmentTransaction = beginFragmentTransaction(aActivity);
        aFragmentTransaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);

        aFragmentTransaction.replace(android.R.id.content, aFragment);

        aFragmentTransaction.commit();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
