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

import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.VclHook;

/**
 *
 */
public class VclMenuItem {

    private int id = -1;

    private String[] path = null;

    private VclMenu menu = null;

    /**
     * Construct menu item with its ID
     *
     * @param id
     */
    public VclMenuItem(int id) {
        this.id = id;
    }

    /**
     * Construct menu item with its path like
     * "RootMenuItem->Level1Item->Level2Item".
     *
     * @param path
     */
    public VclMenuItem(String path) {
        this.path = path.split("->");
    }

    /**
     * Vcl Menu Item on menu bar
     *
     * @param menu
     * @param id
     */
    public VclMenuItem(VclMenu menu, int id) {
        this.id = id;
        this.menu = menu;
    }

    /**
     * Vcl Menu Item on menu bar
     *
     * @param menu
     * @param path
     */
    public VclMenuItem(VclMenu menu, String path) {
        this.path = path.split("->");
        this.menu = menu;
    }

    private Object invoke(int methodId) {
        int id = getId();
        if (id == -1)
            throw new RuntimeException("Menu item '" + path[path.length - 1] + "' can be found!");
        return VclHook.invokeCommand(methodId, new Object[] { id });
    }

    /**
     *
     * @return
     */
    public int getId() {
        VclMenu menu = new VclMenu();
        if (path != null) {
            int count = menu.getItemCount();
            for (int i = 0; i < count; i++) {
                VclMenuItem item = menu.getItem(i);
                if (item == null)
                    continue;
                String itemText = path[path.length - 1];
//              if (item.getTextWithoutMneumonic().contains(itemText)) {
                //Change "contains" into "equals" to avoid the blocking while search for menu item
                if (item.getTextWithoutMneumonic().equals(itemText)) {
                    return item.getId();
                }
            }

            return -1;
        }

        return this.id;
    }

    /**
     * Select the menu item
     *
     */
    public void select() {
        if (menu != null)
            menu.use();
        for (int i = 0; i < path.length; i++) {
            new VclMenuItem(path[i]).pick();
            Tester.sleep(0.5);
        }
    }

    private void pick() {
        invoke(Constant.RC_MenuSelect);
    }

    /**
     * Select the parent of the item
     *
     */
    public void selectParent() {
        if (menu != null)
            menu.use();
        for (int i = 0; i < path.length - 1; i++)
            new VclMenuItem(path[i]).pick();
    }

    /**
     * Check if the menu item exists
     *
     * @return
     */
    public boolean exists() {
        return getId() != -1;
    }

    /**
     * Check if the menu item is selected!
     *
     * @return
     */
    public boolean isSelected() {
        return ((Boolean) invoke(Constant.RC_MenuIsItemChecked)).booleanValue();
    }

    /**
     * Check if the menu item is enabled
     *
     * @return
     */
    public boolean isEnabled() {
        return ((Boolean) invoke(Constant.RC_MenuIsItemEnabled)).booleanValue();
    }

    /**
     * Get the menu item position
     *
     * @return
     */
    public int getPosition() {
        return ((Long) invoke(Constant.RC_MenuGetItemPos)).intValue();
    }

    /**
     * Get the menu item text
     *
     * @return
     */
    public String getText() {
        return (String) invoke(Constant.RC_MenuGetItemText);
    }

    /**
     * Get the command id which is UNO-Slot
     *
     * @return
     */
    public String getCommand() {
        return (String) invoke(Constant.RC_MenuGetItemCommand);
    }

    /**
     * Get the accelerator character
     */
    public int getAccelerator() {
        String text = this.getText();
        if (text == null)
            return 0;
        int index = text.indexOf("~");
        return index != -1 && index + 1 < text.length() ? text.charAt(index + 1) : 0;
    }

    /**
     * Get text without mneumonic
     */
    public String getTextWithoutMneumonic() {
        String text = this.getText();
        return text != null ? text.replace("~", "") : text;
    }

    /**
     * Check if the menu item is showing
     */
    public boolean isShowing() {
        return exists();
    }

    /**
     * Check if the menu item has sub menu
     *
     * @return
     */
    public boolean hasSubMenu() {
        return (Boolean) invoke(Constant.RC_MenuHasSubMenu);
    }

    public String toString() {
        return "ID:" + getId() + ", Text:" + getText() + ", Selected:" + isSelected() + ", Enabled:" + isEnabled() + ", Command:" + getCommand()
                + ", Position:" + getPosition();
    }

    /**
     * Check if the widget exists in a period of time
     */
    public boolean exists(double iTimeout) {
        return exists(iTimeout, 1);
    }

    /**
     * Check if the widget exists in a period of time
     */
    public boolean exists(double iTimeout, double interval) {
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < iTimeout * 1000) {
            if (exists())
                return true;
            SystemUtil.sleep(interval);
        }

        return exists();
    }
}
