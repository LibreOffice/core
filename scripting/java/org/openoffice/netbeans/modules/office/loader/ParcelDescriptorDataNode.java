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
