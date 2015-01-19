/* -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.ui;

import org.libreoffice.R;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.util.Map;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Comparator;
import android.util.Log;

public class FileUtilities {
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

    private static final Map<String,Integer> mExtnMap = new HashMap<String,Integer>();
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
        // mExtnMap.put(".fods",  CALC);

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
    }

    private static final String getExtension(String filename)
    {
        if (filename == null)
            return "";
        int nExt = filename.lastIndexOf('.');
        if (nExt < 0)
            return "";
        return filename.substring(nExt);
    }

    private static final int lookupExtension(String filename)
    {
        String extn = getExtension (filename);
        if (!mExtnMap.containsKey(extn))
            return UNKNOWN;
        return mExtnMap.get (extn);
    }

    static int getType(String filename)
    {
        int type = lookupExtension (filename);
        android.util.Log.d("debug", "extn : " + filename + " -> " + type);
        return type;
    }

    // Filter by mode, and/or in future by filename/wildcard
    static private boolean doAccept(String filename, int byMode, String byFilename)
    {
    android.util.Log.d("debug", "doAccept : " + filename + " mode " + byMode + " byFilename " + byFilename);
    if (filename == null)
        return false;

    if (byMode == ALL && byFilename == "") {
        if( filename.startsWith(".")) {//ignore hidden files
            return false;
        }
        return true;
    }
    // check extension
    if (byMode != ALL) {
        if (mExtnMap.get (getExtension (filename)) != byMode)
            return false;
    }
    if (byFilename != "") {
        // FIXME return false on a non-match
    }
    return true;
    }

    static FileFilter getFileFilter(final int mode)
    {
    return new FileFilter() {
        public boolean accept(File pathname) {
        if (pathname.isDirectory())
            return true;
        if( lookupExtension(pathname.getName()) == UNKNOWN)
            return false;
        return doAccept(pathname.getName(), mode, "");
        }
    };
    }

    static FilenameFilter getFilenameFilter(final int mode)
    {
        return new FilenameFilter() {
            public boolean accept(File dir, String filename) {
                if( new File( dir , filename ).isDirectory() )
                    return true;
                return doAccept(filename, mode, "");
            }
        };
    }

    static void sortFiles(File[] files , int sortMode)
    {
        // Should really change all this to a switch statement...
        if( sortMode == SORT_AZ ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return lhs.getName().compareTo( rhs.getName() );
                }
            });
            return;
        }
        if( sortMode == SORT_ZA ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return rhs.getName().compareTo( lhs.getName() );
                }
            });
            return;
        }
        if( sortMode == SORT_OLDEST ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return Long.valueOf( lhs.lastModified() ).compareTo( rhs.lastModified() );
                }
            });
            return;
        }
        if( sortMode == SORT_NEWEST ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return Long.valueOf( rhs.lastModified() ).compareTo( lhs.lastModified() );
                }
            });
            return;
        }
        if( sortMode == SORT_LARGEST ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return Long.valueOf( rhs.length() ).compareTo( lhs.length() );
                }
            });
            return;
        }
        if( sortMode == SORT_SMALLEST ){
            Arrays.sort( files , new Comparator<File>() {
                public int compare(File lhs, File rhs) {
                    return Long.valueOf( lhs.length() ).compareTo( rhs.length() );
                }
            });
            return;
        }
        return;
    }

    static boolean isHidden( File file ){
        if( file.getName().startsWith(".") )
            return true;
        return false;
    }

    static boolean isThumbnail( File file ){
        if( isHidden(file) && file.getName().endsWith(".png") )
            return true;
        return false;
    }

    static boolean hasThumbnail(File file)
    {
        String filename = file.getName();
        if( lookupExtension( filename ) == DOC ) // only do this for docs for now
        {
            // Will need another method to check if Thumb is up-to-date - or extend this one?
            if( new File( file.getParent() , getThumbnailName( file ) ).isFile() )
                return true;
            return false; // If it's a document with no thumb
        }
        return true;
    }

    static String getThumbnailName( File file )
    {
        return "." + file.getName().split("[.]")[0] + ".png" ;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
