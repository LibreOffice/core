/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.ui;

import org.libreoffice.storage.IFile;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.util.Map;
import java.util.Collections;
import java.util.List;
import java.util.HashMap;
import java.util.Comparator;
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

    static final int SORT_AZ = 0;
    static final int SORT_ZA = 1;
    /** Oldest Files First*/
    static final int SORT_OLDEST = 2;
    /** Newest Files First*/
    static final int SORT_NEWEST = 3;
    /** Largest Files First */
    static final int SORT_LARGEST = 4;
    /** Smallest Files First */
    static final int SORT_SMALLEST = 5;

    private static final Map<String, Integer> mExtnMap = new HashMap<String, Integer>();
    private static final Map<String, String> extensionToMimeTypeMap = new HashMap<String, String>();
    static {
        // Please keep this in sync with AndroidManifest.xml

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
        extensionToMimeTypeMap.put("odg", "application/vnd.oasis.opendocument.graphics");
        extensionToMimeTypeMap.put("otg", "application/vnd.oasis.opendocument.graphics-template");
        extensionToMimeTypeMap.put("odi", "application/vnd.oasis.opendocument.image");
        extensionToMimeTypeMap.put("odp", "application/vnd.oasis.opendocument.presentation");
        extensionToMimeTypeMap.put("otp", "application/vnd.oasis.opendocument.presentation-template");
        extensionToMimeTypeMap.put("ods", "application/vnd.oasis.opendocument.spreadsheet");
        extensionToMimeTypeMap.put("ots", "application/vnd.oasis.opendocument.spreadsheet-template");
        extensionToMimeTypeMap.put("odt", "application/vnd.oasis.opendocument.text");
        extensionToMimeTypeMap.put("odm", "application/vnd.oasis.opendocument.text-master");
        extensionToMimeTypeMap.put("ott", "application/vnd.oasis.opendocument.text-template");
        extensionToMimeTypeMap.put("oth", "application/vnd.oasis.opendocument.text-web");
    }

    private static final String getExtension(String filename) {
        if (filename == null)
            return "";
        int nExt = filename.lastIndexOf('.');
        if (nExt < 0)
            return "";
        return filename.substring(nExt);
    }

    private static final int lookupExtension(String filename) {
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

        if (byMode == ALL && byFilename.equals("")) {
            if (filename.startsWith(".")) {//ignore hidden files
                return false;
            }
            return true;
        }
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

    static FileFilter getFileFilter(final int mode) {
        return new FileFilter() {
            public boolean accept(File pathname) {
                if (pathname.isDirectory())
                    return true;
                if (lookupExtension(pathname.getName()) == UNKNOWN)
                    return false;
                return doAccept(pathname.getName(), mode, "");
            }
        };
    }

    static FilenameFilter getFilenameFilter(final int mode) {
        return new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                if (new File(dir , filename).isDirectory())
                    return true;
                return doAccept(filename, mode, "");
            }
        };
    }

    static void sortFiles(List<IFile> files, int sortMode) {
        if (files == null)
            return;
        switch (sortMode) {
            case SORT_AZ:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return lhs.getName().compareTo(rhs.getName());
                    }
                });
                break;
            case SORT_ZA:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return rhs.getName().compareTo(lhs.getName());
                    }
                });
                break;
            case SORT_OLDEST:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return lhs.getLastModified().compareTo(rhs.getLastModified());
                    }
                });
                break;
            case SORT_NEWEST:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return rhs.getLastModified().compareTo(lhs.getLastModified());
                    }
                });
                break;
            case SORT_LARGEST:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return Long.valueOf(rhs.getSize()).compareTo(lhs.getSize());
                    }
                });
                break;
            case SORT_SMALLEST:
                Collections.sort(files , new Comparator<IFile>() {
                    public int compare(IFile lhs, IFile rhs) {
                        return Long.valueOf(lhs.getSize()).compareTo(rhs.getSize());
                    }
                });
                break;
            default:
                Log.e(LOGTAG, "uncatched sortMode: " + sortMode);
        }
        return;
    }

    static boolean isHidden(File file) {
        if (file.getName().startsWith("."))
            return true;
        return false;
    }

    static boolean isThumbnail(File file) {
        if (isHidden(file) && file.getName().endsWith(".png"))
            return true;
        return false;
    }

    static boolean hasThumbnail(File file) {
        String filename = file.getName();
        if (lookupExtension(filename) == DOC) // only do this for docs for now
        {
            // Will need another method to check if Thumb is up-to-date - or extend this one?
            if (new File(file.getParent() , getThumbnailName(file)).isFile())
                return true;
            return false; // If it's a document with no thumb
        }
        return true;
    }

    static String getThumbnailName(File file) {
        return "." + file.getName().split("[.]")[0] + ".png" ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
