/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
import com.sun.star.awt.Rectangle;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.chart.XDiagram;
import com.sun.star.chart.XChartDocument;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;

import com.sun.star.document.XEmbeddedObjectSupplier;

import com.sun.star.frame.XComponentLoader;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XCellRangeAddressable;

import com.sun.star.table.XTableChart;
import com.sun.star.table.XTableCharts;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableChartsSupplier;
import com.sun.star.table.CellRangeAddress;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XComponentContext;



/** This class loads an OpenOffice.org Calc document and changes the type of the
 * embedded chart.
 */
public class ChartTypeChange {

    /** Table chart, which type will be changed.
     */
    private XTableChart xtablechart = null;

    /** Service factory
     */
    private XMultiComponentFactory xMCF = null;

    /** Component context
     */
    private XComponentContext xCompContext = null;

    /** Beginning of the program.
     * @param args No arguments will be passed to the class.
     */
    public static void main(String args[]) {
        try {
            ChartTypeChange charttypechange = new ChartTypeChange();

            // Double array holding all values the chart should be based on.
            String[][] stringValues = {
                { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" },
            };

            // Create the chart with
            charttypechange.getChart( stringValues );

            String[] stringChartType = {
                "com.sun.star.chart.LineDiagram",
                "com.sun.star.chart.BarDiagram",
                "com.sun.star.chart.PieDiagram",
                "com.sun.star.chart.NetDiagram",
                "com.sun.star.chart.XYDiagram",
                "com.sun.star.chart.StockDiagram",
                "com.sun.star.chart.AreaDiagram"
            };

            for ( int intCounter = 0; intCounter < stringChartType.length;
                  intCounter++ ) {
                charttypechange.changeChartType( stringChartType[ intCounter ],
                                                 false );
                Thread.sleep( 3000 );
            }

            System.exit(0);
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** The constructor connects to the OpenOffice.org.
     * @param args Parameters for this constructor (connection string).
     * @throws Exception All exceptions are thrown from this method.
     */
    public ChartTypeChange()
        throws Exception {

        /* Bootstraps a component context. Component context to be granted
           to a component for running. Arbitrary values can be retrieved
           from the context. */
        xCompContext = com.sun.star.comp.helper.Bootstrap.bootstrap();

        /* Gets the service manager instance to be used (or null). This method has
           been added for convenience, because the service manager is a often used
           object. */
        xMCF = xCompContext.getServiceManager();
    }

    /** This method will change the type of a specified chart.
     * @param stringType The chart will be converted to this type.
     * @param booleanIs3D If the chart should be displayed in 3D this parameter should be set to true.
     * @throws Exception All exceptions are thrown from this method.
     */
    public void changeChartType( String stringType, boolean booleanIs3D )
    throws Exception {
        XEmbeddedObjectSupplier xEmbeddedObjSupplier = (XEmbeddedObjectSupplier)
            UnoRuntime.queryInterface(XEmbeddedObjectSupplier.class, xtablechart);
        XInterface xInterface = xEmbeddedObjSupplier.getEmbeddedObject();

        XChartDocument xChartDoc = (XChartDocument)UnoRuntime.queryInterface(
            XChartDocument.class, xInterface);
        XDiagram xDiagram = (XDiagram) xChartDoc.getDiagram();
        XMultiServiceFactory xMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, xChartDoc );
        Object object = xMSF.createInstance( stringType );
        xDiagram = (XDiagram) UnoRuntime.queryInterface(XDiagram.class, object);

        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
            XPropertySet.class, xDiagram );
        xPropSet.setPropertyValue( "Dim3D", new Boolean( booleanIs3D ) );

        xChartDoc.setDiagram(xDiagram);
    }

    /** Loading an OpenOffice.org Calc document and getting a chart by name.
     * @param stringFileName Name of the OpenOffice.org Calc document which should
     *                       be loaded.
     * @param stringChartName Name of the chart which should get a new chart type.
     */
    public void getChart( String stringFileName, String stringChartName ) {
        try {
            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            XComponentLoader xComponentloader = (XComponentLoader)
                UnoRuntime.queryInterface( XComponentLoader.class,
                    xMCF.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                   xCompContext ) );

            // Load a Writer document, which will be automaticly displayed
            XComponent xComponent = xComponentloader.loadComponentFromURL(
            "file:///" + stringFileName, "_blank", 0,
            new PropertyValue[0] );

            // Query for the interface XSpreadsheetDocument
            XSpreadsheetDocument xSpreadSheetDocument = ( XSpreadsheetDocument )
                UnoRuntime.queryInterface( XSpreadsheetDocument.class, xComponent );

            XSpreadsheets xSpreadsheets = xSpreadSheetDocument.getSheets() ;

            XIndexAccess xIndexAccess = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets );

            XSpreadsheet xSpreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xIndexAccess.getByIndex(0));

            XTableChartsSupplier xTableChartsSupplier = ( XTableChartsSupplier )
                UnoRuntime.queryInterface( XTableChartsSupplier.class, xSpreadsheet );

