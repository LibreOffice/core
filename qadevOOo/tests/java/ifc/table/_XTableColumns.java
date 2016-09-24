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
import lib.Status;
import lib.StatusException;

import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.table.XTableColumns</code>
* interface methods :
* <ul>
*  <li><code> insertByIndex()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
*
* This test needs the following object relations :
* <ul>
*  <li> <code>'XTableColumns.XCellRange'</code> : <code>
*    com.sun.star.table.XCellRange</code> the cell range of
*    columns.</li>
* <ul> <p>
*
* Test is multithread compliant. <p>
* @see com.sun.star.table.XTableColumns
*/
public class _XTableColumns extends MultiMethodTest {

    public XTableColumns oObj = null;
    private XCellRange xCellRange = null;
    private int lastColumn = 0;

    @Override
    public void before() {
        xCellRange = (XCellRange)
            tEnv.getObjRelation("XTableColumns.XCellRange") ;

        if (xCellRange == null) throw new
            StatusException(Status.failed("Relation missing"));

        lastColumn = oObj.getCount() - 1 ;
    }

    /**
     * First a number of cells in cell range are filled with data.
     *
     * Then columns inserted to valid positions : 1 column at 1,
     * 1 column at 0, 2 columns at 0. <p>
     *
     * Then columns inserted to invalid positions : position -1,
     * the column after last, and 0 columns inserted. <p>
     *
     * Has <b> OK </b> status if for valid cases :
     * <ul>
     *  <li> content of other cells are properly shifted </li>
     *  <li> inserted columns are empty </li>
     *  <li> number of columns increases (in case if it is not the whole
     *      spreadsheet) by proper number. </li>
     * </ul>
     * and for invalid cases exception is thrown.
     */
    public void _insertByIndex() {

        boolean result = true;
        int origCnt = oObj.getCount();

        try {
            log.println("Filling range ... ");
            fillRange(xCellRange);

            log.println("Inserting 1 column at position 1 ...");
            oObj.insertByIndex(1,1);

            result &= checkColumn(0, 0);
            result &= checkColumnEmpty(1);
            result &= checkColumn(2, 1);
            result &= checkColumn(3, 2);
            result &= checkColumnEmpty(4);

            if (lastColumn < 200) {
                result &= checkColumn(lastColumn + 1, lastColumn);
                result &= oObj.getCount() == origCnt + 1;
            } else {
                result &= checkColumnEmpty(lastColumn);
            }

            log.println("Inserting 1 column at position 0 ...");
            oObj.insertByIndex(0,1);

            result &= checkColumnEmpty(0);
            result &= checkColumn(1, 0);
            result &= checkColumnEmpty(2);
            result &= checkColumn(3, 1);
            result &= checkColumn(4, 2);
            result &= checkColumnEmpty(5);
            if (lastColumn < 200) {
                result &= checkColumn(lastColumn + 2, lastColumn);
                result &= oObj.getCount() == origCnt + 2;
            }

            log.println("Inserting 2 columns at position 0 ...");
            oObj.insertByIndex(0,2);

            result &= checkColumnEmpty(0);
            result &= checkColumnEmpty(1);
            result &= checkColumnEmpty(2);
            result &= checkColumn(3, 0);
            result &= checkColumnEmpty(4);
            result &= checkColumn(5, 1);
            result &= checkColumn(6, 2);
            result &= checkColumnEmpty(7);
            if (lastColumn < 200) {
                result &= checkColumn(lastColumn + 4, lastColumn);
            }

        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            result = false;
        }


        // spreadsheet supports 256 columns and after inserting
        // or removing a column their number remains the same
        if (origCnt < 200) {
            log.println("Checking that number of column increased.");
            result &= oObj.getCount() == origCnt + 4;
            log.println("Before: " + origCnt + ", After: " + oObj.getCount());
        } else {
            log.println("Number of columns is " + origCnt + ",") ;
            log.println("supposing that this is the whole spreadsheet and ");
            log.println("number of columns should not change.");
        }

        try {
            oObj.insertByIndex(-1,1);
            log.println("No Exception occurred while inserting column at -1");
            result &= false;
        } catch (Exception e) {
            log.println("Inserting column at Index -1 ... OK");
            result &= true;
        }

        int cnt = oObj.getCount();
        try {
            oObj.insertByIndex(cnt, 1);
            log.println("No Exception occurred while inserting column at "
                + cnt);
            result &= false;
        } catch (Exception e) {
            log.println("Inserting column at Index " + cnt + " ... OK");
            result &= true;
        }

        if (tEnv.getTestCase().getObjectName().equals("ScTableColumnsObj")) {

            try {
                oObj.insertByIndex(0,0);
                log.println("No Exception occurred while inserting 0 columns");
                result &= false;
            } catch (Exception e) {
                log.println("Inserting 0 columns ... OK");
                result &= true;
            }

        }

        tRes.tested( "insertByIndex()", result );

    } // end insertByIndex()

