package org.libreoffice.storage.external;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.provider.DocumentFile;

import org.libreoffice.R;
import org.libreoffice.storage.DocumentProviderSettingsActivity;
import org.libreoffice.storage.IFile;

import java.io.File;
import java.net.URI;

/**
 * Implementation of IDocumentProvider for the external file system, for android 4.4+
 *
 * The DocumentFile class is required when accessing files in external storage
 * for Android 4.4+. The ExternalFile class is used to handle this.
 *
 * Android 4.4 & 5+ use different types of root directory paths,
 * 5 using a DirectoryTree Uri and 4.4 using a normal File path.
 * As such, different methods are required to obtain the rootDirectory IFile.
 * 4.4 has to guess the location of the rootDirectory as well.
 */
public class ExtsdDocumentsProvider implements IExternalDocumentProvider,
        OnSharedPreferenceChangeListener{
    private static final String LOGTAG = ExtsdDocumentsProvider.class.getSimpleName();

    private int id;
    private File cacheDir;
    private Context context;
    private String rootPathURI;

    public ExtsdDocumentsProvider(int id, Context context) {
        this.id = id;
        this.context = context;
        setupRootPathUri();
        setupCache();
    }

    private void setupRootPathUri() {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String rootURIGuess = guessRootURI();
        rootPathURI = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_EXTERNAL_SD_PATH_URI, rootURIGuess);
    }

    //Android 4.4 specific
    @TargetApi(Build.VERSION_CODES.KITKAT)
    public String guessRootURI() {
        File[] options = context.getExternalFilesDirs(null);
        File internalSD = Environment.getExternalStorageDirectory();
        String internalSDPath = internalSD.getAbsolutePath();

        for (File option: options) {
            // Returned paths may be null if a storage device is unavailable.
            if (null == option) { continue; }

            String optionPath = option.getAbsolutePath();

            if(optionPath.contains(internalSDPath))
                return option.toURI().toString();

        }

        return "";
    }

    private void setupCache() {
        // TODO: probably we should do smarter cache management
        cacheDir = new File(context.getExternalCacheDir(), "externalFiles");
        if (cacheDir.exists()) {
            deleteRecursive(cacheDir);
        }
        cacheDir.mkdirs();
    }

    private static void deleteRecursive(File file) {
        if (file.isDirectory()) {
            for (File child : file.listFiles())
                deleteRecursive(child);
        }
        file.delete();
    }

    public File getCacheDir() {
        return cacheDir;
    }

    @Override
    public IFile getRootDirectory() {
        if(android.os.Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
            return android4RootDirectory();
        } else {
            return android5RootDirectory();
        }
    }

    private ExternalFile android4RootDirectory() {
        try{
            File f = new File(new URI(rootPathURI));
            return new ExternalFile(this, DocumentFile.fromFile(f), context);
        } catch (Exception e) {
            //invalid rootPathURI
            throw buildRuntimeExceptionForInvalidFileURI();
        }
    }

    private ExternalFile android5RootDirectory() {
        try {
            return new ExternalFile(this,
                                    DocumentFile.fromTreeUri(context, Uri.parse(rootPathURI)),
                                    context);
        } catch (Exception e) {
            //invalid rootPathURI
            throw buildRuntimeExceptionForInvalidFileURI();
        }
    }

    private RuntimeException buildRuntimeExceptionForInvalidFileURI() {
        return new RuntimeException(context.getString(R.string.ext_document_provider_error));
    }

    @Override
    public IFile createFromUri(URI javaURI) {
        //TODO: refactor when new DocumentFile API exist
        //uri must be of a DocumentFile file, not directory.
        Uri androidUri = Uri.parse(javaURI.toString());
        return new ExternalFile(this,
                                DocumentFile.fromSingleUri(context, androidUri),
                                context);
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
