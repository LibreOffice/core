/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package graphical;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

/**
 * Helper for directory access
 *
 * @author lla@openoffice.org
 */
public class DirectoryHelper
{
    ArrayList<String> m_aFileList = new ArrayList<String>();
    boolean m_bRecursiveIsAllowed = true;

    void setRecursiveIsAllowed(boolean _bValue)
        {
            m_bRecursiveIsAllowed = _bValue;
        }

    /**
     * Traverse over a given directory, and filter with a given FileFilter
     * object and gives back the deep directory as a Object[] list, which
     * contain a String object for every directory entry.
     *
     * <B>Example</B>
     * List directory /bin, filter out all files which ends with '.prn'
     *
     *  FileFilter aFileFilter = new FileFilter()
     *  {
     *      public boolean accept( File pathname )
     *          {
     *              if (pathname.getName().endsWith(".prn"))
     *              {
     *                  return false;
     *              }
     *              return true;
     *          }
     *  };
     *
     * Object[] aList = DirectoryHelper.traverse("/bin", aFileFilter);
     * for (int i=0;i<aList.length;i++)
     * {
     *     String aEntry = (String)aList[i];
     *     System.out.println(aEntry);
     * }
     *
     * @param _sDirectory
     * @param _aFileFilter
     * @param _bRecursiveIsAllowed
     * @return list of directories
     */
    public static Object[] traverse( String _sDirectory, FileFilter _aFileFilter, boolean _bRecursiveIsAllowed )
        {
            DirectoryHelper a = new DirectoryHelper();
            a.setRecursiveIsAllowed(_bRecursiveIsAllowed);
            a.traverse_impl(_sDirectory, _aFileFilter);
            return a.m_aFileList.toArray();
        }

    public static Object[] traverse( String _sDirectory, boolean _bRecursiveIsAllowed )
        {
            DirectoryHelper a = new DirectoryHelper();
            a.setRecursiveIsAllowed(_bRecursiveIsAllowed);
            a.traverse_impl(_sDirectory, null);
            return a.m_aFileList.toArray();
        }

    void traverse_impl( String afileDirectory, FileFilter _aFileFilter )
        {
            File fileDirectory = new File(afileDirectory);
            // Testing, if the file is a directory, and if so, it throws an exception
            if ( !fileDirectory.isDirectory() )
            {
                throw new IllegalArgumentException( "not a directory: " + fileDirectory.getName() );
            }

            // Getting all files and directories in the current directory
            File[] aDirEntries;
            if (_aFileFilter != null)
            {
                aDirEntries = fileDirectory.listFiles(_aFileFilter);
            }
            else
            {
                aDirEntries = fileDirectory.listFiles();
            }

            // Iterating for each file and directory
            for ( int i = 0; i < aDirEntries.length; ++i )
            {
                if ( aDirEntries[ i ].isDirectory() )
                {
                    if (m_bRecursiveIsAllowed == true)
                    {
                        // Recursive call for the new directory
                        traverse_impl( aDirEntries[ i ].getAbsolutePath(), _aFileFilter );
                    }
                }
                else
                {
                    // adding file to List
                    try
                    {
                        // Composing the URL by replacing all backslashs
                        // String stringUrl = "file:///" + aFileEntries[ i ].getAbsolutePath().replace( '\\', '/' );
                        String aStr = aDirEntries[ i ].getAbsolutePath();
                        m_aFileList.add(aStr);
                    }
                    catch( Exception exception )
                    {
                        exception.printStackTrace();
                        break;
                    }
                }
            }
        }

    // tests
    // public static void main(String[] args)
    //     {
    //         String sDirectory = "/misc/convwatch/gfxcmp/data/doc-pool/demo";
    //         Object[] aDirectoryList = DirectoryHelper.traverse( sDirectory, false );
    //
    //         for (int i=0;i<aDirectoryList.length;i++)
    //         {
    //             String sEntry = (String)aDirectoryList[i];
    //             System.out.println(sEntry);
    //         }
    //     }
}

