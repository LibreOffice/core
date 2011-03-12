/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
