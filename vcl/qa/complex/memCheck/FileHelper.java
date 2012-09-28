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

package complex.memCheck;

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
