package org.libreoffice.ui;

import android.net.Uri;

/**
 * An entry for a recently used file in the RecentFilesAdapter.
 */
public class RecentFile {

    private Uri uri;
    private String displayName;

    public RecentFile(Uri docUri, String name) {
        uri = docUri;
        displayName = name;
    }

    public Uri getUri() {
        return uri;
    }

    public String getDisplayName() {
        return displayName;
    }
}
