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
