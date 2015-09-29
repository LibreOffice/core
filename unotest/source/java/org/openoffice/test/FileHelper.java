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

package org.openoffice.test;

/*
 * Helper Functions for File handling
 */
public class FileHelper
{
    /*
     * Concat a _sRelativePathToAdd to a _sPath and append a '/' to the _sPath only if need.
     *
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
