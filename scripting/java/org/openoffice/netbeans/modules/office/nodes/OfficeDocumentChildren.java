package org.openoffice.netbeans.modules.office.nodes;

import java.io.IOException;
import java.util.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.openide.nodes.*;
import org.openide.actions.*;
import org.openide.util.actions.SystemAction;
import org.openide.util.HelpCtx;

import org.openoffice.netbeans.modules.office.actions.OfficeDocumentCookie;

public class OfficeDocumentChildren extends Children.Keys implements ChangeListener {

    private OfficeDocumentCookie document = null;

    public OfficeDocumentChildren(OfficeDocumentCookie cookie) {
        document = cookie;
    }

    private void refreshKeys() {
        if (document == null) {
            setKeys(Collections.EMPTY_SET);
            return;
        }

        Enumeration parcels = document.getParcels();
        if (parcels.hasMoreElements() != true) {
            setKeys(Collections.EMPTY_SET);
            return;
        }

        ArrayList keys = new ArrayList();
        while (parcels.hasMoreElements()) {
            String parcel = (String)parcels.nextElement();
            keys.add(parcel);
        }
        setKeys(keys);
    }

    protected void addNotify() {
        super.addNotify();
        document.addChangeListener(this);
        refreshKeys();
    }

    protected void removeNotify() {
        super.removeNotify();
        document.removeChangeListener(this);
        setKeys(Collections.EMPTY_SET);
    }

    protected Node[] createNodes(Object key) {
        String name = (String)key;
        return new Node[] {new ParcelNode(name)};
    }

    public void stateChanged(ChangeEvent e) {
        refreshKeys();
    }

    private class ParcelNode extends AbstractNode {
        private String name;

        public ParcelNode(String name) {
            super(Children.LEAF);
            this.name = name;
            init();
        }

        private void init() {
            setIconBase("/org/openoffice/netbeans/modules/office/resources/OfficeIcon");

            setName(name);
            setDisplayName(name.substring(name.lastIndexOf(".") + 1));
            setShortDescription(name);
        }

        protected SystemAction[] createActions() {
            return new SystemAction[] {
                SystemAction.get(DeleteAction.class),
            };
        }

        public HelpCtx getHelpCtx() {
            return HelpCtx.DEFAULT_HELP;
        }

        public boolean canDestroy() {
            return true;
        }

        public void destroy() throws IOException {
            super.destroy();
            document.removeParcel(name);
        }
    }
}
