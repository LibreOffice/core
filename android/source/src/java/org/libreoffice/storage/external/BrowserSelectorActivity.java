package org.libreoffice.storage.external;

import android.annotation.TargetApi;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.UriPermission;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;

import org.libreoffice.storage.DocumentProviderFactory;

import java.util.Set;

/**
 * Activity to select which directory browser to use.
 * Android 5+ will use the DocumentTree intent to locate a browser.
 * Android 4+ & OTG will use the internal directory browser.
 */
public class BrowserSelectorActivity extends AppCompatActivity {
    public static final String PREFERENCE_KEY_EXTRA = "org.libreoffice.pref_key_extra";
    public static final String MODE_EXTRA = "org.libreoffice.mode_extra";
    public static final String MODE_OTG = "OTG";
    public static final String MODE_EXT_SD = "EXT_SD";

    private static final String LOGTAG = BrowserSelectorActivity.class.getSimpleName();
    private static final int REQUEST_DOCUMENT_TREE = 1;
    private static final int REQUEST_INTERNAL_BROWSER = 2;
    private Set<SharedPreferences.OnSharedPreferenceChangeListener> listeners;
    private String preferenceKey;
    private SharedPreferences preferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        preferenceKey = getIntent().getStringExtra(PREFERENCE_KEY_EXTRA);
        preferences = PreferenceManager.getDefaultSharedPreferences(this);
        String mode = getIntent().getStringExtra(MODE_EXTRA);

        if(mode.equals(MODE_EXT_SD)) {
            findSDCard();
        } else if (mode.equals(MODE_OTG)) {
            findOTGDevice();
        }
    }

    private void findOTGDevice() {
        useInternalBrowser(DocumentProviderFactory.OTG_PROVIDER_INDEX);
    }

    private void findSDCard() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            useDocumentTreeBrowser();
        } else {
            useInternalBrowser(DocumentProviderFactory.EXTSD_PROVIDER_INDEX);
        }
    }

    private void useInternalBrowser(int providerIndex) {
        IExternalDocumentProvider provider =
                (IExternalDocumentProvider) DocumentProviderFactory.getInstance()
                        .getProvider(providerIndex);
        String previousDirectoryPath = preferences.getString(preferenceKey, provider.guessRootURI());
        Intent i = new Intent(this, DirectoryBrowserActivity.class);
        i.putExtra(DirectoryBrowserActivity.DIRECTORY_PATH_EXTRA, previousDirectoryPath);
        startActivityForResult(i, REQUEST_INTERNAL_BROWSER);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void useDocumentTreeBrowser() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(i, REQUEST_DOCUMENT_TREE);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        //listeners are registered here as onActivityResult is called before onResume
        super.onActivityResult(requestCode, resultCode, data);

        registerListeners();
        if(resultCode == RESULT_OK) {
            switch(requestCode) {
                case REQUEST_DOCUMENT_TREE:
                    Uri treeUri = data.getData();
                            preferences.edit()
                            .putString(preferenceKey, treeUri.toString())
                            .apply();

                    updatePersistedUriPermission(treeUri);
                    getContentResolver().takePersistableUriPermission(treeUri,
                                    Intent.FLAG_GRANT_READ_URI_PERMISSION |
                                    Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
                    break;

                case REQUEST_INTERNAL_BROWSER:
                    Uri fileUri = data.getData();
                            preferences.edit()
                            .putString(preferenceKey, fileUri.toString())
                            .apply();
                    break;
                default:
            }
        }
        unregisterListeners();
        Log.d(LOGTAG, "Preference saved: " +
                preferences.getString(preferenceKey, "Directory not saved."));
        finish();
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void updatePersistedUriPermission(Uri treeUri) {
        freePreviousUriPermissions();

        //TODO: Use non-emulator Android 5+ device to check if needed
        /*this.grantUriPermission(this.getPackageName(),
                treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION |
                Intent.FLAG_GRANT_WRITE_URI_PERMISSION); */

        getContentResolver().takePersistableUriPermission(treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION |
                        Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    private void freePreviousUriPermissions() {
        ContentResolver cr = getContentResolver();
        for (UriPermission uriPermission : cr.getPersistedUriPermissions()) {
            cr.releasePersistableUriPermission(uriPermission.getUri(), 0);
        }
    }

    private void registerListeners() {
        listeners = DocumentProviderFactory.getInstance().getChangeListeners();
        for (SharedPreferences.OnSharedPreferenceChangeListener listener : listeners) {
            PreferenceManager.getDefaultSharedPreferences(this)
                    .registerOnSharedPreferenceChangeListener(listener);
        }
    }

    private void unregisterListeners() {
        for (SharedPreferences.OnSharedPreferenceChangeListener listener : listeners) {
            PreferenceManager.getDefaultSharedPreferences(this)
                    .unregisterOnSharedPreferenceChangeListener(listener);
        }
    }
}
