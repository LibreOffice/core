/*************************************************************************
 *
 *  $RCSfile: ParcelDataNode.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-21 16:24:15 $
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
