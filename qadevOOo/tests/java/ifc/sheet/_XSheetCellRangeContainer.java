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

package ifc.sheet;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.sheet.XSheetCellRangeContainer;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetCellRangeContainer</code>
* interface methods :
* <ul>
*  <li><code> addRangeAddress() </code></li>
*  <li><code> removeRangeAddress() </code></li>
*  <li><code> addRangeAddresses() </code></li>
*  <li><code> removeRangeAddresses() </code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.sheet.XSheetCellRangeContainer
*/
public class _XSheetCellRangeContainer extends MultiMethodTest {
    public XSheetCellRangeContainer oObj = null;
    public CellRangeAddress[] rAddr = new CellRangeAddress[3];

    /**
    * After method called, the new array of structures 'CellRangeAddress'
    *  is created. Then container is cleared.
    */
    @Override
    public void before() {
        for ( short i=0; i<=2; i++ ) {
            rAddr[i] = new CellRangeAddress();
            rAddr[i].Sheet = i;
            rAddr[i].StartColumn = i;
            rAddr[i].StartRow = i;
            rAddr[i].EndColumn = i + 3;
            rAddr[i].EndRow = i + 3;
            try {
                oObj.removeRangeAddresses(oObj.getRangeAddresses());
            } catch (com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
                throw new StatusException("Error: Cannot remove "+
                    "range addresses." ,e);
            }
        }
    }

    /**
    * The method called. Then new value is added to Container.
    * Next we try to obtain back added value and check it. <p>
    *
    * Has <b> OK </b> status if the range just added presents among
    * all ranges in the container.
    */
    public void _addRangeAddress() {
        boolean result = true;

        log.println("Elements before adding: " + oObj.getCount());
        oObj.addRangeAddress(rAddr[0], false);
        log.println("Elements after adding: " + oObj.getCount());
        CellRangeAddress[] addr = oObj.getRangeAddresses();
        boolean exist = false ;
        for (int i=0; i<=oObj.getCount()-1; i++) {
            if ( addr[i].Sheet == rAddr[0].Sheet &&
                 addr[i].StartColumn == rAddr[0].StartColumn &&
                 addr[i].StartRow == rAddr[0].StartRow &&
                 addr[i].EndColumn == rAddr[0].EndColumn &&
                 addr[i].EndRow == rAddr[0].EndRow) {

                exist = true;
            }
        }

        result &= exist ;

        tRes.tested("addRangeAddress()" ,result);
    }

    /**
    * The method called. Then a value added before is removed.
    * Next we check Container for existence of removed value. <p>
    * Has <b> OK </b> status if the range just removed doesn't presents among
    * all ranges in the container.
    */
    public void _removeRangeAddress() {
        boolean result = true;

        log.println("Elements before removing: " + oObj.getCount());
        try {
            oObj.removeRangeAddress(rAddr[0]);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            result = false;
        }
        log.println("Elements after removing: " + oObj.getCount());
        CellRangeAddress[] addr = oObj.getRangeAddresses();
        for (int i=0; i<=oObj.getCount()-1; i++) {
            if ( (addr[i].Sheet == rAddr[0].Sheet) &&
                    (addr[i].StartColumn == rAddr[0].StartColumn) &&
                    (addr[i].StartRow == rAddr[0].StartRow) &&
                    (addr[i].EndColumn == rAddr[0].EndColumn) &&
                    (addr[i].EndRow == rAddr[0].EndRow) ) {
                result = false;
            }
        }
        tRes.tested("removeRangeAddress()" ,result);
    }

    /**
     * The method called. Then new values are added to Container.
     * Next we try to obtain back all added values and check it. <p>
     *
     * Has <b> OK </b> status if the count of ranges increases by
     * number of added ranges - 1 (one of ranges already exists in the
     * container). And if all of ranges added exist in the container.
     */
    public void _addRangeAddresses() {
        executeMethod("addRangeAddress()");

        boolean result = true;

        int cntBefore = oObj.getCount();
        log.println("Elements before adding: " + cntBefore);
        oObj.addRangeAddresses(rAddr, false);
        log.println("Elements after adding: " + oObj.getCount());
        CellRangeAddress[] addr = oObj.getRangeAddresses();

        result &= cntBefore + rAddr.length == oObj.getCount();

        for (int j = 0; j < rAddr.length; j++) {
            boolean exist = false ;
            for (int i=0; i < oObj.getCount(); i++) {
                if ( addr[i].Sheet == rAddr[j].Sheet &&
                     addr[i].StartColumn == rAddr[j].StartColumn &&
                     addr[i].StartRow == rAddr[j].StartRow &&
                     addr[i].EndColumn == rAddr[j].EndColumn &&
                     addr[i].EndRow == rAddr[j].EndRow ) {

                    exist = true;
                    break;
                }
            }
            result &= exist;
        }

        tRes.tested("addRangeAddresses()" ,result);
    }

    /**
     * All ranges are remover from container.
     *
     * Has <b> OK </b> status if there are no more ranges in the container.
     */
    public void _removeRangeAddresses() {
        boolean result = false;
        int cnt;

        log.println("Elements before removing: " + oObj.getCount());
        try {
            oObj.removeRangeAddresses(oObj.getRangeAddresses());
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            result = false;
        }
        if ( (cnt = oObj.getCount()) == 0) {
            result = true;
        }
        log.println("Elements after removing: " + cnt);
        tRes.tested("removeRangeAddresses()" ,result);
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

}
