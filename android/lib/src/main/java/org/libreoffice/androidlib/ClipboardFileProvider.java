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
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import org.libreoffice.androidlib.lok.LokClipboardData;
import org.libreoffice.androidlib.lok.LokClipboardEntry;

/**
 * Read-only provider streaming clipboard content to other applications.
 */
public class ClipboardFileProvider extends ContentProvider {
    private static final String TAG = "ClipboardFileProvider";

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

    private LokClipboardData dataFor(Uri uri) throws FileNotFoundException {
        File file = fileFor(getContext(), uri);
        if (file == null || !file.exists())
            throw new FileNotFoundException("no clipboard data for " + uri);

        LokClipboardData data = LokClipboardData.createFromFile(file);
        if (data == null)
            throw new FileNotFoundException("corrupt clipboard data for " + uri);
        return data;
    }

    private static byte[] entryFor(LokClipboardData data, String mimeType) {
        String wanted = MIME_HTML.equals(mimeType) ? MIME_HTML : MIME_PLAIN;
        for (LokClipboardEntry entry : data.clipboardEntries) {
            if (entry.mime != null && entry.mime.startsWith(wanted))
                return entry.data;
        }
        return null;
    }

    private static String streamTypeFor(String mimeTypeFilter) {
        if (mimeTypeFilter == null)
            return null;
        if (ClipDescription.compareMimeTypes(MIME_HTML, mimeTypeFilter)
                && !ClipDescription.compareMimeTypes(MIME_PLAIN, mimeTypeFilter))
            return MIME_HTML;
        if (ClipDescription.compareMimeTypes(MIME_PLAIN, mimeTypeFilter))
            return MIME_PLAIN;
        return null;
    }

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    public String getType(Uri uri) {
        return MIME_HTML;
    }

    @Override
    public String[] getStreamTypes(Uri uri, String mimeTypeFilter) {
        LokClipboardData data;
        try {
            data = dataFor(uri);
        } catch (FileNotFoundException e) {
            return null;
        }

        boolean hasHtml = entryFor(data, MIME_HTML) != null
            && ClipDescription.compareMimeTypes(MIME_HTML, mimeTypeFilter);
        boolean hasPlain = entryFor(data, MIME_PLAIN) != null
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
        LokClipboardData data = dataFor(uri);

        String streamType = streamTypeFor(mimeTypeFilter);
        byte[] bytes = streamType != null ? entryFor(data, streamType) : null;
        if (bytes == null && MIME_PLAIN.equals(streamType)) {
            // no plain text variant... serve the html one rather than nothing
            bytes = entryFor(data, MIME_HTML);
        }
        if (bytes == null)
            throw new FileNotFoundException("no stream of type " + mimeTypeFilter + " for " + uri);

        final byte[] streamBytes = bytes;
        ParcelFileDescriptor readSide = openPipeHelper(uri, streamType, opts, streamBytes,
            new PipeDataWriter<byte[]>() {
                @Override
                public void writeDataToPipe(ParcelFileDescriptor output, Uri uri, String mimeType,
                                            Bundle opts, byte[] args) {
                    try (FileOutputStream out =
                             new FileOutputStream(output.getFileDescriptor())) {
                        out.write(args);
                        out.flush();
                    } catch (IOException e) {
                        Log.w(TAG, "failed to stream clipboard data: " + e);
                    }
                }
            });
        return new AssetFileDescriptor(readSide, 0, AssetFileDescriptor.UNKNOWN_LENGTH);
    }

    @Override
    public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs,
                        String sortOrder) {
        if (projection == null)
            projection = new String[] { OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE };

        MatrixCursor cursor = new MatrixCursor(projection, 1);
        Object[] row = new Object[projection.length];
        for (int i = 0; i < projection.length; ++i) {
            if (OpenableColumns.DISPLAY_NAME.equals(projection[i]))
                row[i] = "clipboard.html";
            else
                row[i] = null;
        }
        cursor.addRow(row);
        return cursor;
    }

    @Override
    public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException {
        throw new FileNotFoundException("use openTypedAssetFile");
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
