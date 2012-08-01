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


public class VclTreeListBox extends VclControl {

    /**
     * Construct the tree with its String ID
     * @param id
     */
    public VclTreeListBox(String id) {
        super(id);
    }


    public VclTreeListBox(VclApp app, String id) {
        super(app, id);
    }

    /**
     * Returns the number of entries in a TreeListBox. Namely all the expanded node.
     *
     * @return Number of list box entries. Error if the return value is -1.
     */
    public int getItemCount() {
        return ((Long)invoke(Constant.M_GetItemCount)).intValue();
    }

    /**
     * Get the text of the specified entry in the tree list box
     * Notice: index,col starting from 0. <br>
     * In some tree list box, there is a hidden column, so if you can get the item text, pls add 1 to the column
     * @param row
     * @param col
     * @return
     */
    public String getItemText(int row, int col) {
        return (String) invoke(Constant.M_GetItemText, new Object[]{row + 1, col + 1});
    }

    /**
     * Returns the text of the first string column of the row at the given index
     * Notice: index starting from 0
     * @param index
     * @return
     */
    public String getItemText(int index) {
        return (String) invoke(Constant.M_GetItemText, new Object[] {index + 1});
    }

    /**
     * Returns the number of selected entries in a TreeListbox(you can select
     * more than one entry).
     *
     * @return The number of selected entries. Error is the return value is -1.
     */
    public int getSelCount() {
        return ((Long)invoke(Constant.M_GetSelCount)).intValue();
    }

    /**
     * Returns the index number of the selected entry in the TreeListBox.
     * Notice: index starting from 0
     * @return The index number of selected entries. Error is the return value
     *         is -1.
     */
    public int getSelIndex() {
        int index = ((Long) invoke(Constant.M_GetSelIndex)).intValue();
        return index - 1;
    }

    /**
     * Returns the text of the first string column of the first selected entry
     *
     * @return
     */
    public String getSelText() {
        return (String) invoke(Constant.M_GetSelText, new Object[]{1});
    }

    /**
     * Returns the text of the n-th column of the first selected entry.
     * If the column is not a string, a VCLException will be throwed.
     * @param col the column index starting from 0
     * @return
     */
    public String getSelText(int col) {
        return (String) invoke(Constant.M_GetSelText, new Object[]{col + 1});
    }

    /**
     * Returns the text of the n-th column of the n-th selected entry.
     * @param index the selected entry index  starting from 0
     * @param col the column index  starting from 0
     * @return
     */
    public String getSelText(int index, int col) {
        return invoke(Constant.M_GetSelText, new Object[]{(index + 1), (col + 1)}).toString();
    }


    /**
     * Select the entry at the given index
     * @param index starting from 0
     */
    public void select(int index) {
        invoke(Constant.M_Select, new Object[]{index+1});
    }

    /**
     * Unselect the entry at the given index
     * @param index starting from 0
     */
    public void unselect(int index) {
        invoke(Constant.M_Select, new Object[]{index + 1, false});
    }

    /**
     * Collapse the specified entry
     * Notice: index starting from 0
     * @param index the node index
     */
    public void collapse(int index) {
//      select(index);
//      Tester.typeKeys("<left>");
        invoke(Constant.M_Collapse, new Object[]{index + 1, false});
    }

    /**
     * Expand the specified entry
     * Notice: index starting from 0
     * @param index the entry index
     */
    public void expand(int index) {
//      select(index);
//      Tester.typeKeys("<right>");
        invoke(Constant.M_Expand, new Object[]{index + 1, false});
    }

    /**
     * Select the node via its text
     * @param str
     * @return the index of the node
     */
    public void select(String str) {
        select(str, 0);
    }

    /**
     * Select the entry with the given string and after the given index
     * @param str
     * @param start
     */
    public void select(String str, int start) {
        int len = getItemCount();
        for (int i = start; i < len; i++) {
            String text = getItemText(i);
            if (str.equals(text)) {
                select(i);
                return;
            }
        }
        throw new RuntimeException(str + " is not found in the tree");
    }


