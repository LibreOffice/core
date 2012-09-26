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

package org.openoffice.netbeans.modules.office.loader;

import java.io.IOException;
import java.io.File;

import org.openide.actions.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.idesupport.zip.ParcelZipper;
import org.openoffice.netbeans.modules.office.actions.*;

/** Recognizes single files in the Repository as being of a certain type.
 */

public class ParcelFolderDataLoader extends UniFileLoader {

    public ParcelFolderDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.ParcelFolder");
    }

    protected ParcelFolderDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Script Parcel Folder";
    }

    protected void initialize() {
        super.initialize();
    }

    protected FileObject findPrimaryFile(FileObject fo) {
        if (fo.isFolder() == false)
            return null;

        FileObject contents = fo.getFileObject(ParcelZipper.CONTENTS_DIRNAME);
        if (contents == null)
            return null;

        FileObject descriptor = contents.getFileObject(ParcelZipper.PARCEL_DESCRIPTOR_XML);
        if (descriptor == null)
            return null;

        return fo;
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            // SystemAction.get(OpenLocalExplorerAction.class),
            // SystemAction.get(FindAction.class),
            // null,
            // SystemAction.get(FileSystemAction.class),
            // null,
            SystemAction.get(CompileParcelAction.class),
            SystemAction.get(BuildParcelAction.class),
            SystemAction.get(ConfigureParcelAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            // SystemAction.get(PasteAction.class),
            null,
            SystemAction.get(DeleteAction.class),
            SystemAction.get(RenameAction.class),
            null,
            // SystemAction.get(SaveAsTemplateAction.class),
            // null,
            // SystemAction.get(ToolsAction.class),
            SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile) throws DataObjectExistsException, IOException {
        return new ParcelFolder(primaryFile, this);
    }

    protected MultiDataObject.Entry createPrimaryEntry(MultiDataObject obj, FileObject primaryFile) {
        return new FileEntry.Folder(obj, primaryFile);
    }
}
