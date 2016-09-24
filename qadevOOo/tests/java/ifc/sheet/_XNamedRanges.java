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

import java.util.Random;
import java.util.StringTokenizer;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.Border;
import com.sun.star.sheet.NamedRangeFlag;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XCellRangeReferrer;
import com.sun.star.sheet.XNamedRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XNamedRanges</code>
* interface methods :
* <ul>
*  <li><code> addNewByName()</code></li>
*  <li><code> addNewFromTitles()</code></li>
*  <li><code> removeByName()</code></li>
*  <li><code> outputList()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SHEET'</code> (of type <code>XSpreadsheet</code>):
*   to have a spreadsheet </li>
* <ul> <p>
* @see com.sun.star.sheet.XNamedRanges
* @see com.sun.star.sheet.XSpreadsheet
*/
public class _XNamedRanges extends MultiMethodTest {

    public XNamedRanges oObj = null;
    String name = "_XNamedRanges";
    XSpreadsheet oSheet = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    protected void before() {
        oSheet = (XSpreadsheet)tEnv.getObjRelation("SHEET");
        if (oSheet == null) throw new StatusException(Status.failed
            ("Relation 'SHEET' not found"));
    }

    /**
    * Test creates and stores random content and random type, calls the method
    * and checks that new range exists in collection using method
    * <code>hasByName()</code>. <p>
    * Has <b> OK </b> status if new range exists in collection
    * and no exceptions were thrown. <p>
    */
    public void _addNewByName() {
        boolean bResult = true;
        CellAddress aPosition = new CellAddress((short)0, 2, 2);
        int nType = getRandomType();
        String sContent = getRandomContent("D3;A6:B9;=F12");
        name += sContent;
        log.println("Adding new range with name=\"" + name +
                    "\", sContent = \"" + sContent +
                    "\", aPosition = (" + aPosition.Sheet + ", "
                                        + aPosition.Column + ", "
                                        + aPosition.Row +
                    "), Type = " + nType + ".");

        oObj.addNewByName(name, sContent, aPosition, nType);

        //inserted for a bug
        CellAddress listOutputPosition = new CellAddress((short)0, 1, 1);
        oObj.outputList(listOutputPosition);
        String s = null;
        String s1 = null;
        try {
            s = oSheet.getCellByPosition(1, 1).getFormula();
            s1 = oSheet.getCellByPosition(2, 1).getFormula();
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Can't get cell by position: " + e);
            bResult = false;
        }
        log.println("Outputlist returns: " + s + " " + s1);
        //end of insertion

        bResult &= oObj.hasByName(name);

        tRes.tested("addNewByName()", bResult);
    }

    /**
    * Test creates a table with left and top titles, creates new ranges from
    * top titles and left titles, checks all new title ranges. <p>
    * Has <b> OK </b> status if all required title ranges are present
    * in collection, if each of them have valid size and position and
    * no exceptions were thrown. <p>
    */
    public void _addNewFromTitles() {
        boolean bResult = true;

        // First, create a small table.
        log.println("Creating a small table.");
        try {
            XCell cell = null;
            XTextRange textrange = null;

            for (int i = 1; i < 4; i++) {
                cell = oSheet.getCellByPosition(0, i);
                textrange = UnoRuntime.
                                    queryInterface(XTextRange.class, cell);
                textrange.setString("Row" + i);

                cell = oSheet.getCellByPosition(i, 0);
                textrange = UnoRuntime.
                                    queryInterface(XTextRange.class, cell);
                textrange.setString("Column" + i);
            }

            for (int i = 1; i < 4; i++)
                for (int j = 1; j < 4; j++) {
                    cell = oSheet.getCellByPosition(i, j);
                    textrange = UnoRuntime.
                                    queryInterface(XTextRange.class, cell);
                    textrange.setString("Val" + ((j - 1) * 3 + i));
                }
            log.println("Finished creating table.");
            log.println("Creating new ranges from titles");

            CellRangeAddress CRA = new CellRangeAddress((short)0, 0, 0, 3, 3);
            Border border = Border.TOP;
            oObj.addNewFromTitles(CRA, border);
            for (int i = 1; i < 4; i++) {
                bResult &= oObj.hasByName("Column" + i);

                Object range = oObj.getByName("Column" + i);
                XCellRangeReferrer CRR = UnoRuntime.
                                queryInterface(XCellRangeReferrer.class,range);

                XCellRange CR = CRR.getReferredCells();
                XCellRangeAddressable xCRA = UnoRuntime.queryInterface(XCellRangeAddressable.class, CR);

                CellRangeAddress objCRA = xCRA.getRangeAddress();

                bResult &= (objCRA.EndColumn == i && objCRA.StartColumn == i);
                bResult &= objCRA.StartRow == 1;
                bResult &= objCRA.EndRow == 3;
                bResult &= objCRA.Sheet == 0;
            }

            border = Border.LEFT;
            oObj.addNewFromTitles(CRA, border);
            for (int i = 1; i < 4; i++) {
                bResult &= oObj.hasByName("Row" + i);

                Object range = oObj.getByName("Row" + i);
                XCellRangeReferrer CRR = UnoRuntime.
                                queryInterface(XCellRangeReferrer.class,range);

                XCellRange CR = CRR.getReferredCells();
                XCellRangeAddressable xCRA = UnoRuntime.queryInterface(XCellRangeAddressable.class, CR);

                CellRangeAddress objCRA = xCRA.getRangeAddress();

                bResult &= (objCRA.EndRow == i && objCRA.StartRow == i);
                bResult &= objCRA.StartColumn == 1;
                bResult &= objCRA.EndColumn == 3;
                bResult &= objCRA.Sheet == 0;
            }

            oObj.outputList(new CellAddress((short)0, 5, 5));

        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            bResult = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            bResult = false;
        }

        tRes.tested("addNewFromTitles()", bResult);
    }

