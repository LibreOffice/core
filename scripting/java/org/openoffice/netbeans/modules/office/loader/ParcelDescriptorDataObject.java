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
