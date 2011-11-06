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
