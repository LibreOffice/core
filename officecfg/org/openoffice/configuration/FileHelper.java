/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FileHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-22 12:32:04 $
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
package org.openoffice.configuration;
import java.io.*;


/**
 * Title:        File
 * Description:  decoding of set element names given encoded (base64)
 */
public class FileHelper extends Object
{
    // returns true if a file exists, otherwise false
    public static boolean exists(String sSource)
    {
        File aFile = new File(sSource);

        try {
            System.out.println("Path: " + aFile.getAbsoluteFile().toURL().toString());
        } catch (Exception e)
        {
            e.printStackTrace();
        }
        return aFile.exists();
    }

    public static String makeAbs(String sSource)
    {
        String absPath = new String();
        File aFile = new File(sSource);

        try {
           absPath = aFile.toURL().toString();
           // http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6351751
           if (absPath.substring(0, 5) == "file:" && absPath.substring(5, 7) != "//")
               absPath = "file://" + absPath.substring(5, absPath.length());
           if (absPath.charAt(absPath.length()-1) == '/')
               absPath = absPath.substring(0, absPath.length()-1);
        } catch (Exception e)
        {
            e.printStackTrace();
        }
        return absPath;
    }
}


