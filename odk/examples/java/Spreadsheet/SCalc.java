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

//***************************************************************************
// comment: Step 1: get the remote component context from the office
//          Step 2: open an empty calc document
//          Step 3: create cell styles
//          Step 4: get the sheet an insert some data
//          Step 5: apply the created cell syles
//          Step 6: insert a 3D Chart
//***************************************************************************

import com.sun.star.awt.Rectangle;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.chart.XDiagram;
import com.sun.star.chart.XChartDocument;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;

import com.sun.star.document.XEmbeddedObjectSupplier;

import com.sun.star.frame.XComponentLoader;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XComponentContext;

import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheetDocument;

import com.sun.star.style.XStyleFamiliesSupplier;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableChart;
import com.sun.star.table.XTableCharts;
import com.sun.star.table.XTableChartsSupplier;


public class SCalc  {

    public static void main(String args[]) {

        //oooooooooooooooooooooooooooStep 1oooooooooooooooooooooooooooooooooooooooooo
        // call UNO bootstrap method and get the remote component context form
        // the a running office (office will be started if necessary)
        //***************************************************************************
        XComponentContext xContext = null;

        // get the remote office component context
        try {
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
        } catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooooo
        // open an empty document. In this case it's a calc document.
        // For this purpose an instance of com.sun.star.frame.Desktop
        // is created. The desktop provides the XComponentLoader interface,
        // which is used to open the document via loadComponentFromURL
        //***************************************************************************

        //Open document

        //Calc
        XSpreadsheetDocument myDoc = null;
//        XCell oCell = null;

        System.out.println("Opening an empty Calc document");
        myDoc = openCalc(xContext);

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooooo
        // create cell styles.
        // For this purpose get the StyleFamiliesSupplier and the the familiy
        // CellStyle. Create an instance of com.sun.star.style.CellStyle and
        // add it to the family. Now change some properties
        //***************************************************************************

        try {
            XStyleFamiliesSupplier xSFS = (XStyleFamiliesSupplier)
                UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, myDoc);
            XNameAccess xSF = (XNameAccess) xSFS.getStyleFamilies();
            XNameAccess xCS = (XNameAccess) UnoRuntime.queryInterface(
                XNameAccess.class, xSF.getByName("CellStyles"));
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, myDoc );
            XNameContainer oStyleFamilyNameContainer = (XNameContainer)
                UnoRuntime.queryInterface(
                XNameContainer.class, xCS);
            XInterface oInt1 = (XInterface) oDocMSF.createInstance(
                "com.sun.star.style.CellStyle");
            oStyleFamilyNameContainer.insertByName("My Style", oInt1);
            XPropertySet oCPS1 = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, oInt1 );
            oCPS1.setPropertyValue("IsCellBackgroundTransparent", new Boolean(false));
            oCPS1.setPropertyValue("CellBackColor",new Integer(6710932));
            oCPS1.setPropertyValue("CharColor",new Integer(16777215));
            XInterface oInt2 = (XInterface) oDocMSF.createInstance(
                "com.sun.star.style.CellStyle");
            oStyleFamilyNameContainer.insertByName("My Style2", oInt2);
            XPropertySet oCPS2 = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, oInt2 );
            oCPS2.setPropertyValue("IsCellBackgroundTransparent", new Boolean(false));
            oCPS2.setPropertyValue("CellBackColor",new Integer(13421823));
        } catch (Exception e) {
            e.printStackTrace(System.err);
        }

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 4oooooooooooooooooooooooooooooooooooooooooo
        // get the sheet an insert some data.
        // Get the sheets from the document and then the first from this container.
        // Now some data can be inserted. For this purpose get a Cell via
        // getCellByPosition and insert into this cell via setValue() (for floats)
        // or setFormula() for formulas and Strings
        //***************************************************************************


        XSpreadsheet xSheet=null;

        try {
            System.out.println("Getting spreadsheet") ;
            XSpreadsheets xSheets = myDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xSheets);
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, oIndexSheets.getByIndex(0));

        } catch (Exception e) {
            System.out.println("Couldn't get Sheet " +e);
            e.printStackTrace(System.err);
        }



        System.out.println("Creating the Header") ;

        insertIntoCell(1,0,"JAN",xSheet,"");
        insertIntoCell(2,0,"FEB",xSheet,"");
        insertIntoCell(3,0,"MAR",xSheet,"");
        insertIntoCell(4,0,"APR",xSheet,"");
        insertIntoCell(5,0,"MAI",xSheet,"");
        insertIntoCell(6,0,"JUN",xSheet,"");
        insertIntoCell(7,0,"JUL",xSheet,"");
        insertIntoCell(8,0,"AUG",xSheet,"");
        insertIntoCell(9,0,"SEP",xSheet,"");
        insertIntoCell(10,0,"OCT",xSheet,"");
        insertIntoCell(11,0,"NOV",xSheet,"");
        insertIntoCell(12,0,"DEC",xSheet,"");
        insertIntoCell(13,0,"SUM",xSheet,"");


        System.out.println("Fill the lines");

        insertIntoCell(0,1,"Smith",xSheet,"");
        insertIntoCell(1,1,"42",xSheet,"V");
        insertIntoCell(2,1,"58.9",xSheet,"V");
        insertIntoCell(3,1,"-66.5",xSheet,"V");
        insertIntoCell(4,1,"43.4",xSheet,"V");
        insertIntoCell(5,1,"44.5",xSheet,"V");
        insertIntoCell(6,1,"45.3",xSheet,"V");
        insertIntoCell(7,1,"-67.3",xSheet,"V");
        insertIntoCell(8,1,"30.5",xSheet,"V");
        insertIntoCell(9,1,"23.2",xSheet,"V");
        insertIntoCell(10,1,"-97.3",xSheet,"V");
        insertIntoCell(11,1,"22.4",xSheet,"V");
        insertIntoCell(12,1,"23.5",xSheet,"V");
        insertIntoCell(13,1,"=SUM(B2:M2)",xSheet,"");


        insertIntoCell(0,2,"Jones",xSheet,"");
        insertIntoCell(1,2,"21",xSheet,"V");
        insertIntoCell(2,2,"40.9",xSheet,"V");
        insertIntoCell(3,2,"-57.5",xSheet,"V");
        insertIntoCell(4,2,"-23.4",xSheet,"V");
        insertIntoCell(5,2,"34.5",xSheet,"V");
        insertIntoCell(6,2,"59.3",xSheet,"V");
        insertIntoCell(7,2,"27.3",xSheet,"V");
        insertIntoCell(8,2,"-38.5",xSheet,"V");
        insertIntoCell(9,2,"43.2",xSheet,"V");
        insertIntoCell(10,2,"57.3",xSheet,"V");
        insertIntoCell(11,2,"25.4",xSheet,"V");
        insertIntoCell(12,2,"28.5",xSheet,"V");
        insertIntoCell(13,2,"=SUM(B3:M3)",xSheet,"");

        insertIntoCell(0,3,"Brown",xSheet,"");
        insertIntoCell(1,3,"31.45",xSheet,"V");
        insertIntoCell(2,3,"-20.9",xSheet,"V");
        insertIntoCell(3,3,"-117.5",xSheet,"V");
        insertIntoCell(4,3,"23.4",xSheet,"V");
        insertIntoCell(5,3,"-114.5",xSheet,"V");
        insertIntoCell(6,3,"115.3",xSheet,"V");
        insertIntoCell(7,3,"-171.3",xSheet,"V");
        insertIntoCell(8,3,"89.5",xSheet,"V");
        insertIntoCell(9,3,"41.2",xSheet,"V");
        insertIntoCell(10,3,"71.3",xSheet,"V");
        insertIntoCell(11,3,"25.4",xSheet,"V");
        insertIntoCell(12,3,"38.5",xSheet,"V");
        insertIntoCell(13,3,"=SUM(A4:L4)",xSheet,"");

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 5oooooooooooooooooooooooooooooooooooooooooo
        // apply the created cell style.
        // For this purpose get the PropertySet of the Cell and change the
        // property CellStyle to the appropriate value.
        //***************************************************************************

        // change backcolor
        chgbColor( 1 , 0, 13, 0, "My Style", xSheet );
        chgbColor( 0 , 1, 0, 3, "My Style", xSheet );
        chgbColor( 1 , 1, 13, 3, "My Style2", xSheet );

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 6oooooooooooooooooooooooooooooooooooooooooo
        // insert a 3D chart.
        // get the CellRange which holds the data for the chart and its RangeAddress
        // get the TableChartSupplier from the sheet and then the TableCharts from it.
        // add a new chart based on the data to the TableCharts.
        // get the ChartDocument, which provide the Diagramm. Change the properties
        // Dim3D (3 dimension) and String (the title) of the diagramm.
        //***************************************************************************

        // insert a chart

        Rectangle oRect = new Rectangle();
        oRect.X = 500;
        oRect.Y = 3000;
        oRect.Width = 25000;
        oRect.Height = 11000;

        XCellRange oRange = (XCellRange)UnoRuntime.queryInterface(
            XCellRange.class, xSheet);
        XCellRange myRange = oRange.getCellRangeByName("A1:N4");
        XCellRangeAddressable oRangeAddr = (XCellRangeAddressable)
            UnoRuntime.queryInterface(XCellRangeAddressable.class, myRange);
        CellRangeAddress myAddr = oRangeAddr.getRangeAddress();

        CellRangeAddress[] oAddr = new CellRangeAddress[1];
        oAddr[0] = myAddr;
        XTableChartsSupplier oSupp = (XTableChartsSupplier)UnoRuntime.queryInterface(
            XTableChartsSupplier.class, xSheet);

        XTableChart oChart = null;

        System.out.println("Insert Chart");

        XTableCharts oCharts = oSupp.getCharts();
        oCharts.addNewByName("Example", oRect, oAddr, true, true);

        // get the diagramm and Change some of the properties

        try {
            oChart = (XTableChart) (UnoRuntime.queryInterface(
                XTableChart.class, ((XNameAccess)UnoRuntime.queryInterface(
                            XNameAccess.class, oCharts)).getByName("Example")));
            XEmbeddedObjectSupplier oEOS = (XEmbeddedObjectSupplier)
                UnoRuntime.queryInterface(XEmbeddedObjectSupplier.class, oChart);
            XInterface oInt = oEOS.getEmbeddedObject();
            XChartDocument xChart = (XChartDocument) UnoRuntime.queryInterface(
                XChartDocument.class,oInt);
            XDiagram oDiag = (XDiagram) xChart.getDiagram();
            System.out.println("Change Diagramm to 3D");
            XPropertySet oCPS = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, oDiag );
            oCPS.setPropertyValue("Dim3D", new Boolean(true));
            System.out.println("Change the title");
            Thread.sleep(200);
            XPropertySet oTPS = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xChart.getTitle() );
            oTPS.setPropertyValue("String","The new title");
            //oDiag.Dim3D();
        } catch (Exception e){
            System.err.println("Changin Properties failed "+e);
            e.printStackTrace(System.err);
        }

        System.out.println("done");
        System.exit(0);
    }

    public static XSpreadsheetDocument openCalc(XComponentContext xContext)
    {
        //define variables
        XMultiComponentFactory xMCF = null;
        XComponentLoader xCLoader;
        XSpreadsheetDocument xSpreadSheetDoc = null;
        XComponent xComp = null;

        try {
            // get the servie manager rom the office
            xMCF = xContext.getServiceManager();

            // create a new instance of the the desktop
            Object oDesktop = xMCF.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xContext );

            // query the desktop object for the XComponentLoader
            xCLoader = ( XComponentLoader ) UnoRuntime.queryInterface(
                XComponentLoader.class, oDesktop );

            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String strDoc = "private:factory/scalc";

            xComp = xCLoader.loadComponentFromURL(strDoc, "_blank", 0, szEmptyArgs );
            xSpreadSheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, xComp);

        } catch(Exception e){
            System.err.println(" Exception " + e);
            e.printStackTrace(System.err);
        }

        return xSpreadSheetDoc;
    }


    public static void insertIntoCell(int CellX, int CellY, String theValue,
                                      XSpreadsheet TT1, String flag)
    {
        XCell xCell = null;

        try {
            xCell = TT1.getCellByPosition(CellX, CellY);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.err.println("Could not get Cell");
            ex.printStackTrace(System.err);
        }

        if (flag.equals("V")) {
            xCell.setValue((new Float(theValue)).floatValue());
        } else {
            xCell.setFormula(theValue);
        }

    }

    public static void chgbColor( int x1, int y1, int x2, int y2,
                                  String template, XSpreadsheet TT )
    {
        XCellRange xCR = null;
        try {
            xCR = TT.getCellRangeByPosition(x1,y1,x2,y2);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.err.println("Could not get CellRange");
            ex.printStackTrace(System.err);
        }

        XPropertySet xCPS = (XPropertySet)UnoRuntime.queryInterface(
            XPropertySet.class, xCR );

        try {
            xCPS.setPropertyValue("CellStyle", template);
        } catch (Exception e) {
            System.err.println("Can't change colors chgbColor" + e);
            e.printStackTrace(System.err);
        }
    }

}
