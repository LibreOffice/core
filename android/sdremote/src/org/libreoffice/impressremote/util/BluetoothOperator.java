/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.util;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;

public final class BluetoothOperator {
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

    public static void enable(Activity aActivity, int nRequestCode) {
        if (!isAvailable()) {
            return;
        }

        if (getAdapter() != null) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            aActivity.startActivityForResult(enableBtIntent, nRequestCode);
        }

    }

    public static void disable() {
        if (!isAvailable()) {
            return;
        }

        getAdapter().disable();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
