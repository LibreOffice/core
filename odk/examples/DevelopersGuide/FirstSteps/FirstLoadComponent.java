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
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.sheet.XCellAddressable;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

public class FirstLoadComponent {

    /** Creates a new instance of FirstLoadComponent */
    public FirstLoadComponent() {
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            // get the remote office component context
            XComponentContext xRemoteContext = Bootstrap.bootstrap();
            if (xRemoteContext == null) {
                System.err.println("ERROR: Could not bootstrap default Office.");
            }

            XMultiComponentFactory xRemoteServiceManager = xRemoteContext.getServiceManager();

            Object desktop = xRemoteServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xRemoteContext);
            XComponentLoader xComponentLoader = (XComponentLoader)
                UnoRuntime.queryInterface(XComponentLoader.class, desktop);

            PropertyValue[] loadProps = new PropertyValue[0];
            XComponent xSpreadsheetComponent = xComponentLoader.loadComponentFromURL("private:factory/scalc", "_blank", 0, loadProps);

            XSpreadsheetDocument xSpreadsheetDocument = (XSpreadsheetDocument)
                UnoRuntime.queryInterface(XSpreadsheetDocument.class,
                                          xSpreadsheetComponent);

            XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
            xSpreadsheets.insertNewByName("MySheet", (short)0);
            com.sun.star.uno.Type elemType = xSpreadsheets.getElementType();

            System.out.println(elemType.getTypeName());
            Object sheet = xSpreadsheets.getByName("MySheet");
            XSpreadsheet xSpreadsheet = (XSpreadsheet)UnoRuntime.queryInterface(
                XSpreadsheet.class, sheet);

            XCell xCell = xSpreadsheet.getCellByPosition(0, 0);
            xCell.setValue(21);
            xCell = xSpreadsheet.getCellByPosition(0, 1);
            xCell.setValue(21);
            xCell = xSpreadsheet.getCellByPosition(0, 2);
            xCell.setFormula("=sum(A1:A2)");

            XPropertySet xCellProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xCell);
            xCellProps.setPropertyValue("CellStyle", "Result");

            XModel xSpreadsheetModel = (XModel)UnoRuntime.queryInterface(
                XModel.class, xSpreadsheetComponent);
            XController xSpreadsheetController = xSpreadsheetModel.getCurrentController();
            XSpreadsheetView xSpreadsheetView = (XSpreadsheetView)
                UnoRuntime.queryInterface(XSpreadsheetView.class,
                                          xSpreadsheetController);
            xSpreadsheetView.setActiveSheet(xSpreadsheet);

            // *********************************************************
            // example for use of enum types
            xCellProps.setPropertyValue("VertJustify",
                                        com.sun.star.table.CellVertJustify.TOP);


            // *********************************************************
            // example for a sequence of PropertyValue structs
            // create an array with one PropertyValue struct, it contains
            // references only
            loadProps = new PropertyValue[1];

            // instantiate PropertyValue struct and set its member fields
            PropertyValue asTemplate = new PropertyValue();
            asTemplate.Name = "AsTemplate";
            asTemplate.Value = new Boolean(true);

            // assign PropertyValue struct to array of references for PropertyValue
            // structs
            loadProps[0] = asTemplate;

            // load calc file as template
            //xSpreadsheetComponent = xComponentLoader.loadComponentFromURL(
            //    "file:///c:/temp/DataAnalysys.ods", "_blank", 0, loadProps);

            // *********************************************************
            // example for use of XEnumerationAccess
            XCellRangesQuery xCellQuery = (XCellRangesQuery)
                UnoRuntime.queryInterface(XCellRangesQuery.class, sheet);
            XSheetCellRanges xFormulaCells = xCellQuery.queryContentCells(
                (short)com.sun.star.sheet.CellFlags.FORMULA);
            XEnumerationAccess xFormulas = xFormulaCells.getCells();
            XEnumeration xFormulaEnum = xFormulas.createEnumeration();

            while (xFormulaEnum.hasMoreElements()) {
                Object formulaCell = xFormulaEnum.nextElement();
                xCell = (XCell)UnoRuntime.queryInterface(XCell.class, formulaCell);
                XCellAddressable xCellAddress = (XCellAddressable)
                    UnoRuntime.queryInterface(XCellAddressable.class, xCell);
                System.out.println("Formula cell in column " +
                                   xCellAddress.getCellAddress().Column
                                   + ", row " + xCellAddress.getCellAddress().Row
                                   + " contains " + xCell.getFormula());
            }

        }
        catch (java.lang.Exception e){
            e.printStackTrace();
        }
        finally {
            System.exit( 0 );
        }
    }

}


// import com.sun.star.uno.UnoRuntime;
// import com.sun.star.uno.XComponentContext;
// import com.sun.star.lang.XMultiComponentFactory;
// import com.sun.star.lang.XComponent;
// import com.sun.star.beans.XPropertySet;
// import com.sun.star.beans.PropertyValue;
// import com.sun.star.sheet.XSpreadsheetDocument;
// import com.sun.star.sheet.XSpreadsheets;
// import com.sun.star.sheet.XSpreadsheet;
// import com.sun.star.sheet.XSpreadsheetView;
// import com.sun.star.sheet.XCellRangesQuery;
// import com.sun.star.sheet.XSheetCellRanges;
// import com.sun.star.sheet.XCellAddressable;
// import com.sun.star.table.XCell;
// import com.sun.star.frame.XModel;
// import com.sun.star.frame.XController;
// import com.sun.star.frame.XComponentLoader;
// import com.sun.star.container.XEnumeration;
// import com.sun.star.container.XEnumerationAccess;