    /**
     * Return the text of all items
     * @return A String[] includes all items text
     */
    public String[] getAllItemsText() {
        int len = getItemCount();
        String[] ret = new String[len];
        for (int i = 0; i < len; i++) {
            ret[i] = getItemText(i);
        }
        return ret;
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
    /**
     *
     * @return
     */
    public boolean isChecked(int row) {
        return ((Boolean)invoke(Constant.M_IsChecked, new Object[]{new Integer(row+1)})).booleanValue();
    }

    public boolean isChecked(int row, int col) {
        return ((Boolean)invoke(Constant.M_IsChecked, new Object[]{new Integer(row+1), new Integer(col+1)})).booleanValue();
    }

    /**
     * @param
     * @return
     */
    public boolean isShowing(String str) {
        boolean ret = false;
        int len = getItemCount();
        for (int i = 0; i < len; i++) {
            String text = getItemText(i);
            if (str.equals(text)) {
                ret = true;
            }
        }
        return ret;
    }

    /**
     * Check if the selected node is tristate
     * @return
     */
    public boolean isTristate(int row) {
        return ((Boolean)invoke(Constant.M_IsTristate, new Object[]{new Integer(row+1)})).booleanValue();
    }

    public boolean isTristate(int row, int col) {
        return ((Boolean)invoke(Constant.M_IsTristate, new Object[]{new Integer(row+1), new Integer(col+1)})).booleanValue();
    }

    /**
     * Check if the selected node is tristate
     * @return
     */
    public boolean isTristate() {
        return ((Boolean)invoke(Constant.M_IsTristate)).booleanValue();
    }

    /**
     * Get the state of the selected node
     * @return
     */
    public int getState() {
        return ((Long)invoke(Constant.M_GetState)).intValue();
    }

    /**
     * Get the state of the selected node
     * @return
     */
    public int getState(int row) {
        return ((Long)invoke(Constant.M_GetState, new Object[]{new Integer(row+1)})).intValue();
    }

    public int getState(int row, int col) {
        return ((Long)invoke(Constant.M_GetState, new Object[]{new Integer(row+1), new Integer(col+1)})).intValue();
    }

    /**
     * check the selected node
     *
     */
    public void check()  {
        invoke(Constant.M_Check);
    }

    public void check(int row)  {
        invoke(Constant.M_Check, new Object[]{new Integer(row+1)});
    }

    public void check(int row, int col)  {
        invoke(Constant.M_Check, new Object[]{new Integer(row+1), new Integer(col+1)});
    }

    /**
     * uncheck the selected node
     *
     */
    public void unCheck() {
        invoke(Constant.M_UnCheck);
    }

    public void unCheck(int row) {
        invoke(Constant.M_UnCheck, new Object[]{new Integer(row+1)});
    }

    public void unCheck(int row, int col) {
        invoke(Constant.M_UnCheck, new Object[]{new Integer(row+1), new Integer(col+1)});
    }

    /**
     * Set the selected node to tristate
     *
     */
    public void triState() {
        invoke(Constant.M_TriState);
    }


    public void triState(int row) {
        invoke(Constant.M_TriState, new Object[]{new Integer(row+1)});
    }

    public void triState(int row, int col) {
        invoke(Constant.M_TriState, new Object[]{new Integer(row+1), new Integer(col+1)});
    }

    /**
     * Collapse all nodes in the tree list box.
     *
     */
    public void collapseAll() {//?after expandAll, can not collapse the first node
        for (int i = 0; i <  getItemCount(); i++) {
            collapse(i);
        }
    }

    /**
     * Expand all nodes in the tree list box
     *
     */
    public void expandAll() {
        for (int i = 0; i <  getItemCount(); i++) {
            expand(i);
        }
    }

    /**
     * Select the specified node by path.
     * @param path The node path likes "Node1->Node2->Node3"
     * @return true if the node exists, false otherwise
     */
    public void selectByPath(String path) {
        collapseAll();
        String[] items = path.split("->");
        int i = 0;
        for (String s : items) {
            int len = getItemCount();
            for (; i < len; i++) {
                String text = getItemText(i);
                if (s.equals(text)) {
                    expand(i);
                    break;
                }
            }
            if (i == len) {
                throw new RuntimeException("Menu item '" + path + "' is not found in the tree");
            }
        }
    }

    /**
     * Get the column type of the given row
     * @param row
     * @param col
     * @return
     */
    public int getItemType(int row, int col) {
        return (Integer)invoke(Constant.M_GetItemType, new Object[]{row + 1, col + 1});
    }
}
