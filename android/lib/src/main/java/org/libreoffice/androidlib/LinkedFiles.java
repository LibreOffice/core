/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.content.ContentResolver;
import android.content.UriPermission;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * The files a document links to, the chapters of a master document for example, by their path
 * relative to the document. A document opened from a document provider is a copy without its
 * folder, so the files are found in a folder tree the user allowed, next to the document there.
 */
final class LinkedFiles {
    private static final String TAG = "LinkedFiles";

    private LinkedFiles() {
    }

    /** Whether the user allowed the app to write in the tree of folders, not only to read it. */
    static boolean canWrite(ContentResolver resolver, Uri treeUri) {
        for (UriPermission permission : resolver.getPersistedUriPermissions()) {
            if (permission.getUri().equals(treeUri))
                return permission.isWritePermission();
        }
        return false;
    }

    /** The trees of folders the user allowed the app to read. */
    static List<Uri> allowedTrees(ContentResolver resolver) {
        List<Uri> trees = new ArrayList<>();
        for (UriPermission permission : resolver.getPersistedUriPermissions()) {
            if (permission.isReadPermission() && DocumentsContract.isTreeUri(permission.getUri()))
                trees.add(permission.getUri());
        }
        return trees;
    }

    /**
     * The document with the given path relative to the document documentUri, inside the tree
     * treeUri. Null when the tree does not hold the document or that path.
     */
    static Uri resolve(ContentResolver resolver, Uri treeUri, Uri documentUri, String relativePath) {
        if (!treeUri.getAuthority().equals(documentUri.getAuthority()))
            return null;

        try {
            Uri inTree = DocumentsContract.buildDocumentUriUsingTree(
                    treeUri, DocumentsContract.getDocumentId(documentUri));
            DocumentsContract.Path path = DocumentsContract.findDocumentPath(resolver, inTree);
            if (path == null)
                return null;

            // From the root of the tree to the folder of the document.
            List<String> folders = new ArrayList<>(path.getPath());
            folders.remove(folders.size() - 1);
            for (String segment : relativePath.split("/")) {
                if (segment.isEmpty() || segment.equals("."))
                    continue;
                if (segment.equals("..")) {
                    // Above the root of the tree is not readable.
                    if (folders.size() <= 1)
                        return null;
                    folders.remove(folders.size() - 1);
                    continue;
                }
                String child = findChild(resolver, treeUri, folders.get(folders.size() - 1),
                        segment);
                if (child == null)
                    return null;
                folders.add(child);
            }
            return DocumentsContract.buildDocumentUriUsingTree(treeUri, folders.get(folders.size() - 1));
        } catch (Exception e) {
            Log.w(TAG, "Cannot find " + relativePath + " next to " + documentUri + " in " + treeUri, e);
            return null;
        }
    }

    /** The document id of the child with the given name of a folder of the tree. */
    private static String findChild(ContentResolver resolver, Uri treeUri, String folderId, String name) {
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, folderId);
        String[] projection = { DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                                DocumentsContract.Document.COLUMN_DISPLAY_NAME };
        try (Cursor cursor = resolver.query(children, projection, null, null, null)) {
            while (cursor != null && cursor.moveToNext()) {
                if (name.equals(cursor.getString(1)))
                    return cursor.getString(0);
            }
        }
        return null;
    }

    /** Removes a copy made by copyToCache, with the folder it was made in. */
    static void removeCopy(File copy) {
        File folder = copy.getParentFile();
        if (copy.exists() && !copy.delete())
            Log.w(TAG, "Cannot remove the copy " + copy);
        if (folder != null && !folder.delete())
            Log.w(TAG, "Cannot remove the folder " + folder);
    }

    /** Copies the document uri to a file of its own name in a new folder under cacheDir. */
    static File copyToCache(ContentResolver resolver, Uri uri, File cacheDir, String name) {
        File folder = new File(cacheDir, "linked/" + UUID.randomUUID());
        if (!folder.mkdirs())
            return null;
        File copy = new File(folder, name);
        if (copyTo(resolver, uri, copy))
            return copy;
        // A copy that failed leaves neither a part of the file nor the folder behind.
        removeCopy(copy);
        return null;
    }

    private static boolean copyTo(ContentResolver resolver, Uri uri, File copy) {
        try (InputStream in = resolver.openInputStream(uri)) {
            if (in == null)
                return false;
            try (OutputStream out = new FileOutputStream(copy)) {
                byte[] buffer = new byte[64 * 1024];
                int read;
                while ((read = in.read(buffer)) != -1)
                    out.write(buffer, 0, read);
            }
            return true;
        } catch (Exception e) {
            Log.w(TAG, "Cannot copy " + uri, e);
            return false;
        }
    }
}
