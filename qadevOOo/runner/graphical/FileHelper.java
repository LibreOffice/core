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

package graphical;

import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.StringTokenizer;
import helper.OSHelper;

import java.io.PrintStream;

public class FileHelper
{
    public FileHelper()
        {
            String sOSName = System.getProperty("os.name");
            String sOSArch = System.getProperty("os.arch");
            String sOSVersion = System.getProperty("os.version");

            GlobalLogWriter.println(sOSName);
            GlobalLogWriter.println(sOSArch);
            GlobalLogWriter.println(sOSVersion);

        }



    public static boolean exists(String _sFile)
        {
            if (_sFile == null)
            {
                return false;
            }

            File aFile = new File(_sFile);
            if (aFile.exists())
            {
                return true;
            }
            return false;
        }



    public static String getBasename(String _sFilename)
        {
            if (_sFilename == null)
            {
                return "";
            }

            int nIdx = _sFilename.lastIndexOf('\\');
            if (nIdx == -1)
            {
                nIdx = _sFilename.lastIndexOf('/');
            }
            if (nIdx > 0)
            {
                return _sFilename.substring(nIdx + 1);
            }
            return _sFilename;
        }

    public static String getNameNoSuffix(String _sFilename)
        {
            if (_sFilename == null)
            {
                return "";
            }
            int nIdx = _sFilename.lastIndexOf('.');
            if (nIdx > 0)
            {
                return _sFilename.substring(0, nIdx);
            }
            return _sFilename;
        }

    public static String getSuffix(String _sFilename)
        {
            if (_sFilename == null)
            {
                return "";
            }
            int nIdx = _sFilename.lastIndexOf('.');
            if (nIdx > 0)
            {
                return _sFilename.substring(nIdx );
            }
            return "";
        }

    public static String getPath(String _sFilename)
        {
            if (_sFilename == null)
            {
                return "";
            }

            int nIdx = _sFilename.lastIndexOf('\\');
            if (nIdx == -1)
            {
                nIdx = _sFilename.lastIndexOf('/');
            }
            if (nIdx > 0)
            {
                return _sFilename.substring(0, nIdx);
            }
            return "";
        }


    // creates all directories /tmp/a/b

    public static void makeDirectories(String first, String path)
        {
            makeDirectories(first, path, "0777");
        }

