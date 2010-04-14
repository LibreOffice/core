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

