/*************************************************************************
 *
 *  $RCSfile: _XSheetCellCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:03:39 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.CellFlags;
import com.sun.star.sheet.XArrayFormulaRange;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSheetCellCursor;
import com.sun.star.sheet.XSheetOperation;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XMergeable;

/**
* Testing <code>com.sun.star.sheet.XSheetCellCursor</code>
* interface methods :
* <ul>
*  <li><code> collapseToCurrentRegion()</code></li>
*  <li><code> collapseToCurrentArray()</code></li>
*  <li><code> collapseToMergedArea()</code></li>
*  <li><code> expandToEntireColumns()</code></li>
*  <li><code> expandToEntireRows()</code></li>
*  <li><code> collapseToSize()</code></li>
* </ul> <p>
* Component must also implement the following interfaces :
* <ul>
*  <li> <code> com.sun.star.sheet.XCellRangeAddressable </code> :
*  to get range address </li>
* <ul> <p>
* Range of cursor must be of size 4 x 4. <p>
* @see com.sun.star.sheet.XSheetCellCursor
*/
public class _XSheetCellCursor extends MultiMethodTest {

    public XSheetCellCursor oObj = null;

    /**
    * Test creates the array formula, assigns this array to another array,
    * collapses cursor into one cell, applies method, checks the size of the
    * result range, erases array formula, checks that array formula has been
    * cleared. <p>
    * Has <b>OK</b> status if no exceptions were thrown, if size of the result
    * range is equal to size of the range where the array formula was set and
    * if array formula was successfully cleared. <p>
    */
    public void _collapseToCurrentArray() {
        boolean bResult = false;

        XCellRangeAddressable crAddr = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        CellRangeAddress addr = crAddr.getRangeAddress() ;
        int leftCol = addr.StartColumn ;
        int topRow = addr.StartRow ;
        int width = addr.EndColumn - addr.StartColumn + 1 ;
        int height = addr.EndRow - addr.StartRow + 1 ;

        log.println( "Object area is ((" + leftCol + "," + topRow + "),(" +
            (leftCol + width - 1) + "," + (topRow + height - 1) + ")" );

        XCellRange new_range = null;
        try {
            // first we need to create an array formula
            new_range =
                oObj.getCellRangeByPosition(0, 0, 0, height - 1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.print("Get cell range by position failed: ");
            e.printStackTrace(log);
            tRes.tested("collapseToCurrentArray()", false);
        }

        log.println("DB: Successfully new range created");
        XArrayFormulaRange arrFormulaRange = (XArrayFormulaRange)
            UnoRuntime.queryInterface (XArrayFormulaRange.class, new_range);
        // write a simple formula (this array assigns another array)
        arrFormulaRange.setArrayFormula("A1:A" + height) ;

        // collapse cursor into one cell and then try to apply the method
        oObj.collapseToSize (1, 1) ;
        oObj.collapseToCurrentArray() ;

        // check the size of result range
        int cols = ( (XColumnRowRange)UnoRuntime.queryInterface(
                  XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ( (XColumnRowRange)UnoRuntime.queryInterface(
                  XColumnRowRange.class, oObj) ).getRows().getCount();

        if (cols == 1 && rows == height) {
            bResult = true;
        } else {
            bResult = false;
            log.println("The size of cell range must be 1x" + height +
                ", but after method call it was " + cols + "x" + rows);
        }

        // erase array formula
        arrFormulaRange.setArrayFormula("");

        // check if array formula has been cleared with last statement
        try {
            // if array formula isn't cleared exception is thrown
            new_range.getCellByPosition(0,0).setValue(111) ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            bResult = false ;
            log.println(
                "Array formula hasn't been cleared with setArrayFormula(\"\")");
            XSheetOperation clearRange = (XSheetOperation)
                UnoRuntime.queryInterface (XSheetOperation.class, new_range);
            int allFlags =
                CellFlags.ANNOTATION | CellFlags.DATETIME | CellFlags.EDITATTR;
            allFlags = allFlags
                | CellFlags.HARDATTR | CellFlags.OBJECTS | CellFlags.STRING;
            allFlags = allFlags
                | CellFlags.VALUE | CellFlags.FORMULA | CellFlags.STYLES;
            clearRange.clearContents(allFlags) ;
        }

        tRes.tested("collapseToCurrentArray()", bResult );
    }

    /**
    * Test clears contents of spreadsheet, collapses cursor to current range,
    * checks size of cursor range, fills a cell that is close to
    * cursor range, collapses cursor to current range, checks size of cursor
    * range again and restores original size. <p>
    * Has <b> OK </b> status if after clearing of content and collapsing cursor
    * range size remains 4 x 4, if after filling of cell and collapsing cursor
    * range extends by one in both dimensions and no exceptions were thrown.<p>
    */
    public void _collapseToCurrentRegion(){
        boolean bResult = true;
        int width = 4, height = 4;
        int leftCol = -1, topRow = -1;

        XSpreadsheet oSheet = oObj.getSpreadsheet();
        ((XSheetOperation) UnoRuntime.queryInterface(
            XSheetOperation.class, oSheet) ).clearContents(65535);
        oObj.collapseToCurrentRegion();
        int cols = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getRows().getCount();

        if (cols != width || rows != height) {
            bResult = false ;
            log.println("After collapseToCurrentRegion()"
                 + " call Region must have size " + width + "x" + height
                 + " but it is " + cols + "x" + rows);
        }

        // if previous test was successful try more complicated case
        if (bResult) {
            if (leftCol != -1 && topRow != -1) {
                try {
                    oSheet.getCellByPosition(
                        leftCol + width, topRow + height).setValue(1);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    log.print("Can't get cell by position:");
                    e.printStackTrace(log);
                    bResult = false;
                }

                oObj.collapseToCurrentRegion() ;

                // checking results
                cols = ((XColumnRowRange)
                    UnoRuntime.queryInterface(
                        XColumnRowRange.class, oObj)).getColumns().getCount();
                rows = ((XColumnRowRange)
                    UnoRuntime.queryInterface(
                        XColumnRowRange.class, oObj)).getRows().getCount();

                if (cols == width + 1 && rows == height + 1) {
                    bResult &= true;
                } else {
                    bResult = false;
                    log.println("After collapseToCurrentRegion() call [2]"
                         + " region must have size " + (width+1) + "x"
                         + (height + 1) + " but it is " + cols + "x" + rows );
                }
            }
        }

        tRes.tested("collapseToCurrentRegion()", bResult);

        // restore original size
        oObj.collapseToSize(width, height);
    }

    /**
    * Test merges a cells of range that has a greater size, collapses cursor to
    * merged area, checks size of cursor range and restores original size
    * of cursor range. <p>
    * Has <b> OK </b> status if after merging of cells and collapsing cursor
    * range extends by one in both dimensions and no exceptions were thrown.<p>
    */
    public void _collapseToMergedArea(){
        int width = 1, height = 1 ;
        int leftCol = 0, topRow = 0 ;

        boolean bResult = true ;

        log.println("DB: Starting collapseToMergedArea() method test ...") ;
        XSpreadsheet oSheet = oObj.getSpreadsheet() ;
        log.println ("DB: got Spreadsheet.") ;

        XCellRange newRange = null;
        try {
            newRange = oSheet.getCellRangeByPosition (
                leftCol + width - 1, topRow + height - 1,
                leftCol + width, topRow + height );
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Can't get cell range by position");
            e.printStackTrace(log);
            bResult = false;
        }

        XMergeable mergeRange = (XMergeable)
            UnoRuntime.queryInterface (XMergeable.class, newRange);
        if (mergeRange == null) {
            log.println("DB: newRange doesn't implement XMergeable interface");
        } else {
            log.println("DB: XMergeable interface successfully queried.");
        }

        mergeRange.merge(true);
        log.println("DB: Successfuly merged.") ;

        oObj.collapseToMergedArea() ;
        log.println("DB: Succesfully collapseToMergedArea() method called");

        // unmerge area to restore SpreadSheet
        mergeRange.merge(false);
        log.println("DB: Successfully unmerged.") ;

        // checking results
        int cols = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getRows().getCount();
        log.println("DB: Column and row numbers succesfully get") ;

        if (cols == width + 1  && rows == height + 3) {
            bResult &= true;
        } else {
            bResult = false;
            log.println(
                "After collapseToMergedArea() call region must have size "
                + (width + 1) + "x" + (height + 1) + " but it is " + cols
                + "x" + rows );
        }

        tRes.tested("collapseToMergedArea()", bResult) ;

        // restore original size
        oObj.collapseToSize(width, height);
    }

    /**
    * Test collapses cursor to the new size, checks size
    * of cursor range and restores original size of cursor range. <p>
    * Has <b> OK </b> status if after collapsing cursor
    * range extends by three in both dimensions. <p>
    */
    public void _collapseToSize(){
        boolean bResult = false;
        int width = 1, height = 1;

        // collapseToSize() method test
        oObj.collapseToSize (width + 3, height + 3);

        // checking results
        int cols = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getRows().getCount();

        if (cols == width + 3  && rows == height + 3) {
            bResult = true ;
        } else {
            bResult = false ;
            log.println( "After collapseToSize() call region must have size "
                + (width + 3) + "x" + (height + 3) + " but it is "
                + cols + "x" +rows);
        }

        tRes.tested("collapseToSize()", bResult) ;

        // restore original size
        oObj.collapseToSize(width, height) ;
    }

    /**
    * Test expands cursor to entire columns, checks size
    * of cursor range and restores original size of cursor range. <p>
    * Has <b> OK </b> status if after expanding cursor
    * range extends to all rows in the columns (number of rows is greater than
    * 32000 and number of columns remains the same). <p>
    */
    public void _expandToEntireColumns(){
        boolean bResult = false;
        int width = 1, height = 1 ;

        // expandToEntireColumns() method test
        oObj.expandToEntireColumns () ;

        // checking results
        int cols = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getRows().getCount();

        if (cols == width && rows >= 32000) {
            bResult = true ;
        } else {
            bResult = false ;
            log.println(
                "After expandToEntireColumns() call region must have size "+
                width + "x(>=32000) but it is " + cols + "x" + rows);
        }

        tRes.tested("expandToEntireColumns()", bResult) ;

        // restore original size
        oObj.collapseToSize(width, height) ;
    }

    /**
    * Test expands cursor to entire rows, checks size
    * of cursor range and restores original size of cursor range. <p>
    * Has <b> OK </b> status if after expanding cursor
    * range extends to all columns in the rows (number of columns is greater
    * than 256 and number of rows remains the same). <p>
    */
    public void _expandToEntireRows(){
        boolean bResult = false;
        int width = 1, height = 1 ;

        // expandToEntireRows() method test
        oObj.expandToEntireRows () ;

        // checking results
        int cols = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getColumns().getCount();
        int rows = ((XColumnRowRange)
            UnoRuntime.queryInterface(
                XColumnRowRange.class, oObj) ).getRows().getCount();

        if (cols >= 256 && rows == height) {
            bResult = true;
        } else {
            bResult = false ;
            log.println("After expandToEntireRows() call region " +
                "must have size (>=256)x" + height + " but it is " +
                cols + "x" + rows );
        }

        tRes.tested("expandToEntireRows()", bResult) ;

        // restore original size
        oObj.collapseToSize(width, height) ;
    }

} // EOC _XSheetCellCursor

