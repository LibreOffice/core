package org.libreoffice.storage.external;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import org.libreoffice.R;
import org.libreoffice.storage.DocumentProviderSettingsActivity;
import org.libreoffice.storage.IFile;
import org.libreoffice.storage.IOUtils;
import org.libreoffice.storage.local.LocalFile;

import java.io.File;
import java.net.URI;

/**
 * TODO: OTG currently uses LocalFile. Change to an IFile that handles abrupt OTG unmounting
 */
public class OTGDocumentsProvider implements IExternalDocumentProvider,
        SharedPreferences.OnSharedPreferenceChangeListener {

    private static final String LOGTAG = OTGDocumentsProvider.class.getSimpleName();

    private Context context;
    private String rootPathURI;
    private int id;

    public OTGDocumentsProvider(int id, Context context) {
        this.context = context;
        this.id = id;
        setupRootPath();
    }

    private void setupRootPath() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        rootPathURI = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_OTG_PATH_URI, "");
    }

    @Override
    public IFile createFromUri(URI uri) {
        return new LocalFile(uri);
    }

    @Override
    public int getNameResource() {
        return R.string.otg_file_system;
    }

    @Override
    public int getId() {
        return id;
    }

    @Override
    public IFile getRootDirectory() {
        File f;

        try {
            f = IOUtils.getFileFromURIString(rootPathURI);
        } catch (IllegalArgumentException e) {
            throw buildRuntimeExceptionForInvalidFileURI(R.string.ext_document_provider_error);
        }

        checkFileValidity(f);
        return new LocalFile(f);
    }

    private void checkFileValidity(File f) {
        if(f == null || !f.exists() || !f.canRead() || !f.canWrite()) {
            throw buildRuntimeExceptionForInvalidFileURI(R.string.otg_missing_error);
        }
    }

    private RuntimeException buildRuntimeExceptionForInvalidFileURI(int msgResource) {
        return new RuntimeException(context.getString(msgResource));
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if (key.equals(DocumentProviderSettingsActivity.KEY_PREF_OTG_PATH_URI)) {
            rootPathURI = sharedPreferences.getString(key, "");
        }
    }

    @Override
    public String guessRootURI() {
        return "";
    }
}
