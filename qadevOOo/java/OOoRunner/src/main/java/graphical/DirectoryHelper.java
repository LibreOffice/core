/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

