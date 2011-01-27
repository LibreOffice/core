/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.table;

import com.sun.star.table.XCellRange;
import lib.MultiMethodTest;

import com.sun.star.table.XTableRows;
import lib.Status;
import lib.StatusException;

/**
* Testing <code>com.sun.star.table.XTableRows</code>
* interface methods :
* <ul>
*  <li><code> insertByIndex()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul>
*/
public class _XTableRows extends MultiMethodTest {

    public XTableRows oObj = null;
    public XCellRange range = null;

    public void before() {
        range = (XCellRange) tEnv.getObjRelation("XTableRows.XCellRange");
        if (range==null) {
            throw new StatusException(Status.failed("ObjectRelation missing"));
        }
        try {
            range.getCellByPosition(0,0).setValue(17);
            range.getCellByPosition(0,1).setValue(15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set value for Cell A1");
        }
    }

    /**
     * First a row inserted to valid position, then to invalid. <p>
     * Has <b> OK </b> status if in the first case number of rows increases
     * by 1, and in the second an exception is thrown. <p>
     */
    public void _insertByIndex() {

        boolean result = true;

        requiredMethod("removeByIndex()");

        int origCnt = oObj.getCount();
        log.println("Inserting row before first row");
        oObj.insertByIndex(0,1);
        result &= checkCell(1,15);
        if (checkCell(1,15)) log.println("... successful");

        try {
            oObj.insertByIndex(-1,1);
            log.println("No Exception occurred while inserting row at -1");
            result &= false;
        } catch (Exception e) {
            log.println("Inserting row at Index -1 ... OK");
            result &= true;
        }

        tRes.tested( "insertByIndex()", result );

    } // end insertByIndex()

    /**
     * First a row removed from valid position, then from invalid. <p>
     *
     * Has <b> OK </b> status if in the first case number of columns decreases
     * by 1, and in the second an exception is thrown. <p>
     */
    public void _removeByIndex() {

        boolean result = true;

        oObj.removeByIndex(0,1);
        log.println("Removing first row");
        result &= checkCell(0,15);
        if (checkCell(0,15)) log.println("... successful");

        try {
            oObj.removeByIndex(-1,1);
            log.println("No Exception occurred while Removing row at -1");
            result &= false;
        } catch (Exception e) {
            log.println("Removing row at Index -1 ... OK");
            result &= true;
        }

        tRes.tested( "removeByIndex()", result );
    } // end removeByIndex()

    public boolean checkCell(int row,double expected) {
        double getting=0;
        try {
            getting = range.getCellByPosition(0,row).getValue();
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set value for Cell A1");
        }

        boolean res = (getting==expected);
        if (!res) {
            log.println("Expected for row "+row+" was "+expected);
            log.println("Getting for row "+row+" - "+getting);
            log.println("=> FAILED");
        }
        return res;
    }

} //finish class _XTableRows

