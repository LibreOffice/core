package org.openoffice.netbeans.modules.office.loader;

import org.openide.loaders.DataFolder;
import org.openide.loaders.DataObjectExistsException;

import org.openide.filesystems.FileObject;

public class ParcelContentsFolder extends DataFolder {
    public ParcelContentsFolder(FileObject pf, ParcelContentsFolderDataLoader loader)
        throws DataObjectExistsException {
        super(pf, loader);
    }
}
