/*************************************************************************
 *
 *  $RCSfile: OfficeDocumentChildren.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-02-21 16:24:14 $
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

public class OfficeDocumentChildren extends Children.Keys
    implements ChangeListener {

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
            setIconBase("/org/openoffice/netbeans/modules/office/resources/ParcelIcon");

            setName(name);
            setDisplayName(name.substring(name.lastIndexOf("/") + 1));
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
