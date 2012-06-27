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
 * Proxy to access the VCL tool box
 *
 * Type: WINDOW_TOOLBOX
 *
 */
public class VclToolBox extends VclDockingWin {

    public VclToolBox(SmartId id) {
        super(id);
    }

    /**
     * Define a vcl tool bar
     * @param uid the string id
     */
    public VclToolBox(String uid) {
        super(uid);
    }

    /**
     * Click the down arrow of tool bar to show the menu
     *
     */
    public void openMenu() {
        invoke(Constant.M_OpenContextMenu);
    }

    /**
     * Returns the count of items in the tool bar
     *
     * @return the count
     */
    public int getItemCount() {
        return ((Long) invoke(Constant.M_GetItemCount)).intValue();
    }

    /**
     * Get the text of the index-th item
     * @param index
     * @return
     */
    public String getItemText(int index) {
        return (String) invoke(Constant.M_GetItemText2, new Object[] {index + 1});
    }

    /**
     * Get the quick tooltip text of the index-th item
     * @param index
     * @return
     */
    public String getItemQuickToolTipText(int index) {
        return (String) invoke(Constant.M_GetItemQuickHelpText, new Object[] {index + 1});
    }

    /**
     * Get the tooltip text of the index-th item
     * @param index
     * @return
     */
    public String getItemToolTipText(int index) {
        return (String) invoke(Constant.M_GetItemHelpText, new Object[] {index + 1});
    }

    /**
     * Get the name of the next tool bar
     * @return
     */
    public String getNextToolBar() {
        return (String) invoke(Constant.M_GetNextToolBox);
    }
}
