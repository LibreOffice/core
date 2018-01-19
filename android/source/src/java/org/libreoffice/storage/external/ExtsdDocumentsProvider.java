package org.libreoffice.storage.external;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.content.ContextCompat;
import android.support.v4.provider.DocumentFile;
import android.util.Log;

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
    private String rootPathURI;

    public ExtsdDocumentsProvider(int id, Context context) {
        this.id = id;
        setupRootPathUri(context);
        setupCache(context);
    }

    private void setupRootPathUri(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        rootPathURI = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_EXTERNAL_SD_PATH_URI, guessRootURI(context));
    }

    public String guessRootURI(Context context) {
        // TODO: unfortunately the getExternalFilesDirs function relies on devices to actually
        // follow guidelines re external storage. Of course device manufacturers don't and as such
        // you cannot rely on it returning the actual paths (neither the compat, nor the native variant)
        File[] possibleRemovables = ContextCompat.getExternalFilesDirs(context,null);
        // the primary dir that is already covered by the "LocalDocumentsProvider"
        // might be emulated/part of internal memory or actual SD card
        // TODO: change to not confuse android's "external storage" with "expandable storage"
        String primaryExternal = Environment.getExternalStorageDirectory().getAbsolutePath();

        for (File option: possibleRemovables) {
            // Returned paths may be null if a storage device is unavailable.
            if (null == option) {
                Log.w(LOGTAG,"path was a null option :-/"); continue; }
            String optionPath = option.getAbsolutePath();
            if(optionPath.contains(primaryExternal)) {
                Log.v(LOGTAG, "did get file path - but is same as primary storage ("+ primaryExternal +")");
                continue;
            }

            return option.toURI().toString();
        }

        // TODO: do some manual probing of possible directories (/storage/sdcard1 and similar)
        Log.i(LOGTAG, "no secondary storage reported");
        return null;
    }

    private void setupCache(Context context) {
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
    public IFile getRootDirectory(Context context) {
        if(android.os.Build.VERSION.SDK_INT <= Build.VERSION_CODES.KITKAT) {
            return android4RootDirectory(context);
        } else {
            return android5RootDirectory(context);
        }
    }

    private ExternalFile android4RootDirectory(Context context) {
        try{
            File f = new File(new URI(rootPathURI));
            return new ExternalFile(this, DocumentFile.fromFile(f), context);
        } catch (Exception e) {
            //invalid rootPathURI
            throw buildRuntimeExceptionForInvalidFileURI(context);
        }
    }

    private ExternalFile android5RootDirectory(Context context) {
        try {
            return new ExternalFile(this,
                                    DocumentFile.fromTreeUri(context, Uri.parse(rootPathURI)),
                                    context);
        } catch (Exception e) {
            //invalid rootPathURI
            throw buildRuntimeExceptionForInvalidFileURI(context);
        }
    }

    private RuntimeException buildRuntimeExceptionForInvalidFileURI(Context context) {
        // ToDo: discarding the original exception / catch-all handling is bad style
        return new RuntimeException(context.getString(R.string.ext_document_provider_error));
    }

    @Override
    public IFile createFromUri(Context context, URI javaURI) {
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
    public boolean checkProviderAvailability(Context context) {
        // too many devices (or I am just unlucky) don't report the mounted state properly, and other
        // devices also consider dedicated part of internal storage to be "mounted" so cannot use
        // getExternalStorageState().equals(Environment.MEDIA_MOUNTED) && isExternalStorageRemovable()
        // but they refer to the primary external storage anyway, so what currently is covered by the
        // "LocalDocumentsProvider"
        return rootPathURI!=null;
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
        if (key.equals(DocumentProviderSettingsActivity.KEY_PREF_EXTERNAL_SD_PATH_URI)) {
            rootPathURI = preferences.getString(key, "");
        }
    }

}
