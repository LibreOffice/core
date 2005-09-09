/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ParcelContentsFolderDataLoader.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:15:09 $
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

import org.openide.loaders.*;
import org.openide.filesystems.FileObject;
import org.openide.actions.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

import org.openoffice.idesupport.zip.ParcelZipper;

public class ParcelContentsFolderDataLoader extends UniFileLoader {

    public ParcelContentsFolderDataLoader() {
        this("org.openide.loaders.DataFolder");
    }

    protected ParcelContentsFolderDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "Office Script Parcel Contents";
    }

    protected void initialize() {
        super.initialize();
    }

    protected FileObject findPrimaryFile(FileObject fo) {
        if (fo.isFolder() == false ||
            fo.getName().equals(ParcelZipper.CONTENTS_DIRNAME) == false ||
            fo.getFileObject(ParcelZipper.PARCEL_DESCRIPTOR_XML) == null)
            return null;

        return fo;
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            SystemAction.get(PasteAction.class),
            SystemAction.get(NewAction.class),
            // null,
            // SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile)
        throws DataObjectExistsException {
        return new ParcelContentsFolder(primaryFile, this);
    }

    protected MultiDataObject.Entry createPrimaryEntry(MultiDataObject obj, FileObject primaryFile) {
        return new FileEntry.Folder(obj, primaryFile);
    }
}
