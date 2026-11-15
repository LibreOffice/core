/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.content.ClipDescription;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.provider.OpenableColumns;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * Read-only provider serving the HTML and the text of a copy to other applications, from the
 * .html and .txt files written next to the .data file that the content URI names.
 */
public class ClipboardFileProvider extends ContentProvider {
    private static final String MIME_HTML = "text/html";
    private static final String MIME_PLAIN = "text/plain";

    public static String authority(Context context) {
        return context.getPackageName() + ".clipboard";
    }

    public static File getClipboardDir(Context context) {
        return new File(context.getCacheDir(), "clipboard");
    }

    /// content URI for a file previously written to getClipboardDir()
    public static Uri uriFor(Context context, String fileName) {
        return new Uri.Builder().scheme("content")
            .authority(authority(context)).appendPath(fileName).build();
    }

    public static File fileFor(Context context, Uri uri) {
        if (uri == null || !"content".equals(uri.getScheme())
                || !authority(context).equals(uri.getAuthority())
                || uri.getPathSegments().size() != 1)
            return null;

        File dir = getClipboardDir(context);
        File file = new File(dir, uri.getLastPathSegment());
        try {
            // guard against path tricks in the last segment
            if (!file.getCanonicalFile().getParentFile().equals(dir.getCanonicalFile()))
                return null;
        } catch (IOException e) {
            return null;
        }
        return file;
    }

    /// the file next to a .data file with the given extension, "html" or "txt"
    public static File sideFile(File dataFile, String extension) {
        String name = dataFile.getName();
        int dot = name.lastIndexOf('.');
        String base = dot > 0 ? name.substring(0, dot) : name;
        return new File(dataFile.getParentFile(), base + "." + extension);
    }

    private File existingSideFile(Uri uri, String extension) {
        File dataFile = fileFor(getContext(), uri);
        if (dataFile == null)
            return null;
        File file = sideFile(dataFile, extension);
        return file.isFile() ? file : null;
    }

    private static String streamTypeFor(String mimeTypeFilter) {
        if (mimeTypeFilter == null)
            return null;
        if (ClipDescription.compareMimeTypes(MIME_PLAIN, mimeTypeFilter))
            return MIME_PLAIN;
        if (ClipDescription.compareMimeTypes(MIME_HTML, mimeTypeFilter))
            return MIME_HTML;
        return null;
    }

    /// the HTML, else the text, else null
    private File defaultFile(Uri uri) {
        File file = existingSideFile(uri, "html");
        return file != null ? file : existingSideFile(uri, "txt");
    }

    private File typedFile(Uri uri, String mimeTypeFilter) {
        if ("*/*".equals(mimeTypeFilter))
            return defaultFile(uri);
        String streamType = streamTypeFor(mimeTypeFilter);
        if (MIME_HTML.equals(streamType))
            return existingSideFile(uri, "html");
        if (MIME_PLAIN.equals(streamType)) {
            File file = existingSideFile(uri, "txt");
            // no plain text variant... serve the html one rather than nothing
            return file != null ? file : existingSideFile(uri, "html");
        }
        return null;
    }

    /// the length comes from the open descriptor, so it survives the next copy unlinking the file
    private static AssetFileDescriptor openSideFile(File file) throws FileNotFoundException {
        ParcelFileDescriptor descriptor =
            ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
        return new AssetFileDescriptor(descriptor, 0, descriptor.getStatSize());
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public String getType(Uri uri) {
        File file = defaultFile(uri);
        if (file == null)
            return null;
        return file.getName().endsWith(".txt") ? MIME_PLAIN : MIME_HTML;
    }

    @Override
    public String[] getStreamTypes(Uri uri, String mimeTypeFilter) {
        boolean hasHtml = existingSideFile(uri, "html") != null
            && ClipDescription.compareMimeTypes(MIME_HTML, mimeTypeFilter);
        boolean hasPlain = existingSideFile(uri, "txt") != null
            && ClipDescription.compareMimeTypes(MIME_PLAIN, mimeTypeFilter);

        if (hasHtml && hasPlain)
            return new String[] { MIME_HTML, MIME_PLAIN };
        if (hasHtml)
            return new String[] { MIME_HTML };
        if (hasPlain)
            return new String[] { MIME_PLAIN };
        return null;
    }

    @Override
    public AssetFileDescriptor openTypedAssetFile(Uri uri, String mimeTypeFilter, Bundle opts)
            throws FileNotFoundException {
        File file = typedFile(uri, mimeTypeFilter);
        if (file == null)
            throw new FileNotFoundException("no stream of type " + mimeTypeFilter + " for " + uri);
        return openSideFile(file);
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
                        String sortOrder) {
        if (projection == null)
            projection = new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE };

        File file = defaultFile(uri);
        MatrixCursor cursor = new MatrixCursor(projection, 1);
        Object[] row = new Object[projection.length];
        for (int i = 0; i < projection.length; ++i) {
            if (OpenableColumns.DISPLAY_NAME.equals(projection[i]))
                row[i] = file != null && file.getName().endsWith(".txt")
                    ? "clipboard.txt" : "clipboard.html";
            else if (OpenableColumns.SIZE.equals(projection[i]))
                row[i] = file != null ? file.length() : null;
            else
                row[i] = null;
        }
        cursor.addRow(row);
        return cursor;
    }

    @Override
    public AssetFileDescriptor openAssetFile(Uri uri, String mode) throws FileNotFoundException {
        if (!"r".equals(mode))
            throw new FileNotFoundException("read-only provider");
        File file = defaultFile(uri);
        if (file == null)
            throw new FileNotFoundException("no clipboard content for " + uri);
        return openSideFile(file);
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        return openAssetFile(uri, mode).getParcelFileDescriptor();
    }

    @Override
    public Uri insert(Uri uri, ContentValues values) {
        throw new UnsupportedOperationException("read-only provider");
    }

    @Override
    public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("read-only provider");
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        throw new UnsupportedOperationException("read-only provider");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
