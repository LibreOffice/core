/*************************************************************************
 *
 *  $RCSfile: _XComboBox.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:10:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XComboBox
*/
public class _XComboBox extends MultiMethodTest {

    public XComboBox oObj = null;

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestActionListener
        implements com.sun.star.awt.XActionListener {
        public boolean disposingCalled = false;
        public boolean actionPerformedCalled = false;

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true;
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent e) {
            actionPerformedCalled = true;
        }

    }

    /**
    * Listener implementation which sets flags on appropriate method calls
    */
    protected class TestItemListener
        implements com.sun.star.awt.XItemListener {
        public boolean disposingCalled = false;
        public boolean itemStateChangedCalled = false;

        public void disposing(com.sun.star.lang.EventObject e) {
            disposingCalled = true;
        }

        public void itemStateChanged(com.sun.star.awt.ItemEvent e) {
            itemStateChangedCalled = true;
        }

    }
    private TestActionListener actionListener = new TestActionListener();
    private TestItemListener itemListener = new TestItemListener();
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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