/*************************************************************************
 *
 *  $RCSfile: DirectoryHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:08:55 $
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
}
