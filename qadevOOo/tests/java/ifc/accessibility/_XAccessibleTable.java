/*************************************************************************
 *
 *  $RCSfile: _XAccessibleTable.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 10:06:04 $
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

import lib.MultiMethodTest;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleTable;
import com.sun.star.uno.UnoRuntime;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleTable</code>
 * interface methods :
 * <ul>
 *  <li><code>getAccessibleRowCount()</code></li>
 *  <li><code>getAccessibleColumnCount()</code></li>
 *  <li><code>getAccessibleRowDescription()</code></li>
 *  <li><code>getAccessibleColumnDescription()</code></li>
 *  <li><code>getAccessibleRowExtentAt()</code></li>
 *  <li><code>getAccessibleColumnExtentAt()</code></li>
 *  <li><code>getAccessibleRowHeaders()</code></li>
 *  <li><code>getAccessibleColumnHeaders()</code></li>
 *  <li><code>getSelectedAccessibleRows()</code></li>
 *  <li><code>getSelectedAccessibleColumns()</code></li>
 *  <li><code>isAccessibleRowSelected()</code></li>
 *  <li><code>isAccessibleColumnSelected()</code></li>
 *  <li><code>getAccessibleCellAt()</code></li>
 *  <li><code>getAccessibleCaption()</code></li>
 *  <li><code>getAccessibleSummary()</code></li>
 *  <li><code>isAccessibleSelected()</code></li>
 *  <li><code>getAccessibleIndex()</code></li>
 *  <li><code>getAccessibleRow()</code></li>
 *  <li><code>getAccessibleColumn()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleTable
 */
public class _XAccessibleTable extends MultiMethodTest {

    public XAccessibleTable oObj = null;
    XAccessibleSelection xASel = null;
    XAccessibleContext xACont = null;

    protected void before() {
        xASel = (XAccessibleSelection)
            UnoRuntime.queryInterface(XAccessibleSelection.class, oObj);
        if (xASel == null) {
            log.println("The component doesn't implement the interface " +
                "XAccessibleSelection.");
            log.println("This interface is required for more detailed tests.");
        }

        xACont = (XAccessibleContext)
            UnoRuntime.queryInterface(XAccessibleContext.class, oObj);
    }

    int rowCount = 0;

    /**
     * Calls the method and stores the returned value to the variable
     * <code>rowCount</code>.
     */
    public void _getAccessibleRowCount() {
        rowCount = oObj.getAccessibleRowCount();
        log.println("Accessible row count: " + rowCount);
        tRes.tested("getAccessibleRowCount()", true);
    }

    int colCount = 0;

    /**
     * Calls the method and stores the returned value to the variable
     * <code>colCount</code>.
     */
    public void _getAccessibleColumnCount() {
        colCount = oObj.getAccessibleColumnCount();
        log.println("Accessible column count: " + colCount);
        tRes.tested("getAccessibleColumnCount()", true);
    }