    private static void makeDirectories(String first, String path, String _sMode)
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
                        GlobalLogWriter.println("Exception caught. FileHelper.makeDirectories('" + new_dir.getAbsolutePath() + "')");
                    }
                }
            }
            // return;
        }

    private static void chmod(File file, String mode) throws java.io.IOException
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
            String fs = System.getProperty("file.separator");
            String sName = sTmpPath + fs + "DOC_COMPARATOR_DEBUG";
            File aFile = new File(sName);
            if (aFile.exists())
            {
                if (!m_bDebugTextShown)
                {
                    GlobalLogWriter.println("Found file: " + sName);
                    GlobalLogWriter.println("Activate debug mode.");
                    GlobalLogWriter.println("If debug mode is no longer necessary, remove the above file.");
                    m_bDebugTextShown = true;
                }
                bDebug = true;
            }
            return bDebug;
        }

    private static void copyStream(InputStream _aIn, OutputStream _aOut) throws java.io.IOException
    {
        byte[] aBuffer = new byte[0xFFFF];
        for (int len; (len = _aIn.read(aBuffer)) != -1; )
        {
            _aOut.write(aBuffer, 0, len);
        }
    }

    public static void copy(String _sSource, String _sDestination)
        {
            FileInputStream aFIS = null;
            FileOutputStream aFOS = null;

            try
            {
                aFIS = new FileInputStream(_sSource);
                aFOS = new FileOutputStream(_sDestination);
                copyStream(aFIS, aFOS);
            }
            catch (java.io.IOException e)
            {
                System.out.println("Error: caught Exception: " + e.getMessage());
            }
            finally
            {
                if (aFIS != null)
                {
                    try
                    {
                        aFIS.close();
                    }
                    catch (java.io.IOException e)
                    {
                        System.out.println("Error: caught Exception: " + e.getMessage());
                    }
                }
                if (aFOS != null)
                {
                    try
                    {
                        aFOS.close();
                    }
                    catch (java.io.IOException e)
                    {
                        System.out.println("Error: caught Exception: " + e.getMessage());
                    }
                }
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
                            // leave out files starts with '.~lock.' these are OpenOffice.org lock files
                            if (pathname.getName().startsWith(".~lock."))
                            {
                                return false;
                            }
                            // leave out files ends with '#' these could be temp files
                            if (pathname.getName().endsWith("#"))
                            {
                                return false;
                            }
                            if (pathname.getName().endsWith(".prn"))
                            {
                                return false;
                            }
                            if (pathname.getName().endsWith(".ps"))
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
    /**
     * Within the directory run through, it's possible to say which file extension types should not
     * consider like '*.prn' because it's not a document.
     *
     * @return a FileFilter function
     */
    public static FileFilter getFileFilterPSorPDF()
        {
            FileFilter aFileFilter = new FileFilter()
                {
                    public boolean accept( File pathname )
                        {
                            if (pathname.getName().endsWith(".ps"))
                            {
                                return true;
                            }
                            if (pathname.getName().endsWith(".pdf"))
                            {
                                return true;
                            }
                            return false;
                        }
                };
            return aFileFilter;
        }
    /**
     * Within the directory run through, it's possible to say which file extension types should not
     * consider like '*.prn' because it's not a document.
     *
     * @return a FileFilter function
     */
    public static FileFilter getFileFilterJPEG()
        {
            FileFilter aFileFilter = new FileFilter()
                {
                    public boolean accept( File pathname )
                        {
                            if (pathname.getName().toLowerCase().endsWith(".jpg"))
                            {
                                return true;
                            }
                            if (pathname.getName().toLowerCase().endsWith(".jpeg"))
                            {
                                return true;
                            }
                            return false;
                        }
                };
            return aFileFilter;
        }


        public static String appendPath(String _sPath, String _sRelativePathToAdd)
        {
            String sNewPath = _sPath;
            String fs = System.getProperty("file.separator");
            if (_sPath.startsWith("file:"))
            {
                fs = "/";                                  // we use a file URL so only '/' is allowed.
            }
            if (! (sNewPath.endsWith("/") || sNewPath.endsWith("\\") ) )
            {
                sNewPath += fs;
            }
            sNewPath += _sRelativePathToAdd;
            return sNewPath;
        }


    public static void createInfoFile(String _sFile, ParameterHelper _aGTA)
        {
            createInfoFile(_sFile, _aGTA, "");
        }

    public static void createInfoFile(String _sFile, ParameterHelper _aGTA, String _sSpecial)
        {
            String sFilename;
            if (_sFile.startsWith("file://"))
            {
                sFilename = FileHelper.getSystemPathFromFileURL(_sFile);
                GlobalLogWriter.println("CreateInfoFile: '" + sFilename + "'" );
            }
            else
            {
                sFilename = _sFile;
            }
            String sFileDir = FileHelper.getPath(sFilename);
            String sBasename = FileHelper.getBasename(sFilename);
            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);

            String sIniFile = FileHelper.appendPath(sFileDir, sBasename + ".ini");
            IniFile aIniFile = new IniFile(sIniFile);

            // OLD INFO FILE

            System.getProperty("line.separator");
            String sInfoFilename = FileHelper.appendPath(sFileDir, sNameNoSuffix + ".info");
            File aInfoFile = new File(sInfoFilename);

            String sBuildID = "";

            try
            {
                FileOutputStream out2 = new FileOutputStream(aInfoFile.toString());
                PrintStream out = new PrintStream(out2);

                out.println("# automatically created file by graphical compare");
                if (_aGTA != null)
                {
                    if (_sSpecial != null && _sSpecial.equals("msoffice"))
                    {
                        out.println("# buildid from wordloadfile");
                        sBuildID = _aGTA.getPerformance().getMSOfficeVersion();
                        out.println("buildid=" + sBuildID);
                    }
                    else
                    {
                        out.println("# buildid is read out of the bootstrap file");
                        sBuildID = _aGTA.getBuildID();
                        out.println("buildid=" + sBuildID);
                    }
                    aIniFile.insertValue("global", "buildid", sBuildID);

                    out.println();
                    out.println("# resolution given in DPI");
                    out.println("resolution=" + _aGTA.getResolutionInDPI());
                    aIniFile.insertValue("global", "resolution", _aGTA.getResolutionInDPI());
                }
                else
                {
                    out.println("buildid=" + _sSpecial);
                    aIniFile.insertValue("global", "buildid", _sSpecial);
                }

                out.println();
                out.println("# Values out of System.getProperty(...)");
                out.println("os.name=" + System.getProperty("os.name"));
                out.println("os.arch=" + System.getProperty("os.arch"));
                out.println("os.version=" + System.getProperty("os.version"));

                aIniFile.insertValue("global", "os.name", System.getProperty("os.name"));
                aIniFile.insertValue("global", "os.arch", System.getProperty("os.arch"));
                aIniFile.insertValue("global", "os.version", System.getProperty("os.version"));

                if (_aGTA != null)
                {
                    out.println();
                    out.println("# Performance output, values are given in milli sec.");
                    _aGTA.getPerformance().print(out);
                    _aGTA.getPerformance().print(aIniFile, "global");
                }

                out.flush();
                out.close();
                out2.close();
            }
            catch (java.io.IOException e)
            {
                GlobalLogWriter.println("can't create Info file.");
                e.printStackTrace();
            }
            aIniFile.close();

        }

        private static void addBasenameToFile(String _sIndexFilename, String _sBasename, String _sCreator, String _sType, String _sSource)
        {
            String sPath;
            if (_sIndexFilename.startsWith("file:"))
            {
                sPath = FileHelper.getSystemPathFromFileURL(_sIndexFilename);
            }
            else
            {
                sPath = _sIndexFilename;
            }
            String sIndexFilename = sPath; // FileHelper.appendPath(sPath, _sFilename);
            IniFile aIniFile = new IniFile(sIndexFilename);
            aIniFile.insertValue(_sBasename, "creator", _sCreator);
            aIniFile.insertValue(_sBasename, "type", _sType);
            aIniFile.insertValue(_sBasename, "source", _sSource);
            aIniFile.close();
        }

        public static void addBasenameToIndex(String _sOutputFilename, String _sBasename, String _sCreator, String _sType, String _sSource)
        {
            String sIndexFilename = FileHelper.appendPath(_sOutputFilename, "index.ini");
            addBasenameToFile(sIndexFilename, _sBasename, _sCreator, _sType, _sSource);
        }

}

