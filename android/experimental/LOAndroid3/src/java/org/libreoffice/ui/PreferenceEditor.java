/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.ui;

import org.libreoffice.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;

public class PreferenceEditor extends PreferenceActivity {
	public final static String FilterTypePrefKey = "FilterTypePreference";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource( R.xml.libreoffice_preferences );
		//mPrefs = getSharedPreferences( LibreOfficeUIActivity.EXPLORER_PREFS_KEY , MODE_PRIVATE );
	}
}
