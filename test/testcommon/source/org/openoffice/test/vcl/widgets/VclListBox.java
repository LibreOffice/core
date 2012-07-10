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


public class VclListBox extends VclControl  {


    public VclListBox(String id) {
        super(id);
    }


    public VclListBox(VclApp app, String id) {
        super(app, id);
    }


    /**
     * Returns the number of entries in a TreeListBox.
     *
     * @return Number of list box entries. Error if the return value is -1.
     */
    public int getItemCount() {
        return ((Long) invoke(Constant.M_GetItemCount)).intValue();
    }

    /**
     * Get the text of the specified entry in the tree list box Notice:
     * index,col starting from 0
     *
     * @param index
     * @param col
     * @return
     */
    public String getItemText(int index, int col) {
        return (String) invoke(Constant.M_GetItemText, new Object[] { new Integer(index + 1), new Integer(col + 1) });
    }

    /**
     * Get the text of the specified node Notice: index starting from 0
     *
     * @param index
     * @return
     */
    public String getItemText(int index) {
        return getItemText(index, 0);
    }

    /**
     * Returns the number of selected entries in a TreeListbox(you can select
     * more than one entry).
     *
     * @return The number of selected entries. Error is the return value is -1.
     */
    public int getSelCount() {
        return ((Long) invoke(Constant.M_GetSelCount)).intValue();
    }

    /**
     * Returns the index number of the selected entry in the TreeListBox.
     * Notice: index starting from 0
     *
     * @return The index number of selected entries. Error is the return value
     *         is -1.
     */
    public int getSelIndex() {
        return ((Long) invoke(Constant.M_GetSelIndex)).intValue() - 1;
    }

    /**
     * Get the text of the selected item
     */
    public String getSelText() {
        return (String) invoke(Constant.M_GetSelText);
    }

    /**
     * Select the specified node via its index Notice: index starting from 0
     *
     * @param index
     */
    public void select(int index) {
        invoke(Constant.M_Select, new Object[] { new Integer(index + 1) });
    }

    /**
     * Selects the text of an entry.
     *
     * @param str
     *            the item string
     */
    public void select(String text) {
        if (getType() == 324) {
            int count = getItemCount();
            for (int i = 0; i < count; i++) {
                if (text.equals(getItemText(i))) {
                    select(i);
                    return;
                }
            }

            throw new RuntimeException(text + " is not found in the list box");
        } else {
            invoke(Constant.M_Select, new Object[] { text });
        }
    }

    /**
     * Append one item to be selected after selected some items.
     *
     * @param i
     *            the index of the item
     */
    public void multiSelect(int i) {
        invoke(Constant.M_MultiSelect, new Object[] { new Integer(i + 1) });
    }

    /**
     * Append one item to be selected after selected some items.
     *
     * @param text
     *            the text of the item
     */
    public void multiSelect(String text) {
        invoke(Constant.M_MultiSelect, new Object[] { text });
    }

    /**
     * get all items'text.
     *
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
     *
     * @param text
     * @return
     */
    public int getItemIndex(String text) {
        int count = getItemCount();
        for (int i = 0; i < count; i++) {
            if (text.equals(getItemText(i)))
                return i;
        }

        throw new RuntimeException(text + " is not found in the list box");
    }


    /**
     * Check if the list box has the specified item
     *
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
