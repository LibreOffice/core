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

import com.sun.star.container.XEnumeration;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetCellRanges</code>
* interface methods :
* <ul>
*  <li><code> getCells()</code></li>
*  <li><code> getRangeAddressesAsString()</code></li>
*  <li><code> getRangeAddresses()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetCellRanges
*/
public class _XSheetCellRanges extends MultiMethodTest{

    public XSheetCellRanges oObj = null;

    /**
    * Test calls the method, creates enumeration of returned value
    * and checks that the enumeration has elements. <p>
    * Has <b> OK </b> status if gained enumeration has elements. <p>
    */
    public void _getCells() {
        log.println("Testing getCells ...");

        XEnumeration oEnum = oObj.getCells().createEnumeration();
        boolean res = oEnum.hasMoreElements();
        if (!res) {
            log.println(
                    "The Enumeration gained via getCells() has no Elements");
        }
        tRes.tested("getCells()", res);
    }

    /**
    * Test calls the method and checks length of returned array. <p>
    * Has <b> OK </b> status if length of returned array is greater than 2.<p>
    */
    public void _getRangeAddresses() {
        log.println("Testing getRangeAddresses ...");
        CellRangeAddress[] oRanges = oObj.getRangeAddresses();
        int howmuch = oRanges.length;
        tRes.tested("getRangeAddresses()", (howmuch > 2) );
    }

    /**
    * Test calls the method and checks returned string. <p>
    * Has <b> OK </b> status if returned string starts from 'Sheet'.<p>
    */
    public void _getRangeAddressesAsString() {
        log.println("Testing getRangeAddressesAsString ...");
        String oRanges = oObj.getRangeAddressesAsString();
        tRes.tested("getRangeAddressesAsString()",oRanges.indexOf("C1:D4")>0);
    }

} // finished class _XSheetCellRanges

