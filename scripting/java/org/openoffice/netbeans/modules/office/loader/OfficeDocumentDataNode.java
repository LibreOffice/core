/*************************************************************************
 *
 *  $RCSfile: OfficeDocumentDataNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-13 17:44:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
            set.setDisplayName(NbBundle.getMessage(StarOfficeDocumentDataNode.class, "LBL_DataNode_exec_sheet"));
            set.setShortDescription(NbBundle.getMessage(StarOfficeDocumentDataNode.class, "HINT_DataNode_exec_sheet"));
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
