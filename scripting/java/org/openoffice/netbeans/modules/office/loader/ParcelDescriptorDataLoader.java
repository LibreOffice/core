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

import java.io.IOException;

import org.openide.actions.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.idesupport.OfficeDocument;

/** Recognizes single files in the Repository as being of a certain type.
 *
 * @author tomaso
 */
public class ParcelDescriptorDataLoader extends UniFileLoader {

    public ParcelDescriptorDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.ParcelDescriptorDataObject");
    }

    // Can be useful for subclasses:
    protected ParcelDescriptorDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return OfficeDocument.OFFICE_PRODUCT_NAME + " Script Parcel Descriptor";
    }

    protected void initialize() {
        super.initialize();

        // ExtensionList extensions = new ExtensionList();
        // extensions.addMimeType("text/x-parcel+xml");
        // extensions.addExtension("pml");
        getExtensions().addMimeType("text/x-parcel+xml");
        // setExtensions(extensions);
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            SystemAction.get(OpenAction.class),
            // SystemAction.get(GenerateParcelAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            SystemAction.get(PasteAction.class),
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
        return new ParcelDescriptorDataObject(primaryFile, this);
    }
}
