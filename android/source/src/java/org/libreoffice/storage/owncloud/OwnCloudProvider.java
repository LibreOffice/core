package org.libreoffice.storage.owncloud;

import java.io.File;
import java.net.URI;

import org.libreoffice.R;
import org.libreoffice.storage.DocumentProviderSettingsActivity;
import org.libreoffice.storage.IDocumentProvider;
import org.libreoffice.storage.IFile;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

import com.owncloud.android.lib.common.OwnCloudClient;
import com.owncloud.android.lib.common.OwnCloudClientFactory;
import com.owncloud.android.lib.common.OwnCloudCredentialsFactory;
import com.owncloud.android.lib.common.operations.RemoteOperationResult;
import com.owncloud.android.lib.resources.files.FileUtils;
import com.owncloud.android.lib.resources.files.ReadRemoteFileOperation;
import com.owncloud.android.lib.resources.files.RemoteFile;

/**
 * Implementation of IDocumentProvider for ownCloud servers.
 */
public class OwnCloudProvider implements IDocumentProvider {

    private OwnCloudClient client;
    private File cacheDir;

    private String serverUrl;
    private String userName;
    private String password;

    public OwnCloudProvider(Context context) {
        // read preferences
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        serverUrl = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_OWNCLOUD_SERVER, "");
        userName = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_OWNCLOUD_USER_NAME, "");
        password = preferences.getString(
                DocumentProviderSettingsActivity.KEY_PREF_OWNCLOUD_PASSWORD, "");

        Uri serverUri = Uri.parse(serverUrl);
        client = OwnCloudClientFactory.createOwnCloudClient(serverUri,
                context, true);
        client.setCredentials(OwnCloudCredentialsFactory.newBasicCredentials(
                userName, password));

        // make sure cache directory exists, and clear it
        // TODO: probably we should do smarter cache management
        cacheDir = new File(context.getCacheDir(), "ownCloud");
        if (cacheDir.exists()) {
            deleteRecursive(cacheDir);
        }
        cacheDir.mkdirs();
    }

    @Override
    public IFile getRootDirectory() {
        return createFromUri(URI.create(FileUtils.PATH_SEPARATOR));
    }

    @Override
    public IFile createFromUri(URI uri) {
        ReadRemoteFileOperation refreshOperation = new ReadRemoteFileOperation(
                uri.getPath());
        RemoteOperationResult result = refreshOperation.execute(client);
        if (!result.isSuccess()) {
            throw new RuntimeException(result.getLogMessage(),
                    result.getException());
        }
        if (result.getData().size() > 0) {
            return new OwnCloudFile(this, (RemoteFile) result.getData().get(0));
        }
        return null;
    }

    @Override
    public int getNameResource() {
        return R.string.owncloud;
    }

    /**
     * Used by OwnCloudFiles to get a configured client to run their own
     * operations.
     *
     * @return configured OwnCloudClient.
     */
    protected OwnCloudClient getClient() {
        return client;
    }

    /**
     * Used by OwnCloudFiles to get the cache directory they should download
     * files to.
     *
     * @return cache directory.
     */
    protected File getCacheDir() {
        return cacheDir;
    }

    /**
     * Deletes files and recursively deletes directories.
     *
     * @param file
     *            File or directory to be deleted.
     */
    private void deleteRecursive(File file) {
        if (file.isDirectory()) {
            for (File child : file.listFiles())
                deleteRecursive(child);
        }
        file.delete();
    }
}
