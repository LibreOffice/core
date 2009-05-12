/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParcelDescriptorDataObject.java,v $
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
