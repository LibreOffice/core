/*************************************************************************
 *
 *  $RCSfile: FileHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:09:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package convwatch;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.StringTokenizer;

public class FileHelper
{
    public FileHelper()
        {
            // fs = System.getProperty("file.separator");


            String sOSName = System.getProperty("os.name");
            String sOSArch = System.getProperty("os.arch");
            String sOSVersion = System.getProperty("os.version");

            System.out.println(sOSName);
            System.out.println(sOSArch);
            System.out.println(sOSVersion);

        }

    public static boolean exists(String _sFile)
        {
            if (_sFile == null) return false;

            File aFile = new File(_sFile);
            if (aFile.exists())
            {
                return true;
            }
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
                System.out.println("Exception caught. FileHelper.isDir('" + _sDir + "')");
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
    //
    public static void makeDirectories(String first, String path)
        {
            String already_done = null;
            String fs = System.getProperty("file.separator");
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
            }
            // return;
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
            String a = removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/a/b/c");
            // assure("", a.equals("d/e"));
            String b = removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/a/b/c/");
            // assure("", b.equals("d/e"));
            String c = removeFirstDirectorysAndBasenameFrom("/a/b/c/d/e/f.g", "/b/c");
            // assure("", c.equals("a/b/c/d/e"));
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
            sSystemFile = _sFileURL.substring(7);
        }
        String fs = System.getProperty("file.separator");
        if (! fs.equals("/"))
        {
            sSystemFile = sSystemFile.replace ('/', fs.toCharArray ()[0]);
        }
        return sSystemFile;
    }

}
