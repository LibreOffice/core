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
 */
public class OfficeDocumentDataNode extends DataNode {

    public OfficeDocumentDataNode(OfficeDocumentDataObject obj) {
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
