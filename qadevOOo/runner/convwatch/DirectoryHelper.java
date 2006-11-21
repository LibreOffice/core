/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DirectoryHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 14:09:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package convwatch;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

/**
 * Helper for directory access
 *
 * @author Lars.Langhans@sun.com
 */
public class DirectoryHelper
{
    ArrayList m_aFileList = new ArrayList();
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

