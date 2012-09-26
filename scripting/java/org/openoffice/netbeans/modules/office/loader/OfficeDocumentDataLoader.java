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

import org.openoffice.netbeans.modules.office.actions.MountDocumentAction;

/** Recognizes single files in the Repository as being of a certain type.
 */
public class OfficeDocumentDataLoader extends UniFileLoader {

    public OfficeDocumentDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.OfficeDocumentDataObject");
    }

    // Can be useful for subclasses:
    protected OfficeDocumentDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Document";
    }

    protected void initialize() {
        super.initialize();

        ExtensionList extensions = new ExtensionList();
        extensions.addExtension("sxw");
        extensions.addExtension("sxc");
        extensions.addExtension("sxd");
        extensions.addExtension("sxi");
        setExtensions(extensions);
    }

    protected FileObject findPrimaryFile(FileObject fo) {
        ExtensionList extensions = getExtensions();
        if (extensions.isRegistered(fo) == false)
            return null;

        File document = FileUtil.toFile(fo);
        JarFileSystem jarFs = new JarFileSystem();

        try {
            jarFs.setJarFile(document);
        }
        catch (IOException e) {
            // TopManager.getDefault().notify(new NotifyDescriptor.Exception(e, "asdf"));
            return null;
        }
        catch (Exception e) {
            return null;
        }
        return fo;
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            SystemAction.get(OpenAction.class),
            // SystemAction.get(MountDocumentAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            SystemAction.get(PasteAction.class),
            null,
            SystemAction.get(DeleteAction.class),
            SystemAction.get(RenameAction.class),
            null,
            // SystemAction.get(ToolsAction.class),
            SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile) throws DataObjectExistsException, IOException {
        return new OfficeDocumentDataObject(primaryFile, this);
    }
}
