/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidapp;

import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Insets;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.WindowInsets;
import android.widget.RadioGroup;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.core.view.WindowCompat;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import org.libreoffice.androidapp.ui.LibreOfficeUIActivity;
import org.libreoffice.androidlib.LOActivity;

public class SettingsActivity extends AppCompatActivity {

    private static SharedPreferences prefs;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setTheme(R.style.LibreOfficeTheme_Base);
        super.onCreate(savedInstanceState);

        // Display the fragment as the main content.
        getSupportFragmentManager().beginTransaction()
                .replace(android.R.id.content, new SettingsFragment())
                .commit();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            findViewById(android.R.id.content).setOnApplyWindowInsetsListener((v, windowInsets) -> {
                Insets insets = windowInsets.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());

                v.setPadding(insets.left, insets.top, insets.right, insets.bottom);

                return WindowInsets.CONSUMED;
            });

            boolean lightMode = (getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_YES) == 0;
            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).setAppearanceLightStatusBars(lightMode);
            WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView()).setAppearanceLightNavigationBars(lightMode);
        }

        prefs = getSharedPreferences(LibreOfficeUIActivity.EXPLORER_PREFS_KEY, MODE_PRIVATE);
    }

    public static class SettingsFragment extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener {

        int dayNightMode;
        int selectedThemeBtnId;
        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
            addPreferencesFromResource(R.xml.libreoffice_preferences);
            if (!BuildConfig.DEBUG) {
                findPreference("ENABLE_SHOW_DEBUG_INFO").setVisible(false);
            }
            Preference themePreference = findPreference("THEME_SELECTION");

            dayNightMode = AppCompatDelegate.getDefaultNightMode();
            selectedThemeBtnId = R.id.radioBtn_default;
            switch (dayNightMode) {
                case AppCompatDelegate.MODE_NIGHT_YES:
                    themePreference.setSummary(R.string.theme_dark);
                    selectedThemeBtnId = R.id.radioBtn_dark;
                    break;
                case AppCompatDelegate.MODE_NIGHT_NO:
                    themePreference.setSummary(R.string.theme_light);
                    selectedThemeBtnId = R.id.radioBtn_light;
                    break;
                case AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM:
                    themePreference.setSummary(R.string.theme_system_default);
                    selectedThemeBtnId = R.id.radioBtn_default;
                    break;
            }

            themePreference.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    View view = getLayoutInflater().inflate(R.layout.theme_selection, null);
                    RadioGroup group = (RadioGroup) view.findViewById(R.id.radioGrp_theme);
                    group.check(selectedThemeBtnId);

                    group.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                        @Override
                        public void onCheckedChanged(RadioGroup radioGroup, int i) {
                            if (i == R.id.radioBtn_dark) {
                                dayNightMode = AppCompatDelegate.MODE_NIGHT_YES;
                            } else if (i == R.id.radioBtn_light) {
                                dayNightMode = AppCompatDelegate.MODE_NIGHT_NO;
                            } else {
                                dayNightMode = AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM;
                            }
                            AppCompatDelegate.setDefaultNightMode(dayNightMode);
                            prefs.edit().putInt(LibreOfficeUIActivity.NIGHT_MODE_KEY, dayNightMode).commit();
                            prefs.edit().putInt(LOActivity.NIGHT_MODE_KEY, dayNightMode).commit();
                            getActivity().recreate();

                        }
                    });

                    AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity());
                    dialog.setView(view);
                    dialog.setTitle(getString(R.string.choose_theme));
                    dialog.setCancelable(true);
                    dialog.setNegativeButton(getString(R.string.cancel_label), null);
                    dialog.show();
                    return false;
                }
            });
        }

        @Override
        public void onResume() {
            super.onResume();
            getPreferenceScreen().getSharedPreferences()
                    .registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onPause() {
            super.onPause();
            getPreferenceScreen().getSharedPreferences()
                    .unregisterOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
            SettingsListenerModel.getInstance().changePreferenceState(sharedPreferences, key);
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
