/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParcelDataObject.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:16:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.netbeans.modules.office.loader;

import org.openide.actions.*;
import org.openide.cookies.*;
import org.openide.filesystems.FileObject;
import org.openide.loaders.*;
import org.openide.nodes.*;
import org.openide.util.HelpCtx;

import org.openoffice.netbeans.modules.office.actions.*;

/** Represents a Parcel object in the Repository.
 *
 * @author tomaso
 */
public class ParcelDataObject extends MultiDataObject {

    public ParcelDataObject(FileObject pf, ParcelDataLoader loader) throws DataObjectExistsException {
        super(pf, loader);
        init();
    }

    private void init() {
        CookieSet cookies = getCookieSet();
        cookies.add(new ParcelSupport(getPrimaryFile()));
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
        // If you add context help, change to:
        // return new HelpCtx(ParcelDataObject.class);
    }

    protected Node createNodeDelegate() {
        return new ParcelDataNode(this);
    }

    /* If you made an Editor Support you will want to add these methods:

    final void addSaveCookie(SaveCookie save) {
        getCookieSet().add(save);
    }

    final void removeSaveCookie(SaveCookie save) {
        getCookieSet().remove(save);
    }

     */

}
