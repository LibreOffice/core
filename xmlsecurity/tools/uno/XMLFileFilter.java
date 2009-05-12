/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLFileFilter.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.xml.security.uno;

import java.io.File;
import javax.swing.filechooser.FileFilter;

/*
 * this class is used as a file filter for the XML file
 * (*.xml) and the batch file (*.txt).
 */
class XMLFileFilter extends FileFilter
{
    public static String getExtension(File f)
    {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) {
            ext = s.substring(i+1).toLowerCase();
        }

        return ext;
    }

    public boolean accept(File f)
    {
        boolean rc = false;

        if (f.isDirectory())
        {
            rc = true;
        }
        else
        {
            String extension = getExtension(f);
            if (extension != null)
            {
                if (extension.equals("xml") || extension.equals("txt"))
                {
                    rc = true;
                }
            }
        }

        return rc;
    }

    public String getDescription()
    {
        return "XML and batch files (.xml,.txt)";
    }
}
