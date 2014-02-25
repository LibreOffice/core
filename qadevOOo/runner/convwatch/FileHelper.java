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

package convwatch;

import java.io.File;
import java.io.FileFilter;
import java.util.StringTokenizer;
import helper.OSHelper;

import javax.swing.JOptionPane;

public class FileHelper
{
    public FileHelper()
        {
            // fs = System.getProperty("file.separator");


            String sOSName = System.getProperty("os.name");
            String sOSArch = System.getProperty("os.arch");
            String sOSVersion = System.getProperty("os.version");

            GlobalLogWriter.get().println(sOSName);
            GlobalLogWriter.get().println(sOSArch);
            GlobalLogWriter.get().println(sOSVersion);

        }

    public static void MessageBox(String _sStr)
        {
            String sVersion = System.getProperty("java.version");
            String sOSName  = System.getProperty("os.name");
            JOptionPane.showMessageDialog( null, _sStr, sVersion + " " + sOSName + " Hello World Debugger", JOptionPane.INFORMATION_MESSAGE );
        }

    public static boolean exists(String _sFile)
        {
            if (_sFile == null) return false;

            File aFile = new File(_sFile);
            if (aFile.exists())
            {
                return true;
            }
            // This is just nice for DEBUG behaviour
            // due to the fact this is absolutly context dependency no one should use it.
            // else
            // {
            //     System.out.println("FileHelper:exists() tell this path doesn't exists. Check it. path is:" );
            //     System.out.println( _sFile );
            //     System.out.println( aFile.getAbsolutePath() );
            //     MessageBox("Der JavaProzess wartet auf eine interaktion ihrerseits.");

            //     File aFile2 = new File(_sFile);
            //     if (aFile2.exists())
            //     {
            //         System.out.println("Thanks, file exists." );
            //         return true;
            //     }
            // }
            return false;
        }

    public static boolean isDir(String _sDir)
        {
            if (_sDir == null) return false;
            try
            {
                File aFile = new File(_sDir);
                if (aFile.exists() && aFile.isDirectory())
                {
                    return true;
                }
            }
            catch (NullPointerException e)
            {
                GlobalLogWriter.get().println("Exception caught. FileHelper.isDir('" + _sDir + "')");
                e.printStackTrace();
            }
            return false;
        }

    public static String getBasename(String _sFilename)
        {
            if (_sFilename == null) return "";
            String fs = System.getProperty("file.separator");

            int nIdx = _sFilename.lastIndexOf(fs);
            if (nIdx > 0)
            {
                return _sFilename.substring(nIdx + 1);
            }
            return _sFilename;
        }

    public static String getNameNoSuffix(String _sFilename)
        {
            if (_sFilename == null) return "";
            int nIdx = _sFilename.lastIndexOf(".");
            if (nIdx > 0)
            {
                return _sFilename.substring(0, nIdx);
            }
            return _sFilename;
        }

    public static String getSuffix(String _sFilename)
        {
            if (_sFilename == null) return "";
            int nIdx = _sFilename.lastIndexOf(".");
            if (nIdx > 0)
            {
                return _sFilename.substring(nIdx );
            }
            return "";
        }

    public static String getPath(String _sFilename)
        {
            if (_sFilename == null) return "";
            String fs = System.getProperty("file.separator");

            int nIdx = _sFilename.lastIndexOf(fs);
            if (nIdx > 0)
            {
                return _sFilename.substring(0, nIdx);
            }
            return "";
        }

/*
    static ArrayList files = new ArrayList();
    public static Object[] traverse( String afileDirectory )
        {

            File fileDirectory = new File(afileDirectory);
            // Testing, if the file is a directory, and if so, it throws an exception
            if ( !fileDirectory.isDirectory() )
            {
                throw new IllegalArgumentException( "not a directory: " + fileDirectory.getName() );
            }

            // Getting all files and directories in the current directory
            File[] entries = fileDirectory.listFiles();

            // Iterating for each file and directory
            for ( int i = 0; i < entries.length; ++i )
            {
                // adding file to List
                try
                {
                    // Composing the URL by replacing all backslashs
                    String stringUrl = "file:///"
                        + entries[ i ].getAbsolutePath().replace( '\\', '/' );
                    files.add(stringUrl);
                }
                catch( Exception exception )
                {
                    exception.printStackTrace();
                }
            }
            return files.toArray();
        }
*/

    // makeDirectories("", "/tmp/a/b");
    // creates all directories /tmp/a/b

    public static void makeDirectories(String first, String path)
        {
            makeDirectories(first, path, "0777");
        }

    public static void makeDirectories(String first, String path, String _sMode)
        {
            String fs = System.getProperty("file.separator");
            if (path.startsWith(fs + fs)) // starts with UNC Path
            {
                int n = path.indexOf(fs, 2);
                n = path.indexOf(fs, n + 1);
                first = path.substring(0, n);
                path = path.substring(n + 1);
            }

            String already_done = null;
            StringTokenizer path_tokenizer = new StringTokenizer(path,fs,false);
            already_done = first;
            while (path_tokenizer.hasMoreTokens())
            {
                String part = path_tokenizer.nextToken();
                File new_dir = new File(already_done + File.separatorChar + part);
                already_done = new_dir.toString();
                // System.out.println(already_done);
                //create the directory
                new_dir.mkdirs();
                if (OSHelper.isUnix() &&
                    _sMode.length() > 0)
                {
                    try
                    {
                        chmod(new_dir, _sMode);
                    }
                    catch (java.io.IOException e)
                    {
                        GlobalLogWriter.get().println("Exception caught. FileHelper.makeDirectories('" + new_dir.getAbsolutePath() + "')");
                    }
                }
            }
            // return;
        }

