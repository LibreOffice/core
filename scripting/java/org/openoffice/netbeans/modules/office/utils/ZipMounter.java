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
