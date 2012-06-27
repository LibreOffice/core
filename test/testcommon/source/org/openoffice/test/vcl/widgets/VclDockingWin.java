/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;



/**
 * Proxy used to access VCL Docking window
 *
 */
public class VclDockingWin extends VclWindow {
    /**
     * Define VCL Docking window
     * @param uid the string id
     */
    public VclDockingWin(String uid) {
        super(uid);
    }

    public VclDockingWin(SmartId id) {
        super(id);
    }

    /**
     * Docks a window on one edge of the desktop.
     */
    public void dock() {
        if (!isDocked())
            invoke(Constant.M_Dock);
    }

    /**
     * Undocks a docking window.
     */
    public void undock() {
        if (isDocked())
            invoke(Constant.M_Undock);
    }

    /**
     * Returns the docking state.
     * @return Returns TRUE if the window is docking, otherwise FALSE is
     *         returned.
     */
    public boolean isDocked() {
        return (Boolean) invoke(Constant.M_IsDocked);
    }
}
