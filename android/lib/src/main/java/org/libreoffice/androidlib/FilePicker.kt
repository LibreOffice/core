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

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Intent
import android.net.Uri
import android.util.Log
import android.widget.Toast
import java.io.File
import java.util.concurrent.Executor

/** File picks of the engine. The system picker is an activity, so one pick is open at a time. */
internal class FilePicker(private val activity: LOActivity, private val executor: Executor) {
    /** Guards pendingId, so a pick gets either its answer or its cancel. */
    private val lock = Any()

    /** The open pick, or -1. It stays set while the picked file is copied. */
    private var pendingId = -1

    // executor only
    private val copies = ArrayList<File>()

    fun show(id: Int, title: String, extensions: Array<String>) {
        activity.runOnUiThread {
            // The document may have closed since the pick was handed over.
            if (activity.isDestroyed || activity.isFinishing || !activity.isDocumentLoaded) {
                activity.filePickerResult(id, null)
                return@runOnUiThread
            }

            // The engine asks for one file at a time, so a pick still open here was superseded.
            cancelPending()

            // The system picker has no title.
            Log.d(TAG, "File pick $id \"$title\" for ${extensions.joinToString(",")}")

            val intent = Intent(Intent.ACTION_OPEN_DOCUMENT)
            intent.addCategory(Intent.CATEGORY_OPENABLE)
            LOActivity.restrictToAcceptedTypes(intent, extensions)

            synchronized(lock) { pendingId = id }
            try {
                activity.startActivityForResult(intent, LOActivity.REQUEST_PICK_FILE)
            } catch (e: ActivityNotFoundException) {
                Toast.makeText(activity, activity.getString(R.string.cannot_open_file_chooser),
                               Toast.LENGTH_LONG).show()
                answer(id, null)
            }
        }
    }

    fun onActivityResult(resultCode: Int, intent: Intent?) {
        val id = synchronized(lock) { pendingId }
        // A cancelled pick has no id here.
        if (id < 0)
            return
        val uri = if (resultCode == Activity.RESULT_OK) intent?.data else null
        if (uri == null) {
            answer(id, null)
            return
        }
        executor.execute { deliver(id, uri) }
    }

    /** Cancels the open pick, also while its file is copied. Any thread. */
    fun cancelPending() {
        synchronized(lock) {
            val id = pendingId
            if (id < 0)
                return
            pendingId = -1
            activity.filePickerResult(id, null)
        }
    }

    /** False when the pick has ended already. */
    private fun answer(id: Int, url: String?): Boolean {
        synchronized(lock) {
            if (pendingId != id)
                return false
            pendingId = -1
            return activity.filePickerResult(id, url)
        }
    }

    private fun deliver(id: Int, uri: Uri) {
        val resolver = activity.contentResolver
        val name = LinkedFiles.displayName(resolver, uri)
        val copy = LinkedFiles.copyToCache(resolver, uri, activity.cacheDir, name)
        if (copy == null) {
            Log.w(TAG, "Cannot copy the picked file $uri")
            answer(id, null)
            return
        }

        // The engine reads a copy when its pick completes, so only the latest is kept.
        removeCopies()
        copies.add(copy)

        if (!answer(id, Uri.fromFile(copy).toString())) {
            // The pick ended as cancelled while the copy was made.
            removeCopies()
        }
    }

    fun removeCopies() {
        for (copy in copies)
            LinkedFiles.removeCopy(copy)
        copies.clear()
    }

    private companion object {
        const val TAG = "FilePicker"
    }
}