// import com.sun.star.uno.AnyConverter;

// public class FirstLoadComponent {

//     /** Creates a new instance of FirstLoadComponent */
//     public FirstLoadComponent() {
//     }

//     /**
//      * @param args the command line arguments
//      */
//     private XComponentContext xRemoteContext = null;
//     private XMultiComponentFactory xRemoteServiceManager = null;

//     public static void main(String[] args) {
//         FirstLoadComponent firstLoadComponent1 = new FirstLoadComponent();
//         try {
//             firstLoadComponent1.useConnection();
//         }
//         catch (java.lang.Exception e){
//             System.out.println(e.getMessage());
//             e.printStackTrace();
//         }
//         finally {
//             System.exit(0);
//         }
//     }

//     private void useConnection() throws java.lang.Exception {
//         try {
//             // get the remote office component context
//             xRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
//             System.out.println("Connected to a running office ...");

//             xRemoteServiceManager = xRemoteContext.getServiceManager();
//         }
//         catch( Exception e) {
//             e.printStackTrace();
//             System.exit(1);
//         }

//         try {
//             Object desktop = xRemoteServiceManager.createInstanceWithContext(
//                 "com.sun.star.frame.Desktop", xRemoteContext);
//             XComponentLoader xComponentLoader = (XComponentLoader)
//                 UnoRuntime.queryInterface(XComponentLoader.class, desktop);

//             PropertyValue[] loadProps = new PropertyValue[0];
//             XComponent xSpreadsheetComponent = xComponentLoader.loadComponentFromURL("private:factory/scalc", "_blank", 0, loadProps);

//             XSpreadsheetDocument xSpreadsheetDocument = (XSpreadsheetDocument)
//                 UnoRuntime.queryInterface(XSpreadsheetDocument.class,
//                                           xSpreadsheetComponent);

//             XSpreadsheets xSpreadsheets = xSpreadsheetDocument.getSheets();
//             xSpreadsheets.insertNewByName("MySheet", (short)0);
//             com.sun.star.uno.Type elemType = xSpreadsheets.getElementType();

//             System.out.println(elemType.getTypeName());
//             Object sheet = xSpreadsheets.getByName("MySheet");
//             XSpreadsheet xSpreadsheet = (XSpreadsheet)UnoRuntime.queryInterface(
//                 XSpreadsheet.class, sheet);

//             XCell xCell = xSpreadsheet.getCellByPosition(0, 0);
//             xCell.setValue(21);
//             xCell = xSpreadsheet.getCellByPosition(0, 1);
//             xCell.setValue(21);
//             xCell = xSpreadsheet.getCellByPosition(0, 2);
//             xCell.setFormula("=sum(A1:A2)");

//             XPropertySet xCellProps = (XPropertySet)UnoRuntime.queryInterface(
//                 XPropertySet.class, xCell);
//             xCellProps.setPropertyValue("CellStyle", "Result");

//             XModel xSpreadsheetModel = (XModel)UnoRuntime.queryInterface(
//                 XModel.class, xSpreadsheetComponent);
//             XController xSpreadsheetController = xSpreadsheetModel.getCurrentController();
//             XSpreadsheetView xSpreadsheetView = (XSpreadsheetView)
//                 UnoRuntime.queryInterface(XSpreadsheetView.class,
//                                           xSpreadsheetController);
//             xSpreadsheetView.setActiveSheet(xSpreadsheet);

//             // *********************************************************
//             // example for use of enum types
//             xCellProps.setPropertyValue("VertJustify",
//                                         com.sun.star.table.CellVertJustify.TOP);


//             // *********************************************************
//             // example for a sequence of PropertyValue structs
//             // create an array with one PropertyValue struct, it contains
//             // references only
//             loadProps = new PropertyValue[1];

//             // instantiate PropertyValue struct and set its member fields
//             PropertyValue asTemplate = new PropertyValue();
//             asTemplate.Name = "AsTemplate";
//             asTemplate.Value = new Boolean(true);

//             // assign PropertyValue struct to array of references for PropertyValue
//             // structs
//             loadProps[0] = asTemplate;

//             // load calc file as template
//             //xSpreadsheetComponent = xComponentLoader.loadComponentFromURL(
//             //    "file:///c:/temp/DataAnalysys.ods", "_blank", 0, loadProps);

//             // *********************************************************
//             // example for use of XEnumerationAccess
//             XCellRangesQuery xCellQuery = (XCellRangesQuery)
//                 UnoRuntime.queryInterface(XCellRangesQuery.class, sheet);
//             XSheetCellRanges xFormulaCells = xCellQuery.queryContentCells(
//                 (short)com.sun.star.sheet.CellFlags.FORMULA);
//             XEnumerationAccess xFormulas = xFormulaCells.getCells();
//             XEnumeration xFormulaEnum = xFormulas.createEnumeration();

//             while (xFormulaEnum.hasMoreElements()) {
//                 Object formulaCell = xFormulaEnum.nextElement();
//                 xCell = (XCell)UnoRuntime.queryInterface(XCell.class, formulaCell);
//                 XCellAddressable xCellAddress = (XCellAddressable)
//                     UnoRuntime.queryInterface(XCellAddressable.class, xCell);
//                 System.out.println("Formula cell in column " +
//                                    xCellAddress.getCellAddress().Column
//                                    + ", row " + xCellAddress.getCellAddress().Row
//                                    + " contains " + xCell.getFormula());
//             }

//         }
//         catch( com.sun.star.lang.DisposedException e ) { //works from Patch 1
//             xRemoteContext = null;
//             throw e;
//         }
//     }
// }
