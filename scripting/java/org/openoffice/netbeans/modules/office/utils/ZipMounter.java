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



package org.openoffice.netbeans.modules.office.utils;

import java.io.*;
import java.util.zip.*;
import java.beans.PropertyVetoException;

import org.openide.filesystems.FileSystem;
import org.openide.filesystems.Repository;

import org.openoffice.netbeans.modules.office.filesystem.OpenOfficeDocFileSystem;

public class ZipMounter
{
    private static ZipMounter mounter = null;

    private ZipMounter() {
    }

    public static ZipMounter getZipMounter() {
        if (mounter == null) {
            synchronized(ZipMounter.class) {
                if (mounter == null)
                    mounter = new ZipMounter();
            }
        }
        return mounter;
    }

    public void mountZipFile(File zipfile)
        throws IOException, PropertyVetoException
    {
        if (zipfile != null) {
            addDocumentToRepository(zipfile, true);
        }
    }

    private FileSystem addDocumentToRepository(File rootFile, boolean writeable)
        throws IOException, PropertyVetoException
    {
        Repository repo = Repository.getDefault();
        OpenOfficeDocFileSystem oofs;
        oofs = (OpenOfficeDocFileSystem)getMountedDocument(rootFile);
        if(oofs != null)
            repo.removeFileSystem(oofs);
        oofs = new OpenOfficeDocFileSystem();
        oofs.setDocument(rootFile);
        repo.addFileSystem(oofs);
        return oofs;
    }

    /** @return FileSystem which has given jar file as its root or
    * null if no such file system could be found in repository */
    private FileSystem getMountedDocument(File rootFile)
    {
        if (rootFile == null)
            return null;
        FileSystem oofs = null;
        try {
            oofs = Repository.getDefault().findFileSystem(
                OpenOfficeDocFileSystem.computeSystemName(rootFile));
        } catch(Exception exp) {
        }
        return oofs;
    }
}
