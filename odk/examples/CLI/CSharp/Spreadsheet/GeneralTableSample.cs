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

using System;

// __________  implementation  ____________________________________

/** Create a spreadsheet document and provide access to table contents.
 */
public class GeneralTableSample : SpreadsheetDocHelper
{

    public static void Main( String [] args )
    {
        try
        {
            using ( GeneralTableSample aSample =
                      new GeneralTableSample( args ) )
            {
                aSample.doSampleFunction();
            }
            Console.WriteLine( "Sample done." );
        }
        catch (Exception ex)
        {
            Console.WriteLine( "Sample caught exception! " + ex );
        }
    }


    
    public GeneralTableSample( String[] args ) : base( args )
    {
    }



    /// This sample function modifies cells and cell ranges.
    public void doSampleFunction()
    {
        // for common usage
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        unoidl.com.sun.star.beans.XPropertySet xPropSet = null;
        unoidl.com.sun.star.table.XCell xCell = null;
        unoidl.com.sun.star.table.XCellRange xCellRange = null;

        // *** Access and modify a VALUE CELL ***
        Console.WriteLine( "*** Sample for service table.Cell ***" );

        xCell = xSheet.getCellByPosition( 0, 0 );
        // Set cell value.
        xCell.setValue( 1234 );

        // Get cell value.
        double nDblValue = xCell.getValue() * 2;
        xSheet.getCellByPosition( 0, 1 ).setValue( nDblValue );

        // *** Create a FORMULA CELL and query error type ***
        xCell = xSheet.getCellByPosition( 0, 2 );
        // Set formula string.
        xCell.setFormula( "=1/0" );

        // Get error type.
        bool bValid = (xCell.getError() == 0);
        // Get formula string.
        String aText = "The formula " + xCell.getFormula() + " is ";
        aText += bValid ? "valid." : "erroneous.";

        // *** Insert a TEXT CELL using the XText interface ***
        xCell = xSheet.getCellByPosition( 0, 3 );
        unoidl.com.sun.star.text.XText xCellText =
            (unoidl.com.sun.star.text.XText) xCell;
        unoidl.com.sun.star.text.XTextCursor xTextCursor =
            xCellText.createTextCursor();
        xCellText.insertString( xTextCursor, aText, false );

        // *** Change cell properties ***
        int nValue = bValid ? 0x00FF00 : 0xFF4040;
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCell;
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) nValue ) );


        // *** Accessing a CELL RANGE ***
        Console.WriteLine( "*** Sample for service table.CellRange ***" );

        // Accessing a cell range over its position.
        xCellRange = xSheet.getCellRangeByPosition( 2, 0, 3, 1 );

        // Change properties of the range.
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0x8080FF ) );

        // Accessing a cell range over its name.
        xCellRange = xSheet.getCellRangeByName( "C4:D5" );

        // Change properties of the range.
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0xFFFF80 ) );


        // *** Using the CELL CURSOR to add some data below of
        // the filled area ***
        Console.WriteLine( "*** Sample for service table.CellCursor ***" );

        // Create a cursor using the XSpreadsheet method createCursorByRange()
        xCellRange = xSheet.getCellRangeByName( "A1" );
        unoidl.com.sun.star.sheet.XSheetCellRange xSheetCellRange =
            (unoidl.com.sun.star.sheet.XSheetCellRange) xCellRange;

        unoidl.com.sun.star.sheet.XSheetCellCursor xSheetCellCursor =
            xSheet.createCursorByRange( xSheetCellRange );
        unoidl.com.sun.star.table.XCellCursor xCursor =
            (unoidl.com.sun.star.table.XCellCursor) xSheetCellCursor;

        // Move to the last filled cell.
        xCursor.gotoEnd();
        // Move one row down.
        xCursor.gotoOffset( 0, 1 );
        xCursor.getCellByPosition( 0, 0 ).setFormula(
            "Beyond of the last filled cell." );


        // *** Modifying COLUMNS and ROWS ***
        Console.WriteLine( "*** Sample for services table.TableRows and " +
                           "table.TableColumns ***" );

        unoidl.com.sun.star.table.XColumnRowRange xCRRange =
            (unoidl.com.sun.star.table.XColumnRowRange) xSheet;
        unoidl.com.sun.star.table.XTableColumns xColumns =
            xCRRange.getColumns();
        unoidl.com.sun.star.table.XTableRows xRows = xCRRange.getRows();

        // Get column C by index (interface XIndexAccess).
        uno.Any aColumnObj = xColumns.getByIndex( 2 );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aColumnObj.Value;
        xPropSet.setPropertyValue( "Width", new uno.Any( (Int32) 5000 ) );

        // Get the name of the column.
        unoidl.com.sun.star.container.XNamed xNamed =
            (unoidl.com.sun.star.container.XNamed) aColumnObj.Value;
        aText = "The name of this column is " + xNamed.getName() + ".";
        xSheet.getCellByPosition( 2, 2 ).setFormula( aText );

        // Get column D by name (interface XNameAccess).
        unoidl.com.sun.star.container.XNameAccess xColumnsName =
            (unoidl.com.sun.star.container.XNameAccess) xColumns;

        aColumnObj = xColumnsName.getByName( "D" );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aColumnObj.Value;
        xPropSet.setPropertyValue(
            "IsVisible", new uno.Any( (Boolean) false ) );

        // Get row 7 by index (interface XIndexAccess)
        uno.Any aRowObj = xRows.getByIndex( 6 );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aRowObj.Value;
        xPropSet.setPropertyValue( "Height", new uno.Any( (Int32) 5000 ) );

        xSheet.getCellByPosition( 2, 6 ).setFormula( "What a big cell." );

        // Create a cell series with the values 1 ... 7.
        for (int nRow = 8; nRow < 15; ++nRow)
            xSheet.getCellByPosition( 0, nRow ).setValue( nRow - 7 );
        // Insert a row between 1 and 2
        xRows.insertByIndex( 9, 1 );
        // Delete the rows with the values 3 and 4.
        xRows.removeByIndex( 11, 2 );

        // *** Inserting CHARTS ***
        Console.WriteLine( "*** Sample for service table.TableCharts ***" );

        unoidl.com.sun.star.table.XTableChartsSupplier xChartsSupp =
            (unoidl.com.sun.star.table.XTableChartsSupplier) xSheet;
        unoidl.com.sun.star.table.XTableCharts xCharts =
            xChartsSupp.getCharts();

        // The chart will base on the last cell series, initializing all values.
        String aName = "newChart";
        unoidl.com.sun.star.awt.Rectangle aRect =
            new unoidl.com.sun.star.awt.Rectangle();
        aRect.X = 10000;
        aRect.Y = 3000;
        aRect.Width = aRect.Height = 5000;
        unoidl.com.sun.star.table.CellRangeAddress[] aRanges =
            new unoidl.com.sun.star.table.CellRangeAddress[1];
        aRanges[0] = createCellRangeAddress( xSheet, "A9:A14" );

        // Create the chart.
        xCharts.addNewByName( aName, aRect, aRanges, false, false );

        // Get the chart by name.
        uno.Any aChartObj = xCharts.getByName( aName );
        unoidl.com.sun.star.table.XTableChart xChart =
            (unoidl.com.sun.star.table.XTableChart) aChartObj.Value;

        // Query the state of row and column headers.
        aText = "Chart has column headers: ";
        aText += xChart.getHasColumnHeaders() ? "yes" : "no";
        xSheet.getCellByPosition( 2, 8 ).setFormula( aText );
        aText = "Chart has row headers: ";
        aText += xChart.getHasRowHeaders() ? "yes" : "no";
        xSheet.getCellByPosition( 2, 9 ).setFormula( aText );
    }

}
