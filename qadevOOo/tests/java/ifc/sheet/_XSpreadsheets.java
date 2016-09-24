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

import java.util.concurrent.atomic.AtomicInteger;

import com.sun.star.sheet.XSpreadsheets;

import lib.MultiMethodTest;


/**
* Testing <code>com.sun.star.sheet.XSpreadsheets</code>
* interface methods :
* <ul>
*  <li><code> insertNewByName()</code></li>
*  <li><code> moveByName()</code></li>
*  <li><code> copyByName()</code></li>
* </ul> <p>
* Test is multithread compliant. <p>
* @see com.sun.star.sheet.XSpreadsheets
*/
public class _XSpreadsheets extends MultiMethodTest {
    private static final AtomicInteger uniqCount = new AtomicInteger(0);
    public XSpreadsheets oObj = null;
    protected int uniqNumber = 0;

    /**
    * Sets the unique number for the current test.
    */
    @Override
    protected synchronized void before() {
        uniqNumber = uniqCount.getAndIncrement();
    }

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, copies inserted sheet with the new name,
    * checks existence of the sheet with this name in collection and removes
    * the both sheets from the collection. <p>
    * Has <b> OK </b> status if the sheet with the name of the copy exists
    * in the collection and no exceptions were thrown. <p>
    */
    public void _copyByName() {
        boolean result = true;

        //first insert one that should be copied
        String iS = newName("copyFrom");
        log.println("Inserting sheet '" + iS + "'");
        oObj.insertNewByName(iS, (short) 0);

        String[] eNames = oObj.getElementNames();
        String NewSheet = newName("copyTo");
        log.println("Try to copy " + eNames[0] + " to " + NewSheet);
        oObj.copyByName(eNames[0], NewSheet, (short) 0);
        result = oObj.hasByName(NewSheet);

        //remove all inserted sheets
        try {
            oObj.removeByName(NewSheet);
            oObj.removeByName(iS);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet by name");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet by name");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("copyByName()", result);
    } // finished _copyByName

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, moves the inserted sheet to the new position
    * in collection, gets all element names in collection and checks the name
    * of the sheet in the new position. <p>
    * Has <b> OK </b> status if the sheet name in the new position is equal to
    * the name of the sheet that was moved. <p>
    */
    public void _moveByName() {
        //first insert one that should be moved
        String iS = newName("move");
        oObj.insertNewByName(iS, (short) 0);

        String[] eNames = oObj.getElementNames();
        String sheetToMove = eNames[0];
        log.println("Try to move " + sheetToMove);
        oObj.moveByName(sheetToMove, (short) 2);
        eNames = oObj.getElementNames();
        tRes.tested("moveByName()", sheetToMove.equals(eNames[1]));
    } // finished _moveByName

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, checks the existence of the inserted sheet in
    * the collection, removes the sheet, tries to insert the sheet with the
    * bad name returned by method <code>badName()</code>. <p>
    * Has <b> OK </b> status if the inserted sheet exists in the collection
    * after first method call and if exception occurred during the second call. <p>
    */
    public void _insertNewByName() {
        boolean result = false;

        String NewSheet = newName("insert");
        log.println("Try to insert " + NewSheet);
        oObj.insertNewByName(NewSheet, (short) 0);
        result = oObj.hasByName(NewSheet);

        try {
            oObj.removeByName(NewSheet);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        }

        try {
            NewSheet = badName();
            log.println("Try to insert " + NewSheet);
            oObj.insertNewByName(NewSheet, (short) 0);
            log.println(
                    "No Exception thrown while inserting sheet with invalid name");
            result &= false;
            oObj.removeByName(NewSheet);
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println(
                    "Expected exception occurred during testing 'insertNewByName'");
            result &= true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("insertNewByName()", result);
    } // finished _insertByName

    /**
    * Method returns unique new name using passed prefix and unique number
    * of the current test.
    */
    public String newName(String prefix) {
        return prefix + uniqNumber;
    } // finished newName

    /**
    * Method return bad name for a sheet using the name of the current thread.
    */
    public String badName() {
        return "$%#/?\\";
    } // finished badName
} //finish class _XSpreadsheets
