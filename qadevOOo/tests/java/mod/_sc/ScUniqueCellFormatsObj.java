/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ScUniqueCellFormatsObj.java,v $
 * $Revision: 1.5 $
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
    static XSpreadsheetDocument xSheetDoc = null;
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
        //add this lines after synchronisation
    //log.println("    disposing xSheetDoc ");
        //DesktopTools.closeDoc(xSheetDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        log.println("Getting the first sheet");

        XIndexAccess xIA = (XIndexAccess) UnoRuntime.queryInterface(
                                   XIndexAccess.class, xSheetDoc.getSheets());

        try {
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
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

        XUniqueCellFormatRangesSupplier xUCRS = (XUniqueCellFormatRangesSupplier) UnoRuntime.queryInterface(
                                                        XUniqueCellFormatRangesSupplier.class,
                                                        oSheet);

        XInterface oObj = xUCRS.getUniqueCellFormatRanges();
        log.println("Implementationname: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }

    protected void changeColor(String RangeName, int r, int g, int b) {
        XCellRange xRange = oSheet.getCellRangeByName(RangeName);
        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(
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
