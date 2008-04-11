/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParcelDescriptorDataNode.java,v $
 * $Revision: 1.4 $
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
import org.openide.nodes.*;
import org.openide.util.NbBundle;

import org.openoffice.netbeans.modules.office.nodes.*;
import org.openoffice.netbeans.modules.office.actions.*;

/** A node to represent this object.
 *
 * @author tomaso
 */
public class ParcelDescriptorDataNode extends DataNode {

    public ParcelDescriptorDataNode(ParcelDescriptorDataObject obj) {
        this(obj, Children.LEAF);
    }

    public ParcelDescriptorDataNode(ParcelDescriptorDataObject obj, Children ch) {
        super(obj, ch);
        setHidden(true);
        setIconBase("/org/openoffice/netbeans/modules/office/resources/OfficeIcon");
    }

    protected ParcelDescriptorDataObject getParcelDescriptorDataObject() {
        return (ParcelDescriptorDataObject)getDataObject();
    }

    public boolean canRename() {
        return false;
    }

    /* Example of adding Executor / Debugger / Arguments to node:
    protected Sheet createSheet() {
        Sheet sheet = super.createSheet();
        Sheet.Set set = sheet.get(ExecSupport.PROP_EXECUTION);
        if (set == null) {
            set = new Sheet.Set();
            set.setName(ExecSupport.PROP_EXECUTION);
            set.setDisplayName(NbBundle.getMessage(ParcelDescriptorDataNode.class, "LBL_DataNode_exec_sheet"));
            set.setShortDescription(NbBundle.getMessage(ParcelDescriptorDataNode.class, "HINT_DataNode_exec_sheet"));
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
