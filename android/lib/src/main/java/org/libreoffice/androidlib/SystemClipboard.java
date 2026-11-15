/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.libreoffice.androidlib;

import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;
import android.net.Uri;
import android.os.Process;
import android.os.SystemClock;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;

import com.android.tools.r8.keepanno.annotations.KeepItemKind;
import com.android.tools.r8.keepanno.annotations.UsedByNative;

import org.libreoffice.androidlib.lok.LokClipboardData;
import org.libreoffice.androidlib.lok.LokClipboardEntry;

/**
 * The system clipboard side of the engine's clipboard provider: writes the engine's copy to the
 * system clipboard and reads a foreign one back. The engine picks the formats in both directions.
 *
 * The native side calls these on the kit thread, so nothing here touches the WebView or an
 * activity. ClipboardManager works from any thread.
 */
@UsedByNative(kind = KeepItemKind.ONLY_CLASS) // AndroidClipboard.cpp resolveSystemClipboardClass
public class SystemClipboard {
    private static final String TAG = "SystemClipboard";

    /// prefix of the clip label of a copy made by this app, in any of its processes
    private static final String SIGNATURE_PREFIX = "cool-clip-magic-4a22437e49a8-";

    private static final int SYSTEM_LIMIT_BYTES = 512 * 1024;

    private static final String MIME_HTML = "text/html";
    private static final String MIME_PLAIN_UTF8 = "text/plain;charset=utf-8";

    /// label of the clips this process writes: the prefix plus an id of this process
    private static final String sSignature =
        SIGNATURE_PREFIX + Process.myPid() + "-" + SystemClock.elapsedRealtime();

    private static Context sContext;
    private static ClipboardManager sClipboardManager;

    /// clipboard file of an earlier process, parsed once
    private static File sParsedFile;
    private static LokClipboardData sParsedData;

    public static void initialize(Context context) {
        sContext = context.getApplicationContext();
        sClipboardManager = (ClipboardManager) sContext.getSystemService(Context.CLIPBOARD_SERVICE);
    }

    /// Copy: put every format the engine offers on the system clipboard
    @UsedByNative // AndroidClipboard.cpp clipboardProviderAdvertise
    public static void advertise(String[] mimeTypes, byte[][] data) {
        if (sClipboardManager == null) {
            Log.e(TAG, "advertise called before initialize");
            return;
        }

        LokClipboardData clipboardData = new LokClipboardData();
        for (int i = 0; i < mimeTypes.length; ++i) {
            LokClipboardEntry entry = new LokClipboardEntry();
            entry.mime = mimeTypes[i];
            entry.data = data[i];
            clipboardData.clipboardEntries.add(entry);
        }

        // The full content always goes to a file: a later process of the app reads it
        // back from there. The HTML and the plain text also go to files of their own
        // next to it, for other apps, both without any size constraint.
        File clipboardFile = newClipboardFile();
        if (clipboardFile != null && !clipboardData.writeToFile(clipboardFile)) {
            Log.e(TAG, "failed to write the clipboard file");
            clipboardFile = null;
        }

        String text = clipboardData.getText();
        String html = clipboardData.getHtml();
        if (clipboardFile != null) {
            if (html != null)
                writeSideFile(clipboardFile, "html", entryBytes(clipboardData, MIME_HTML));
            if (text != null)
                writeSideFile(clipboardFile, "txt", entryBytes(clipboardData, "text/plain"));
        }
        Uri clipUri = clipboardFile != null
            ? ClipboardFileProvider.uriFor(sContext, clipboardFile.getName()) : null;

        if (text == null && html == null && clipUri == null) {
            Log.w(TAG, "nothing to put on the system clipboard");
            return;
        }

        // a copy with neither text nor HTML, such as a shape, is only read back by this app;
        // other apps get an empty text for it
        ArrayList<String> declaredTypes = new ArrayList<>();
        if (html != null)
            declaredTypes.add(ClipDescription.MIMETYPE_TEXT_HTML);
        if (text != null)
            declaredTypes.add(ClipDescription.MIMETYPE_TEXT_PLAIN);
        if (declaredTypes.isEmpty())
            declaredTypes.add(ClipDescription.MIMETYPE_TEXT_URILIST);

        // Binder transactions share a ~1 MB buffer per process, and larger
        // setPrimaryClip() calls throw TransactionTooLargeException
        // (strings are parceled as UTF-16, two bytes per char). Oversized
        // content is therefore not inlined into the clip - receivers
        // stream it from clipUri instead, which coerceToText() /
        // coerceToHtmlText() do transparently when there is no inline text.
        int estimatedParcelSize =
            2 * ((html != null ? html.length() : 0) + (text != null ? text.length() : 0));
        boolean inline = estimatedParcelSize <= SYSTEM_LIMIT_BYTES || clipUri == null;

        Log.d(TAG, "advertise: engine formats " + String.join(", ", mimeTypes) + "; html "
              + (html != null ? html.length() + " chars" : "none") + ", text "
              + (text != null ? text.length() + " chars" : "none") + ", inline " + inline
              + ", file " + (clipUri != null));

        ClipData clipData = makeClipData(inline ? text : null, inline ? html : null, clipUri,
                                         declaredTypes);
        try {
            sClipboardManager.setPrimaryClip(clipData);
        } catch (RuntimeException e) {
            // Some vendors reject clips well below the nominal limit; a
            // URI-only clip is tiny and always fits.
            Log.w(TAG, "setPrimaryClip failed, retrying with the URI only: " + e);
            if (clipUri != null)
                sClipboardManager.setPrimaryClip(makeClipData(null, null, clipUri, declaredTypes));
        }
    }

