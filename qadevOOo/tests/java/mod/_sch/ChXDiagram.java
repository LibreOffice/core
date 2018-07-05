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

package mod._sch;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.awt.Rectangle;
import com.sun.star.chart.XChartDataArray;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableChart;
import com.sun.star.table.XTableCharts;
import com.sun.star.table.XTableChartsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by the following services:
* <ul>
*   <li> <code>com.sun.star.chart.Dim3DDiagram</code> </li>
*   <li> <code>com.sun.star.chart.StockDiagram</code> </li>
*   <li> <code>com.sun.star.chart.LineDiagram</code> </li>
*   <li> <code>com.sun.star.chart.BarDiagram</code> </li>
*   <li> <code>com.sun.star.chart.StackableDiagram</code> </li>
* </ul>
*  <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::chart::XDiagram</code></li>
*  <li> <code>com::sun::star::chart::ChartAxisXSupplier</code></li>
*  <li> <code>com::sun::star::chart::Dim3DDiagram</code></li>
*  <li> <code>com::sun::star::chart::StockDiagram</code></li>
*  <li> <code>com::sun::star::chart::ChartAxisZSupplier</code></li>
*  <li> <code>com::sun::star::chart::XTwoAxisXSupplier</code></li>
*  <li> <code>com::sun::star::chart::LineDiagram</code></li>
*  <li> <code>com::sun::star::chart::BarDiagram</code></li>
*  <li> <code>com::sun::star::chart::XAxisYSupplier</code></li>
*  <li> <code>com::sun::star::chart::Diagram</code></li>
*  <li> <code>com::sun::star::chart::X3DDisplay</code></li>
*  <li> <code>com::sun::star::chart::ChartTwoAxisYSupplier</code></li>
*  <li> <code>com::sun::star::chart::StackableDiagram</code></li>
*  <li> <code>com::sun::star::chart::ChartAxisYSupplier</code></li>
*  <li> <code>com::sun::star::chart::XAxisXSupplier</code></li>
*  <li> <code>com::sun::star::chart::ChartTwoAxisXSupplier</code></li>
*  <li> <code>com::sun::star::drawing::XShape</code></li>
*  <li> <code>com::sun::star::chart::XTwoAxisYSupplier</code></li>
*  <li> <code>com::sun::star::chart::ChartStatistics</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
*  <li> <code>com::sun::star::chart::XAxisZSupplier</code></li>
*  <li> <code>com::sun::star::chart::XStatisticDisplay</code></li>
* </ul>
* @see com.sun.star.chart.XDiagram
* @see com.sun.star.chart.ChartAxisXSupplier
* @see com.sun.star.chart.Dim3DDiagram
* @see com.sun.star.chart.StockDiagram
* @see com.sun.star.chart.ChartAxisZSupplier
* @see com.sun.star.chart.XTwoAxisXSupplier
* @see com.sun.star.chart.LineDiagram
* @see com.sun.star.chart.BarDiagram
* @see com.sun.star.chart.XAxisYSupplier
* @see com.sun.star.chart.Diagram
* @see com.sun.star.chart.X3DDisplay
* @see com.sun.star.chart.ChartTwoAxisYSupplier
* @see com.sun.star.chart.StackableDiagram
* @see com.sun.star.chart.ChartAxisYSupplier
* @see com.sun.star.chart.XAxisXSupplier
* @see com.sun.star.chart.ChartTwoAxisXSupplier
* @see com.sun.star.drawing.XShape
* @see com.sun.star.chart.XTwoAxisYSupplier
* @see com.sun.star.chart.ChartStatistics
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.drawing.XShapeDescriptor
* @see com.sun.star.chart.XAxisZSupplier
* @see com.sun.star.chart.XStatisticDisplay
* @see ifc.chart._XDiagram
* @see ifc.chart._ChartAxisXSupplier
* @see ifc.chart._Dim3DDiagram
* @see ifc.chart._StockDiagram
* @see ifc.chart._ChartAxisZSupplier
* @see ifc.chart._XTwoAxisXSupplier
* @see ifc.chart._LineDiagram
* @see ifc.chart._BarDiagram
* @see ifc.chart._XAxisYSupplier
* @see ifc.chart._Diagram
* @see ifc.chart._X3DDisplay
* @see ifc.chart._ChartTwoAxisYSupplier
* @see ifc.chart._StackableDiagram
* @see ifc.chart._ChartAxisYSupplier
* @see ifc.chart._XAxisXSupplier
* @see ifc.chart._ChartTwoAxisXSupplier
* @see ifc.drawing._XShape
* @see ifc.chart._XTwoAxisYSupplier
* @see ifc.chart._ChartStatistics
* @see ifc.beans._XPropertySet
* @see ifc.drawing._XShapeDescriptor
* @see ifc.chart._XAxisZSupplier
* @see ifc.chart._XStatisticDisplay
*/
public class ChXDiagram extends TestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a sheetdocument" );
        xSheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    closing xSheetDoc " );
        util.DesktopTools.closeDoc(xSheetDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Inserts some values into the cells of the same cell
    * range address. Adds and retrieves the chart that using the data from
    * the cells of this cell range address. Obtains the chart document which is
    * embedded into the retrieved chart using the interface
    * <code>XEmbeddedObjectSupplier</code>. Retrieves the diagram from
    * the obtained chart document. The retrieved diagram is the instance of
    * the service <code>com.sun.star.chart.Diagram</code>.
    * Obtains the data source of the chart from the chart document.
    * Creates a stock-diagram, a bar-diagram, a XY-diagram and line-diagram
    * that are the instances of the following services:
    * <ul>
    *   <li> <code>com.sun.star.chart.StockDiagram</code> </li>
    *   <li> <code>com.sun.star.chart.BarDiagram</code> </li>
    *   <li> <code>com.sun.star.chart.LineDiagram</code> </li>
    *   <li> <code>com.sun.star.chart.StackableDiagram</code> </li>
    * </ul>.
    * Object relations created :
    * <ul>
    *  <li> <code>'CHARTDOC'</code> for
    *      {@link ifc.chart._Dim3DDiagram}, {@link ifc.chart._StockDiagram},
    *      {@link ifc.chart._ChartAxisZSupplier}, {@link ifc.chart._LineDiagram},
    *      {@link ifc.chart._BarDiagram}, {@link ifc.chart._Diagram},
    *      {@link ifc.chart._ChartTwoAxisYSupplier},
    *      {@link ifc.chart._StackableDiagram}, {@link ifc.chart._Diagram},
    *      {@link ifc.chart._ChartAxisYSupplier},
    *      {@link ifc.chart._ChartTwoAxisXSupplier},
    *      {@link ifc.chart._ChartStatistics} (the obtained chart document)</li>
    *  <li> <code>'ROWAMOUNT', 'COLAMOUNT'</code> for
    *      {@link ifc.chart._XDiagram}(the number of chart columns and
    *       the number of chart rows) </li>
    *  <li> <code>'STOCK'</code> for
    *      {@link ifc.chart._StockDiagram}(the created stock-diagram) </li>
    *  <li> <code>'BAR'</code> for
    *      {@link ifc.chart._BarDiagram}, {@link ifc.chart._ChartAxisZSupplier},
    *      {@link ifc.chart._ChartTwoAxisXSupplier},
    *      {@link ifc.chart._ChartTwoAxisYSupplier}(the created bar-diagram)</li>
    *  <li> <code>'LINE'</code> for
    *      {@link ifc.chart._LineDiagram}(the created XY-diagram) </li>
    *  <li> <code>'STACK'</code> for
    *      {@link ifc.chart._StackableDiagram}(the created Line-diagram) </li>
    * </ul>
    * @see com.sun.star.document.XEmbeddedObjectSupplier
    * @see com.sun.star.chart.Diagram
    * @see com.sun.star.chart.StockDiagram
    */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        XSpreadsheet oSheet=null;
        XChartDocument xChartDoc=null;
        XDiagram oObj = null;

        System.out.println("Getting spreadsheet") ;
        XSpreadsheets oSheets = xSheetDoc.getSheets() ;
        XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
        oSheet = (XSpreadsheet) AnyConverter.toObject(
                new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

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


        if (!oCharts.hasByName("ChXDiagram")) {
            oCharts.addNewByName("ChXDiagram", oRect, oAddr, true, true);
        }

        // get the TableChart
        XTableChart oChart = (XTableChart) AnyConverter.toObject(
            new Type(XTableChart.class),UnoRuntime.queryInterface(
                XNameAccess.class, oCharts).getByName("ChXDiagram"));

        XEmbeddedObjectSupplier oEOS = UnoRuntime.queryInterface(XEmbeddedObjectSupplier.class, oChart);
        XInterface oInt = oEOS.getEmbeddedObject();
        xChartDoc = UnoRuntime.queryInterface(XChartDocument.class,oInt);
        oObj = xChartDoc.getDiagram();

        log.println( "creating a new environment for chartdocument object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println( "adding ChartDocument as mod relation to environment" );
        tEnv.addObjRelation("CHARTDOC", xChartDoc);

        XChartDataArray da = UnoRuntime.queryInterface(XChartDataArray.class, xChartDoc.getData());
        int cols = da.getColumnDescriptions().length;
        int rows = da.getRowDescriptions().length;

        tEnv.addObjRelation("ROWAMOUNT", Integer.valueOf(rows));
        tEnv.addObjRelation("COLAMOUNT", Integer.valueOf(cols));

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF());
        Object stock = SOF.createDiagram(xChartDoc,"StockDiagram");
        tEnv.addObjRelation("STOCK",stock);

        Object bar = SOF.createDiagram(xChartDoc,"BarDiagram");
        tEnv.addObjRelation("BAR",bar);

        Object line = SOF.createDiagram(xChartDoc,"XYDiagram");
        tEnv.addObjRelation("LINE",line);

        Object stack = SOF.createDiagram(xChartDoc,"LineDiagram");
        tEnv.addObjRelation("STACK",stack);

        return tEnv;
    } // finish method getTestEnvironment

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
        int CellX, int CellY, String theValue, XSpreadsheet TT1, String flag)
            throws com.sun.star.lang.IndexOutOfBoundsException {

        XCell oCell = TT1.getCellByPosition(CellX, CellY);

        if (flag.equals("V")) {
            oCell.setValue(Float.parseFloat(theValue));
        }
        else {
            oCell.setFormula(theValue);
        }

    } // end of insertIntoCell


}    // finish class ChXDiagram

