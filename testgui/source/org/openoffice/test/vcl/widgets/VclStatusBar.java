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
 * VCL status bar proxy
 *
 */
public class VclStatusBar extends VclControl {

    public VclStatusBar(SmartId id) {
        super(id);
    }

    public VclStatusBar(String uid) {
        super(uid);
    }

    /**
     * Get the text of the item with the given ID
     * @param id
     * @return
     */
    public String getItemTextById(int id) {
        return (String) invoke(Constant.M_StatusGetText, new Object[]{id});
    }

    /**
     * Get the text of the item at the given index
     * @param i
     * @return
     */
    public String getItemText(int i) {
        return getItemTextById(getItemId(i));
    }

    /**
     * Get the item count
     * @return
     */
    public int getItemCount() {
        return ((Long) invoke(Constant.M_StatusGetItemCount)).intValue();
    }

    /**
     * Get the item ID at the given index
     * @param i
     * @return
     */
    public int getItemId(int i) {
        return ((Long) invoke(Constant.M_StatusGetItemId, new Object[]{i + 1})).intValue();
    }

    /**
     * Check if the status box is progress box.
     * @return
     */
    public boolean isProgress() {
        return (Boolean) invoke(Constant.M_StatusIsProgress);
    }
}
