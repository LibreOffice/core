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