    /// true while the system clipboard still holds the copy this process made last
    @UsedByNative // AndroidClipboard.cpp clipboardProviderOwns
    public static boolean ownsClipboard() {
        ClipDescription description = primaryClipDescription();
        CharSequence label = description != null ? description.getLabel() : null;
        return label != null && sSignature.contentEquals(label);
    }

    /// Paste: the mime types the system clipboard offers. The engine lists the formats more
    /// often than it pastes, for example to see if paste is possible at the cursor, and reading
    /// a foreign clip shows the Android 12 clipboard notice. So the types come from the clip
    /// description, and the clip itself is read only for our own clip of an earlier process,
    /// which the system does not announce, or for a clip that declares only a list of URIs.
    @UsedByNative // AndroidClipboard.cpp clipboardProviderGetMimeTypes
    public static String[] getMimeTypes() {
        ArrayList<String> mimeTypes = new ArrayList<>();
        ClipDescription description = primaryClipDescription();
        if (description == null)
            return mimeTypes.toArray(new String[0]);

        // our own clip from an earlier process: the formats are in the clipboard file, and
        // there is no notice for reading our own clip
        if (isEarlierProcessLabel(description.getLabel())) {
            LokClipboardData earlierProcessData = earlierProcessData(primaryClip());
            if (earlierProcessData != null) {
                for (LokClipboardEntry entry : earlierProcessData.clipboardEntries) {
                    if (entry.mime != null && !mimeTypes.contains(entry.mime))
                        mimeTypes.add(entry.mime);
                }
                return mimeTypes.toArray(new String[0]);
            }
        }

        // foreign clip; an image shared from a gallery app declares only text/uri-list
        for (int i = 0; i < description.getMimeTypeCount(); ++i) {
            String mimeType = description.getMimeType(i);
            if (ClipDescription.MIMETYPE_TEXT_HTML.equals(mimeType))
                mimeType = MIME_HTML;
            else if (ClipDescription.MIMETYPE_TEXT_PLAIN.equals(mimeType))
                mimeType = MIME_PLAIN_UTF8;
            else if (ClipDescription.MIMETYPE_TEXT_URILIST.equals(mimeType))
                mimeType = imageMimeTypeOf(primaryClip());
            else if (!mimeType.startsWith("image/"))
                mimeType = null;
            if (mimeType != null && !mimeTypes.contains(mimeType))
                mimeTypes.add(mimeType);
        }

        return mimeTypes.toArray(new String[0]);
    }

    /// Paste: the bytes of one mime type, or null when the clipboard does not hold it
    @UsedByNative // AndroidClipboard.cpp clipboardProviderGetData
    public static byte[] getData(String mimeType) {
        ClipData clipData = primaryClip();
        if (clipData == null)
            return null;

        LokClipboardData earlierProcessData = earlierProcessData(clipData);
        if (earlierProcessData != null) {
            for (LokClipboardEntry entry : earlierProcessData.clipboardEntries) {
                if (mimeType.equals(entry.mime))
                    return entry.data;
            }
            return null;
        }

        ClipData.Item item = clipData.getItemAt(0);
        if (MIME_HTML.equals(mimeType)) {
            String html = item.coerceToHtmlText(sContext);
            return html != null && !html.isEmpty() ? html.getBytes(StandardCharsets.UTF_8) : null;
        }
        if (MIME_PLAIN_UTF8.equals(mimeType)) {
            CharSequence text = item.coerceToText(sContext);
            return text != null ? text.toString().getBytes(StandardCharsets.UTF_8) : null;
        }
        if (mimeType.startsWith("image/")) {
            Uri uri = item.getUri();
            return uri != null ? readUriContent(uri) : null;
        }
        return null;
    }