    /**
     * Columns removed from valid positions : 1 column at 1,
     * 1 column at 0, 2 columns at 0. <p>
     *
     * Then columns removed from invalid positions : position -1,
     * the column after last, and 0 columns removed. <p>
     *
     * Has <b> OK </b> status if for valid cases :
     * <ul>
     *  <li> content of other cells are properly shifted </li>
     *  <li> columns which are shifted left are empty </li>
     *  <li> number of columns decreases (in case if it is not the whole
     *      spreadsheet) by proper number. </li>
     * </ul>
     * and for invalid cases exception is thrown.
     */
    public void _removeByIndex() {
        executeMethod("insertByIndex()");

        boolean result = true;
        int origCnt = oObj.getCount();

        try {
            log.println("Filling range ... ");

            log.println("Removing 2 columns at position 0 ...");
            oObj.removeByIndex(0,2);

            result &= checkColumnEmpty(0);
            result &= checkColumn(1, 0);
            result &= checkColumnEmpty(2);
            result &= checkColumn(3, 1);
            result &= checkColumn(4, 2);
            result &= checkColumnEmpty(5);
            if (lastColumn < 200) {
                result &= checkColumn(lastColumn + 2, lastColumn);
                result &= oObj.getCount() == origCnt - 2;
            }

            log.println("Removing 1 column at position 0 ...");
            oObj.removeByIndex(0,1);

            result &= checkColumn(0, 0);
            result &= checkColumnEmpty(1);
            result &= checkColumn(2, 1);
            result &= checkColumn(3, 2);
            result &= checkColumnEmpty(4);
            if (lastColumn < 200) {
                result &= checkColumn(lastColumn + 1, lastColumn);
                result &= oObj.getCount() == origCnt - 3;
            }

            log.println("Removing 1 column at position 1 ...");
            oObj.removeByIndex(1,1);

            result &= checkColumn(0, 0);
            result &= checkColumn(1, 1);
            result &= checkColumn(2, 2);
            result &= checkColumnEmpty(3);
            if (lastColumn < 200) {
                result &= checkColumn(lastColumn, lastColumn);
            }

        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            result = false;
        }


        // spreadsheet supports 256 columns and after inserting
        // or removing a column their number remains the same
        if (origCnt < 200) {
            log.println("Checking that number of column increased.");
            result &= oObj.getCount() == origCnt - 4;
            log.println("Before: " + origCnt + ", After: " + oObj.getCount());
        } else {
            log.println("Number of columns is " + origCnt + ",") ;
            log.println("supposing that this is the whole spreadsheet and ");
            log.println("number of columns should not change.");
        }

        try {
            oObj.removeByIndex(-1,1);
            log.println("No Exception occurred while removing column at -1");
            result &= false;
        } catch (Exception e) {
            log.println("removing column at Index -1 ... OK");
            result &= true;
        }

        int cnt = oObj.getCount();
        try {
            oObj.removeByIndex(cnt, 1);
            log.println("No Exception occurred while removing column at "
                + cnt);
            result &= false;
        } catch (Exception e) {
            log.println("Removing column at Index " + cnt + " ... OK");
            result &= true;
        }

        if (tEnv.getTestCase().getObjectName().equals("ScTableColumnsObj")) {
            try {
                oObj.removeByIndex(0,0);
                log.println("No Exception occurred while removing 0 columns");
                result &= false;
            } catch (Exception e) {
                log.println("removing 0 columns ... OK");
                result &= true;
            }
        }

        tRes.tested( "removeByIndex()", result );
    } // end removeByIndex()

    private void setCellText(XCell cell, String text) {
        XSimpleText xText = UnoRuntime.queryInterface
            (XSimpleText.class, cell) ;
        xText.setString(text);
    }
    private String getCellText(XCell cell) {
        XSimpleText xText = UnoRuntime.queryInterface
            (XSimpleText.class, cell) ;
        return xText.getString();
    }

    /**
     * Fills the range with some data : two rows and 3 columns, and
     * some columns are cleared.
     *
     * @param xRange Range to fill
     * @throws IndexOutOfBoundsException if any errors occur during filling.
     */
    private void fillRange(XCellRange xRange)
            throws com.sun.star.lang.IndexOutOfBoundsException {

        for (int i = 0; i <= lastColumn && i < 3; i++) {
            setCellText(xRange.getCellByPosition(i, 0), i + "a");
            setCellText(xRange.getCellByPosition(i, 1), i + "b");
        }

        for (int i = 3; i <= lastColumn && i < 10; i++) {
            setCellText(xRange.getCellByPosition(i, 0), "");
            setCellText(xRange.getCellByPosition(i, 1), "");
        }
    }

    /**
     * Check the column (first two rows) if it has values with
     * index specified.
     *
     * @param col Column to check
     * @param idx What indexes must be in cells
     * @return <code>true</code> if expected indexes are found,
     *  <code>false</code> otherwise.
     * @throws IndexOutOfBoundsException
     */
    private boolean checkColumn(int col, int idx)
            throws com.sun.star.lang.IndexOutOfBoundsException {

        if (col >= oObj.getCount()) return true;

        String c1 = getCellText(xCellRange.getCellByPosition(col, 0));
        String c2 = getCellText(xCellRange.getCellByPosition(col, 1));

        if (!((idx + "a").equals(c1) && (idx + "b").equals(c2))) {

            log.println("FAILED for column " + col + " and index " + idx + "("
                + c1 + "," + c2 + ")");
            return false ;
        }
        return true;
    }

    /**
     * Checks if the column (first two rows) has no data in its cells.
     *
     * @param col Column to check
     * @return <code>true</code> if the column is empty, <code>false</code>
     *   if first two cells contains some strings.
     * @throws IndexOutOfBoundsException
     */
    private boolean checkColumnEmpty(int col)
            throws com.sun.star.lang.IndexOutOfBoundsException {

        if (col >= oObj.getCount()) return true;

        String c1 = getCellText(xCellRange.getCellByPosition(col, 0));
        String c2 = getCellText(xCellRange.getCellByPosition(col, 1));
        if (!("".equals(c1) && "".equals(c2))) {
            log.println("FAILED for column " + col + " is not empty ("
                + c1 + "," + c2 + ")");
            return false ;
        }
        return true;
    }

    } //finish class _XTableColumns

