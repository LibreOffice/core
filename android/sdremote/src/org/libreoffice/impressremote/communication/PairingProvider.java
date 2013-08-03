/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.os.Build;

import org.libreoffice.impressremote.util.Preferences;

final class PairingProvider {
    private final Preferences mAuthorizedServersPreferences;

    private PairingProvider(Context aContext) {
        mAuthorizedServersPreferences = Preferences.getAuthorizedServersInstance(aContext);
    }

    public static boolean isPairingNecessary(Server aServer) {
        return aServer.getProtocol() == Server.Protocol.TCP;
    }

    public static String getPairingPin(Context aContext, Server aServer) {
        return new PairingProvider(aContext).getPairingPin(aServer);
    }

    private String getPairingPin(Server aServer) {
        if (isPinSaved(aServer)) {
            return getSavedPin(aServer);
        }

        String aPin = Protocol.Pin.generate();

        savePin(aServer, aPin);

        return aPin;
    }

    private boolean isPinSaved(Server aServer) {
        return getSavedPin(aServer) != null;
    }

    private String getSavedPin(Server aServer) {
        return mAuthorizedServersPreferences.getString(aServer.getAddress());
    }

    private void savePin(Server aServer, String aPin) {
        mAuthorizedServersPreferences.setString(aServer.getAddress(), aPin);
    }

    public static String getPairingDeviceName(Context aContext) {
        return new PairingProvider(aContext).getPairingDeviceName();
    }

    private String getPairingDeviceName() {
        if (BluetoothAdapter.getDefaultAdapter() == null) {
            return Build.MODEL;
        }

        if (BluetoothAdapter.getDefaultAdapter().getName() == null) {
            return Build.MODEL;
        }

        return BluetoothAdapter.getDefaultAdapter().getName();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
