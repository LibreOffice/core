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

import org.openide.loaders.*;
import org.openide.filesystems.FileObject;
import org.openide.actions.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.idesupport.zip.ParcelZipper;

public class ParcelContentsFolderDataLoader extends UniFileLoader {

    public ParcelContentsFolderDataLoader() {
        this("org.openide.loaders.DataFolder");
    }

    protected ParcelContentsFolderDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Script Parcel Contents";
    }

    protected void initialize() {
        super.initialize();
    }

    protected FileObject findPrimaryFile(FileObject fo) {
        if (fo.isFolder() == false ||
            fo.getName().equals(ParcelZipper.CONTENTS_DIRNAME) == false ||
            fo.getFileObject(ParcelZipper.PARCEL_DESCRIPTOR_XML) == null)
            return null;

        return fo;
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            SystemAction.get(PasteAction.class),
            SystemAction.get(NewAction.class),
            // null,
            // SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile)
        throws DataObjectExistsException {
        return new ParcelContentsFolder(primaryFile, this);
    }

    protected MultiDataObject.Entry createPrimaryEntry(MultiDataObject obj, FileObject primaryFile) {
        return new FileEntry.Folder(obj, primaryFile);
    }
}