    /**
     * Calls the method with the wrong indexes and with the correct index,
     * checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value isn't <code>null</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleRowDescription() {
        requiredMethod("getAccessibleRowCount()");
        boolean res = true;

        try {
            log.print("getAccessibleRowDescription(-1): ");
            String descr = oObj.getAccessibleRowDescription(-1);
            log.println("'" + descr + "'");
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowDescription(" + rowCount + "): ");
            String descr = oObj.getAccessibleRowDescription(rowCount);
            log.println("'" + descr + "'");
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowDescription(" + (rowCount - 1) + "): ");
            String descr =
                oObj.getAccessibleRowDescription(rowCount - 1);
            res &= descr != null;
            log.println("'" + descr + "'");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleRowDescription()", res);
    }

    /**
     * Calls the method with the wrong indexes and with the correct index,
     * checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value isn't <code>null</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleColumnDescription() {
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;

        try {
            log.print("getAccessibleColumnDescription(-1): ");
            String descr = oObj.getAccessibleColumnDescription(-1);
            log.println("'" + descr + "'");
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnDescription(" + colCount + "): ");
            String descr = oObj.getAccessibleColumnDescription(colCount);
            log.println("'" + descr + "'");
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnDescription(" + (colCount - 1) + "): ");
            String descr =
                oObj.getAccessibleColumnDescription(colCount - 1);
            res &= descr != null;
            log.println("'" + descr + "'");
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleColumnDescription()", res);
    }


     /**
     * Calls the method with the wrong parameters and with the correct
     * parameters, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value is greater than or is equal to 1.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleRowExtentAt() {
        requiredMethod("getAccessibleRowCount()");
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;

        try {
            log.print("getAccessibleRowExtentAt(-1," + (colCount-1) + "):");
            int ext = oObj.getAccessibleRowExtentAt(-1, colCount - 1);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowExtentAt(" + (rowCount-1) + ",-1):");
            int ext = oObj.getAccessibleRowExtentAt(rowCount - 1, -1);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowExtentAt(0," + colCount + "):");
            int ext = oObj.getAccessibleRowExtentAt(0, colCount);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowExtentAt(" + rowCount + ",0):");
            int ext = oObj.getAccessibleRowExtentAt(rowCount, 0);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRowExtentAt(" +
                (rowCount-1) + "," + (colCount-1) + "):");
            int ext = oObj.getAccessibleRowExtentAt(rowCount-1, colCount - 1);
            log.println(ext);
            res &= ext >= 1;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleRowExtentAt()", res);
    }

    /**
     * Calls the method with the wrong parameters and with the correct
     * parameters, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value is greater than or is equal to 1.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleColumnExtentAt() {
        requiredMethod("getAccessibleRowCount()");
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;

        try {
            log.print("getAccessibleColumnExtentAt(-1," + (colCount-1) + "):");
            int ext = oObj.getAccessibleColumnExtentAt(-1, colCount - 1);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnExtentAt(" + (rowCount-1) + ",-1):");
            int ext = oObj.getAccessibleColumnExtentAt(rowCount - 1, -1);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnExtentAt(0," + colCount + "):");
            int ext = oObj.getAccessibleColumnExtentAt(0, colCount);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnExtentAt(" + rowCount + ",0):");
            int ext = oObj.getAccessibleColumnExtentAt(rowCount, 0);
            log.println(ext);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumnExtentAt(" +
                (rowCount-1) + "," + (colCount-1) + "):");
            int ext = oObj.getAccessibleColumnExtentAt(rowCount-1,colCount - 1);
            log.println(ext);
            res &= ext >= 1;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleColumnExtentAt()", res);
    }

    /**
     * Calls the method and checks a returned value.
     * Has OK status if returned value isn't <code>null</code>.
     */
    public void _getAccessibleRowHeaders() {
        XAccessibleTable rowHeaders = oObj.getAccessibleRowHeaders();
        log.println("getAccessibleRowHeaders(): " + rowHeaders);
        tRes.tested("getAccessibleRowHeaders()", true);
    }

    /**
     * Calls the method and checks a returned value.
     * Has OK status if returned value isn't <code>null</code>.
     */
    public void _getAccessibleColumnHeaders() {
        XAccessibleTable colHeaders = oObj.getAccessibleColumnHeaders();
        log.println("getAccessibleColumnHeaders(): " + colHeaders);
        tRes.tested("getAccessibleColumnHeaders()", true);
    }