            xIndexAccess = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xTableChartsSupplier.getCharts() );

            this.xtablechart = (XTableChart) UnoRuntime.queryInterface(
                XTableChart.class,  xIndexAccess.getByIndex( 0 ) );
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** Creating an empty OpenOffice.org Calc document, inserting data, and getting a
     * chart by name.
     * @param stringValues Double array with the values for the chart.
     */
    public void getChart( String[][] stringValues ) {
        try {
            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            XComponentLoader xcomponentloader = ( XComponentLoader )
                UnoRuntime.queryInterface( XComponentLoader.class,
                                           xMCF.createInstanceWithContext(
                                               "com.sun.star.frame.Desktop",
                                               xCompContext ) );

            // Create an empty calc document, which will be automaticly displayed
            XComponent xComponent = xcomponentloader.loadComponentFromURL(
            "private:factory/scalc", "_blank", 0,
            new PropertyValue[0] );

            // Query for the interface XSpreadsheetDocument
            XSpreadsheetDocument xspreadsheetdocument = ( XSpreadsheetDocument )
                UnoRuntime.queryInterface( XSpreadsheetDocument.class, xComponent );

            // Get all sheets of the spreadsheet document.
            XSpreadsheets xspreadsheets = xspreadsheetdocument.getSheets() ;

            // Get the index of the spreadsheet document.
            XIndexAccess xindexaccess = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xspreadsheets );

            // Get the first spreadsheet.
            XSpreadsheet xspreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xindexaccess.getByIndex(0));

            // The double array will written to the spreadsheet
            for ( int intY = 0; intY < stringValues.length; intY++ ) {
                for ( int intX = 0; intX < stringValues[ intY ].length;
                intX++ ) {
                    // Insert the value to the cell, specified by intY and intX.
                    this.insertIntoCell( intY, intX,
                    stringValues[ intY ][ intX ], xspreadsheet, "" );
                }
            }

            // Create a rectangle, which holds the size of the chart.
            Rectangle rectangle = new Rectangle();
            rectangle.X = 500;
            rectangle.Y = 3000;
            rectangle.Width = 25000;
            rectangle.Height = 11000;

            // Get the cell range of the spreadsheet.
            XCellRange xcellrange = ( XCellRange ) UnoRuntime.queryInterface(
                XCellRange.class, xspreadsheet );

            // Create the Unicode of the character for the column name.
            char charRectangle = ( char ) ( 65 + stringValues.length - 1 );

            // Get maximum length all rows in the double array.
            int intMaximumWidthRow = 0;
            for ( int intRow = 0; intRow < stringValues.length; intRow++ ) {
                if ( stringValues[ intRow ].length > intMaximumWidthRow ) {
                    intMaximumWidthRow = stringValues[ intRow ].length;
                }
            }

            // Get the cell range of the written values.
            XCellRange xcellrangeChart = xcellrange.getCellRangeByName( "A1:" +
            charRectangle + intMaximumWidthRow );

            // Get the addressable cell range.
            XCellRangeAddressable xcellrangeaddressable =
            ( XCellRangeAddressable ) UnoRuntime.queryInterface(
                XCellRangeAddressable.class, xcellrangeChart );

            // Get the cell range address.
            CellRangeAddress cellrangeaddress = xcellrangeaddressable.getRangeAddress();

            // Create the cell range address for the chart.
            CellRangeAddress[] cellrangeaddressChart =
            new CellRangeAddress[ 1 ];
            cellrangeaddressChart[ 0 ] = cellrangeaddress;

            // Get the table charts supplier of the spreadsheet.
            XTableChartsSupplier xtablechartssupplier = ( XTableChartsSupplier )
                UnoRuntime.queryInterface( XTableChartsSupplier.class, xspreadsheet );

            // Get all table charts of the spreadsheet.
            XTableCharts xtablecharts = xtablechartssupplier.getCharts();

            // Create a table chart with all written values.
            xtablecharts.addNewByName( "Example", rectangle,
            cellrangeaddressChart, true, true );

            // Get the created table chart.
            this.xtablechart = ( XTableChart ) UnoRuntime.queryInterface(
                XTableChart.class, (( XNameAccess ) UnoRuntime.queryInterface(
                    XNameAccess.class, xtablecharts ) ).getByName( "Example" ));
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** Inserting a given value to a cell, that is specified by the parameters intX
     * and intY.
     * @param intX Column on the spreadsheet.
     * @param intY Row on the spreadsheet.
     * @param stringValue Value to be inserted to a cell.
     * @param xspreadsheet Spreadsheet of the cell, which will be changed.
     * @param stringFlag If the value of stringFlag is "V", the stringValue
     *                   will be converted to the
     * float type. Otherwise the stringValue will be written as a formula.
     */
    public static void insertIntoCell( int intX, int intY, String stringValue,
                                       XSpreadsheet xspreadsheet, String stringFlag )
    {
        XCell xcell = null;

        try {
            xcell = xspreadsheet.getCellByPosition( intX, intY );
        }
        catch ( com.sun.star.lang.IndexOutOfBoundsException exception ) {
            System.out.println( "Could not get cell." );
        }
        if ( stringFlag.equals( "V" ) ) {
            xcell.setValue( ( new Float( stringValue ) ).floatValue() );
        }
        else {
            xcell.setFormula( stringValue );
        }
    }
}
