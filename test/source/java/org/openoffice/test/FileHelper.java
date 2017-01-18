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



package org.openoffice.test;

/**
 * Helper Functions for File handling
 */
public class FileHelper
{
    public FileHelper()
        {
        }
    /**
     * Concat a _sRelativePathToAdd to a _sPath and append a '/' to the _sPath only if need.
     *
     * @param _sPath
     * @param _sRelativePathToAdd
     * @return a right concated path
     */
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
}
