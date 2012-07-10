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
 * Proxy used to access Vcl Combo Box
 *
 */
public class VclComboBox extends VclControl {

    public VclComboBox(String id) {
        super(id);
    }

    public VclComboBox(VclApp app, String id) {
        super(app, id);
    }

    /**
     * Get the item count
     */
    public int getItemCount() {
        return ((Long) invoke(Constant.M_GetItemCount)).intValue();
    }

    /**
     * Get the text of the index-th item.
     * @index The index starts from 0.
     */
    public String getItemText(int index) {
        return (String) invoke(Constant.M_GetItemText, new Object[] {index + 1});
    }

    /**
     * Get the index of the selected item.
     * @index The index starts from 0.
     */
    public int getSelIndex() {
        int index = ((Long) invoke(Constant.M_GetSelIndex)).intValue();
        return index - 1;
    }

    /**
     * Get the text of the selected item.
     */
    public String getSelText() {
        return (String) invoke(Constant.M_GetSelText);
    }

    /**
     * Get the text in the combo box
     */
    public String getText() {
        // Fix: Use M_Caption to get the text. M_GetText does not work
        return (invoke(Constant.M_Caption)).toString();
    }

    /**
     * Get the text of all items
     */
    public String[] getItemsText() {
        int count = getItemCount();
        String[] res = new String[count];
        for (int i = 0; i < count; i++) {
            res[i] = getItemText(i);
        }
        return res;
    }

    /**
     * Select the index-th item.
     * @index The index starts from 0.
     */
    public void select(int index) {
        invoke(Constant.M_Select, new Object[] {index + 1});
    }

    /**
     * Select the item with the given text
     */
    public void select(String text) {
        invoke(Constant.M_Select, new Object[] {text});
    }

    /**
     * Sets no selection in a list (Sometimes this corresponds to the first
     * entry in the list)
     *
     */
    public void setNoSelection() {
        invoke(Constant.M_SetNoSelection);
    }

    /**
     * Set the text of the combo box
     */
    public void setText(String text) {
        invoke(Constant.M_SetText, new Object[] {text});
    }

    /**
     * Check if the list box has the specified item
     * @param str
     * @return
     */
    public boolean hasItem(String str) {
        int len = getItemCount();
        for (int i = 0; i < len; i++) {
            String text = getItemText(i);
            if (str.equals(text))
                return true;
        }
        return false;
    }
}
