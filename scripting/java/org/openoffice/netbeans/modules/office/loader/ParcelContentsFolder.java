/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParcelContentsFolder.java,v $
 * $Revision: 1.6 $
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

package org.openoffice.netbeans.modules.office.loader;

import java.io.IOException;

import org.openide.TopManager;
import org.openide.NotifyDescriptor;
import org.openide.ErrorManager;

import org.openide.filesystems.FileObject;
import org.openide.filesystems.FileLock;
import org.openide.filesystems.FileUtil;
import org.openide.filesystems.FileSystem;
import org.openide.filesystems.Repository;

import org.openide.loaders.DataObject;
import org.openide.loaders.DataFolder;
import org.openide.loaders.DataObjectExistsException;

import org.openide.nodes.Node;
import org.openide.util.datatransfer.NewType;

import org.openoffice.netbeans.modules.office.actions.ParcelFolderCookie;
import org.openoffice.netbeans.modules.office.utils.PackageRemover;

public class ParcelContentsFolder extends DataFolder {
    public ParcelContentsFolder(FileObject pf, ParcelContentsFolderDataLoader loader)
        throws DataObjectExistsException {
        super(pf, loader);
    }

    public Node createNodeDelegate() {
        return new DataFolder.FolderNode() {
            public NewType[] getNewTypes() {
                NewType[] newtypes = new NewType[1];
                newtypes[0] = new NewType() {
                    public String getName() {
                        return "New Script";
                    }

                    public void create() {
                        DataFolder contents = (DataFolder)getDataObject();
                        ParcelFolderCookie cookie =
                            (ParcelFolderCookie)contents.getFolder().
                                getCookie(ParcelFolderCookie.class);

                        String language = cookie.getLanguage();
                        ParcelContentsFolder.createEmptyScript(contents,
                            language);
                    }
                };
                return newtypes;
            }
        };
    }

    public static void createEmptyScript(DataFolder parent, String language) {
        String sourceFile = "Templates/OfficeScripting/EmptyScript/Empty";

        if (language.toLowerCase().equals("java")) {
            sourceFile += ".java";
        }
        else if (language.toLowerCase().equals("beanshell")) {
            sourceFile += ".bsh";
        }
        else {
            NotifyDescriptor d = new NotifyDescriptor.Message(
                "Language not defined for this Parcel Folder");
            TopManager.getDefault().notify(d);
            return;
        }

        FileSystem fs = Repository.getDefault().getDefaultFileSystem();
        DataObject result = null;
        try {
            DataObject dObj = DataObject.find(fs.findResource(sourceFile));
            result = dObj.createFromTemplate(parent);
        }
        catch (IOException ioe) {
            ErrorManager.getDefault().notify(ioe);
        }

        FileObject fo = result.getPrimaryFile();
        if (fo.getExt().equals("java")) {
            FileLock lock = null;
            try {
                PackageRemover.removeDeclaration(FileUtil.toFile(fo));

                // IssueZilla 11986 - rename the FileObject
                // so the JavaNode is resynchronized
                lock = fo.lock();
                if (lock != null) {
                    fo.rename(lock, fo.getName(), fo.getExt());
                }
            }
            catch (IOException ioe) {
                NotifyDescriptor d = new NotifyDescriptor.Message(
                 "Error removing package declaration from file: " +
                 fo.getNameExt() +
                 ". You should manually remove this declaration " +
                 "before building the Parcel Recipe");
                TopManager.getDefault().notify(d);
            }
            finally {
                if (lock != null) {
                    lock.releaseLock();
                }
            }
        }
    }
}
