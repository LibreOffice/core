/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.util;

import android.bluetooth.BluetoothAdapter;

public final class BluetoothOperator {
    public static final class State {
        private final boolean mWasBluetoothEnabled;

        private State(boolean aIsBluetoothEnabled) {
            mWasBluetoothEnabled = aIsBluetoothEnabled;
        }

        public boolean wasBluetoothEnabled() {
            return mWasBluetoothEnabled;
        }
    }

    private BluetoothOperator() {
    }

    public static boolean isAvailable() {
        return getAdapter() != null;
    }

    public static BluetoothAdapter getAdapter() {
        // TODO: should be acquired other way on Jelly Bean MR2
        // Look at the BluetoothAdapter’s docs for details.
        // It will require to use the latest version of SDK to get needed constant.

        return BluetoothAdapter.getDefaultAdapter();
    }

    public static State getState() {
        return new State(getAdapter().isEnabled());
    }

    public static void enable() {
        if (!isAvailable()) {
            return;
        }

        getAdapter().enable();
    }

    public static void disable() {
        if (!isAvailable()) {
            return;
        }

        getAdapter().disable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
