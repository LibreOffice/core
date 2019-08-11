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

import lib.MultiMethodTest;

import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;

/**
* Testing <code>com.sun.star.table.XCellRange</code>
* interface methods :
* <ul>
*  <li><code> getCellByPosition()</code></li>
*  <li><code> getCellRangeByPosition()</code></li>
*  <li><code> getCellRangeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ValidRange'</code> (of type <code>String</code>):
*   cell range that can be defined by the object test instead of
*   definition at this test ("<code>A1:A1</code>")</li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.table.XCellRange
*/
public class _XCellRange extends MultiMethodTest {
    public XCellRange oObj = null;

    /**
     * First a cell get from valid position, second - from invalid. <p>
     * Has <b> OK </b> status if in the first case not null value is
     * returned and no exceptions are thrown, and in the second
     * case <code>IndexOutOfBoundsException</code> is thrown. <p>
     */
    public void _getCellByPosition() {

        boolean result = false;

        try {
            XCell cell = oObj.getCellByPosition(0,0);
            result = cell != null ;
            log.println("Getting cell by position with a valid position ... OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while getting cell by position with a valid position");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getCellByPosition(-1,1);
            log.println("No Exception occurred while getting cell by position with invalid position");
            result &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Getting cell by position with an invalid position ... OK");
            result &= true;
        }

        tRes.tested( "getCellByPosition()", result );

    } // end getCellByPosition()

    /**
     * A range is tried to obtain with valid name. <p>
     * Has <b> OK </b> status if not null range is
     * returned. <p>
     */
    public void _getCellRangeByName() {

        boolean result = false;

        String valid = (String) tEnv.getObjRelation("ValidRange");
        if (valid == null ) valid = "A1:A1";
        XCellRange range = oObj.getCellRangeByName(valid);
        result = range != null ;
        log.println("Getting cellrange by name with a valid name ... OK");

        tRes.tested( "getCellRangeByName()", result );


    } // end getCellRangeByName()

    /**
     * First a range is tried to obtain with valid bounds,
     * second - with invalid. <p>
     * Has <b> OK </b> status if in the first case not null range is
     * returned and no exceptions are thrown, and in the second
     * case <code>IndexOutOfBoundsException</code> is thrown. <p>
     */
    public void _getCellRangeByPosition() {

        boolean result = false;

        try {
            XCellRange range = oObj.getCellRangeByPosition(0,0,0,0);
            result = range != null;
            log.println("Getting cellrange by Position with a valid position ... OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while getting cellrange by position with a valid position");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getCellRangeByPosition(-1,0,-1,1);
            log.println("No Exception occurred while getting cellrange by position with invalid position");
            result &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Getting cellrange by position with an invalid position ... OK");
            result &= true;
        }

        tRes.tested( "getCellRangeByPosition()", result );


    } // end getCellRangeByPosition()

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

} // finish class _XCellRange