    /**
     * If the interface <code>XAccessibleSelection</code> is supported by
     * the component than selects all accessible childs.
     * Calls the method and checks a returned sequence.
     * Has OK status if a returned sequince is in ascending order.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getSelectedAccessibleRows() {
        requiredMethod("getAccessibleRowCount()");
        boolean res = true;
        boolean locRes = true;
        int selRows[] = null;

        if (xASel != null) {
            log.println("XAccessibleSelection.selectAllAccessibleChildren()");
            xASel.selectAllAccessibleChildren();
        }

        log.println("getSelectedAccessibleRows()");
        selRows = oObj.getSelectedAccessibleRows();
        log.println("Length of the returned sequince: " + selRows.length);
        if (xASel != null) {
            res &= selRows.length == rowCount;
        } else {
            res &= selRows.length == 0;
        }

        if (selRows.length > 0) {
            log.println("Checking that returned sequence is" +
                " in ascending order");
        }

        for(int i = 1; i < selRows.length; i++) {
            locRes &= selRows[i] >= selRows[i - 1];
            res &= locRes;
            if (!locRes) {
                log.println("Element #" + i + ":" + selRows[i] +
                    " is less than element #" + (i-1) + ": " +
                    selRows[i-1]);
                break;
            }
        }

        tRes.tested("getSelectedAccessibleRows()", res);
    }

    /**
     * If the interface <code>XAccessibleSelection</code> is supported by
     * the component than selects all accessible childs.
     * Calls the method and checks a returned sequence.
     * Has OK status if a returned sequince is in ascending order.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     * </ul>
     */
    public void _getSelectedAccessibleColumns() {
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;
        boolean locRes = true;
        int selCols[] = null;

        if (xASel != null) {
            log.println("XAccessibleSelection.selectAllAccessibleChildren()");
            xASel.selectAllAccessibleChildren();
        }

        log.println("getSelectedAccessibleColumns()");
        selCols = oObj.getSelectedAccessibleColumns();
        log.println("Length of the returned sequince: " + selCols.length);

        if (xASel != null) {
            res &= selCols.length == colCount;
        } else {
            res &= selCols.length == 0;
        }

        if (selCols.length > 0) {
            log.println("Checking that returned sequence is" +
                " in ascending order");
        }

        for(int i = 1; i < selCols.length; i++) {
            locRes &= selCols[i] >= selCols[i - 1];
            res &= locRes;
            if (!locRes) {
                log.println("Element #" + i + ":" + selCols[i] +
                    " is less than element #" + (i-1) + ": " +
                    selCols[i-1]);
                break;
            }
        }

        tRes.tested("getSelectedAccessibleColumns()", res);
    }

