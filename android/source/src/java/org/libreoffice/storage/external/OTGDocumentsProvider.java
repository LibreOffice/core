package org.libreoffice.storage.external;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.preference.PreferenceManager;
import android.util.Log;

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

    private String rootPathURI;
    private int id;

    public OTGDocumentsProvider(int id, Context context) {
        this.id = id;
        setupRootPath(context);
    }

    private void setupRootPath(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        rootPathURI = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_OTG_PATH_URI, "");
    }

    @Override
    public IFile createFromUri(Context context, URI uri) {
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
    public IFile getRootDirectory(Context context) {
        // TODO: handle this with more fine-grained exceptions
        if(rootPathURI.equals("")) {
            Log.e(LOGTAG, "rootPathURI is empty");
            throw new RuntimeException(context.getString(R.string.ext_document_provider_error));
        }

        File f = IOUtils.getFileFromURIString(rootPathURI);
        if(IOUtils.isInvalidFile(f)) {
            Log.e(LOGTAG, "rootPathURI is invalid - missing device?");
            throw new RuntimeException(context.getString(R.string.otg_missing_error));
        }

        return new LocalFile(f);
    }


    @Override
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if (key.equals(DocumentProviderSettingsActivity.KEY_PREF_OTG_PATH_URI)) {
            rootPathURI = sharedPreferences.getString(key, "");
        }
    }

    @Override
    public String guessRootURI(Context context) {
        return "";
    }

    @Override
    public boolean checkProviderAvailability(Context context) {
        // check if system supports USB Host
        return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_USB_HOST);
    }
}
