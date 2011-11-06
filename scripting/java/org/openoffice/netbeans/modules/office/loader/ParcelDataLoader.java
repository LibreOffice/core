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
import java.io.File;

import org.openide.actions.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.netbeans.modules.office.actions.*;
import org.openoffice.idesupport.zip.ParcelZipper;

/** Recognizes single files in the Repository as being of a certain type.
 *
 * @author tomaso
 */
public class ParcelDataLoader extends UniFileLoader {

    public ParcelDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.ParcelDataObject");
    }

    // Can be useful for subclasses:
    protected ParcelDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Script Parcel";
    }

    protected void initialize() {
        super.initialize();

        ExtensionList extensions = new ExtensionList();
        extensions.addExtension(ParcelZipper.PARCEL_EXTENSION);
        setExtensions(extensions);
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            // SystemAction.get(MountParcelAction.class),
            SystemAction.get(DeployParcelAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            // SystemAction.get(PasteAction.class),
            null,
            SystemAction.get(DeleteAction.class),
            SystemAction.get(RenameAction.class),
            null,
            // SystemAction.get(ToolsAction.class),
            SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile) throws DataObjectExistsException, IOException {
        return new ParcelDataObject(primaryFile, this);
    }
}
