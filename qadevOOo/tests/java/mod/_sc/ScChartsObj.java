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

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.awt.Rectangle;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableCharts;
import com.sun.star.table.XTableChartsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

/**
* Test for object which is represented by service
* <code>com.sun.star.table.TableCharts</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::table::XTableCharts</code></li>
* </ul>
* @see com.sun.star.table.TableCharts
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.table.XTableCharts
* @see ifc.container._XNameAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.table._XTableCharts
*/
public class ScChartsObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Inserts some values into the cells of the some cell
    * range address. Obtains the collection of the charts using the interface
    * <code>XTableChartsSupplier</code>. Creates and adds the chart that using
    * the data from the cells of this cell range address. Collection of
    * the charts is the instance of the service
    * <code>com.sun.star.table.TableCharts</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'RECT'</code> for
    *      {@link ifc.table._XTableCharts}(of type <code>Rectangle</code>
    *       the position of the chart)</li>
    *  <li> <code>'ADDR'</code> for
    *      {@link ifc.table._XTableCharts}(of type
    *      <code>com.sun.star.table.CellRangeAddress[]</code> data source ranges
    *      for chart creating)</li>
    * </ul>
    * @see com.sun.star.container.XNamed
    * @see com.sun.star.table.XTableChartsSupplier
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XSpreadsheet oSheet=null;

        try {
            log.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get Sheet ");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get sheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get Sheet ");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get sheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't get Sheet ");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get sheet", e);
        }

        log.println("Creating the Header") ;

        insertIntoCell(1,0,"JAN",oSheet,"");
        insertIntoCell(2,0,"FEB",oSheet,"");
        insertIntoCell(3,0,"MAR",oSheet,"");
        insertIntoCell(4,0,"APR",oSheet,"");
        insertIntoCell(5,0,"MAI",oSheet,"");
        insertIntoCell(6,0,"JUN",oSheet,"");
        insertIntoCell(7,0,"JUL",oSheet,"");
        insertIntoCell(8,0,"AUG",oSheet,"");
        insertIntoCell(9,0,"SEP",oSheet,"");
        insertIntoCell(10,0,"OCT",oSheet,"");
        insertIntoCell(11,0,"NOV",oSheet,"");
        insertIntoCell(12,0,"DEC",oSheet,"");
        insertIntoCell(13,0,"SUM",oSheet,"");

        log.println("Fill the lines");

        insertIntoCell(0,1,"Smith",oSheet,"");
        insertIntoCell(1,1,"42",oSheet,"V");
        insertIntoCell(2,1,"58.9",oSheet,"V");
        insertIntoCell(3,1,"-66.5",oSheet,"V");
        insertIntoCell(4,1,"43.4",oSheet,"V");
        insertIntoCell(5,1,"44.5",oSheet,"V");
        insertIntoCell(6,1,"45.3",oSheet,"V");
        insertIntoCell(7,1,"-67.3",oSheet,"V");
        insertIntoCell(8,1,"30.5",oSheet,"V");
        insertIntoCell(9,1,"23.2",oSheet,"V");
        insertIntoCell(10,1,"-97.3",oSheet,"V");
        insertIntoCell(11,1,"22.4",oSheet,"V");
        insertIntoCell(12,1,"23.5",oSheet,"V");
        insertIntoCell(13,1,"=SUM(B2:M2)",oSheet,"");

        insertIntoCell(0,2,"Jones",oSheet,"");
        insertIntoCell(1,2,"21",oSheet,"V");
        insertIntoCell(2,2,"40.9",oSheet,"V");
        insertIntoCell(3,2,"-57.5",oSheet,"V");
        insertIntoCell(4,2,"-23.4",oSheet,"V");
        insertIntoCell(5,2,"34.5",oSheet,"V");
        insertIntoCell(6,2,"59.3",oSheet,"V");
        insertIntoCell(7,2,"27.3",oSheet,"V");
        insertIntoCell(8,2,"-38.5",oSheet,"V");
        insertIntoCell(9,2,"43.2",oSheet,"V");
        insertIntoCell(10,2,"57.3",oSheet,"V");
        insertIntoCell(11,2,"25.4",oSheet,"V");
        insertIntoCell(12,2,"28.5",oSheet,"V");
        insertIntoCell(13,2,"=SUM(B3:M3)",oSheet,"");

        insertIntoCell(0,3,"Brown",oSheet,"");
        insertIntoCell(1,3,"31.45",oSheet,"V");
        insertIntoCell(2,3,"-20.9",oSheet,"V");
        insertIntoCell(3,3,"-117.5",oSheet,"V");
        insertIntoCell(4,3,"23.4",oSheet,"V");
        insertIntoCell(5,3,"-114.5",oSheet,"V");
        insertIntoCell(6,3,"115.3",oSheet,"V");
        insertIntoCell(7,3,"-171.3",oSheet,"V");
        insertIntoCell(8,3,"89.5",oSheet,"V");
        insertIntoCell(9,3,"41.2",oSheet,"V");
        insertIntoCell(10,3,"71.3",oSheet,"V");
        insertIntoCell(11,3,"25.4",oSheet,"V");
        insertIntoCell(12,3,"38.5",oSheet,"V");
        insertIntoCell(13,3,"=SUM(A4:L4)",oSheet,"");

        // insert a chart
        Rectangle oRect = new Rectangle(500, 3000, 25000, 11000);

        XCellRange oRange = UnoRuntime.queryInterface(XCellRange.class, oSheet);
        XCellRange myRange = oRange.getCellRangeByName("A1:N4");
        XCellRangeAddressable oRangeAddr = UnoRuntime.queryInterface(XCellRangeAddressable.class, myRange);
        CellRangeAddress myAddr = oRangeAddr.getRangeAddress();

        CellRangeAddress[] oAddr = new CellRangeAddress[1];
        oAddr[0] = myAddr;
        XTableChartsSupplier oSupp = UnoRuntime.queryInterface(XTableChartsSupplier.class, oSheet);


        log.println("Insert Chart");
        XTableCharts oCharts = oSupp.getCharts();
        oCharts.addNewByName("ScChartObj", oRect, oAddr, true, true);

        log.println("creating a new environment for object");
        TestEnvironment tEnv = new TestEnvironment(oCharts);

        tEnv.addObjRelation("RECT", oRect);
        tEnv.addObjRelation("ADDR", oAddr);

        return tEnv;
    }

    /**
    * Inserts a value or a formula in the cell of the spreasheet.
    * @param CellX is the column index of the cell
    * @param CellY is the row index of the cell
    * @param theValue string representation of the value
    * @param TT1 specify the spreadsheet, the interface
    * <code>com.sun.star.sheet.XSpreadsheet</code>
    * @param flag if it's equal to <code>'V'</code> then the method inserts
    * a double-value in the cell else it inserts a formula in the cell
    */
    public static void insertIntoCell(
        int CellX, int CellY, String theValue, XSpreadsheet TT1, String flag) {

        XCell oCell = null;

        try {
            oCell = TT1.getCellByPosition(CellX, CellY);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.out.println("Could not get Cell");
        }

        if (flag.equals("V")) {
            oCell.setValue(new Float(theValue).floatValue());
        } else {
            oCell.setFormula(theValue);
        }

    } // end of insertIntoCell
}

