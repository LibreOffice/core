/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.RuntimeException;


// __________  implementation  ____________________________________

/** Create a spreadsheet document and provide access to table contents.
 */
public class GeneralTableSample extends SpreadsheetDocHelper
{



    public static void main( String args[] )
    {
        try
        {
            GeneralTableSample aSample = new GeneralTableSample( args );
            aSample.doSampleFunction();
        }
        catch (Exception ex)
        {
            System.out.println( "Error: Sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
            System.exit( 1 );
        }

        System.out.println( "Sample done." );
        System.exit( 0 );
    }



    /// This sample function modifies cells and cell ranges.
    public void doSampleFunction() throws RuntimeException, Exception
    {
        // for common usage
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        com.sun.star.beans.XPropertySet xPropSet = null;
        com.sun.star.table.XCell xCell = null;
        com.sun.star.table.XCellRange xCellRange = null;

    // *** Access and modify a VALUE CELL ***
        System.out.println( "*** Sample for service table.Cell ***" );

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
        boolean bValid = (xCell.getError() == 0);
        // Get formula string.
        String aText = "The formula " + xCell.getFormula() + " is ";
        aText += bValid ? "valid." : "erroneous.";

    // *** Insert a TEXT CELL using the XText interface ***
        xCell = xSheet.getCellByPosition( 0, 3 );
        com.sun.star.text.XText xCellText = UnoRuntime.queryInterface( com.sun.star.text.XText.class, xCell );
        com.sun.star.text.XTextCursor xTextCursor = xCellText.createTextCursor();
        xCellText.insertString( xTextCursor, aText, false );

    // *** Change cell properties ***
        int nValue = bValid ? 0x00FF00 : 0xFF4040;
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCell );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( nValue ) );


    // *** Accessing a CELL RANGE ***
        System.out.println( "*** Sample for service table.CellRange ***" );

        // Accessing a cell range over its position.
        xCellRange = xSheet.getCellRangeByPosition( 2, 0, 3, 1 );

        // Change properties of the range.
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( 0x8080FF ) );

        // Accessing a cell range over its name.
        xCellRange = xSheet.getCellRangeByName( "C4:D5" );

        // Change properties of the range.
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( 0xFFFF80 ) );


    // *** Using the CELL CURSOR to add some data below of the filled area ***
        System.out.println( "*** Sample for service table.CellCursor ***" );

        // Create a cursor using the XSpreadsheet method createCursorByRange()
        xCellRange = xSheet.getCellRangeByName( "A1" );
        com.sun.star.sheet.XSheetCellRange xSheetCellRange = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetCellRange.class, xCellRange );

        com.sun.star.sheet.XSheetCellCursor xSheetCellCursor =
            xSheet.createCursorByRange( xSheetCellRange );
        com.sun.star.table.XCellCursor xCursor = UnoRuntime.queryInterface( com.sun.star.table.XCellCursor.class, xSheetCellCursor );

        // Move to the last filled cell.
        xCursor.gotoEnd();
        // Move one row down.
        xCursor.gotoOffset( 0, 1 );
        xCursor.getCellByPosition( 0, 0 ).setFormula( "Beyond of the last filled cell." );


    // *** Modifying COLUMNS and ROWS ***
        System.out.println( "*** Sample for services table.TableRows and table.TableColumns ***" );

        com.sun.star.table.XColumnRowRange xCRRange = UnoRuntime.queryInterface( com.sun.star.table.XColumnRowRange.class, xSheet );
        com.sun.star.table.XTableColumns xColumns = xCRRange.getColumns();
        com.sun.star.table.XTableRows xRows = xCRRange.getRows();

        // Get column C by index (interface XIndexAccess).
        Object aColumnObj = xColumns.getByIndex( 2 );
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aColumnObj );
        xPropSet.setPropertyValue( "Width", Integer.valueOf( 5000 ) );

        // Get the name of the column.
        com.sun.star.container.XNamed xNamed = UnoRuntime.queryInterface( com.sun.star.container.XNamed.class, aColumnObj );
        aText = "The name of this column is " + xNamed.getName() + ".";
        xSheet.getCellByPosition( 2, 2 ).setFormula( aText );

        // Get column D by name (interface XNameAccess).
        com.sun.star.container.XNameAccess xColumnsName = UnoRuntime.queryInterface( com.sun.star.container.XNameAccess.class, xColumns );

        aColumnObj = xColumnsName.getByName( "D" );
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aColumnObj );
        xPropSet.setPropertyValue( "IsVisible", Boolean.FALSE );

        // Get row 7 by index (interface XIndexAccess)
        Object aRowObj = xRows.getByIndex( 6 );
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aRowObj );
        xPropSet.setPropertyValue( "Height", Integer.valueOf( 5000 ) );

        xSheet.getCellByPosition( 2, 6 ).setFormula( "What a big cell." );

        // Create a cell series with the values 1 ... 7.
        for (int nRow = 8; nRow < 15; ++nRow)
            xSheet.getCellByPosition( 0, nRow ).setValue( nRow - 7 );
        // Insert a row between 1 and 2
        xRows.insertByIndex( 9, 1 );
        // Delete the rows with the values 3 and 4.
        xRows.removeByIndex( 11, 2 );

    // *** Inserting CHARTS ***
        System.out.println( "*** Sample for service table.TableCharts ***" );

        com.sun.star.table.XTableChartsSupplier xChartsSupp =
            UnoRuntime.queryInterface(
            com.sun.star.table.XTableChartsSupplier.class, xSheet );
        com.sun.star.table.XTableCharts xCharts = xChartsSupp.getCharts();

        // The chart will base on the last cell series, initializing all values.
        String aName = "newChart";
        com.sun.star.awt.Rectangle aRect = new com.sun.star.awt.Rectangle();
        aRect.X = 10000;
        aRect.Y = 3000;
        aRect.Width = aRect.Height = 5000;
        com.sun.star.table.CellRangeAddress[] aRanges = new com.sun.star.table.CellRangeAddress[1];
        aRanges[0] = createCellRangeAddress( xSheet, "A9:A14" );

        // Create the chart.
        xCharts.addNewByName( aName, aRect, aRanges, false, false );

        // Get the chart by name.
        Object aChartObj = xCharts.getByName( aName );
        com.sun.star.table.XTableChart xChart = UnoRuntime.queryInterface( com.sun.star.table.XTableChart.class, aChartObj );

        // Query the state of row and column headers.
        aText = "Chart has column headers: ";
        aText += xChart.getHasColumnHeaders() ? "yes" : "no";
        xSheet.getCellByPosition( 2, 8 ).setFormula( aText );
        aText = "Chart has row headers: ";
        aText += xChart.getHasRowHeaders() ? "yes" : "no";
        xSheet.getCellByPosition( 2, 9 ).setFormula( aText );
    }



    public GeneralTableSample( String[] args )
    {
        super( args );
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
