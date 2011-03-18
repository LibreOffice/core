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

import java.awt.datatransfer.Transferable;
import java.util.List;
import java.io.*;

import org.openide.loaders.*;
import org.openide.nodes.*;
import org.openide.util.NbBundle;
import org.openide.filesystems.*;

import org.openoffice.netbeans.modules.office.actions.OfficeDocumentCookie;
import org.openoffice.netbeans.modules.office.nodes.OfficeDocumentChildren;

/** A node to represent this object.
 *
 * @author tomaso
 */
public class OfficeDocumentDataNode extends DataNode {

    public OfficeDocumentDataNode(OfficeDocumentDataObject obj) {
        // this(obj, Children.LEAF);
        this(obj, new OfficeDocumentChildren((OfficeDocumentCookie)
            obj.getCookie(OfficeDocumentCookie.class)));
    }

    public OfficeDocumentDataNode(OfficeDocumentDataObject obj, Children ch) {
        super(obj, ch);
        setIconBase("/org/openoffice/netbeans/modules/office/resources/OfficeIcon");
    }

    protected OfficeDocumentDataObject getOfficeDocumentDataObject() {
        return (OfficeDocumentDataObject)getDataObject();
    }

    // Allow for pasting of Script Parcels to Office Documents
    protected void createPasteTypes(Transferable t, List ls) {
        Node[] copies = NodeTransfer.nodes(t, NodeTransfer.COPY);

        if (copies != null) {
            for (int i = 0; i < copies.length; i++) {
                if (copies[i] instanceof ParcelDataNode) {
                    File source = FileUtil.toFile(((ParcelDataNode)copies[i]).getDataObject().getPrimaryFile());
                    File target = FileUtil.toFile(getDataObject().getPrimaryFile());

                    if (source.exists()  && source.canRead() &&
                        target.exists() && target.canWrite()) {
                        ls.add(new ParcelDataNode.ParcelPasteType((ParcelDataNode)copies[i], target, false));
                    }
                }
            }
        }

        Node[] moves = NodeTransfer.nodes(t, NodeTransfer.MOVE);
        if (moves != null) {
            for (int i = 0; i < moves.length; i++) {
                if (moves[i] instanceof ParcelDataNode) {
                    File source = FileUtil.toFile(((ParcelDataNode)moves[i]).getDataObject().getPrimaryFile());
                    File target = FileUtil.toFile(getDataObject().getPrimaryFile());

                    if (source.exists() && source.canRead() &&
                        target.exists() && target.canWrite()) {
                        ls.add(new ParcelDataNode.ParcelPasteType((ParcelDataNode)moves[i], target, true));
                    }
                }
            }
        }

        // Also try superclass, but give it lower priority:
        super.createPasteTypes(t, ls);
    }

    /* Example of adding Executor / Debugger / Arguments to node:
    protected Sheet createSheet() {
        Sheet sheet = super.createSheet();
        Sheet.Set set = sheet.get(ExecSupport.PROP_EXECUTION);
        if (set == null) {
            set = new Sheet.Set();
            set.setName(ExecSupport.PROP_EXECUTION);
            set.setDisplayName(NbBundle.getMessage(OfficeDocumentDataNode.class, "LBL_DataNode_exec_sheet"));
            set.setShortDescription(NbBundle.getMessage(OfficeDocumentDataNode.class, "HINT_DataNode_exec_sheet"));
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
