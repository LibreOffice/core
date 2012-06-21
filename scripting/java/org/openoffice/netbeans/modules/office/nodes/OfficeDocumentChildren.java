/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.netbeans.modules.office.nodes;

import java.io.IOException;
import java.util.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.openide.nodes.*;
import org.openide.actions.*;
import org.openide.util.actions.SystemAction;
import org.openide.util.HelpCtx;

import org.openoffice.netbeans.modules.office.options.OfficeSettings;
import org.openoffice.netbeans.modules.office.utils.NagDialog;
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
            OfficeSettings settings = OfficeSettings.getDefault();
            String message = "If you already have this document open in " +
                "Office, please close it before continuing. Click OK to " +
                "delete this parcel.";

            if (settings.getWarnBeforeParcelDelete() == true) {
                NagDialog warning = NagDialog.createConfirmationDialog(
                    message, "Show this message in future", true);

                boolean result = warning.show();

                if (warning.getState() == false)
                    settings.setWarnBeforeParcelDelete(false);

                if (result == false)
                    return;
            }
            super.destroy();
            document.removeParcel(name);
        }
    }
}
