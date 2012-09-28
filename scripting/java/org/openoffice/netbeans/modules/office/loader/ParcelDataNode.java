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

import java.io.*;
import java.awt.datatransfer.Transferable;
import java.util.zip.*;

import org.openide.loaders.*;
import org.openide.nodes.*;
import org.openide.filesystems.FileObject;
import org.openide.util.NbBundle;
import org.openide.util.datatransfer.*;
import org.openide.ErrorManager;
import org.openide.windows.OutputWriter;

import org.openoffice.netbeans.modules.office.actions.ParcelCookie;

/** A node to represent this object.
 */
public class ParcelDataNode extends DataNode {

    public ParcelDataNode(ParcelDataObject obj) {
        this(obj, Children.LEAF);
    }

    public ParcelDataNode(ParcelDataObject obj, Children ch) {
        super(obj, ch);
        setIconBase("/org/openoffice/netbeans/modules/office/resources/ParcelIcon");
    }

    protected ParcelDataObject getParcelDataObject() {
        return (ParcelDataObject)getDataObject();
    }

    public static class ParcelPasteType extends PasteType {
        ParcelDataNode sourceParcel = null;
        File targetDocument = null;
        boolean isCut = false;

        public ParcelPasteType(ParcelDataNode sourceParcel,
            File targetDocument, boolean isCut) {
            this.sourceParcel = sourceParcel;
            this.targetDocument = targetDocument;
            this.isCut = isCut;
        }

        public Transferable paste() {
            ParcelCookie parcelCookie =
                (ParcelCookie)sourceParcel.getCookie(ParcelCookie.class);
            parcelCookie.deploy(targetDocument);

            if (isCut == true) {
                FileObject fo = sourceParcel.getDataObject().getPrimaryFile();
                try {
                    fo.delete();
                }
                catch (IOException ioe) {}
                return ExTransferable.EMPTY;
            }
            else {
                return null;
            }
        }
    }

    /* Example of adding Executor / Debugger / Arguments to node:
    protected Sheet createSheet() {
        Sheet sheet = super.createSheet();
        Sheet.Set set = sheet.get(ExecSupport.PROP_EXECUTION);
        if (set == null) {
            set = new Sheet.Set();
            set.setName(ExecSupport.PROP_EXECUTION);
            set.setDisplayName(NbBundle.getMessage(ParcelDataNode.class, "LBL_DataNode_exec_sheet"));
            set.setShortDescription(NbBundle.getMessage(ParcelDataNode.class, "HINT_DataNode_exec_sheet"));
        }
        ((ExecSupport)getCookie(ExecSupport.class)).addProperties(set);
        // Maybe:
        ((CompilerSupport)getCookie(CompilerSupport.class)).addProperties(set);
        sheet.put(set);
        return sheet;
    }
     */

    // Don't use getDefaultAction(); just make that first in the data loader's getActions list

}
