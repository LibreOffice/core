package org.openoffice.netbeans.modules.office.loader;

import java.io.IOException;

import org.openide.actions.*;
import org.openide.filesystems.*;
import org.openide.loaders.*;
import org.openide.util.NbBundle;
import org.openide.util.actions.SystemAction;

/** Recognizes single files in the Repository as being of a certain type.
 *
 * @author tomaso
 */
public class ParcelDescriptorDataLoader extends UniFileLoader {

    public ParcelDescriptorDataLoader() {
        this("org.openoffice.netbeans.modules.office.loader.ParcelDescriptorDataObject");
    }

    // Can be useful for subclasses:
    protected ParcelDescriptorDataLoader(String recognizedObjectClass) {
        super(recognizedObjectClass);
    }

    protected String defaultDisplayName() {
        return "StarOffice Script Parcel Descriptor";
    }

    protected void initialize() {
        super.initialize();

        // ExtensionList extensions = new ExtensionList();
        // extensions.addMimeType("text/x-parcel+xml");
        // extensions.addExtension("pml");
        getExtensions().addMimeType("text/x-parcel+xml");
        // setExtensions(extensions);
    }

    protected SystemAction[] defaultActions() {
        return new SystemAction[] {
            SystemAction.get(OpenAction.class),
            // SystemAction.get(GenerateParcelAction.class),
            null,
            SystemAction.get(CutAction.class),
            SystemAction.get(CopyAction.class),
            SystemAction.get(PasteAction.class),
            null,
            SystemAction.get(DeleteAction.class),
            SystemAction.get(RenameAction.class),
            null,
            // SystemAction.get(SaveAsTemplateAction.class),
            // null,
            // SystemAction.get(ToolsAction.class),
            SystemAction.get(PropertiesAction.class),
        };
    }

    protected MultiDataObject createMultiObject(FileObject primaryFile) throws DataObjectExistsException, IOException {
        return new ParcelDescriptorDataObject(primaryFile, this);
    }
}
