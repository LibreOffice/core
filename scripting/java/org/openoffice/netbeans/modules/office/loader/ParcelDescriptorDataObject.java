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

import org.openoffice.netbeans.modules.office.actions.ParcelDescriptorEditorSupport;
import org.openoffice.netbeans.modules.office.actions.ParcelDescriptorParserSupport;

/** Represents a ParcelDescriptor object in the Repository.
 *
 * @author tomaso
 */
public class ParcelDescriptorDataObject extends MultiDataObject {

    private boolean canParse = false;

    public ParcelDescriptorDataObject(FileObject pf, ParcelDescriptorDataLoader loader) throws DataObjectExistsException {
        super(pf, loader);
        init();
    }

    private void init() {
        FileObject fo = getPrimaryFile();
        if (FileUtil.toFile(fo) != null)
            canParse = true;

        CookieSet cookies = getCookieSet();
        cookies.add(new ParcelDescriptorEditorSupport(this));
        if (canParse == true)
            cookies.add(new ParcelDescriptorParserSupport(getPrimaryFile()));
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    protected Node createNodeDelegate() {
        if (canParse == true)
            return new ParcelDescriptorDataNode(this);
        else
            return new ParcelDescriptorDataNode(this, Children.LEAF);
    }

    // If you made an Editor Support you will want to add these methods:
    public final void addSaveCookie(SaveCookie save) {
        getCookieSet().add(save);
    }

    public final void removeSaveCookie(SaveCookie save) {
        getCookieSet().remove(save);
    }
}
