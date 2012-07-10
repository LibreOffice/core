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
 * Proxy to access the VCL tool box
 *
 * Type: WINDOW_TOOLBOX
 *
 */
public class VclToolBox extends VclDockingWin {


    public VclToolBox(String uid) {
        super(uid);
    }


    public VclToolBox(VclApp app, String id) {
        super(app, id);
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
