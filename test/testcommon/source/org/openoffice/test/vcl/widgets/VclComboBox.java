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
 * Proxy used to access Vcl Combo Box
 *
 */
public class VclComboBox extends VclControl {

    public VclComboBox(SmartId id) {
        super(id);
    }

    /**
     * Define VclComboBox with String id
     *
     * @param uid
     */
    public VclComboBox(String uid) {
        super(uid);
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
