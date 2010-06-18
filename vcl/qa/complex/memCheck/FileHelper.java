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

package complex.memCheck;

import java.io.File;

/**
 *
 * @author ll93751
 */
public class FileHelper
{
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
    public static String getJavaCompatibleFilename(String _sFilename)
    {
        // It is a little bit stupid that office urls not compatible to java file urls
        // System.out.println("java.io.File can't access Office file urls.");
        if(_sFilename.startsWith("path:"))
        {
            final String sPath = _sFilename.substring(5);
            return sPath;
        }

        String sSystemPath = graphical.FileHelper.getSystemPathFromFileURL(_sFilename);
        if (sSystemPath == null)
        {
            sSystemPath = _sFilename;
        }
        return sSystemPath;
    }

public static String getBasename(String _sFilename)
        {
            if (_sFilename == null)
            {
                return "";
            }
            // String fs = System.getProperty("file.separator");

            int nIdx = _sFilename.lastIndexOf("\\");
            if (nIdx == -1)
            {
                nIdx = _sFilename.lastIndexOf("/");
            }
            if (nIdx > 0)
            {
                return _sFilename.substring(nIdx + 1);
            }
            return _sFilename;
        }
}
