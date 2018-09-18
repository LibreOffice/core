package org.libreoffice;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.preference.PreferenceManager;

import java.util.Locale;

public class LocaleHelper {

    private static final String SELECTED_LANG = "org.libreoffice.selected.lang";

    public static Context onAttach(Context context){
        String lang = getPersistedData(context, Locale.getDefault().getLanguage());
        return setLocale(context, lang);
    }

    public static Context onAttach(Context context, String defLang){
        String lang = getPersistedData(context, defLang);
        return setLocale(context, lang);
    }

    public static Context setLocale(Context context, String lang) {
        persist(context, lang);
        return updateResources(context, lang);
    }

    @SuppressWarnings("deprecation")
    private static Context updateResources(Context context, String lang) {
        Locale locale = new Locale(lang);
        Locale.setDefault(locale);

        Resources res = context.getResources();
        Configuration cfg = res.getConfiguration();
        cfg.locale = locale;
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1)
            cfg.setLayoutDirection(locale);

        res.updateConfiguration(cfg, res.getDisplayMetrics());
        return context;
    }

    private static void persist(Context context, String lang) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        preferences.edit().putString(SELECTED_LANG, lang);
        preferences.edit().apply();
    }

    private static String getPersistedData(Context context, String lang) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        return preferences.getString(SELECTED_LANG, lang);
    }
}
