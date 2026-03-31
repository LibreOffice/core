/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice;

import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.graphics.Insets;
import androidx.fragment.app.FragmentActivity;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceGroup;

public class SettingsActivity extends FragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        WindowCompat.setDecorFitsSystemWindows(getWindow(), true);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Window window = getWindow();
            View decorView = window.getDecorView();
            int systemUiVisibility = decorView.getSystemUiVisibility() | View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            decorView.setSystemUiVisibility(systemUiVisibility);
        }

        // Display the fragment as the main content.
        getSupportFragmentManager().beginTransaction()
            .replace(android.R.id.content, new SettingsFragment())
            .commit();
        ViewCompat.setOnApplyWindowInsetsListener(getWindow().getDecorView().findViewById(android.R.id.content), (v, windowInsets) -> {
            WindowInsetsCompat compat = WindowInsetsCompat.toWindowInsetsCompat(windowInsets.toWindowInsets(), v);
            Insets systemBars = compat.getInsets(WindowInsetsCompat.Type.systemBars());
            Insets displayCutout = compat.getInsets(WindowInsetsCompat.Type.displayCutout());
            int top = Math.max(systemBars.top, displayCutout.top);
            int bottom = Math.max(systemBars.bottom, displayCutout.bottom);
            int left = Math.max(systemBars.left, displayCutout.left);
            int right = Math.max(systemBars.right, displayCutout.right);
            v.setPadding(left, top, right, bottom);
            return windowInsets;
        });
    }

    public static class SettingsFragment extends PreferenceFragmentCompat {

        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            setPreferencesFromResource(R.xml.libreoffice_preferences, rootKey);
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
            if(!BuildConfig.ALLOW_EDITING) {
                PreferenceGroup generalGroup = findPreference("PREF_CATEGORY_GENERAL");
                generalGroup.removePreference(generalGroup.findPreference("ENABLE_EXPERIMENTAL"));
                generalGroup.removePreference(generalGroup.findPreference("ENABLE_DEVELOPER"));
            }
        }
    }

    public static boolean hasSettings() {
        return BuildConfig.ALLOW_EDITING;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
