/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package helper;

// __________ Imports __________

// exceptions
import java.io.File;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Iterator;


/**
 * It collects some static helper functions to handle URLs.
 * Sometimes it's necessary to convert URL from/to system paths.
 * Or from string to strutural notations (e.g. com.sun.star.util.URL).
 * And sometimes java had another notation then the office it has.
 *
 */
public class URLHelper
{


    /**
     * Because the office need URLs for loading/saving documents
     * we must convert used system paths.
     * And java use another notation for file URLs ... correct it.
     *
     * @param aSystemPath
     *          represent the file in system notation
     *
     * @return [String]
     *          a file url which represent the given system path
     */
    public static String getFileURLFromSystemPath( File aSystemPath )
    {
        String sFileURL = null;
        try
        {
            sFileURL = aSystemPath.toURI().toURL().toString();
        }
        catch( MalformedURLException exWrong )
        {
        }

        // problem of java: file URL's are coded with 1 slash instead of 2 or 3 ones!
        // => correct this problem first, otherwise office can't use these URL's
        if(
            (sFileURL                       != null ) &&
            sFileURL.startsWith("file:/") &&
            !sFileURL.startsWith("file://")
          )
        {
            StringBuffer sWorkBuffer = new StringBuffer(sFileURL);
            sWorkBuffer.insert(6,"//");
            sFileURL = sWorkBuffer.toString();
        }

        return sFileURL;
    }



    /**
     * The same as getFileURLFromSystemPath() before but uses string parameter instead
     * of a File type. It exists to suppress converting of necessary parameters in the
     * outside code. But of course getFileURLFromSystemPath(File) will be a little bit faster
     * then this method ...
     *
     * @param sSystemPath
     *          represent the file in system notation
     *
     * @return [String]
     *          a file url which represent the given system path
     */
    public static String getFileURLFromSystemPath( String sSystemPath )
    {
        return getFileURLFromSystemPath(new File(sSystemPath));
    }



    /**
     * Return a name list of all available files of a directory.
     * We filter pure sub directories names. All other files
     * are returned as full qualified URL strings. So they can be
     * used for further purposes. One parameter define the start directory,
     * another one describe the url protocol, which the return URL names should have.
     *
     * @param   sStartDir
     *              the start directory, which should include all test files
     *
     * @return  [Vector]
     *              a filtered list of java File objects of all available files of the start dir
     *              and all accessible sub directories.
     */
    public static ArrayList<File> getSystemFilesFromDir(String sStartDir)
    {
        File aRoot = new File(sStartDir);

        if (! aRoot.exists())
            return null;

        if (! aRoot.isDirectory())
            return null;

        File[] lAllFiles = aRoot.listFiles();
        if (lAllFiles == null )
            return null;

        ArrayList<File> lFilteredFiles = new ArrayList<File>(lAllFiles.length);

        for (int i=0; i<lAllFiles.length; ++i)
        {
            if (lAllFiles[i].isFile())
                lFilteredFiles.add(lAllFiles[i]);
            else
            if (lAllFiles[i].isDirectory())
            {
                // recursion!
                ArrayList<File> lSubFiles = URLHelper.getSystemFilesFromDir(lAllFiles[i].getPath());
                if (lSubFiles != null)
                {
                    Iterator<File> aSnapshot = lSubFiles.iterator();
                    while (aSnapshot.hasNext()) {
                        lFilteredFiles.add(aSnapshot.next());
                    }
                }
            }
        }

        return lFilteredFiles;
    }
}
