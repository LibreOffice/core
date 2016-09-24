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

package ifc.awt;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.awt.XComboBox;

/**
* Testing <code>com.sun.star.awt.XComboBox</code>
* interface methods :
* <ul>
*  <li><code> addItemListener()</code></li>
*  <li><code> removeItemListener()</code></li>
*  <li><code> addActionListener()</code></li>
*  <li><code> removeActionListener()</code></li>
*  <li><code> addItem()</code></li>
*  <li><code> addItems()</code></li>
*  <li><code> removeItems()</code></li>
*  <li><code> getItemCount()</code></li>
*  <li><code> getItem()</code></li>
*  <li><code> getItems()</code></li>
*  <li><code> getDropDownLineCount()</code></li>
*  <li><code> setDropDownLineCount()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XComboBox
*/
public class _XComboBox extends MultiMethodTest {

    public XComboBox oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestActionListener
        implements com.sun.star.awt.XActionListener {

        public void disposing(com.sun.star.lang.EventObject e) {}

        public void actionPerformed(com.sun.star.awt.ActionEvent e) {}
    }

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected static class TestItemListener
        implements com.sun.star.awt.XItemListener {

        public void disposing(com.sun.star.lang.EventObject e) {}

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {}
    }
    private final TestActionListener actionListener = new TestActionListener();
    private final TestItemListener itemListener = new TestItemListener();
    short lineCount = 0;
    short itemCount = 0;

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _addItemListener() {

        oObj.addItemListener(itemListener);

        tRes.tested("addItemListener()", Status.skipped(true));
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _removeItemListener() {
        requiredMethod("addItemListener()");

        oObj.removeItemListener(itemListener);

        tRes.tested("removeItemListener()", Status.skipped(true));
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _addActionListener() {

        oObj.addActionListener(actionListener);

        tRes.tested("addActionListener()", Status.skipped(true));
    }

    /**
    * !!! Can be checked only interactively !!!
    */
    public void _removeActionListener() {
        requiredMethod("addActionListener()");

        oObj.removeActionListener(actionListener);

        tRes.tested("removeActionListener()", Status.skipped(true));
    }

    /**
    * Adds one item to the last position and check the number of
    * items after addition. <p>
    * Has <b>OK</b> status if the number of items increased by 1.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getItemCount </code> </li>
    * </ul>
    */
    public void _addItem() {
        requiredMethod("getItemCount()");

        boolean result = true;
        oObj.addItem("Item1", itemCount);
        result = oObj.getItemCount() == itemCount + 1;

        tRes.tested("addItem()", result);
    }

    /**
    * Adds one two items to the last position and check the number of
    * items after addition. <p>
    * Has <b>OK</b> status if the number of items increased by 2.<p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> addItem </code> </li>
    * </ul>
    */
    public void _addItems() {
        executeMethod("addItem()");

        boolean result = true;
        short oldCnt = oObj.getItemCount();
        oObj.addItems(new String[] { "Item2", "Item3" }, oldCnt);
        result = oObj.getItemCount() == oldCnt + 2;

        tRes.tested("addItems()", result);
    }

    /**
    * Gets the current number of items and tries to remove them all
    * then checks number of items. <p>
    * Has <b>OK</b> status if no items remains. <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getItems </code> </li>
    *  <li> <code> getItem </code> </li>
    * </ul>
    */
    public void _removeItems() {
        executeMethod("getItems()");
        executeMethod("getItem()");

        boolean result = true;
        short oldCnt = oObj.getItemCount();
        oObj.removeItems((short) 0, oldCnt);
        result = oObj.getItemCount() == 0;

        tRes.tested("removeItems()", result);
    }

    /**
    * Just retrieves current number of items and stores it. <p>
    * Has <b>OK</b> status if the count is not less than 0.
    */
    public void _getItemCount() {

        itemCount = oObj.getItemCount();

        tRes.tested("getItemCount()", itemCount >= 0);
    }

    /**
    * After <code>addItem</code> and <code>addItems</code> methods
    * test the following items must exist {..., "Item1", "Item2", "Item3"}
    * Retrieves the item from the position which was ititially the last.<p>
    * Has <b>OK</b> status if the "Item1" was retrieved. <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> addItems </code> </li>
    * </ul>
    */
    public void _getItem() {
        requiredMethod("addItems()");

        boolean result = true;
        String item = oObj.getItem(itemCount);
        result = "Item1".equals(item);

        tRes.tested("getItem()", result);
    }

    /**
    * After <code>addItem</code> and <code>addItems</code> methods
    * test the following items must exist {..., "Item1", "Item2", "Item3"}
    * Retrieves all items. <p>
    * Has <b>OK</b> status if the last three items retrieved are
    * "Item1", "Item2" and "Item3". <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> addItems </code> </li>
    * </ul>
    */
    public void _getItems() {
        requiredMethod("addItems()");

        boolean result = true;
        String[] items = oObj.getItems();
        for (int i = itemCount; i < (itemCount + 3); i++) {
            result &= ("Item" + (i + 1)).equals(items[i]);
        }

        tRes.tested("getItems()", result);
    }

    /**
    * Gets line count and stores it. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getDropDownLineCount() {

        boolean result = true;
        lineCount = oObj.getDropDownLineCount();

        tRes.tested("getDropDownLineCount()", result);
    }

    /**
    * Sets a new value and then checks get value. <p>
    * Has <b>OK</b> status if set and get values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDropDownLineCount </code>  </li>
    * </ul>
    */
    public void _setDropDownLineCount() {
        requiredMethod("getDropDownLineCount()");

        boolean result = true;
        oObj.setDropDownLineCount((short) (lineCount + 1));
        result = oObj.getDropDownLineCount() == lineCount + 1;

        tRes.tested("setDropDownLineCount()", result);
    }
}
