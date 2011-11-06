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

import org.openide.actions.*;
import org.openide.cookies.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.nodes.*;
import org.openide.util.HelpCtx;

import org.openoffice.netbeans.modules.office.actions.*;

public class OfficeDocumentDataObject extends MultiDataObject {

    public OfficeDocumentDataObject(FileObject pf, OfficeDocumentDataLoader loader) throws DataObjectExistsException {
        super(pf, loader);
        init();
    }

    private void init() {
        CookieSet cookies = getCookieSet();
        cookies.add(new OfficeDocumentSupport(this));
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    protected Node createNodeDelegate() {
        return new OfficeDocumentDataNode(this);
    }
}
