/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.SmartId;


public class VclTabPage extends VclDialog {

    private VclTabControl tabControl = null;

    public VclTabPage(SmartId id, VclTabControl tabControl) {
        super(id);
        this.tabControl = tabControl;
    }

    public VclTabPage(String uid, VclTabControl tabControl) {
        super(uid);
        this.tabControl = tabControl;
    }


    /**
     * Selects the tab page to be active.
     *
     */
    public void select() {
        if (tabControl != null)
            tabControl.setPage(this.getUID());
    }
}
