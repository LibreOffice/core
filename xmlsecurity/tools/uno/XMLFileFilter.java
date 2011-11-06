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
