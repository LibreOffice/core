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

public final class Fragments {
    private Fragments() {
    }

    public static final class Arguments {
        private Arguments() {
        }

        public static final String COMPUTER = "COMPUTER";
        public static final String MINUTES = "MINUTES";
        public static final String TYPE = "TYPE";
    }

    public static final class Operator {
        private Operator() {
        }

        public static void add(FragmentActivity aActivity, Fragment aFragment) {
            if (isAdded(aActivity)) {
                return;
            }

            FragmentTransaction aFragmentTransaction = beginTransaction(aActivity);

            aFragmentTransaction.add(android.R.id.content, aFragment);

            aFragmentTransaction.commit();
        }

        private static boolean isAdded(FragmentActivity aActivity) {
            FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();

            return aFragmentManager.findFragmentById(android.R.id.content) != null;
        }

        private static FragmentTransaction beginTransaction(FragmentActivity aActivity) {
            FragmentManager aFragmentManager = aActivity.getSupportFragmentManager();

            return aFragmentManager.beginTransaction();
        }

        public static void replaceAnimated(FragmentActivity aActivity, Fragment aFragment) {
            FragmentTransaction aFragmentTransaction = beginTransaction(aActivity);
            aFragmentTransaction.setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out);

            aFragmentTransaction.replace(android.R.id.content, aFragment);

            aFragmentTransaction.commit();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
