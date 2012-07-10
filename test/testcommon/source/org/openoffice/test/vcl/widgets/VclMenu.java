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
 * Define VCL menu on a window
 *
 */
public class VclMenu extends VclWidget {

    private VclControl window = null;

    /**
     * Construct the popup menu
     *
     */
    public VclMenu() {
        super();
    }


    public VclMenu(VclApp app) {
        super(app);
    }


    /**
     * Construct the menu on the given window
     *
     * @param window
     */
    public VclMenu(VclControl window) {
        this.window = window;
        this.app = window.app;
    }

    /**
     * Returns the numbers of menu items (including the menu separators)
     *
     * @return Number of menu items in a menu . -1 : Return value error
     *
     */
    public int getItemCount() {
        use();
        return ((Long) app.caller.callCommand(Constant.RC_MenuGetItemCount)).intValue();
    }

    /**
     * Return the menu item at the n-th index
     *
     * @param index
     * @return null when the item is separator
     */
    public VclMenuItem getItem(int index) {
        use();
        long id = ((Long) app.caller.callCommand(Constant.RC_MenuGetItemId, new Object[] { new Integer(index + 1) })).intValue();
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
