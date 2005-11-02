/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUsedAreaCursor.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:49:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

