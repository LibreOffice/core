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

    @Override
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

        oObj.getCount();
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

