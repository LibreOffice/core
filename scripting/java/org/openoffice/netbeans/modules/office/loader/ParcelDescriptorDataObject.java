/*************************************************************************
 *
 *  $RCSfile: ParcelDescriptorDataObject.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-13 17:44:33 $
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
