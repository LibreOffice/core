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
