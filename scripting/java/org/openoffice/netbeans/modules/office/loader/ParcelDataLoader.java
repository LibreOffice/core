/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParcelDataLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:15:42 $
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

import java.io.IOException;
import java.io.File;

import org.openide.actions.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.netbeans.modules.office.actions.*;
import org.openoffice.idesupport.zip.ParcelZipper;

/** Recognizes single files in the Repository as being of a certain type.
 *
 * @author tomaso
 */
public class ParcelDataLoader extends UniFileLoader {

    public ParcelDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.ParcelDataObject");
    }

    // Can be useful for subclasses:
    protected ParcelDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Script Parcel";
    }

    protected void initialize() {
        super.initialize();

        ExtensionList extensions = new ExtensionList();
        extensions.addExtension(ParcelZipper.PARCEL_EXTENSION);
        setExtensions(extensions);
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            // SystemAction.get(MountParcelAction.class),
            SystemAction.get(DeployParcelAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            // SystemAction.get(PasteAction.class),
            null,
            SystemAction.get(DeleteAction.class),
            SystemAction.get(RenameAction.class),
            null,
            // SystemAction.get(ToolsAction.class),
            SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile) throws DataObjectExistsException, IOException {
        return new ParcelDataObject(primaryFile, this);
    }
}
