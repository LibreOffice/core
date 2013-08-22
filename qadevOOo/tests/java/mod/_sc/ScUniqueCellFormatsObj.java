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
package mod._sc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XUniqueCellFormatRangesSupplier;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.awt.Color;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;


public class ScUniqueCellFormatsObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;
    static XSpreadsheet oSheet = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a sheetdocument");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        util.DesktopTools.closeDoc(xSheetDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        log.println("Getting the first sheet");

        XIndexAccess xIA = UnoRuntime.queryInterface(
                                   XIndexAccess.class, xSheetDoc.getSheets());

        try {
            oSheet = UnoRuntime.queryInterface(
                             XSpreadsheet.class, xIA.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        }

        changeColor("A1:A5", 0, 255, 0);
        changeColor("A6:B10", 255, 0, 0);
        changeColor("B1:B6", 0, 0, 255);
        changeColor("B7", 0, 255, 0);
        changeColor("B8:B10", 0, 0, 255);
        changeColor("C1:C10", 0, 0, 255);
        changeColor("D1:D10", 0, 255, 0);

        XUniqueCellFormatRangesSupplier xUCRS = UnoRuntime.queryInterface(
                                                        XUniqueCellFormatRangesSupplier.class,
                                                        oSheet);

        XInterface oObj = xUCRS.getUniqueCellFormatRanges();
        log.println("Implementationname: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }

    protected void changeColor(String RangeName, int r, int g, int b) {
        XCellRange xRange = oSheet.getCellRangeByName(RangeName);
        XPropertySet xPropertySet = UnoRuntime.queryInterface(
                                            XPropertySet.class, xRange);
        Color c = new Color(r, g, b);
        int c2int = 16777216 + c.hashCode();

        try {
            xPropertySet.setPropertyValue("CellBackColor", new Integer(c2int));
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't change CellFormat");
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't change CellFormat");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't change CellFormat");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't change CellFormat");
        }
    }
}
