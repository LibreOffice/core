package org.libreoffice.storage.external;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;

import org.libreoffice.R;
import org.libreoffice.storage.DocumentProviderSettingsActivity;
import org.libreoffice.storage.IFile;
import org.libreoffice.storage.IOUtils;
import org.libreoffice.storage.local.LocalFile;

import java.io.File;
import java.net.URI;

/**
 * Legacy document provider for External SD cards, for Android 4.3 and below.
 *
 * Uses the LocalFile class.
 */
public class LegacyExtSDDocumentsProvider implements IExternalDocumentProvider,
        SharedPreferences.OnSharedPreferenceChangeListener{
    private static final String LOGTAG = LegacyExtSDDocumentsProvider.class.getSimpleName();

    private int id;
    private Context context;
    private String rootPathURI;

    public LegacyExtSDDocumentsProvider(int id, Context context) {
        this.id = id;
        this.context = context;
        setupRootPathUri();
    }

    private void setupRootPathUri() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String rootURIGuess = guessRootURI();

        rootPathURI = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_EXTERNAL_SD_PATH_URI, rootURIGuess);
    }

    public String guessRootURI() {
        //hacky method of obtaining extsdcard root
        final String value = System.getenv("SECONDARY_STORAGE");
        Log.d(LOGTAG, "guesses: " + value);
        if (!TextUtils.isEmpty(value)) {
            final String[] paths = value.split(":");
            for (String path : paths) {
                File file = new File(path);
                if(path.contains("ext") && file.isDirectory()) {
                    return file.toURI().toString();
                }
            }
        }
        return "";
    }

    @Override
    public IFile getRootDirectory() {
        if(rootPathURI.equals("")) {
            throw new RuntimeException(context.getString(R.string.ext_document_provider_error));
        }

        File f = IOUtils.getFileFromURIString(rootPathURI);
        if(IOUtils.isInvalidFile(f)) {
            //missing device
            throw new RuntimeException(context.getString(R.string.legacy_extsd_missing_error));
        }
        return new LocalFile(f);
    }

    @Override
    public IFile createFromUri(URI uri) {
        return new LocalFile(uri);
    }

    @Override
    public int getNameResource() {
        return R.string.external_sd_file_system;
    }

    @Override
    public int getId() {
        return id;
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
        if (key.equals(DocumentProviderSettingsActivity.KEY_PREF_EXTERNAL_SD_PATH_URI)) {
            rootPathURI = preferences.getString(key, "");
        }
    }

}
