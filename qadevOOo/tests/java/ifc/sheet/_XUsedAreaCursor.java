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

import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSheetCellCursor;
import com.sun.star.sheet.XUsedAreaCursor;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XUsedAreaCursor</code>
* interface methods :
* <ul>
*  <li><code> gotoStartOfUsedArea()</code></li>
*  <li><code> gotoEndOfUsedArea()</code></li>
* </ul> <p>
* Component must also implement the following interfaces :
* <ul>
*  <li> <code> com.sun.star.XCellRangeAddressable </code> : to check the current
*  position of the cursor </li>
* <ul> <p>
* @see com.sun.star.sheet.XUsedAreaCursor
*/
public class _XUsedAreaCursor extends MultiMethodTest {

    public XUsedAreaCursor oObj = null;
    public XSheetCellCursor oC = null;
    CellRangeAddress sAddr = null;

    /**
    * Test points the cursor to the start of used area, expands cursor to the
    * end of the used area, gets and checks current range address, then
    * points the cursor to the end of the used area, gets and checks current
    * range address again. <p>
    * Has <b> OK </b> status if the range address expands at all used area
    * in first case and if the range address just points to the cell at the end
    * of the used area in second case. <p>
    */
    public void _gotoEndOfUsedArea() {
        boolean result = true ;

        XCellRangeAddressable oAddr = (XCellRangeAddressable)
                UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj) ;

        // first with true argument
        oObj.gotoStartOfUsedArea(false);
        oObj.gotoEndOfUsedArea(true);
        sAddr = oAddr.getRangeAddress();

        result &= (sAddr.StartColumn == 1);
        result &= (sAddr.StartRow == 1);
        result &= (sAddr.EndColumn == 4);
        result &= (sAddr.EndRow == 5);

        oObj.gotoEndOfUsedArea(false);
        sAddr = oAddr.getRangeAddress();

        result &= (sAddr.StartColumn == 4);
        result &= (sAddr.StartRow == 5);
        result &= (sAddr.EndColumn == 4);
        result &= (sAddr.EndRow == 5);

        tRes.tested("gotoEndOfUsedArea()", result) ;
    }

    /**
    * Test points the cursor to the end of used area, expands cursor to the
    * start of the used area, gets and checks current range address, then
    * points the cursor to the start of the used area, gets and checks current
    * range address again. <p>
    * Has <b> OK </b> status if the range address expands at all used area
    * in first case and if the range address just points to the cell at the
    * start of the used area in second case. <p>
    */
    public void _gotoStartOfUsedArea() {
        XCellRangeAddressable oAddr = (XCellRangeAddressable)
                UnoRuntime.queryInterface (XCellRangeAddressable.class, oObj) ;

        boolean result = true ;

        // with true parameter first
        oObj.gotoEndOfUsedArea(false);
        oObj.gotoStartOfUsedArea(true);
        sAddr = oAddr.getRangeAddress();

        result &= (sAddr.StartColumn == 1);
        result &= (sAddr.StartRow == 1);
        result &= (sAddr.EndColumn == 4);
        result &= (sAddr.EndRow == 5);

        // now testing with false parameter
        oObj.gotoStartOfUsedArea(false);
        sAddr = oAddr.getRangeAddress();

        result &= (sAddr.StartColumn == 1);
        result &= (sAddr.StartRow == 1);
        result &= (sAddr.EndColumn == 1);
        result &= (sAddr.EndRow == 1);

        tRes.tested("gotoStartOfUsedArea()", result) ;
    } // finished gotoStartOfUsedArea

    /**
    * Forces object environment recreation.
    */
    protected void after() {
        this.disposeEnvironment();
    }
} // finished class _XUsedAreaCursor

