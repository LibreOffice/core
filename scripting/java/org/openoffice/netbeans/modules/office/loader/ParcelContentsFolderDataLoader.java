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
