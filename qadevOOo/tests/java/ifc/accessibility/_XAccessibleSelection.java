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
package ifc.accessibility;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


/**
 * Testing <code>com.sun.star.accessibility.XAccessibleSelection</code>
 * interface methods :
 * <ul>
 *  <li><code>selectAccessibleChild()</code></li>
 *  <li><code>isAccessibleChildSelected()</code></li>
 *  <li><code>clearAccessibleSelection()</code></li>
 *  <li><code>selectAllAccessibleChildren()</code></li>
 *  <li><code>getSelectedAccessibleChildCount()</code></li>
 *  <li><code>getSelectedAccessibleChild()</code></li>
 *  <li><code>deselectAccessibleChild()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XAccessibleSelection.multiSelection'</code>
 *  (of type <code>Boolean</code>) <b> optional </b>:
 *   Indicates whether or not mutiply children could be selected.
 *   If the relation is <code>false</code> then more than 1 child
 *   couldn't be selected. </li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleSelection
 */
public class _XAccessibleSelection extends MultiMethodTest {
    private static final String className = "com.sun.star.accessibility.XAccessibleSelection";
    public XAccessibleSelection oObj = null;
    XAccessibleContext xAC = null;
    int childCount;
    protected boolean multiSelection = true;
    protected boolean OneAlwaysSelected = false;

    // temporary while accessibility package is in com.sun.star
    protected String getTestedClassName() {
        return className;
    }

    /**
     * Retrieves the interface <code>XAccessibleContext</code>
     * and object relation.
     * @see com.sun.star.accessibility.XAccessibleContext
     * @see ifc.accessibility._XAccessibleContext
     */
    protected void before() {
        xAC = UnoRuntime.queryInterface(
                      XAccessibleContext.class, oObj);

        if (xAC == null) {
            throw new StatusException(Status.failed(
                                              "Couldn't query XAccessibleContext. Test must be modified"));
        }

        Boolean b = (Boolean) tEnv.getObjRelation(
                            "XAccessibleSelection.multiSelection");

        if (b != null) {
            multiSelection = b.booleanValue();
        }

        Boolean b2 = (Boolean) tEnv.getObjRelation(
                             "XAccessibleSelection.OneAlwaysSelected");

        if (b2 != null) {
            OneAlwaysSelected = b2.booleanValue();
        }

        childCount = xAC.getAccessibleChildCount();
        log.println("Child count: " + childCount);
    }

