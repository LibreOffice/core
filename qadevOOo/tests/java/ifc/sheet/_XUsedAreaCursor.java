/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

