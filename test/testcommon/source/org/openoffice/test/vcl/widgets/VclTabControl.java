/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;

/**
 * Proxy to access VCL tab control
 *
 */
public class VclTabControl extends VclControl {

    public VclTabControl() {
        super(Constant.UID_ACTIVE);
    }

    public VclTabControl(String id) {
        super(id);
    }

    public VclTabControl(VclApp app, String id) {
        super(app, id);
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
    public void setPage(String id) {
        invoke(Constant.M_SetPage, new Object[] {id});
    }
}
