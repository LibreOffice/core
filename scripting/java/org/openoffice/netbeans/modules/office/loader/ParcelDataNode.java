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
 *
 * @author tomaso
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
