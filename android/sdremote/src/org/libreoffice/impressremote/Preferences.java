/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import android.content.Context;
import android.content.SharedPreferences;

public final class Preferences {
    public static final class Locations {
        private Locations() {
        }

        public static final String AUTHORIZED_REMOTES = "sdremote_authorisedremotes";
        public static final String STORED_SERVERS = "sdremote_storedServers";
    }

    private Preferences() {
    }

    public static boolean doContain(Context aContext, String aLocation, String aKey) {
        return getPreferences(aContext, aLocation).contains(aKey);
    }

    private static SharedPreferences getPreferences(Context aContext, String aLocation) {
        return aContext.getSharedPreferences(aLocation, Context.MODE_PRIVATE);
    }

    public static String getString(Context aContext, String aLocation, String aKey) {
        return getPreferences(aContext, aLocation).getString(aKey, null);
    }

    public static void set(Context aContext, String aLocation, String aKey, String aValue) {
        SharedPreferences.Editor aPreferencesEditor = getPreferences(aContext,
            aLocation).edit();

        aPreferencesEditor.putString(aKey, aValue);

        aPreferencesEditor.commit();
    }

    public static void remove(Context aContext, String aLocation, String aKey) {
        SharedPreferences.Editor aPreferencesEditor = getPreferences(aContext,
            aLocation).edit();

        aPreferencesEditor.remove(aKey);

        aPreferencesEditor.commit();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
