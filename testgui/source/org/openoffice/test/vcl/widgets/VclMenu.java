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
import org.openoffice.test.vcl.client.VclHook;

/**
 * Define VCL menu on a window
 *
 */
public class VclMenu {

    private VclControl window = null;

    /**
     * Construct the popup menu
     *
     */
    public VclMenu() {

    }

    /**
     * Construct the menu on the given window
     *
     * @param window
     */
    public VclMenu(VclControl window) {
        this.window = window;
    }

    /**
     * Returns the numbers of menu items (including the menu separators)
     *
     * @return Number of menu items in a menu . -1 : Return value error
     *
     */
    public int getItemCount() {
        use();
        return ((Long) VclHook.invokeCommand(Constant.RC_MenuGetItemCount)).intValue();
    }

    /**
     * Return the menu item at the n-th index
     *
     * @param index
     * @return null when the item is separator
     */
    public VclMenuItem getItem(int index) {
        use();
        long id = ((Long) VclHook.invokeCommand(Constant.RC_MenuGetItemId, new Object[] { new Integer(index + 1) })).intValue();
        if (id == 0)
            return null;
        return new VclMenuItem(this, (int) id);
    }

    protected void use() {
        if (window != null) {
            window.useMenu();
        }
    }
}