    /**
     * Selects accessible child with index some wrong indexes
     * and with legal index.
     * Has OK status if exception was thrown for wrong indexes
     * and if exception wasn't thrown for correct index.
     */
    public void _selectAccessibleChild() {
        boolean res = true;

        try {
            log.println("Try to select child with index " + childCount);
            oObj.selectAccessibleChild(childCount);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res = true;
        }

        try {
            log.println("Try to select child with index -1");
            oObj.selectAccessibleChild(-1);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        log.println("ChildCount: " + childCount);

        int usedChildren = childCount;

        if (childCount > 500) {
            log.println("Restricting to 500");
            usedChildren = 500;
        }

        if (usedChildren > 0) {
            try {
                for (int i = 0; i < usedChildren; i++) {
                    log.print("Trying to select child with index " + i + ": ");

                    if (isSelectable(tEnv.getTestObject(), i)) {
                        oObj.selectAccessibleChild(i);
                        log.println("OK");
                    } else {
                        log.println("Child isn't selectable");
                    }
                }

                res &= true;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("selectAccessibleChild()", res);
    }

    /**
     * Calls the method with the wrong index and with the correct index.
     * Has OK status if exception was thrown for wrong index and
     * if exception wasn't thrown for the correct index.
     */
    public void _isAccessibleChildSelected() {
        executeMethod("selectAccessibleChild()");

        boolean res = true;
        boolean isSelected = false;

        try {
            log.print("isAccessibleChildSelected(-1)? ");
            isSelected = oObj.isAccessibleChildSelected(-1);
            log.println(res);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res = true;
        }

        try {
            log.print("isAccessibleChildSelected(" + childCount + ")? ");
            isSelected = oObj.isAccessibleChildSelected(childCount);
            log.println(isSelected);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        int SelectableChildCount = chkSelectable(tEnv.getTestObject());

        if (SelectableChildCount > 500) {
            SelectableChildCount = 500;
        }

        log.println("SelectableChildCount: " + SelectableChildCount);

        if (SelectableChildCount > 0) {
            try {
                oObj.selectAllAccessibleChildren();

                for (int k = 0; k < SelectableChildCount; k++) {
                    log.println("Trying to select child with index " + k);

                    if (isSelectable(tEnv.getTestObject(), k)) {
                        oObj.selectAccessibleChild(k);
                        shortWait();
                        isSelected = oObj.isAccessibleChildSelected(k);
                        log.println("isAccessibleChildSelected - " +
                                    isSelected);
                        res &= isSelected;
                    } else {
                        log.println("Child isn't selectable");
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("isAccessibleChildSelected()", res);
    }

    /**
     * Calls the method.
     * Has OK status if the method <code>isAccessibleChildSelected()</code>
     * returned <code>false</code>.
     */
    public void _clearAccessibleSelection() {
        executeMethod("isAccessibleChildSelected()");

        boolean res = true;

        log.println("clearAccessibleSelection");
        oObj.clearAccessibleSelection();


        // clearAccessibleSelection() call is oneway so we need
        // some waiting
        shortWait();

        if ((childCount > 0) && !OneAlwaysSelected) {
            try {
                log.print("isAccessibleChildSelected(" + (childCount - 1) +
                          ")? ");

                boolean isSel = oObj.isAccessibleChildSelected(childCount - 1);
                log.println(isSel);
                res = !isSel;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res = false;
            }
        } else if (OneAlwaysSelected) {
            log.println("Can't clear selection, one child is always selected");
        }

        tRes.tested("clearAccessibleSelection()", res);
    }

    /**
     * Calls the method.
     * Has OK status if the method <code>isAccessibleChildSelected()</code>
     * returns <code>true</code> for first and for last accessible child
     * or if multiselection is not allowed.
     */
    public void _selectAllAccessibleChildren() {
        executeMethod("clearAccessibleSelection()");

        log.println("selectAllAccessibleChildren...");
        oObj.selectAllAccessibleChildren();


        // selectAllAccessibleChildren() call is oneway so we need
        // some waiting
        shortWait();

        boolean res = true;
        boolean isSelected = true;

        int SelectableChildCount = chkSelectable(tEnv.getTestObject());

        if ((SelectableChildCount > 0) && multiSelection) {
            try {
                log.print("isAccessibleChildSelected(1)? ");
                isSelected = oObj.isAccessibleChildSelected(1);
                log.println(isSelected);
                res = isSelected;

                log.print("isAccessibleChildSelected(" + (childCount - 1) +
                          ")? ");
                isSelected = oObj.isAccessibleChildSelected(childCount - 1);
                log.println(isSelected);
                res &= isSelected;
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("selectAllAccessibleChildren()", res);
    }

    /**
     * Calls the method. Clears accessible selection and calls the method again.
     * <p>
     * Has OK status if the method returns number equal to number of accessible
     * child count after first call if multiselection allowed, or
     * 1 returned if multiselection not allowed.
     * And if the method returns a zero after clearing selection.
     */
    public void _getSelectedAccessibleChildCount() {
        log.println("getSelectedAccessibleChildCount():");

        if (multiSelection) {
            oObj.selectAllAccessibleChildren();
        } else {
            int usedChildren = childCount;

            if (childCount > 500) {
                log.println("Restricting to 500");
                usedChildren = 500;
            }

            if (usedChildren > 0) {
                try {
                    for (int i = 0; i < usedChildren; i++) {

                        if (isSelectable(tEnv.getTestObject(), i)) {
                            log.print("Trying to select child with index "+i+": ");
                            oObj.selectAccessibleChild(i);
                            long curtime = System.currentTimeMillis();
                            long checktime = System.currentTimeMillis();

                            while (!oObj.isAccessibleChildSelected(i) && (checktime-curtime<5000)) {
                                checktime = System.currentTimeMillis();
                            }

                            log.println("OK");
                        }
                    }
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.println("Unexpected exception");
                    e.printStackTrace(log);
                }
            }
        }

        int sCount = chkSelectable(tEnv.getTestObject());
        log.println("Found " + sCount + " selectable Children");

        int selectedCount = oObj.getSelectedAccessibleChildCount();
        log.println("After selecting all accessible " + selectedCount +
                    " are selected");

        boolean res = true;

        if (multiSelection) {
            res &= (selectedCount == sCount);
        } else {
            res &= (selectedCount == 1);
        }

        log.println("clearAccessibleSelection...");
        oObj.clearAccessibleSelection();
        log.print("getSelectedAccessibleChildCount: ");
        selectedCount = oObj.getSelectedAccessibleChildCount();
        log.println(selectedCount);

        if (OneAlwaysSelected) {
            res &= (selectedCount == 1);
        } else {
            res &= (selectedCount == 0);
        }

        tRes.tested("getSelectedAccessibleChildCount()", res);
    }

    /**
     * Calls the method with wrong and correct indexes.
     * Has OK status if exception was thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if the method have returned a not null for the correct index.
     */
    public void _getSelectedAccessibleChild() {
        executeMethod("getSelectedAccessibleChildCount()");

        boolean res = true;
        int selectedCount = oObj.getSelectedAccessibleChildCount();
        log.println("getSelectedAccessibleChildCount: " + selectedCount);

        try {
            log.println("getSelectedAccessibleChild(-1)");
            oObj.getSelectedAccessibleChild(-1);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res = true;
        }

        try {
            log.println("getSelectedAccessibleChild(" + selectedCount + ")");
            oObj.getSelectedAccessibleChild(selectedCount);
            log.println("Exception was expected");
            res &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        int SelectableChildCount = chkSelectable(tEnv.getTestObject());

        if (SelectableChildCount > 500) {
            SelectableChildCount = 500;
        }

        if (SelectableChildCount > 0) {
            int k = 0;
            try {
                for (k = 0; k < SelectableChildCount; k++) {
                    log.println("Trying to select child with index " + k);

                    if (isSelectable(tEnv.getTestObject(), k)) {
                        oObj.selectAccessibleChild(k);
                        shortWait();
                        log.println("selected child count: " +
                                    oObj.getSelectedAccessibleChildCount());
                        XAccessible selChild = oObj.getSelectedAccessibleChild(0);
                        res &= (selChild != null);
                        log.println("valid child - " + (selChild != null));
                    } else {
                        log.println("Child isn't selectable");
                    }
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception: Last relevant calls:\n " +
                            "\toObj.selectAccessibleChild("+k+")\n" +
                            "\toObj.getSelectedAccessibleChild(0)");
                e.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("getSelectedAccessibleChild()", res);
    }

    /**
     * Calls the method with wrong and with correct indexes.
     * Has OK status if exceptions were thrown for the calls with
     * the wrong indexes, if exception wasn't thrown for the call
     * with correct index and if number of selected child was
     * decreased after the correct call.
     */
    public void _deselectAccessibleChild() {
        executeMethod("getSelectedAccessibleChild()");

        boolean res = true;
        int selCount = oObj.getSelectedAccessibleChildCount();
        log.println("getSelectedAccessibleChildCount():" + selCount);

        try {
            log.println("deselectAccessibleChild(-1)");
            oObj.deselectAccessibleChild(-1);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        try {
            log.println("deselectAccessibleChild(" + (childCount + 1) + ")");
            oObj.deselectAccessibleChild(childCount + 1);
            log.println("Exception was expected");
            res = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Expected exception");
            res &= true;
        }

        log.println("#################");
        log.println("Selecting all accessible");
        oObj.selectAllAccessibleChildren();
        selCount = oObj.getSelectedAccessibleChildCount();
        log.println("getSelectedAccessibleChildCount():" + selCount);

        if ((childCount > 0) && (selCount > 0)) {
            try {
                int maxCount = chkSelectable(tEnv.getTestObject());

                if (childCount > 100) {
                    maxCount = 100;
                }

                for (int k = 0; k < maxCount; k++) {
                    log.println("deselectAccessibleChild(" + k + ")");

                    if (oObj.isAccessibleChildSelected(k)) {
                        oObj.deselectAccessibleChild(k);
                    }
                }

                int newSelCount = oObj.getSelectedAccessibleChildCount();
                log.println("getSelectedAccessibleChildCount():" +
                            newSelCount);

                if (OneAlwaysSelected && (selCount == 1)) {
                    log.println("One Child is always selected");
                    res &= true;
                } else {
                    res &= (selCount > newSelCount);
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res = false;
            }
        }

        tRes.tested("deselectAccessibleChild()", res);
    }

    protected static int chkSelectable(Object Testcase) {
        int ret = 0;
        XAccessibleContext accCon = UnoRuntime.queryInterface(
                                            XAccessibleContext.class, Testcase);
        int cc = accCon.getAccessibleChildCount();

        if (cc > 500) {
            return cc;
        }

        for (int i = 0; i < cc; i++) {
            try {
                if (accCon.getAccessibleChild(i).getAccessibleContext()
                          .getAccessibleStateSet()
                          .contains(com.sun.star.accessibility.AccessibleStateType.SELECTABLE)) {
                    ret = ret + 1;
                    System.out.println("Child " + i + " is selectable");
                }
            } catch (com.sun.star.lang.IndexOutOfBoundsException iab) {
            }
        }

        return ret;
    }

    protected static boolean isSelectable(Object Testcase, int index) {
        XAccessibleContext accCon = UnoRuntime.queryInterface(
                                            XAccessibleContext.class, Testcase);
        boolean res = false;

        try {
            if (accCon.getAccessibleChild(index).getAccessibleContext()
                      .getAccessibleStateSet()
                      .contains(com.sun.star.accessibility.AccessibleStateType.SELECTABLE)) {
                res = true;
            }

            //selecting menuitems or the separator will lead to closing the menu
            if ((accCon.getAccessibleChild(index).getAccessibleContext()
                       .getAccessibleRole() == AccessibleRole.MENU_ITEM) ||
                    (accCon.getAccessibleChild(index).getAccessibleContext()
                           .getAccessibleRole() == AccessibleRole.SEPARATOR)) {
                res = false;
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Exception while checking for selectability");
        }

        return res;
    }

    private void shortWait() {
        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {
        }
    }
}