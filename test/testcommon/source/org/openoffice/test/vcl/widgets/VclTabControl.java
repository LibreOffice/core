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
 * Proxy to access VCL tab control
 *
 */
public class VclTabControl extends VclControl {

    /**
     * Construct the tab folder with its string ID
     * @param uid
     */
    public VclTabControl(String uid) {
        super(uid);
    }

    public VclTabControl(SmartId id) {
        super(id);
    }

    /**
     * Get the current page
     * @return
     */
    public int getPage() {
        return ((Long) invoke(Constant.M_GetPage)).intValue();
    }

    /**
     * Returns the number of tab pages in the TabControl.
     * <p>
     *
     * @return number of tab pages in the dialog. -1 : Return value error
     *         <p>
     */
    public int getPageCount() {
        return ((Long) invoke(Constant.M_GetPageCount)).intValue();
    }

    /**
     * Returns the TabpageID of current Tab Page in the Tab dialog. This not the
     * UniqueID and is only needed for the SetPageID instruction..
     * <p>
     *
     * @return TabpageID used in SetPageID instruction; -1 : Return value error
     *         <p>
     */
    public int getPageId() {
        return ((Long) invoke(Constant.M_GetPageId)).intValue();
    }

    /**
     * Returns the TabpageID of specified Tab page in the Tab dialog. This not
     * the UniqueID and is only needed for the SetPageID instruction..
     * <p>
     *
     * @param nTabID :
     *            Specified Tab Page which order from 1. eg. A tab dialog have
     *            two Tab pages, nTabID is 2 if you want to get the TabpageID of
     *            second Tab page
     * @return TabpageID used in SetPageID instruction; -1 : Return value error
     *         <p>
     */
    public int getPageId(short nTabID) {
        return ((Long) invoke(Constant.M_GetPageId, new Object[] { nTabID }))
                .intValue();
    }

    /**
     * Changes to the tab page that has the TabpageID that you specify.
     * <p>
     *
     * @param id
     *            TabpageID of tab page
     */
    public void setPageId(int id) {
        invoke(Constant.M_SetPageId, new Object[] { id });
    }

    /**
     * Change to the tab page you specify
     * <p>
     *
     * @param nTabResID
     *            The resource ID of the specified Tab page in Tab Dialog
     */
    public void setPage(SmartId nTabResID) {
        invoke(Constant.M_SetPage, new Object[] { nTabResID.getSid() == null ? nTabResID.getId() : nTabResID.getSid()});
    }

    /**
     * Change to the specified tab page
     * @param widget the TabPage widget (Type: 372)
     * @throws Exception
     */
    public void setPage(VclControl widget) {
        setPage(widget.getUID());
    }
}