    public static void chmod(File file, String mode) throws java.io.IOException
        {
            Runtime.getRuntime().exec
                (new String[]
                    {"chmod", mode, file.getAbsolutePath()});
        }

    public static String removeFirstDirectorysAndBasenameFrom(String _sName, String _sRemovePath)
        {
            // pre: _sName: /a/b/c/d/e/f.g _sRemovePath /a/b/c
            // result: d/e
            String fs = System.getProperty("file.separator");

            String sBasename = FileHelper.getBasename(_sName);
            String sSubDirs = "";
            if (_sName.startsWith(_sRemovePath))
            {
                // if _sName starts with _sRemovePath
                int nRemovePathIndex = _sRemovePath.length();
                if (! _sRemovePath.endsWith(fs))
                {
                    // add 1 if we not ends with file separator
                    nRemovePathIndex ++;
                }
                int nBasenameIndex = _sName.length() - sBasename.length() - 1;
                if (nRemovePathIndex < nBasenameIndex)
                {
                    sSubDirs = _sName.substring(nRemovePathIndex, nBasenameIndex);
                }
            }
            else
            {
                // special case, the _sRemovePath is not part of _sName
                sSubDirs = FileHelper.getPath(_sName);
                if (sSubDirs.startsWith(fs))
                {
                    // remove leading file separator
                    sSubDirs = sSubDirs.substring(1);
                }
            }

            return sSubDirs;
        }

    public static void test_removeFirstDirectorysAndBasenameFrom()
        {
            removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/a/b/c");
            removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/a/b/c/");
            removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/b/c");
        }


    public static String getSystemPathFromFileURL( String _sFileURL )
    {
        String sSystemFile = null;

        if(_sFileURL.startsWith("file:///"))
        {
            if (OSHelper.isWindows())
            {
                sSystemFile = _sFileURL.substring(8);
            }
            else
            {
                sSystemFile = _sFileURL.substring(7);
            }
        }
        else if (_sFileURL.startsWith("file://"))
        {
            sSystemFile = _sFileURL.substring(5);
        }
        String fs = System.getProperty("file.separator");
        if (! fs.equals("/"))
        {
            sSystemFile = sSystemFile.replace ('/', fs.toCharArray ()[0]);
        }
// FEATURE FOR UNC NEED!!!
        return sSystemFile;
    }

    private static boolean m_bDebugTextShown = false;
    public static boolean isDebugEnabled()
        {
            boolean bDebug = false;
            String sTmpPath = util.utils.getUsersTempDir();
            //util.utils.getUsersTempDir();
            String fs = System.getProperty("file.separator");
            String sName = sTmpPath + fs + "DOC_COMPARATOR_DEBUG";
            File aFile = new File(sName);
            if (aFile.exists())
            {
                if (m_bDebugTextShown == false)
                {
                    GlobalLogWriter.get().println("Found file: " + sName);
                    GlobalLogWriter.get().println("Activate debug mode.");
                    GlobalLogWriter.get().println("If debug mode is no longer necessary, remove the above file.");
                    m_bDebugTextShown = true;
                }
                bDebug = true;
            }
            return bDebug;
        }

    public static void copy(String _sSource, String _sDestination)
        {
            try
            {
                File inputFile = new File(_sSource);
                File outputFile = new File(_sDestination);

                java.io.FileReader in = new java.io.FileReader(inputFile);
                java.io.FileWriter out = new java.io.FileWriter(outputFile);
                int c;

                while ((c = in.read()) != -1)
                    out.write(c);

                in.close();
                out.close();
            }
            catch (java.io.IOException e)
            {
                GlobalLogWriter.get().println("Exception caught. FileHelper.copy('" + _sSource + ", " + _sDestination + "')");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
            }
        }

    /**
     * Within the directory run through, it's possible to say which file extension types should not
     * consider like '*.prn' because it's not a document.
     *
     * @return a FileFilter function
     */
    public static FileFilter getFileFilter()
        {
            FileFilter aFileFilter = new FileFilter()
                {
                    public boolean accept( File pathname )
                        {
                            // leave out files which started by '~$' these are Microsoft Office temp files
                            if (pathname.getName().startsWith("~$"))
                            {
                                return false;
                            }

                            if (pathname.getName().endsWith(".prn"))
                            {
                                return false;
                            }
                            // This type of document no one would like to load.
                            if (pathname.getName().endsWith(".zip"))
                            {
                                return false;
                            }
                            // just a hack
                            if (pathname.getName().endsWith("_"))
                            {
                                return false;
                            }
                            return true;
                        }
                };
            return aFileFilter;
        }
}

