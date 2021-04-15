/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.ui;

import java.io.File;
import java.util.Map;
import java.util.HashMap;

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import android.util.Log;
import android.webkit.MimeTypeMap;

public class FileUtilities {

    private static String LOGTAG = FileUtilities.class.getSimpleName();

    static final int ALL = -1;

    // These have to be in sync with the file_view_modes resource.
    static final int DOC = 0;
    static final int CALC = 1;
    static final int IMPRESS = 2;
    static final int DRAWING = 3;

    static final int UNKNOWN = 10;

    public static final String DEFAULT_WRITER_EXTENSION = ".odt";
    public static final String DEFAULT_IMPRESS_EXTENSION = ".odp";
    public static final String DEFAULT_SPREADSHEET_EXTENSION = ".ods";
    public static final String DEFAULT_DRAWING_EXTENSION = ".odg";

    public static final String MIMETYPE_OPENDOCUMENT_TEXT = "application/vnd.oasis.opendocument.text";
    public static final String MIMETYPE_OPENDOCUMENT_SPREADSHEET = "application/vnd.oasis.opendocument.spreadsheet";
    public static final String MIMETYPE_OPENDOCUMENT_PRESENTATION = "application/vnd.oasis.opendocument.presentation";
    public static final String MIMETYPE_OPENDOCUMENT_GRAPHICS = "application/vnd.oasis.opendocument.graphics";
    public static final String MIMETYPE_PDF = "application/pdf";

    private static final Map<String, Integer> mExtnMap = new HashMap<String, Integer>();
    private static final Map<String, String> extensionToMimeTypeMap = new HashMap<String, String>();
    static {
        // Please keep this in sync with AndroidManifest.xml
        // and 'SUPPORTED_MIME_TYPES' in LibreOfficeUIActivity.java

        // ODF
        mExtnMap.put(".odt", DOC);
        mExtnMap.put(".odg", DRAWING);
        mExtnMap.put(".odp",  IMPRESS);
        mExtnMap.put(".ods",  CALC);
        mExtnMap.put(".fodt", DOC);
        mExtnMap.put(".fodg", DRAWING);
        mExtnMap.put(".fodp",  IMPRESS);
        mExtnMap.put(".fods",  CALC);

        // ODF templates
        mExtnMap.put(".ott", DOC);
        mExtnMap.put(".otg", DRAWING);
        mExtnMap.put(".otp",  IMPRESS);
        mExtnMap.put(".ots",  CALC);

        // MS
        mExtnMap.put(".rtf",  DOC);
        mExtnMap.put(".doc",  DOC);
        mExtnMap.put(".vsd", DRAWING);
        mExtnMap.put(".vsdx", DRAWING);
        mExtnMap.put(".pub", DRAWING);
        mExtnMap.put(".ppt",  IMPRESS);
        // mExtnMap.put(".pps",  IMPRESS);
        mExtnMap.put(".xls",  CALC);

        // MS templates
        mExtnMap.put(".dot",  DOC);
        mExtnMap.put(".pot",  IMPRESS);
        mExtnMap.put(".xlt",  CALC);

        // OOXML
        mExtnMap.put(".docx", DOC);
        mExtnMap.put(".pptx", IMPRESS);
        // mExtnMap.put(".ppsx", IMPRESS);
        mExtnMap.put(".xlsx", CALC);

        // OOXML templates
        mExtnMap.put(".dotx", DOC);
        mExtnMap.put(".potx", IMPRESS);
        mExtnMap.put(".xltx", CALC);

        // Other
        mExtnMap.put(".csv",  CALC);
        mExtnMap.put(".wps",  DOC);
        mExtnMap.put(".key",  IMPRESS);
        mExtnMap.put(".abw",  DOC);
        mExtnMap.put(".pmd",  DRAWING);
        mExtnMap.put(".emf",  DRAWING);
        mExtnMap.put(".svm",  DRAWING);
        mExtnMap.put(".wmf",  DRAWING);
        mExtnMap.put(".svg",  DRAWING);

        // Some basic MIME types
        // Android's MimeTypeMap lacks some types that we need
        extensionToMimeTypeMap.put("odb", "application/vnd.oasis.opendocument.database");
        extensionToMimeTypeMap.put("odf", "application/vnd.oasis.opendocument.formula");
        extensionToMimeTypeMap.put("odg", MIMETYPE_OPENDOCUMENT_GRAPHICS);
        extensionToMimeTypeMap.put("otg", "application/vnd.oasis.opendocument.graphics-template");
        extensionToMimeTypeMap.put("odi", "application/vnd.oasis.opendocument.image");
        extensionToMimeTypeMap.put("odp", MIMETYPE_OPENDOCUMENT_PRESENTATION);
        extensionToMimeTypeMap.put("otp", "application/vnd.oasis.opendocument.presentation-template");
        extensionToMimeTypeMap.put("ods", MIMETYPE_OPENDOCUMENT_SPREADSHEET);
        extensionToMimeTypeMap.put("ots", "application/vnd.oasis.opendocument.spreadsheet-template");
        extensionToMimeTypeMap.put("odt", MIMETYPE_OPENDOCUMENT_TEXT);
        extensionToMimeTypeMap.put("odm", "application/vnd.oasis.opendocument.text-master");
        extensionToMimeTypeMap.put("ott", "application/vnd.oasis.opendocument.text-template");
        extensionToMimeTypeMap.put("oth", "application/vnd.oasis.opendocument.text-web");
    }

    public static String getExtension(String filename) {
        if (filename == null)
            return "";
        int nExt = filename.lastIndexOf('.');
        if (nExt < 0)
            return "";
        return filename.substring(nExt);
    }

    private static int lookupExtension(String filename) {
        String extn = getExtension(filename);
        if (!mExtnMap.containsKey(extn))
            return UNKNOWN;
        return mExtnMap.get(extn);
    }

    static int getType(String filename) {
        int type = lookupExtension (filename);
        Log.d(LOGTAG, "extn : " + filename + " -> " + type);
        return type;
    }

    static String getMimeType(String filename) {
        String extension = MimeTypeMap.getFileExtensionFromUrl(filename);
        String mime = extensionToMimeTypeMap.get(extension);
        if (mime == null) {
            //fallback to Android's MimeTypeMap
            mime = MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                    extension);
        }
        return mime;
    }

    // Filter by mode, and/or in future by filename/wildcard
    private static boolean doAccept(String filename, int byMode, String byFilename) {
        Log.d(LOGTAG, "doAccept : " + filename + " mode " + byMode + " byFilename " + byFilename);
        if (filename == null)
            return false;

        // check extension
        if (byMode != ALL) {
            if (mExtnMap.get (getExtension (filename)) != byMode)
                return false;
        }
        if (!byFilename.equals("")) {
            // FIXME return false on a non-match
        }
        return true;
    }

    static boolean isHidden(File file) {
        return file.getName().startsWith(".");
    }

    static boolean isThumbnail(File file) {
        return isHidden(file) && file.getName().endsWith(".png");
    }

    /**
     * Tries to retrieve the display (which should be the document name)
     * for the given URI using the given resolver.
     */
    public static String retrieveDisplayNameForDocumentUri(ContentResolver resolver, Uri docUri) {
        String displayName = "";
        // try to retrieve original file name
        Cursor cursor = null;
        try {
            String[] columns = {OpenableColumns.DISPLAY_NAME};
            cursor = resolver.query(docUri, columns, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                displayName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return displayName;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