    /**
     * Calls the method with invalid indexes.
     * If the interface <code>XAccessibleSelection</code> is supported by
     * the component than selects all accessible childs.
     * Calls the method for every row and checks returned values.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _isAccessibleRowSelected() {
        requiredMethod("getAccessibleRowCount()");
        boolean res = true;
        boolean locRes = true;

        try {
            log.print("isAccessibleRowSelected(-1): ");
            locRes = oObj.isAccessibleRowSelected(-1);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("isAccessibleRowSelected(" + rowCount + "): ");
            locRes = oObj.isAccessibleRowSelected(rowCount);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        if (xASel != null) {
            log.println("XAccessibleSelection.selectAllAccessibleChildren()");
            xASel.selectAllAccessibleChildren();
        }

        try {
            log.println("Checking of every row selection...");
            for(int i = 0; i < rowCount; i++) {
                boolean isSel = oObj.isAccessibleRowSelected(i);
                locRes = (xASel == null) ? !isSel : isSel;
                res &= locRes;
                if (!locRes) {
                    log.println("isAccessibleRowSelected(" + i + "): " + isSel);
                    break;
                }
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("isAccessibleRowSelected()", res);
    }

    /**
     * Calls the method with invalid indexes.
     * If the interface <code>XAccessibleSelection</code> is supported by
     * the component than selects all accessible childs.
     * Calls the method for every column and checks returned values.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _isAccessibleColumnSelected() {
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;
        boolean locRes = true;

        try {
            log.print("isAccessibleColumnSelected(-1): ");
            locRes = oObj.isAccessibleColumnSelected(-1);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("isAccessibleColumnSelected(" + colCount + "): ");
            locRes = oObj.isAccessibleColumnSelected(colCount);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        if (xASel != null) {
            log.println("XAccessibleSelection.selectAllAccessibleChildren()");
            xASel.selectAllAccessibleChildren();
        }

        try {
            log.println("Checking of every column selection...");
            for(int i = 0; i < colCount; i++) {
                boolean isSel = oObj.isAccessibleColumnSelected(i);
                locRes = (xASel == null) ? !isSel : isSel;
                res &= locRes;
                if (!locRes) {
                    log.println("isAccessibleColumnSelected(" + i + "): " + isSel);
                    break;
                }
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("isAccessibleColumnSelected()", res);
    }

    XAccessible xCellAc = null;

    /**
     * Calls the method with the wrong parameters and with the correct
     * parameter, checks a returned value and stores it to the variable
     * <code>xCellAc</code>.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value isn't null.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleCellAt() {
        requiredMethod("getAccessibleRowCount()");
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;

        try {
            log.print("getAccessibleCellAt(-1," + (colCount-1) + "):");
            xCellAc = oObj.getAccessibleCellAt(-1, colCount - 1);
            log.println(xCellAc);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleCellAt(" + (rowCount-1) + ",-1):");
            xCellAc = oObj.getAccessibleCellAt(rowCount - 1, -1);
            log.println(xCellAc);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleCellAt(0, " + colCount + "):");
            xCellAc = oObj.getAccessibleCellAt(0, colCount);
            log.println(xCellAc);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleCellAt(" + rowCount + ",0):");
            XAccessible xCellAc = oObj.getAccessibleCellAt(rowCount, 0);
            log.println(xCellAc);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleCellAt(" + (rowCount-1) + "," +
                (colCount-1) + "): ");
            xCellAc = oObj.getAccessibleCellAt(
                rowCount - 1, colCount - 1);
            log.println(xCellAc);
            res &= xCellAc != null;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleCellAt()", res);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleCaption() {
        XAccessible caption = oObj.getAccessibleCaption();
        log.println("getAccessibleCaption(): " + caption);
        tRes.tested("getAccessibleCaption()", true);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleSummary() {
        XAccessible summary = oObj.getAccessibleSummary();
        log.println("getAccessibleSummary(): " + summary);
        tRes.tested("getAccessibleSummary()", true);
    }

    /**
     * Calls the method with the wrong parameters and with the correct
     * parameter, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _isAccessibleSelected() {
        requiredMethod("getAccessibleRowCount()");
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;
        boolean locRes = true;

        try {
            log.print("isAccessibleSelected(-1," + (colCount-1) + "):");
            locRes = oObj.isAccessibleSelected(-1, colCount - 1);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("isAccessibleSelected(" + (rowCount-1) + ",-1):");
            locRes = oObj.isAccessibleSelected(rowCount - 1, -1);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("isAccessibleSelected(0, " + colCount + "):");
            locRes = oObj.isAccessibleSelected(0, colCount);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("isAccessibleSelected(" + rowCount + ",0):");
            locRes = oObj.isAccessibleSelected(rowCount, 0);
            log.println(locRes);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        if (xASel != null) {
            log.println("XAccessibleSelection.selectAllAccessibleChildren()");
            xASel.selectAllAccessibleChildren();
        }

        try {
            log.print("isAccessibleSelected(" + (rowCount-1) + "," +
                (colCount-1) + "): ");
            boolean isSel = oObj.isAccessibleSelected(
                rowCount - 1, colCount - 1);
            log.println(isSel);
            locRes = (xASel == null) ? !isSel : isSel ;
            res &= locRes;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("isAccessibleSelected()", res);
    }

    /**
     * Calls the method with the wrong parameters and with the correct
     * parameter, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value is equal to value returned by calling
     * <code>XAccessibleContext::getAccessibleIndexInParent</code> for the cell.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleCellAt()</code> </li>
     * </ul>
     */
    public void _getAccessibleIndex() {
        executeMethod("getAccessibleCellAt()");
        boolean res = true;

        try {
            log.print("getAccessibleIndex(-1," + (colCount-1) + "):");
            int indx = oObj.getAccessibleIndex(-1, colCount - 1);
            log.println(indx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleIndex(" + (rowCount-1) + ",-1):");
            int indx = oObj.getAccessibleIndex(rowCount - 1, -1);
            log.println(indx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleIndex(0," + colCount + "):");
            int indx = oObj.getAccessibleIndex(0, colCount);
            log.println(indx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleIndex(" + rowCount + ",0):");
            int indx = oObj.getAccessibleIndex(rowCount, 0);
            log.println(indx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleIndex(" + (rowCount-1) + "," +
                (colCount-1) + "): ");
            int indx = oObj.getAccessibleIndex(
                rowCount - 1, colCount - 1);
            log.println(indx);
            if (xCellAc != null) {
                XAccessibleContext xAC = xCellAc.getAccessibleContext();
                int expIndx = xAC.getAccessibleIndexInParent();
                log.println("Expected index: " + expIndx);
                res &= expIndx == indx;
            } else {
                res &= true;
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleIndex()", res);
    }

    /**
     * Receives an accessible child count using the interface
     * <code>XAccessibleContext</code>.
     * Calls the method with the wrong parameters and with the correct
     * parameter, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value is greater than zero and is less than
     * accessible row count.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleRowCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleRow() {
        requiredMethod("getAccessibleRowCount()");
        boolean res = true;

        if (xACont != null) {
            int childCount = xACont.getAccessibleChildCount();
            log.println("accessible child count: " + childCount);

            try {
                log.print("getAccessibleRow(" + childCount + "): ");
                int rowIndx = oObj.getAccessibleRow(childCount);
                log.println(rowIndx);
                log.println("Exception was expected");
                res &= false;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("expected exception");
                res &= true;
            }

            try {
                log.print("getAccessibleRow(" + (childCount-1) + "): ");
                int rowIndx = oObj.getAccessibleRow(childCount - 1);
                log.println(rowIndx);
                res &= (rowIndx >= 0 && rowIndx <= rowCount);
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res &= false;
            }
        }

        try {
            log.print("getAccessibleRow(-1): ");
            int rowIndx = oObj.getAccessibleRow(-1);
            log.println(rowIndx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleRow(0): ");
            int rowIndx = oObj.getAccessibleRow(0);
            log.println(rowIndx);
            res &= (rowIndx >= 0 && rowIndx <= rowCount);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleRow()", res);
    }

    /**
     * Receives an accessible child count using the interface
     * <code>XAccessibleContext</code>.
     * Calls the method with the wrong parameters and with the correct
     * parameter, checks a returned value.
     * Has OK status if exceptions were thrown for the wrong indexes,
     * if exception wasn't thrown for the correct index and
     * if returned value is greater than zero and is less than
     * accessible column count.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>getAccessibleColumnCount()</code> </li>
     * </ul>
     */
    public void _getAccessibleColumn() {
        requiredMethod("getAccessibleColumnCount()");
        boolean res = true;

        if (xACont != null) {
            int childCount = xACont.getAccessibleChildCount();
            log.println("accessible child count: " + childCount);

            try {
                log.print("getAccessibleColumn(" + childCount + "): ");
                int colIndx = oObj.getAccessibleColumn(childCount);
                log.println(colIndx);
                log.println("Exception was expected");
                res &= false;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("expected exception");
                res &= true;
            }

            try {
                log.print("getAccessibleColumn(" + (childCount-1) + "): ");
                int colIndx = oObj.getAccessibleColumn(childCount - 1);
                log.println(colIndx);
                res &= (colIndx >= 0 && colIndx <= colCount);
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected exception");
                e.printStackTrace(log);
                res &= false;
            }
        }

        try {
            log.print("getAccessibleColumn(-1): ");
            int colIndx = oObj.getAccessibleColumn(-1);
            log.println(colIndx);
            log.println("Exception was expected");
            res &= false;
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("expected exception");
            res &= true;
        }

        try {
            log.print("getAccessibleColumn(0): ");
            int colIndx = oObj.getAccessibleColumn(0);
            log.println(colIndx);
            res &= (colIndx >= 0 && colIndx <= rowCount);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
            res &= false;
        }

        tRes.tested("getAccessibleColumn()", res);
    }
}