/*************************************************************************
 *
 *  $RCSfile: _XCellCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:10:16 $
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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSheetCellCursor;
import com.sun.star.sheet.XSheetCellRange;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellCursor;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.table.XCellCursor</code>
* interface methods :
* <ul>
*  <li><code> gotoStart()</code></li>
*  <li><code> gotoEnd()</code></li>
*  <li><code> gotoNext()</code></li>
*  <li><code> gotoPrevious()</code></li>
*  <li><code> gotoOffset()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SHEET'</code> (of type <code>XSpreadsheet</code>):
*   is used for creating a new cell range.</li>
* <ul> <p>
*
* The component tested <b>must implement</b>
* <code>XCellRangeAddressable</code> interface. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.table.XCellCursor
*/
public class _XCellCursor extends MultiMethodTest {

    public static XCellCursor oObj = null;
    public static XSpreadsheet oSheet = null;

    /**
     * <code>XCellRangeAddressable</code> interface is queried
     * first for getting current position of cursor. The cursor
     * is moved to next cell. Address of cursor obtained before
     * and after moving. <p>
     * Has <b> OK </b> status if cursor column is changed after
     * movement. <p>
     */
    public void _gotoNext(){
        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;
        int startCol2, endCol2, startRow2, endRow2 = 0;

        XCellRangeAddressable oRange = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;

        oObj.gotoNext();

        oAddr = oRange.getRangeAddress();
        startRow2 = oAddr.StartRow;
        startCol2 = oAddr.StartColumn;
        endRow2 = oAddr.EndRow;
        endCol2 = oAddr.EndColumn;

        if (!(startCol == startCol2)){
            bResult = true;
        }
        tRes.tested( "gotoNext()", bResult );
    }

    /**
     * <code>XCellRangeAddressable</code> interface is queried
     * first for getting current position of cursor. The cursor
     * is moved then. Address of cursor obtained before
     * and after moving. <p>
     * Has <b> OK </b> status if starting column and row of
     * cursor is changed after movement. <p>
     */
    public void _gotoOffset(){
        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;
        int startCol2, endCol2, startRow2, endRow2 = 0;

        XCellRangeAddressable oRange = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;

        oObj.gotoOffset(4,4);

        oAddr = oRange.getRangeAddress();
        startRow2 = oAddr.StartRow;
        startCol2 = oAddr.StartColumn;
        endRow2 = oAddr.EndRow;
        endCol2 = oAddr.EndColumn;
        if (!(startCol == startCol2) || (startRow == startRow2)){
            bResult = true;
        }
        tRes.tested( "gotoOffset()", bResult );
    }

    /**
     * <code>XCellRangeAddressable</code> interface is queried
     * first for getting current position of cursor. The cursor
     * is moved to previous cell. Address of cursor obtained before
     * and after moving. <p>
     * Has <b> OK </b> status if cursor column is changed after
     * movement. <p>
     */
    public void _gotoPrevious(){
        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;
        int startCol2, endCol2, startRow2, endRow2 = 0;

        XCellRangeAddressable oRange = (XCellRangeAddressable)
                  UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;

        oObj.gotoPrevious();

        oAddr = oRange.getRangeAddress();
        startRow2 = oAddr.StartRow;
        startCol2 = oAddr.StartColumn;
        endRow2 = oAddr.EndRow;
        endCol2 = oAddr.EndColumn;

        if (!(startCol == startCol2)){
            bResult = true;
        }
        tRes.tested( "gotoPrevious()", bResult );
    }

    /**
     * <code>XCellRangeAddressable</code> interface is queried
     * first for getting current position of cursor. The cursor
     * is moved to the start of its range .
     * Address of cursor obtained before and after moving. <p>
     * Has <b> OK </b> status if cursor was collapsed to a single
     * cell (i.e. start column is the same as end column) after
     * movement. <p>
     */
    public void _gotoStart(){
        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;

        XCellRangeAddressable oRange = (XCellRangeAddressable)
                UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        oObj.gotoStart();
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;
        if ((startCol == endCol) && (endRow == startRow)){
            bResult = true;
        }

        tRes.tested( "gotoStart()", bResult );
    }

    /**
     * A new cell range is created using spreadsheet passed by relation.
     * The method is tested on that range. <code>gotoEnd</code> is
     * called and range address is checked.<p>
     * Has <b> OK </b> status if cursor was collapsed to a single
     * cell (i.e. start column is the same as end column) after
     * movement. <p>
     */
    public void _gotoEnd(){
        //gotoEnd gets it's own cursor to see a change
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");
        XCellRange testRange = oSheet.getCellRangeByName("$A$1:$g$7") ;
        XSheetCellRange testSheetRange = (XSheetCellRange)
                    UnoRuntime.queryInterface(XSheetCellRange.class,testRange);
        XSheetCellCursor oCellCursor = oSheet.createCursorByRange
            (testSheetRange);
        XCellCursor oCursor = (XCellCursor)
            UnoRuntime.queryInterface(XCellCursor.class,oCellCursor);

        boolean bResult = false;
        int startCol, endCol, startRow, endRow = 0;

        XCellRangeAddressable oRange = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, oCursor);
        oCursor.gotoEnd();
        CellRangeAddress oAddr = oRange.getRangeAddress();
        startRow = oAddr.StartRow;
        startCol = oAddr.StartColumn;
        endRow = oAddr.EndRow;
        endCol = oAddr.EndColumn;
        if ((startCol == endCol) && (endRow == startRow)){
            bResult = true;
        }

        tRes.tested( "gotoEnd()", bResult );
    }

    /**
    * Forces object environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }


} //EOC _XCellCursor