    private static ClipDescription primaryClipDescription() {
        if (sClipboardManager == null)
            return null;
        return sClipboardManager.getPrimaryClipDescription();
    }

    /// the primary clip when it has at least one item, else null
    private static ClipData primaryClip() {
        if (sClipboardManager == null)
            return null;
        ClipData clipData = sClipboardManager.getPrimaryClip();
        if (clipData == null || clipData.getDescription() == null || clipData.getItemCount() < 1)
            return null;
        return clipData;
    }

    private static boolean isEarlierProcessLabel(CharSequence label) {
        return label != null && label.toString().startsWith(SIGNATURE_PREFIX)
            && !sSignature.contentEquals(label);
    }

    /// the clipboard file content of a clip from an earlier process of this app, else null
    private static LokClipboardData earlierProcessData(ClipData clipData) {
        if (clipData == null || !isEarlierProcessLabel(clipData.getDescription().getLabel()))
            return null;

        File clipboardFile =
            ClipboardFileProvider.fileFor(sContext, clipData.getItemAt(0).getUri());
        if (clipboardFile == null || !clipboardFile.exists())
            return null;

        if (!clipboardFile.equals(sParsedFile)) {
            sParsedData = LokClipboardData.createFromFile(clipboardFile);
            sParsedFile = clipboardFile;
        }
        return sParsedData;
    }

    /// a clipboard carrying our magic in the label, the full-fidelity data
    /// behind 'uri', and - when small enough - the inline text and HTML
    private static ClipData makeClipData(String text, String html, Uri uri,
                                         ArrayList<String> declaredTypes) {
        // ClipData.Item refuses HTML without plain text
        if (html != null && text == null)
            text = "";
        // A clip that declares only the URI list holds no text or HTML for other apps, and
        // coerceToText would give them the URI as a string, so the item carries an empty text.
        if (text == null && declaredTypes.size() == 1
                && ClipDescription.MIMETYPE_TEXT_URILIST.equals(declaredTypes.get(0)))
            text = "";
        ClipDescription description =
            new ClipDescription(sSignature, declaredTypes.toArray(new String[0]));
        return new ClipData(description, new ClipData.Item(text, html, null, uri));
    }

    /// create a fresh file for the current clipboard... open streams of a
    /// deleted file keep working, so a paste that is happening while we copy
    /// is allowed to finish. We prefer this than deleting/recreating due to
    /// potential permission issues if we or the smartphone vendor does that
    /// improperly...
    private static File newClipboardFile() {
        File directory = ClipboardFileProvider.getClipboardDir(sContext);
        if (!directory.isDirectory() && !directory.mkdirs()) {
            Log.e(TAG, "failed to create the clipboard directory");
            return null;
        }

        File[] oldFiles = directory.listFiles();
        if (oldFiles != null)
            for (File oldFile : oldFiles)
                oldFile.delete();

        return new File(directory, "clip-" + System.currentTimeMillis() + ".data");
    }

    private static byte[] entryBytes(LokClipboardData clipboardData, String mimePrefix) {
        for (LokClipboardEntry entry : clipboardData.clipboardEntries) {
            if (entry.mime.startsWith(mimePrefix))
                return entry.data;
        }
        return null;
    }

    /// Write one format of the copy next to the data file.
    private static void writeSideFile(File clipboardFile, String extension, byte[] content) {
        File file = ClipboardFileProvider.sideFile(clipboardFile, extension);
        try (FileOutputStream out = new FileOutputStream(file)) {
            out.write(content);
        } catch (IOException e) {
            Log.e(TAG, "failed to write " + file.getName() + ": " + e);
            file.delete();
        }
    }

    /// Image mime type of a clipboard uri, or null when it is not an image.
    private static String imageMimeTypeOf(ClipData clipData) {
        Uri uri = clipData != null ? clipData.getItemAt(0).getUri() : null;
        if (uri == null)
            return null;
        String mimeType = sContext.getContentResolver().getType(uri);
        if (mimeType != null && mimeType.startsWith("image/"))
            return mimeType;
        return null;
    }

    /// Read the whole uri content, or null when that fails.
    private static byte[] readUriContent(Uri uri) {
        try (InputStream stream = sContext.getContentResolver().openInputStream(uri)) {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] chunk = new byte[16384];
            int read;
            while ((read = stream.read(chunk, 0, chunk.length)) != -1)
                buffer.write(chunk, 0, read);
            return buffer.toByteArray();
        } catch (Exception e) {
            Log.d(TAG, "Failed to read clipboard content: " + e.getMessage());
            return null;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
