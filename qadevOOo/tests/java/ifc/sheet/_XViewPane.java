/*************************************************************************
 *
 *  $RCSfile: _XViewPane.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:07:34 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XViewPane;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XViewPane</code>
* interface methods :
* <ul>
*  <li><code> getFirstVisibleColumn()</code></li>
*  <li><code> setFirstVisibleColumn()</code></li>
*  <li><code> getFirstVisibleRow()</code></li>
*  <li><code> setFirstVisibleRow()</code></li>
*  <li><code> getVisibleRange()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewPane
*/
public class _XViewPane extends MultiMethodTest {

    public XViewPane oObj = null;
    int row = 3;
    int col = 5;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to value that was set
    * by method <code>setFirstVisibleColumn()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleColumn() </code> : to set specific value
    *  of the first column that is visible in the pane </li>
    * </ul>
    */
    public void _getFirstVisibleColumn() {
        requiredMethod("setFirstVisibleColumn()");
        boolean result = col == oObj.getFirstVisibleColumn();
        tRes.tested("getFirstVisibleColumn()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to value that was set
    * by method <code>setFirstVisibleRow()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleRow() </code> : to set specific value of
    *  the first row that is visible in the pane </li>
    * </ul>
    */
    public void _getFirstVisibleRow() {
        requiredMethod("setFirstVisibleRow()");
        boolean result = row == oObj.getFirstVisibleRow();
        tRes.tested("getFirstVisibleRow()", result);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setFirstVisibleColumn() {
        oObj.setFirstVisibleColumn(col);
        tRes.tested("setFirstVisibleColumn()", true);
    }

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setFirstVisibleRow() {
        oObj.setFirstVisibleRow(row);
        tRes.tested("setFirstVisibleRow()", true);
    }

    /**
    * Test calls the method, checks returned value and adds object relation
    * 'DATAAREA' to test environment. <p>
    * Has <b> OK </b> status if returned value isn't null and if start row and
    * start column are equal to values that was set by methods
    * <code>setFirstVisibleRow</code> and <code>setFirstVisibleColumn</code>.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFirstVisibleRow() </code> : to set specific value of
    *  the first row that is visible in the pane </li>
    *  <li> <code> setFirstVisibleColumn() </code> : to set specific value of
    *  the first column that is visible in the pane </li>
    * </ul>
    */
    public void _getVisibleRange() {
        requiredMethod("setFirstVisibleRow()");
        requiredMethod("setFirstVisibleColumn()");

        CellRangeAddress RA = oObj.getVisibleRange();
        boolean result = RA != null;
        if (result) {
            result &= RA.Sheet == 0;
            result &= RA.StartRow == row;
            result &= RA.StartColumn == col;
            tEnv.addObjRelation("DATAAREA", RA);
        }

        tRes.tested("getVisibleRange()", result);
    }
}

