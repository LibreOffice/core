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

    public static void setUpFragment(FragmentActivity aActivity, Fragment aFragment) {
        if (isFragmentSetUp(aActivity)) {
            return;
        }

        installFragment(aActivity, aFragment);
    }

    private static boolean isFragmentSetUp(FragmentActivity aActivity) {
        FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();

        return aFragmentManager.findFragmentById(android.R.id.content) != null;
    }

    private static void installFragment(FragmentActivity aActivity, Fragment aFragment) {
        FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();
        FragmentTransaction aFragmentTransaction = aFragmentManager.beginTransaction();

        aFragmentTransaction.add(android.R.id.content, aFragment);

        aFragmentTransaction.commit();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
