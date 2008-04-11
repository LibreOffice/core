/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FileHelper.java,v $
 * $Revision: 1.5 $
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


