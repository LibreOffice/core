/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package util;

import java.io.PrintWriter;
import java.util.ArrayList;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.awt.XWindow;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * This class supports some functions to handle easily accessible objects
 */
public class UITools {

    private final XAccessible mXRoot;

    public UITools(XWindow xWindow)
    {
        mXRoot = makeRoot(xWindow);
    }

    private static String getString(XInterface xInt)
    {
        XAccessibleText oText = UnoRuntime.queryInterface(XAccessibleText.class, xInt);
        return oText.getText();
    }

    private static void setString(XInterface xInt, String cText)
    {
        XAccessibleEditableText oText = UnoRuntime.queryInterface(XAccessibleEditableText.class, xInt);

        oText.setText(cText);
    }

    private static XAccessible makeRoot(XWindow xWindow)
    {
        return AccessibilityTools.getAccessibleObject(xWindow);
    }

    /**
     * get the root element of the accessible tree
     * @return the root element
     */
    public XAccessible getRoot()
    {
        return mXRoot;
    }

    /**
     * Helper method: set a text into AccessibleEdit field
     * @param textfiledName is the name of the text field
     * @param stringToSet is the string to set
     * @throws java.lang.Exception if something fail
     */
    public void setTextEditFiledText(String textfiledName, String stringToSet)
                        throws java.lang.Exception
    {
        XInterface oTextField = AccessibilityTools.getAccessibleObjectForRole(mXRoot,
                                            AccessibleRole.TEXT, textfiledName);
        setString(oTextField, stringToSet);
    }

    /**
     * returns the button by the given name
     * @param buttonName is the name of the button to get
     * @return a XAccessibleContext of the button
     * @throws java.lang.Exception if something fail
     */
    public XAccessibleContext getButton(String buttonName) throws java.lang.Exception
    {
        return AccessibilityTools.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.PUSH_BUTTON, buttonName);
    }

    /**
     * Helper method: gets button via accessibility and 'click' it</code>
     * @param buttonName is the name of the button to click
     * @throws java.lang.Exception if something fail
     */
     public void clickButton(String buttonName) throws java.lang.Exception
     {

        XAccessibleContext oButton =AccessibilityTools.getAccessibleObjectForRole
                                (mXRoot, AccessibleRole.PUSH_BUTTON, buttonName);
        if (oButton == null){
            throw new Exception("Could not get button '" + buttonName + "'");
        }
        XAccessibleAction oAction = UnoRuntime.queryInterface(XAccessibleAction.class, oButton);

        // "click" the button
        try{
            oAction.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
          throw new Exception("Could not do accessible action with '" +
                               buttonName + "'", e);
        }
     }

    /**
      * Helper method: returns the entry manes of a List-Box
      * @param ListBoxName the name of the listbox
      * @return the listbox entry names
      * @throws java.lang.Exception if something fail
      */
     public String[] getListBoxItems(String ListBoxName)
            throws java.lang.Exception
     {
         ArrayList<String> Items = new ArrayList<String>();
         try {
            XAccessibleContext xListBox = null;
            XAccessibleContext xList = null;

            xListBox =AccessibilityTools.getAccessibleObjectForRole(mXRoot,
                                         AccessibleRole.COMBO_BOX, ListBoxName);
            if (xListBox == null){
                xListBox =AccessibilityTools.getAccessibleObjectForRole(mXRoot,
                                             AccessibleRole.PANEL, ListBoxName);
            }

            if (xListBox == null){
                // get the list of TreeListBox
                xList =AccessibilityTools.getAccessibleObjectForRole(mXRoot,
                                              AccessibleRole.TREE, ListBoxName);

            // all other list boxes have a children of kind of LIST
            } else {

                XAccessible xListBoxAccess = UnoRuntime.queryInterface(XAccessible.class, xListBox);
                // if a List is not pulled to be open all entries are not visible, therefore the
                // boolean argument
                xList =AccessibilityTools.getAccessibleObjectForRole(
                                              xListBoxAccess, AccessibleRole.LIST, true);
            }

            for (int i=0;i<xList.getAccessibleChildCount();i++) {
                try {
                    XAccessible xChild = xList.getAccessibleChild(i);
                    XAccessibleContext xChildCont =
                                                  xChild.getAccessibleContext();
                    XInterface xChildInterface = UnoRuntime.queryInterface(XInterface.class, xChildCont);
                    Items.add(getString(xChildInterface));

                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                      throw new Exception("Could not get child form list of '"
                                         + ListBoxName + "'", e);
                }
            }

         } catch (Exception e) {
            throw new Exception("Could not get list of items from '"
                                         + ListBoxName + "'", e);
        }
        String[]ret = new String[Items.size()];
        return Items.toArray(ret);
     }

     /**
      * set a value to a named check box
      * @param CheckBoxName the name of the check box
      * @param Value the value to set
      *<ul>
      *    <li>0: not checked </li>
      *    <li>1: checked </li>
      *    <li>2: don't know </li>
      *</ul>
      * @throws java.lang.Exception if something fail
      */
     public void setCheckBoxValue(String CheckBoxName, Integer Value)
            throws java.lang.Exception
     {
         try {
            XInterface xCheckBox =AccessibilityTools.getAccessibleObjectForRole(mXRoot,
                                     AccessibleRole.CHECK_BOX, CheckBoxName);
            XAccessibleValue xCheckBoxValue = UnoRuntime.queryInterface(XAccessibleValue.class, xCheckBox);
            xCheckBoxValue.setCurrentValue(Value);

         } catch (Exception e) {
            throw new Exception("Could not set value to CheckBox '"
                                       + CheckBoxName + "'", e);
        }
     }

    /**
     * Prints the accessible tree to the <CODE>logWriter</CODE> only if <CODE>debugIsActive</CODE>
     * is set to <CODE>true</CODE>
     * @param log logWriter
     * @param debugIsActive prints only if this parameter is set to TRUE
     */
    public void printAccessibleTree(PrintWriter log, boolean debugIsActive) {
        AccessibilityTools.printAccessibleTree(log, mXRoot, debugIsActive);
    }

}
