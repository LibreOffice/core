/*************************************************************************
 *
 *  $RCSfile: _XAccessibleSelection.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change:$Date: 2003-11-18 16:19:52 $
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
     * @see ifc.accessibility.XAccessibleContext
     */
    protected void before() {
        xAC = (XAccessibleContext) UnoRuntime.queryInterface(
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

        int usedChilds = childCount;

        if (childCount > 500) {
            log.println("Restricting to 500");
            usedChilds = 500;
        }

        if (usedChilds > 0) {
            try {
                for (int i = 0; i < usedChilds; i++) {
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
            int usedChilds = childCount;

            if (childCount > 500) {
                log.println("Restricting to 500");
                usedChilds = 500;
            }

            if (usedChilds > 0) {
                try {
                    for (int i = 0; i < usedChilds; i++) {

                        if (isSelectable(tEnv.getTestObject(), i)) {
                            log.print("Trying to select child with index "+i+": ");
                            oObj.selectAccessibleChild(i);
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
        log.println("Found " + sCount + " selectable Childs");

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
        XAccessibleContext accCon = (XAccessibleContext) UnoRuntime.queryInterface(
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
        XAccessibleContext accCon = (XAccessibleContext) UnoRuntime.queryInterface(
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