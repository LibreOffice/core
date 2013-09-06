/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.util;

import java.util.Map;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public final class Preferences {
    private static final class Locations {
        private Locations() {
        }

        public static final String AUTHORIZED_SERVERS = "authorized_servers";
        public static final String SAVED_SERVERS = "saved_servers";
        public static final String APPLICATION_STATES = "application_states";
    }

    public static final class Keys {
        private Keys() {
        }

        public static final String SELECTED_COMPUTERS_TAB_INDEX = "selected_computers_tab_index";

        public static final String VOLUME_KEYS_ACTIONS = "volume_keys_actions";
        public static final String KEEP_SCREEN_ON = "keep_screen_on";
    }

    private static final class Defaults {
        private Defaults() {
        }

        public static final String STRING = null;
        public static final int INT = 0;
        public static final boolean BOOLEAN = false;
    }

    private final SharedPreferences mPreferences;

    public static Preferences getAuthorizedServersInstance(Context aContext) {
        return new Preferences(aContext, Locations.AUTHORIZED_SERVERS);
    }

    private Preferences(Context aContext, String aLocation) {
        mPreferences = getPreferences(aContext, aLocation);
    }

    private SharedPreferences getPreferences(Context aContext, String aLocation) {
        return aContext.getSharedPreferences(aLocation, Context.MODE_PRIVATE);
    }

    public static Preferences getSavedServersInstance(Context aContext) {
        return new Preferences(aContext, Locations.SAVED_SERVERS);
    }

    public static Preferences getApplicationStatesInstance(Context aContext) {
        return new Preferences(aContext, Locations.APPLICATION_STATES);
    }

    public static Preferences getSettingsInstance(Context context) {
        return new Preferences(context);
    }

    private Preferences(Context context) {
        mPreferences = PreferenceManager.getDefaultSharedPreferences(context);
    }

    public Map<String, ?> getAll() {
        return mPreferences.getAll();
    }

    public String getString(String aKey) {
        return mPreferences.getString(aKey, Defaults.STRING);
    }

    public int getInt(String aKey) {
        return mPreferences.getInt(aKey, Defaults.INT);
    }

    public void setString(String aKey, String aValue) {
        mPreferences.edit().putString(aKey, aValue).commit();
    }

    public boolean getBoolean(String aKey) {
        return mPreferences.getBoolean(aKey, Defaults.BOOLEAN);
    }

    public void setInt(String aKey, int aValue) {
        mPreferences.edit().putInt(aKey, aValue).commit();
    }

    public void remove(String aKey) {
        mPreferences.edit().remove(aKey).commit();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
