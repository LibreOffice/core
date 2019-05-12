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
            // Some exception occured.FAILED
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
