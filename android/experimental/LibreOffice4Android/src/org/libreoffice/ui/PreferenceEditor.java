package org.libreoffice.ui;

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
