/* -*- Mode: Kotlin; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib

import android.content.ContentResolver
import android.net.Uri
import android.provider.DocumentsContract
import android.util.Log
import java.io.File
import java.io.FileOutputStream
import java.util.UUID

/**
 * The files a document links to, the chapters of a master document for example, by their path
 * relative to the document. A document opened from a document provider is a copy without its
 * folder, so the files are found in a folder tree the user allowed, next to the document there.
 */
internal object LinkedFiles {
    private const val TAG = "LinkedFiles"

    /** Whether the user allowed the app to write in the tree of folders, not only to read it. */
    @JvmStatic
    fun canWrite(resolver: ContentResolver, treeUri: Uri): Boolean =
        resolver.persistedUriPermissions.firstOrNull { it.uri == treeUri }?.isWritePermission
            ?: false

    /** The trees of folders the user allowed the app to read. */
    @JvmStatic
    fun allowedTrees(resolver: ContentResolver): List<Uri> =
        resolver.persistedUriPermissions
            .filter { it.isReadPermission && DocumentsContract.isTreeUri(it.uri) }
            .map { it.uri }

    /**
     * The document with the given path relative to the document documentUri, inside the tree
     * treeUri. Null when the tree does not hold the document or that path.
     */
    @JvmStatic
    fun resolve(resolver: ContentResolver, treeUri: Uri, documentUri: Uri,
                relativePath: String): Uri? {
        if (treeUri.authority != documentUri.authority)
            return null

        try {
            val inTree = DocumentsContract.buildDocumentUriUsingTree(
                treeUri, DocumentsContract.getDocumentId(documentUri))
            val path = DocumentsContract.findDocumentPath(resolver, inTree) ?: return null

            // From the root of the tree to the folder of the document.
            val folders = ArrayList(path.path)
            folders.removeAt(folders.size - 1)
            for (segment in relativePath.split("/")) {
                if (segment.isEmpty() || segment == ".")
                    continue
                if (segment == "..") {
                    // Above the root of the tree is not readable.
                    if (folders.size <= 1)
                        return null
                    folders.removeAt(folders.size - 1)
                    continue
                }
                val child = findChild(resolver, treeUri, folders.last(), segment) ?: return null
                folders.add(child)
            }
            return DocumentsContract.buildDocumentUriUsingTree(treeUri, folders.last())
        } catch (e: Exception) {
            Log.w(TAG, "Cannot find $relativePath next to $documentUri in $treeUri", e)
            return null
        }
    }

    /** The document id of the child with the given name of a folder of the tree. */
    private fun findChild(resolver: ContentResolver, treeUri: Uri, folderId: String,
                          name: String): String? {
        val children = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, folderId)
        val projection = arrayOf(DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                                 DocumentsContract.Document.COLUMN_DISPLAY_NAME)
        resolver.query(children, projection, null, null, null)?.use { cursor ->
            while (cursor.moveToNext()) {
                if (name == cursor.getString(1))
                    return cursor.getString(0)
            }
        }
        return null
    }

    /** Removes a copy made by copyToCache, with the folder it was made in. */
    @JvmStatic
    fun removeCopy(copy: File) {
        val folder = copy.parentFile
        if (copy.exists() && !copy.delete())
            Log.w(TAG, "Cannot remove the copy $copy")
        if (folder != null && !folder.delete())
            Log.w(TAG, "Cannot remove the folder $folder")
    }

    /** Copies the document uri to a file of its own name in a new folder under cacheDir. */
    @JvmStatic
    fun copyToCache(resolver: ContentResolver, uri: Uri, cacheDir: File, name: String): File? {
        val folder = File(cacheDir, "linked/" + UUID.randomUUID())
        if (!folder.mkdirs())
            return null
        val copy = File(folder, name)
        if (copyTo(resolver, uri, copy))
            return copy
        // A copy that failed leaves neither a part of the file nor the folder behind.
        removeCopy(copy)
        return null
    }

    private fun copyTo(resolver: ContentResolver, uri: Uri, copy: File): Boolean {
        try {
            val input = resolver.openInputStream(uri) ?: return false
            input.use {
                FileOutputStream(copy).use { output -> input.copyTo(output, 64 * 1024) }
            }
            return true
        } catch (e: Exception) {
            Log.w(TAG, "Cannot copy $uri", e)
            return false
        }
    }
}
