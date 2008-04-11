/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ZipMounter.java,v $
 * $Revision: 1.4 $
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
