/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.Log;

import java.io.File;

/** Finds the file in the filesystem that a content URI stands for. */
public final class UriPathResolver {
    private static final String TAG = "UriPathResolver";

    private static final String EXTERNAL_STORAGE_AUTHORITY = "com.android.externalstorage.documents";
    private static final String DOWNLOADS_AUTHORITY = "com.android.providers.downloads.documents";

    private UriPathResolver() {
    }

    /** Tries to resolve the URI to an actual file path. */
    public static File resolve(Context context, Uri uri) {
        if (uri == null)
            return null;

        if (ContentResolver.SCHEME_FILE.equals(uri.getScheme())) {
            String path = uri.getPath();
            return path == null ? null : new File(path);
        }

        if (!ContentResolver.SCHEME_CONTENT.equals(uri.getScheme()))
            return null;

        String authority = uri.getAuthority();
        boolean isDocument = DocumentsContract.isDocumentUri(context, uri);

        // The media provider names the file it has indexed in its own data column.
        if (MediaStore.AUTHORITY.equals(authority) && !isDocument)
            return queryPathColumn(context, uri);

        if (!isDocument)
            return null;

        String documentId = DocumentsContract.getDocumentId(uri);
        if (documentId == null)
            return null;

        if (EXTERNAL_STORAGE_AUTHORITY.equals(authority))
            return fromExternalStorageDocument(documentId);
        if (DOWNLOADS_AUTHORITY.equals(authority) && documentId.startsWith("raw:"))
            return new File(documentId.substring("raw:".length()));

        return null;
    }

    /** The path of a document that the system provider for the storage volumes handed out. */
    private static File fromExternalStorageDocument(String documentId) {
        int colon = documentId.indexOf(':');
        if (colon < 0)
            return null;

        String volume = documentId.substring(0, colon);
        String relativePath = documentId.substring(colon + 1);
        if (relativePath.isEmpty())
            return null;

        if ("primary".equalsIgnoreCase(volume))
            return new File(Environment.getExternalStorageDirectory(), relativePath);

        // A memory card or a USB stick is named by the identifier of the volume, and the system
        // mounts it under /storage with that identifier as the directory name.
        return new File("/storage/" + volume, relativePath);
    }

    /** The path that the media provider reports in its data column for the file this URI names. */
    private static File queryPathColumn(Context context, Uri uri) {
        String[] projection = new String[]{MediaStore.MediaColumns.DATA};
        try (Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null)) {
            if (cursor == null || !cursor.moveToFirst())
                return null;

            int column = cursor.getColumnIndex(MediaStore.MediaColumns.DATA);
            if (column < 0)
                return null;

            String path = cursor.getString(column);
            return (path == null || path.isEmpty()) ? null : new File(path);
        } catch (Exception e) {
            Log.i(TAG, "no path column for " + uri + ": " + e.getMessage());
            return null;
        }
    }
}