    /**
    * Test calls the method and checks existing of named ranges obtained
    * by relation <code>'SHEET'</code>. <p>
    * Has <b> OK </b> status if all output named ranges exist
    * and no exceptions were thrown. <p>
    */
    public void _outputList() {
        boolean bResult = true;
        CellAddress CA = new CellAddress((short)0, 0, 0);

        XIndexAccess IA = UnoRuntime.
                            queryInterface(XIndexAccess.class, oObj);

        int elementsCount = IA.getCount();

        oObj.outputList(CA);

        try {
            for (int i = 0; i < elementsCount; i++) {
                XCell cell = oSheet.getCellByPosition(0, i);
                XTextRange textrange = UnoRuntime.queryInterface(XTextRange.class, cell);
                String str = textrange.getString();
                bResult &= oObj.hasByName(str);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            bResult = false;
        }

        tRes.tested("outputList()", bResult);
    }

    /**
    * Test calls the method for existing range, checks number of ranges in
    * collection after method call, calls method for non-existent named range.
    * <p>Has <b> OK </b> status if number of named ranges is less by one than
    * before method call and exception was thrown during second call. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNewByName() </code> : to have name of existent
    *  named range </li>
    * </ul>
    */
    public void _removeByName() {
        requiredMethod("addNewByName()");
        boolean bResult = true;
        XIndexAccess IA = UnoRuntime.
                            queryInterface(XIndexAccess.class, oObj);

        int elementsCount = IA.getCount();

        // Removing existent element
        oObj.removeByName(name);
        bResult = elementsCount == IA.getCount() + 1;

        try {
            // Removing unexistent element.
            oObj.removeByName(name);
            log.println("Exception expected when removed unexistent element!");
            bResult = false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception occurred while testing" +
                      "removeByName() when removed unexistent element.");

        }

        tRes.tested("removeByName()", bResult);
    }

    /**
    * Method make string of random content.
    * @return string of random content
    */
    String getRandomContent(String str) {
        String gRS = "none";
        Random rnd = new Random();

        StringTokenizer ST = new StringTokenizer(str, ";");
        int nr = rnd.nextInt(ST.countTokens());
        if (nr < 1)
            nr++;

        for (int i=1; i < nr + 1; i++)
            gRS = ST.nextToken();

        return gRS;
    }

    /**
     * Returns random value of named range flag.
     */
    int getRandomType(){
        int types[] = { 0,
                        NamedRangeFlag.COLUMN_HEADER,
                        NamedRangeFlag.FILTER_CRITERIA,
                        NamedRangeFlag.PRINT_AREA,
                        NamedRangeFlag.ROW_HEADER
                      };

        Random rnd = new Random();
        return types[rnd.nextInt(5)];
    }
}

