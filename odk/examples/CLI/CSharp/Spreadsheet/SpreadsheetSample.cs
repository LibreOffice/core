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

/** Create and modify a spreadsheet document.
 */
public class SpreadsheetSample : SpreadsheetDocHelper
{

    public static void Main( String [] args )
    {
        try
        {
            using ( SpreadsheetSample aSample = new SpreadsheetSample( args ) )
            {
                aSample.doSampleFunctions();
            }
            Console.WriteLine( "\nSamples done." );
        }
        catch (Exception ex)
        {
            Console.WriteLine( "Sample caught exception! " + ex );
        }
    }

    public SpreadsheetSample( String[] args )
        : base( args )
    {
    }
    
    /** This sample function performs all changes on the document. */
    public void doSampleFunctions()
    {
        doCellSamples();
        doCellRangeSamples();
        doCellRangesSamples();
        doCellCursorSamples();
        doFormattingSamples();
        doDocumentSamples();
        doDatabaseSamples();
        doDataPilotSamples();
        doNamedRangesSamples();
        doFunctionAccessSamples();
        doApplicationSettingsSamples();
    }



    /** All samples regarding the service com.sun.star.sheet.SheetCell. */
    private void doCellSamples()
    {
        Console.WriteLine( "\n*** Samples for service sheet.SheetCell ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        unoidl.com.sun.star.table.XCell xCell = null;
        unoidl.com.sun.star.beans.XPropertySet xPropSet = null;
        String aText;
        prepareRange( xSheet, "A1:C7", "Cells and Cell Ranges" );

        // --- Get cell B3 by position - (column, row) ---
        xCell = xSheet.getCellByPosition( 1, 2 );

        // --- Insert two text paragraphs into the cell. ---
        unoidl.com.sun.star.text.XText xText =
            (unoidl.com.sun.star.text.XText) xCell;
        unoidl.com.sun.star.text.XTextCursor xTextCursor =
            xText.createTextCursor();

        xText.insertString( xTextCursor, "Text in first line.", false );
        xText.insertControlCharacter( xTextCursor,
            unoidl.com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );
        xText.insertString( xTextCursor, "And a ", false );

        // create a hyperlink
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceMan =
            (unoidl.com.sun.star.lang.XMultiServiceFactory) getDocument();
        Object aHyperlinkObj =
            xServiceMan.createInstance( "com.sun.star.text.TextField.URL" );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aHyperlinkObj;
        xPropSet.setPropertyValue(
            "URL", new uno.Any( "http://www.example.org" ) );
        xPropSet.setPropertyValue(
            "Representation", new uno.Any( "hyperlink" ) );
        // ... and insert
        unoidl.com.sun.star.text.XTextContent xContent =
            (unoidl.com.sun.star.text.XTextContent) aHyperlinkObj;
        xText.insertTextContent( xTextCursor, xContent, false );
        
        // --- Query the separate paragraphs. ---
        unoidl.com.sun.star.container.XEnumerationAccess xParaEA =
            (unoidl.com.sun.star.container.XEnumerationAccess) xCell;
        unoidl.com.sun.star.container.XEnumeration xParaEnum =
            xParaEA.createEnumeration();
        // Go through the paragraphs
        while( xParaEnum.hasMoreElements() )
        {
            uno.Any aPortionObj = xParaEnum.nextElement();
            unoidl.com.sun.star.container.XEnumerationAccess xPortionEA =
                (unoidl.com.sun.star.container.XEnumerationAccess)
                aPortionObj.Value;
            unoidl.com.sun.star.container.XEnumeration xPortionEnum =
                xPortionEA.createEnumeration();
            aText = "";
            // Go through all text portions of a paragraph and construct string.
            while( xPortionEnum.hasMoreElements() )
            {
                unoidl.com.sun.star.text.XTextRange xRange =
                    (unoidl.com.sun.star.text.XTextRange)
                    xPortionEnum.nextElement().Value;
                aText += xRange.getString();
            }
            Console.WriteLine( "Paragraph text: " + aText );
        }
        
        
        // --- Change cell properties. ---
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCell;
        // from styles.CharacterProperties
        xPropSet.setPropertyValue(
            "CharColor", new uno.Any( (Int32) 0x003399 ) );
        xPropSet.setPropertyValue(
            "CharHeight", new uno.Any( (Single) 20.0 ) );
        // from styles.ParagraphProperties
        xPropSet.setPropertyValue(
            "ParaLeftMargin", new uno.Any( (Int32) 500 ) );
        // from table.CellProperties
        xPropSet.setPropertyValue(
            "IsCellBackgroundTransparent", new uno.Any( false ) );
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0x99CCFF ) );
        
        
        // --- Get cell address. ---
        unoidl.com.sun.star.sheet.XCellAddressable xCellAddr =
            (unoidl.com.sun.star.sheet.XCellAddressable) xCell;
        unoidl.com.sun.star.table.CellAddress aAddress =
            xCellAddr.getCellAddress();
        aText = "Address of this cell:  Column=" + aAddress.Column;
        aText += ";  Row=" + aAddress.Row;
        aText += ";  Sheet=" + aAddress.Sheet;
        Console.WriteLine( aText );
    
    
        // --- Insert an annotation ---
        unoidl.com.sun.star.sheet.XSheetAnnotationsSupplier xAnnotationsSupp =
        (unoidl.com.sun.star.sheet.XSheetAnnotationsSupplier) xSheet;
        unoidl.com.sun.star.sheet.XSheetAnnotations xAnnotations =
            xAnnotationsSupp.getAnnotations();
        xAnnotations.insertNew( aAddress, "This is an annotation" );

        unoidl.com.sun.star.sheet.XSheetAnnotationAnchor xAnnotAnchor =
            (unoidl.com.sun.star.sheet.XSheetAnnotationAnchor) xCell;
        unoidl.com.sun.star.sheet.XSheetAnnotation xAnnotation =
            xAnnotAnchor.getAnnotation();
        xAnnotation.setIsVisible( true );
    }



    /** All samples regarding the service com.sun.star.sheet.SheetCellRange. */
    private void doCellRangeSamples()
    {
        Console.WriteLine(
            "\n*** Samples for service sheet.SheetCellRange ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        unoidl.com.sun.star.table.XCellRange xCellRange = null;
        unoidl.com.sun.star.beans.XPropertySet xPropSet = null;
        unoidl.com.sun.star.table.CellRangeAddress aRangeAddress = null;

        // Preparation
        setFormula( xSheet, "B5", "First cell" );
        setFormula( xSheet, "B6", "Second cell" );
        // Get cell range B5:B6 by position - (column, row, column, row)
        xCellRange = xSheet.getCellRangeByPosition( 1, 4, 1, 5 );


        // --- Change cell range properties. ---
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        // from com.sun.star.styles.CharacterProperties
        xPropSet.setPropertyValue(
            "CharColor", new uno.Any( (Int32) 0x003399 ) );
        xPropSet.setPropertyValue(
            "CharHeight", new uno.Any( (Single) 20.0 ) );
        // from com.sun.star.styles.ParagraphProperties
        xPropSet.setPropertyValue(
            "ParaLeftMargin", new uno.Any( (Int32) 500 ) );
        // from com.sun.star.table.CellProperties
        xPropSet.setPropertyValue(
            "IsCellBackgroundTransparent", new uno.Any( false ) );
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0x99CCFF ) );


        // --- Replace text in all cells. ---
        unoidl.com.sun.star.util.XReplaceable xReplace =
            (unoidl.com.sun.star.util.XReplaceable) xCellRange;
        unoidl.com.sun.star.util.XReplaceDescriptor xReplaceDesc =
            xReplace.createReplaceDescriptor();
        xReplaceDesc.setSearchString( "cell" );
        xReplaceDesc.setReplaceString( "text" );
        // property SearchWords searches for whole cells!
        xReplaceDesc.setPropertyValue( "SearchWords", new uno.Any( false ) );
        int nCount = xReplace.replaceAll( xReplaceDesc );
        Console.WriteLine( "Search text replaced " + nCount + " times." );


        // --- Merge cells. ---
        xCellRange = xSheet.getCellRangeByName( "F3:G6" );
        prepareRange( xSheet, "E1:H7", "XMergeable" );
        unoidl.com.sun.star.util.XMergeable xMerge =
            (unoidl.com.sun.star.util.XMergeable) xCellRange;
        xMerge.merge( true );


        // --- Change indentation. ---
/* does not work (bug in XIndent implementation)
        prepareRange( xSheet, "I20:I23", "XIndent" );
        setValue( xSheet, "I21", 1 );
        setValue( xSheet, "I22", 1 );
        setValue( xSheet, "I23", 1 );

        xCellRange = xSheet.getCellRangeByName( "I21:I22" );
        unoidl.com.sun.star.util.XIndent xIndent =
        (unoidl.com.sun.star.util.XIndent) xCellRange;
        xIndent.incrementIndent();

        xCellRange = xSheet.getCellRangeByName( "I22:I23" );
        xIndent = (unoidl.com.sun.star.util.XIndent) xCellRange;
        xIndent.incrementIndent();
*/


        // --- Column properties. ---
        xCellRange = xSheet.getCellRangeByName( "B1" );
        unoidl.com.sun.star.table.XColumnRowRange xColRowRange =
            (unoidl.com.sun.star.table.XColumnRowRange) xCellRange;
        unoidl.com.sun.star.table.XTableColumns xColumns =
            xColRowRange.getColumns();

        uno.Any aColumnObj = xColumns.getByIndex( 0 );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aColumnObj.Value;
        xPropSet.setPropertyValue( "Width", new uno.Any( (Int32) 6000 ) );

        unoidl.com.sun.star.container.XNamed xNamed =
            (unoidl.com.sun.star.container.XNamed) aColumnObj.Value;
        Console.WriteLine(
            "The name of the wide column is " + xNamed.getName() + "." );


        // --- Cell range data ---
        prepareRange( xSheet, "A9:C30", "XCellRangeData" );

        xCellRange = xSheet.getCellRangeByName( "A10:C30" );
        unoidl.com.sun.star.sheet.XCellRangeData xData =
            (unoidl.com.sun.star.sheet.XCellRangeData) xCellRange;
        uno.Any [][] aValues =
        {
            new uno.Any [] { new uno.Any( "Name" ),
                             new uno.Any( "Fruit" ),
                             new uno.Any( "Quantity" ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 7.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 9.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 5.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 6.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 2.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 7.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 7.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 9.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Oranges" ),
                             new uno.Any( (Double) 4.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( "Apples" ),
                             new uno.Any( (Double) 9.0 ) }
        };
        xData.setDataArray( aValues );


        // --- Get cell range address. ---
        unoidl.com.sun.star.sheet.XCellRangeAddressable xRangeAddr =
            (unoidl.com.sun.star.sheet.XCellRangeAddressable) xCellRange;
        aRangeAddress = xRangeAddr.getRangeAddress();
        Console.WriteLine(
            "Address of this range:  Sheet=" + aRangeAddress.Sheet );
        Console.WriteLine(
            "Start column=" + aRangeAddress.StartColumn + ";  Start row=" +
            aRangeAddress.StartRow );
        Console.WriteLine(
            "End column  =" + aRangeAddress.EndColumn   + ";  End row  =" +
            aRangeAddress.EndRow );


        // --- Sheet operation. ---
        // uses the range filled with XCellRangeData
        unoidl.com.sun.star.sheet.XSheetOperation xSheetOp =
            (unoidl.com.sun.star.sheet.XSheetOperation) xData;
        double fResult = xSheetOp.computeFunction(
            unoidl.com.sun.star.sheet.GeneralFunction.AVERAGE );
        Console.WriteLine(
            "Average value of the data table A10:C30: " + fResult );


        // --- Fill series ---
        // Prepare the example
        setValue( xSheet, "E10", 1 );
        setValue( xSheet, "E11", 4 );
        setDate( xSheet, "E12", 30, 1, 2002 );
        setFormula( xSheet, "I13", "Text 10" );
        setFormula( xSheet, "E14", "Jan" );
        setValue( xSheet, "K14", 10 );
        setValue( xSheet, "E16", 1 );
        setValue( xSheet, "F16", 2 );
        setDate( xSheet, "E17", 28, 2, 2002 );
        setDate( xSheet, "F17", 28, 1, 2002 );
        setValue( xSheet, "E18", 6 );
        setValue( xSheet, "F18", 4 );

        unoidl.com.sun.star.sheet.XCellSeries xSeries = null;
        // Fill 2 rows linear with end value
        // -> 2nd series is not filled completely
        xSeries = getCellSeries( xSheet, "E10:I11" );
        xSeries.fillSeries(
            unoidl.com.sun.star.sheet.FillDirection.TO_RIGHT,
            unoidl.com.sun.star.sheet.FillMode.LINEAR,
            unoidl.com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 9 );
        // Add months to a date
        xSeries = getCellSeries( xSheet, "E12:I12" );
        xSeries.fillSeries(
            unoidl.com.sun.star.sheet.FillDirection.TO_RIGHT,
            unoidl.com.sun.star.sheet.FillMode.DATE,
            unoidl.com.sun.star.sheet.FillDateMode.FILL_DATE_MONTH,
            1, 0x7FFFFFFF );
        // Fill right to left with a text containing a value
        xSeries = getCellSeries( xSheet, "E13:I13" );
        xSeries.fillSeries(
            unoidl.com.sun.star.sheet.FillDirection.TO_LEFT,
            unoidl.com.sun.star.sheet.FillMode.LINEAR,
            unoidl.com.sun.star.sheet.FillDateMode.FILL_DATE_DAY,
            10, 0x7FFFFFFF );
        // Fill with an user defined list
        xSeries = getCellSeries( xSheet, "E14:I14" );
        xSeries.fillSeries(
            unoidl.com.sun.star.sheet.FillDirection.TO_RIGHT,
            unoidl.com.sun.star.sheet.FillMode.AUTO,
            unoidl.com.sun.star.sheet.FillDateMode.FILL_DATE_DAY,
            1, 0x7FFFFFFF );
        // Fill bottom to top with a geometric series
        xSeries = getCellSeries( xSheet, "K10:K14" );
        xSeries.fillSeries(
            unoidl.com.sun.star.sheet.FillDirection.TO_TOP,
            unoidl.com.sun.star.sheet.FillMode.GROWTH,
            unoidl.com.sun.star.sheet.FillDateMode.FILL_DATE_DAY,
            2, 0x7FFFFFFF );
        // Auto fill
        xSeries = getCellSeries( xSheet, "E16:K18" );
        xSeries.fillAuto(
            unoidl.com.sun.star.sheet.FillDirection.TO_RIGHT, 2 );
        // Fill series copies cell formats -> draw border here
        prepareRange( xSheet, "E9:K18", "XCellSeries" );


        // --- Array formulas ---
        xCellRange = xSheet.getCellRangeByName( "E21:G23" );
        prepareRange( xSheet, "E20:G23", "XArrayFormulaRange" );
        unoidl.com.sun.star.sheet.XArrayFormulaRange xArrayFormula =
            (unoidl.com.sun.star.sheet.XArrayFormulaRange) xCellRange;
        // Insert a 3x3 unit matrix.
        xArrayFormula.setArrayFormula( "=A10:C12" );
        Console.WriteLine(
            "Array formula is: " + xArrayFormula.getArrayFormula() );


        // --- Multiple operations ---
        setFormula( xSheet, "E26", "=E27^F26" );
        setValue( xSheet, "E27", 1 );
        setValue( xSheet, "F26", 1 );
        getCellSeries( xSheet, "E27:E31" ).fillAuto(
            unoidl.com.sun.star.sheet.FillDirection.TO_BOTTOM, 1 );
        getCellSeries( xSheet, "F26:J26" ).fillAuto(
            unoidl.com.sun.star.sheet.FillDirection.TO_RIGHT, 1 );
        setFormula( xSheet, "F33", "=SIN(E33)" );
        setFormula( xSheet, "G33", "=COS(E33)" );
        setFormula( xSheet, "H33", "=TAN(E33)" );
        setValue( xSheet, "E34", 0 );
        setValue( xSheet, "E35", 0.2 );
        getCellSeries( xSheet, "E34:E38" ).fillAuto(
            unoidl.com.sun.star.sheet.FillDirection.TO_BOTTOM, 2 );
        prepareRange( xSheet, "E25:J38", "XMultipleOperation" );

        unoidl.com.sun.star.table.CellRangeAddress aFormulaRange =
            createCellRangeAddress( xSheet, "E26" );
        unoidl.com.sun.star.table.CellAddress aColCell =
            createCellAddress( xSheet, "E27" );
        unoidl.com.sun.star.table.CellAddress aRowCell =
            createCellAddress( xSheet, "F26" );

        xCellRange = xSheet.getCellRangeByName( "E26:J31" );
        unoidl.com.sun.star.sheet.XMultipleOperation xMultOp =
            (unoidl.com.sun.star.sheet.XMultipleOperation) xCellRange;
        xMultOp.setTableOperation(
            aFormulaRange, unoidl.com.sun.star.sheet.TableOperationMode.BOTH,
            aColCell, aRowCell );

        aFormulaRange = createCellRangeAddress( xSheet, "F33:H33" );
        aColCell = createCellAddress( xSheet, "E33" );
        // Row cell not needed

        xCellRange = xSheet.getCellRangeByName( "E34:H38" );
        xMultOp = (unoidl.com.sun.star.sheet.XMultipleOperation) xCellRange;
        xMultOp.setTableOperation(
            aFormulaRange, unoidl.com.sun.star.sheet.TableOperationMode.COLUMN,
            aColCell, aRowCell );


        // --- Cell Ranges Query ---
        xCellRange = xSheet.getCellRangeByName( "A10:C30" );
        unoidl.com.sun.star.sheet.XCellRangesQuery xRangesQuery =
            (unoidl.com.sun.star.sheet.XCellRangesQuery) xCellRange;
        unoidl.com.sun.star.sheet.XSheetCellRanges xCellRanges =
            xRangesQuery.queryContentCells(
                (short) unoidl.com.sun.star.sheet.CellFlags.STRING );
        Console.WriteLine(
            "Cells in A10:C30 containing text: "
            + xCellRanges.getRangeAddressesAsString() );
    }

    /** Returns the XCellSeries interface of a cell range.
        @param xSheet  The spreadsheet containing the cell range.
        @param aRange  The address of the cell range.
        @return  The XCellSeries interface. */
    private unoidl.com.sun.star.sheet.XCellSeries getCellSeries(
            unoidl.com.sun.star.sheet.XSpreadsheet xSheet, String aRange )
    {
        return (unoidl.com.sun.star.sheet.XCellSeries)
            xSheet.getCellRangeByName( aRange );
    }



    /** All samples regarding cell range collections. */
    private void doCellRangesSamples()
    {
        Console.WriteLine( "\n*** Samples for cell range collections ***\n" );

        // Create a new cell range container
        unoidl.com.sun.star.lang.XMultiServiceFactory xDocFactory =
            (unoidl.com.sun.star.lang.XMultiServiceFactory) getDocument();
        unoidl.com.sun.star.sheet.XSheetCellRangeContainer xRangeCont =
            (unoidl.com.sun.star.sheet.XSheetCellRangeContainer)
            xDocFactory.createInstance(
                "com.sun.star.sheet.SheetCellRanges" );


        // --- Insert ranges ---
        insertRange( xRangeCont, 0, 0, 0, 0, 0, false );    // A1:A1
        insertRange( xRangeCont, 0, 0, 1, 0, 2, true );     // A2:A3
        insertRange( xRangeCont, 0, 1, 0, 1, 2, false );    // B1:B3


        // --- Query the list of filled cells ---
        Console.WriteLine( "All filled cells: " );
        unoidl.com.sun.star.container.XEnumerationAccess xCellsEA =
            xRangeCont.getCells();
        unoidl.com.sun.star.container.XEnumeration xEnum =
            xCellsEA.createEnumeration();
        while( xEnum.hasMoreElements() )
        {
            uno.Any aCellObj = xEnum.nextElement();
            unoidl.com.sun.star.sheet.XCellAddressable xAddr =
                (unoidl.com.sun.star.sheet.XCellAddressable) aCellObj.Value;
            unoidl.com.sun.star.table.CellAddress aAddr =
                xAddr.getCellAddress();
            Console.WriteLine(
                getCellAddressString( aAddr.Column, aAddr.Row ) + " " );
        }
        Console.WriteLine();
    }

    /** Inserts a cell range address into a cell range container and prints
        a message.
        @param xContainer  unoidl.com.sun.star.sheet.XSheetCellRangeContainer
                           interface of the container.
        @param nSheet  Index of sheet of the range.
        @param nStartCol  Index of first column of the range.
        @param nStartRow  Index of first row of the range.
        @param nEndCol  Index of last column of the range.
        @param nEndRow  Index of last row of the range.
        @param bMerge  Determines whether the new range should be merged
                       with the existing ranges.
    */
    private void insertRange(
            unoidl.com.sun.star.sheet.XSheetCellRangeContainer xContainer,
            int nSheet, int nStartCol, int nStartRow, int nEndCol, int nEndRow,
            bool bMerge )
    {
        unoidl.com.sun.star.table.CellRangeAddress aAddress =
            new unoidl.com.sun.star.table.CellRangeAddress();
        aAddress.Sheet = (short)nSheet;
        aAddress.StartColumn = nStartCol;
        aAddress.StartRow = nStartRow;
        aAddress.EndColumn = nEndCol;
        aAddress.EndRow = nEndRow;
        xContainer.addRangeAddress( aAddress, bMerge );
        Console.WriteLine(
            "Inserting " + getCellRangeAddressString( aAddress )
            + " " + (bMerge ? "   with" : "without") + " merge,"
            + " resulting list: " + xContainer.getRangeAddressesAsString() );
    }



    /** All samples regarding cell cursors. */
    private void doCellCursorSamples()
    {
        Console.WriteLine( "\n*** Samples for cell cursor ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );


        // --- Find the array formula using a cell cursor ---
        unoidl.com.sun.star.table.XCellRange xRange =
            xSheet.getCellRangeByName( "F22" );
        unoidl.com.sun.star.sheet.XSheetCellRange xCellRange =
            (unoidl.com.sun.star.sheet.XSheetCellRange) xRange;
        unoidl.com.sun.star.sheet.XSheetCellCursor xCursor =
            xSheet.createCursorByRange( xCellRange );

        xCursor.collapseToCurrentArray();
        unoidl.com.sun.star.sheet.XArrayFormulaRange xArray =
            (unoidl.com.sun.star.sheet.XArrayFormulaRange) xCursor;
        Console.WriteLine(
            "Array formula in " + getCellRangeAddressString( xCursor, false )
            + " contains formula " + xArray.getArrayFormula() );


        // --- Find the used area ---
        unoidl.com.sun.star.sheet.XUsedAreaCursor xUsedCursor =
            (unoidl.com.sun.star.sheet.XUsedAreaCursor) xCursor;
        xUsedCursor.gotoStartOfUsedArea( false );
        xUsedCursor.gotoEndOfUsedArea( true );
        // xUsedCursor and xCursor are interfaces of the same object -
        // so modifying xUsedCursor takes effect on xCursor:
        Console.WriteLine(
            "The used area is: " + getCellRangeAddressString( xCursor, true ) );
    }



    /** All samples regarding the formatting of cells and ranges. */
    private void doFormattingSamples()
    {
        Console.WriteLine( "\n*** Formatting samples ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 1 );
        unoidl.com.sun.star.table.XCellRange xCellRange;
        unoidl.com.sun.star.beans.XPropertySet xPropSet = null;
        unoidl.com.sun.star.container.XIndexAccess xRangeIA = null;
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceManager;


        // --- Cell styles ---
        // get the cell style container
        unoidl.com.sun.star.style.XStyleFamiliesSupplier xFamiliesSupplier =
            (unoidl.com.sun.star.style.XStyleFamiliesSupplier) getDocument();
        unoidl.com.sun.star.container.XNameAccess xFamiliesNA =
            xFamiliesSupplier.getStyleFamilies();
        uno.Any aCellStylesObj = xFamiliesNA.getByName( "CellStyles" );
        unoidl.com.sun.star.container.XNameContainer xCellStylesNA =
            (unoidl.com.sun.star.container.XNameContainer) aCellStylesObj.Value;

        // create a new cell style
        xServiceManager =
            (unoidl.com.sun.star.lang.XMultiServiceFactory) getDocument();
        Object aCellStyle = xServiceManager.createInstance(
            "com.sun.star.style.CellStyle" );
        String aStyleName = "MyNewCellStyle";
        xCellStylesNA.insertByName(
            aStyleName, new uno.Any( typeof (Object), aCellStyle ) );

        // modify properties of the new style
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) aCellStyle;
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0x888888 ) );
        xPropSet.setPropertyValue(
            "IsCellBackgroundTransparent", new uno.Any( false ) );



        // --- Query equal-formatted cell ranges ---
        // prepare example, use the new cell style
        xCellRange = xSheet.getCellRangeByName( "D2:F2" );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        xPropSet.setPropertyValue( "CellStyle", new uno.Any( aStyleName ) );
        
        xCellRange = xSheet.getCellRangeByName( "A3:G3" );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        xPropSet.setPropertyValue( "CellStyle", new uno.Any( aStyleName ) );

        // All ranges in one container
        xCellRange = xSheet.getCellRangeByName( "A1:G3" );
        Console.WriteLine( "Service CellFormatRanges:" );
        unoidl.com.sun.star.sheet.XCellFormatRangesSupplier xFormatSupp =
            (unoidl.com.sun.star.sheet.XCellFormatRangesSupplier) xCellRange;
        xRangeIA = xFormatSupp.getCellFormatRanges();
        Console.WriteLine( getCellRangeListString( xRangeIA ) );
        
        // Ranges sorted in SheetCellRanges containers
        Console.WriteLine( "\nService UniqueCellFormatRanges:" );
        unoidl.com.sun.star.sheet.XUniqueCellFormatRangesSupplier
            xUniqueFormatSupp =
            (unoidl.com.sun.star.sheet.XUniqueCellFormatRangesSupplier)
              xCellRange;
        unoidl.com.sun.star.container.XIndexAccess xRangesIA =
            xUniqueFormatSupp.getUniqueCellFormatRanges();
        int nCount = xRangesIA.getCount();
        for (int nIndex = 0; nIndex < nCount; ++nIndex)
        {
            uno.Any aRangesObj = xRangesIA.getByIndex( nIndex );
            xRangeIA =
                (unoidl.com.sun.star.container.XIndexAccess) aRangesObj.Value;
            Console.WriteLine(
                "Container " + (nIndex + 1) + ": " +
                getCellRangeListString( xRangeIA ) );
        }
        

        // --- Table auto formats ---
        // get the global collection of table auto formats,
        // use global service manager
        xServiceManager = getServiceManager();
        Object aAutoFormatsObj = xServiceManager.createInstance(
            "com.sun.star.sheet.TableAutoFormats" );
        unoidl.com.sun.star.container.XNameContainer xAutoFormatsNA =
            (unoidl.com.sun.star.container.XNameContainer) aAutoFormatsObj;
        
        // create a new table auto format and insert into the container
        String aAutoFormatName =  "Temp_Example";
        bool bExistsAlready = xAutoFormatsNA.hasByName( aAutoFormatName );
        uno.Any aAutoFormatObj;
        if (bExistsAlready)
            // auto format already exists -> use it
            aAutoFormatObj = xAutoFormatsNA.getByName( aAutoFormatName );
        else
        {
            // create a new auto format (with document service manager!)
            xServiceManager =
                (unoidl.com.sun.star.lang.XMultiServiceFactory) getDocument();
            aAutoFormatObj = new uno.Any(
                typeof (Object),
                xServiceManager.createInstance(
                    "com.sun.star.sheet.TableAutoFormat" ) );
            xAutoFormatsNA.insertByName( aAutoFormatName, aAutoFormatObj );
        }
        // index access to the auto format fields
        unoidl.com.sun.star.container.XIndexAccess xAutoFormatIA =
            (unoidl.com.sun.star.container.XIndexAccess) aAutoFormatObj.Value;

        // set properties of all auto format fields
        for (int nRow = 0; nRow < 4; ++nRow)
        {
            int nRowColor = 0;
            switch (nRow)
            {
                case 0:     nRowColor = 0x999999;   break;
                case 1:     nRowColor = 0xFFFFCC;   break;
                case 2:     nRowColor = 0xEEEEEE;   break;
                case 3:     nRowColor = 0x999999;   break;
            }

            for (int nColumn = 0; nColumn < 4; ++nColumn)
            {
                int nColor = nRowColor;
                if ((nColumn == 0) || (nColumn == 3))
                    nColor -= 0x333300;

                // get the auto format field and apply properties
                uno.Any aFieldObj = xAutoFormatIA.getByIndex(
                    4 * nRow + nColumn );
                xPropSet =
                    (unoidl.com.sun.star.beans.XPropertySet) aFieldObj.Value;
                xPropSet.setPropertyValue(
                    "CellBackColor", new uno.Any( (Int32) nColor ) );
            }
        }

        // set the auto format to the spreadsheet
        xCellRange = xSheet.getCellRangeByName( "A5:H25" );
        unoidl.com.sun.star.table.XAutoFormattable xAutoForm =
            (unoidl.com.sun.star.table.XAutoFormattable) xCellRange;
        xAutoForm.autoFormat( aAutoFormatName );

        // remove the auto format
        if (!bExistsAlready)
            xAutoFormatsNA.removeByName( aAutoFormatName );


        // --- Conditional formats ---
        xSheet = getSpreadsheet( 0 );
        prepareRange( xSheet, "K20:K23", "Cond. Format" );
        setValue( xSheet, "K21", 1 );
        setValue( xSheet, "K22", 2 );
        setValue( xSheet, "K23", 3 );

        // get the conditional format object of the cell range
        xCellRange = xSheet.getCellRangeByName( "K21:K23" );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        unoidl.com.sun.star.sheet.XSheetConditionalEntries xEntries =
            (unoidl.com.sun.star.sheet.XSheetConditionalEntries)
              xPropSet.getPropertyValue( "ConditionalFormat" ).Value;
        
        // create a condition and apply it to the range
        unoidl.com.sun.star.beans.PropertyValue[] aCondition =
            new unoidl.com.sun.star.beans.PropertyValue[3];
        aCondition[0] = new unoidl.com.sun.star.beans.PropertyValue();
        aCondition[0].Name  = "Operator";
        aCondition[0].Value =
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.ConditionOperator),
                unoidl.com.sun.star.sheet.ConditionOperator.GREATER );
        aCondition[1] = new unoidl.com.sun.star.beans.PropertyValue();
        aCondition[1].Name  = "Formula1";
        aCondition[1].Value = new uno.Any( "1" );
        aCondition[2] = new unoidl.com.sun.star.beans.PropertyValue();
        aCondition[2].Name  = "StyleName";
        aCondition[2].Value = new uno.Any( aStyleName );
        xEntries.addNew( aCondition );
        xPropSet.setPropertyValue(
            "ConditionalFormat",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.XSheetConditionalEntries),
                xEntries ) );
    }



    /** All samples regarding the spreadsheet document. */
    private void doDocumentSamples()
    {
        Console.WriteLine( "\n*** Samples for spreadsheet document ***\n" );


        // --- Insert a new spreadsheet ---
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
            insertSpreadsheet( "A new sheet", (short) 0x7FFF );


        // --- Copy a cell range ---
        prepareRange( xSheet, "A1:B3", "Copy from" );
        prepareRange( xSheet, "D1:E3", "To" );
        setValue( xSheet, "A2", 123 );
        setValue( xSheet, "B2", 345 );
        setFormula( xSheet, "A3", "=SUM(A2:B2)" );
        setFormula( xSheet, "B3", "=FORMULA(A3)" );

        unoidl.com.sun.star.sheet.XCellRangeMovement xMovement =
            (unoidl.com.sun.star.sheet.XCellRangeMovement) xSheet;
        unoidl.com.sun.star.table.CellRangeAddress aSourceRange =
            createCellRangeAddress( xSheet, "A2:B3" );
        unoidl.com.sun.star.table.CellAddress aDestCell =
            createCellAddress( xSheet, "D2" );
        xMovement.copyRange( aDestCell, aSourceRange );


        // --- Print automatic column page breaks ---
        unoidl.com.sun.star.sheet.XSheetPageBreak xPageBreak =
            (unoidl.com.sun.star.sheet.XSheetPageBreak) xSheet;
        unoidl.com.sun.star.sheet.TablePageBreakData[] aPageBreakArray =
            xPageBreak.getColumnPageBreaks();

        Console.Write( "Automatic column page breaks:" );
        for (int nIndex = 0; nIndex < aPageBreakArray.Length; ++nIndex)
            if (!aPageBreakArray[nIndex].ManualBreak)
                Console.Write( " " + aPageBreakArray[nIndex].Position );
        Console.WriteLine();


        // --- Document properties ---
        unoidl.com.sun.star.beans.XPropertySet xPropSet =
            (unoidl.com.sun.star.beans.XPropertySet) getDocument();
        
        String aText = "Value of property IsIterationEnabled: ";
        aText +=
            (Boolean) xPropSet.getPropertyValue( "IsIterationEnabled" ).Value;
        Console.WriteLine( aText );
        aText = "Value of property IterationCount: ";
        aText += (Int32) xPropSet.getPropertyValue( "IterationCount" ).Value;
        Console.WriteLine( aText );
        aText = "Value of property NullDate: ";
        unoidl.com.sun.star.util.Date aDate = (unoidl.com.sun.star.util.Date)
            xPropSet.getPropertyValue( "NullDate" ).Value;
        aText += aDate.Year + "-" + aDate.Month + "-" + aDate.Day;
        Console.WriteLine( aText );


        // --- Data validation ---
        prepareRange( xSheet, "A5:C7", "Validation" );
        setFormula( xSheet, "A6", "Insert values between 0.0 and 5.0 below:" );

        unoidl.com.sun.star.table.XCellRange xCellRange =
            xSheet.getCellRangeByName( "A7:C7" );
        unoidl.com.sun.star.beans.XPropertySet xCellPropSet =
            (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        // validation properties
        unoidl.com.sun.star.beans.XPropertySet xValidPropSet =
            (unoidl.com.sun.star.beans.XPropertySet)
              xCellPropSet.getPropertyValue( "Validation" ).Value;
        xValidPropSet.setPropertyValue(
            "Type",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.ValidationType),
                unoidl.com.sun.star.sheet.ValidationType.DECIMAL ) );
        xValidPropSet.setPropertyValue(
            "ShowErrorMessage", new uno.Any( true ) );
        xValidPropSet.setPropertyValue(
            "ErrorMessage", new uno.Any( "This is an invalid value!" ) );
        xValidPropSet.setPropertyValue(
            "ErrorAlertStyle",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.ValidationAlertStyle),
                unoidl.com.sun.star.sheet.ValidationAlertStyle.STOP ) );
        // condition
        unoidl.com.sun.star.sheet.XSheetCondition xCondition =
            (unoidl.com.sun.star.sheet.XSheetCondition) xValidPropSet;
        xCondition.setOperator(
            unoidl.com.sun.star.sheet.ConditionOperator.BETWEEN );
        xCondition.setFormula1( "0.0" );
        xCondition.setFormula2( "5.0" );
        // apply on cell range
        xCellPropSet.setPropertyValue(
            "Validation",
            new uno.Any(
                typeof (unoidl.com.sun.star.beans.XPropertySet),
                xValidPropSet ) );
        

        // --- Scenarios ---
        uno.Any [][] aValues = {
            new uno.Any [] { uno.Any.VOID, uno.Any.VOID },
            new uno.Any [] { uno.Any.VOID, uno.Any.VOID }
        };
        
        aValues[ 0 ][ 0 ] = new uno.Any( (Double) 11 );
        aValues[ 0 ][ 1 ] = new uno.Any( (Double) 12 );
        aValues[ 1 ][ 0 ] = new uno.Any( "Test13" );
        aValues[ 1 ][ 1 ] = new uno.Any( "Test14" );
        insertScenario(
            xSheet, "B10:C11", aValues,
            "First Scenario", "The first scenario." );

        aValues[ 0 ][ 0 ] = new uno.Any( "Test21" );
        aValues[ 0 ][ 1 ] = new uno.Any( "Test22" );
        aValues[ 1 ][ 0 ] = new uno.Any( (Double) 23 );
        aValues[ 1 ][ 1 ] = new uno.Any( (Double) 24 );
        insertScenario(
            xSheet, "B10:C11", aValues,
            "Second Scenario", "The visible scenario." );

        aValues[ 0 ][ 0 ] = new uno.Any( (Double) 31 );
        aValues[ 0 ][ 1 ] = new uno.Any( (Double) 32 );
        aValues[ 1 ][ 0 ] = new uno.Any( "Test33" );
        aValues[ 1 ][ 1 ] = new uno.Any( "Test34" );
        insertScenario(
            xSheet, "B10:C11", aValues,
            "Third Scenario", "The last scenario." );

        // show second scenario
        showScenario( xSheet, "Second Scenario" );
    }

    /** Inserts a scenario containing one cell range into a sheet and
        applies the value array.
        @param xSheet           The XSpreadsheet interface of the spreadsheet.
        @param aRange           The range address for the scenario.
        @param aValueArray      The array of cell contents.
        @param aScenarioName    The name of the new scenario.
        @param aScenarioComment The user comment for the scenario. */
    private void insertScenario(
            unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
            String aRange,
            uno.Any [][] aValueArray,
            String aScenarioName,
            String aScenarioComment )
    {
        // get the cell range with the given address
        unoidl.com.sun.star.table.XCellRange xCellRange =
            xSheet.getCellRangeByName( aRange );

        // create the range address sequence
        unoidl.com.sun.star.sheet.XCellRangeAddressable xAddr =
            (unoidl.com.sun.star.sheet.XCellRangeAddressable) xCellRange;
        unoidl.com.sun.star.table.CellRangeAddress[] aRangesSeq =
            new unoidl.com.sun.star.table.CellRangeAddress[1];
        aRangesSeq[0] = xAddr.getRangeAddress();

        // create the scenario
        unoidl.com.sun.star.sheet.XScenariosSupplier xScenSupp =
            (unoidl.com.sun.star.sheet.XScenariosSupplier) xSheet;
        unoidl.com.sun.star.sheet.XScenarios xScenarios =
            xScenSupp.getScenarios();
        xScenarios.addNewByName( aScenarioName, aRangesSeq, aScenarioComment );

        // insert the values into the range
        unoidl.com.sun.star.sheet.XCellRangeData xData =
            (unoidl.com.sun.star.sheet.XCellRangeData) xCellRange;
        xData.setDataArray( aValueArray );
    }

    /** Activates a scenario.
        @param xSheet           The XSpreadsheet interface of the spreadsheet.
        @param aScenarioName    The name of the scenario. */
    private void showScenario(
            unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
            String aScenarioName )
    {
        // get the scenario set
        unoidl.com.sun.star.sheet.XScenariosSupplier xScenSupp =
            (unoidl.com.sun.star.sheet.XScenariosSupplier) xSheet;
        unoidl.com.sun.star.sheet.XScenarios xScenarios =
            xScenSupp.getScenarios();

        // get the scenario and activate it
        uno.Any aScenarioObj = xScenarios.getByName( aScenarioName );
        unoidl.com.sun.star.sheet.XScenario xScenario =
            (unoidl.com.sun.star.sheet.XScenario) aScenarioObj.Value;
        xScenario.apply();
    }



    private void doNamedRangesSamples()
    {
        Console.WriteLine( "\n*** Samples for named ranges ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheetDocument xDocument =
            getDocument();
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
            getSpreadsheet( 0 );


        // --- Named ranges ---
        prepareRange( xSheet, "G42:H45", "Named ranges" );
        xSheet.getCellByPosition( 6, 42 ).setValue( 1 );
        xSheet.getCellByPosition( 6, 43 ).setValue( 2 );
        xSheet.getCellByPosition( 7, 42 ).setValue( 3 );
        xSheet.getCellByPosition( 7, 43 ).setValue( 4 );

        // insert a named range
        unoidl.com.sun.star.beans.XPropertySet xDocProp =
            (unoidl.com.sun.star.beans.XPropertySet) xDocument;
        uno.Any aRangesObj = xDocProp.getPropertyValue( "NamedRanges" );
        unoidl.com.sun.star.sheet.XNamedRanges xNamedRanges =
            (unoidl.com.sun.star.sheet.XNamedRanges) aRangesObj.Value;
        unoidl.com.sun.star.table.CellAddress aRefPos =
            new unoidl.com.sun.star.table.CellAddress();
        aRefPos.Sheet  = 0;
        aRefPos.Column = 6;
        aRefPos.Row    = 44;
        xNamedRanges.addNewByName( "ExampleName", "SUM(G43:G44)", aRefPos, 0 );

        // use the named range in formulas
        xSheet.getCellByPosition( 6, 44 ).setFormula( "=ExampleName" );
        xSheet.getCellByPosition( 7, 44 ).setFormula( "=ExampleName" );


        // --- Label ranges ---
        prepareRange( xSheet, "G47:I50", "Label ranges" );
        unoidl.com.sun.star.table.XCellRange xRange =
            xSheet.getCellRangeByPosition( 6, 47, 7, 49 );
        unoidl.com.sun.star.sheet.XCellRangeData xData =
            ( unoidl.com.sun.star.sheet.XCellRangeData ) xRange;
        uno.Any [][] aValues =
        {
            new uno.Any [] { new uno.Any( "Apples" ),
                             new uno.Any( "Oranges" ) },
            new uno.Any [] { new uno.Any( (Double) 5 ),
                             new uno.Any( (Double) 7 ) },
            new uno.Any [] { new uno.Any( (Double) 6 ),
                             new uno.Any( (Double) 8 ) }
        };
        xData.setDataArray( aValues );

        // insert a column label range
        uno.Any aLabelsObj = xDocProp.getPropertyValue( "ColumnLabelRanges" );
        unoidl.com.sun.star.sheet.XLabelRanges xLabelRanges =
            (unoidl.com.sun.star.sheet.XLabelRanges) aLabelsObj.Value;
        unoidl.com.sun.star.table.CellRangeAddress aLabelArea =
            new unoidl.com.sun.star.table.CellRangeAddress();
        aLabelArea.Sheet       = 0;
        aLabelArea.StartColumn = 6;
        aLabelArea.StartRow    = 47;
        aLabelArea.EndColumn   = 7;
        aLabelArea.EndRow      = 47;
        unoidl.com.sun.star.table.CellRangeAddress aDataArea =
            new unoidl.com.sun.star.table.CellRangeAddress();
        aDataArea.Sheet       = 0;
        aDataArea.StartColumn = 6;
        aDataArea.StartRow    = 48;
        aDataArea.EndColumn   = 7;
        aDataArea.EndRow      = 49;
        xLabelRanges.addNew( aLabelArea, aDataArea );

        // use the label range in formulas
        xSheet.getCellByPosition( 8, 48 ).setFormula( "=Apples+Oranges" );
        xSheet.getCellByPosition( 8, 49 ).setFormula( "=Apples+Oranges" );
    }



    /** Helper for doDatabaseSamples: get name of first database. */
    private String getFirstDatabaseName()
    {
        String aDatabase = null;
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceManager =
            getServiceManager();
        unoidl.com.sun.star.container.XNameAccess xContext =
            (unoidl.com.sun.star.container.XNameAccess)
            xServiceManager.createInstance(
                "com.sun.star.sdb.DatabaseContext" );
        String[] aNames = xContext.getElementNames();
        if ( aNames.Length > 0 )
            aDatabase = aNames[0];
        return aDatabase;
    }

    /** Helper for doDatabaseSamples: get name of first table in a database. */
    private String getFirstTableName( String aDatabase )
    {
        if ( aDatabase == null )
            return null;

        String aTable = null;
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceManager =
            getServiceManager();
        unoidl.com.sun.star.container.XNameAccess xContext =
            (unoidl.com.sun.star.container.XNameAccess)
            xServiceManager.createInstance(
                "com.sun.star.sdb.DatabaseContext" );
        unoidl.com.sun.star.sdb.XCompletedConnection xSource =
            (unoidl.com.sun.star.sdb.XCompletedConnection)
              xContext.getByName( aDatabase ).Value;
        unoidl.com.sun.star.task.XInteractionHandler xHandler =
            (unoidl.com.sun.star.task.XInteractionHandler)
              xServiceManager.createInstance(
                  "com.sun.star.task.InteractionHandler" );
         unoidl.com.sun.star.sdbcx.XTablesSupplier xSupplier =
            (unoidl.com.sun.star.sdbcx.XTablesSupplier)
              xSource.connectWithCompletion( xHandler );
        unoidl.com.sun.star.container.XNameAccess xTables =
            xSupplier.getTables();
        String[] aNames = xTables.getElementNames();
        if ( aNames.Length > 0 )
            aTable = aNames[0];
        return aTable;
    }

    private void doDatabaseSamples()
    {
        Console.WriteLine( "\n*** Samples for database operations ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 2 );


        // --- put some example data into the sheet ---
        unoidl.com.sun.star.table.XCellRange xRange =
            xSheet.getCellRangeByName( "B3:D24" );
        unoidl.com.sun.star.sheet.XCellRangeData xData =
            (unoidl.com.sun.star.sheet.XCellRangeData) xRange;
        uno.Any [][] aValues =
        {
            new uno.Any [] { new uno.Any( "Name" ),
                             new uno.Any( "Year" ),
                             new uno.Any( "Sales" ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( (Double) 2001 ),
                             new uno.Any( (Double) 4.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
                             new uno.Any( (Double) 1997 ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
                             new uno.Any( (Double) 1998 ),
                             new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( (Double) 1997 ),
                             new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( (Double) 2002 ),
                             new uno.Any( (Double) 9.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( (Double) 1999 ),
                             new uno.Any( (Double) 7.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
                             new uno.Any( (Double) 1996 ),
                             new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( (Double) 2000 ),
                             new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
                             new uno.Any( (Double) 1999 ),
                             new uno.Any( (Double) 5.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
                             new uno.Any( (Double) 2002 ),
                             new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
              new uno.Any( (Double) 2001 ),
              new uno.Any( (Double) 5.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
              new uno.Any( (Double) 2000 ),
              new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
              new uno.Any( (Double) 1996 ),
              new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
              new uno.Any( (Double) 1996 ),
              new uno.Any( (Double) 7.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
              new uno.Any( (Double) 1997 ),
              new uno.Any( (Double) 3.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
              new uno.Any( (Double) 2000 ),
              new uno.Any( (Double) 9.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
              new uno.Any( (Double) 1998 ),
              new uno.Any( (Double) 1.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
              new uno.Any( (Double) 1999 ),
              new uno.Any( (Double) 6.0 ) },
            new uno.Any [] { new uno.Any( "Carol" ),
              new uno.Any( (Double) 2002 ),
              new uno.Any( (Double) 8.0 ) },
            new uno.Any [] { new uno.Any( "Alice" ),
              new uno.Any( (Double) 1998 ),
              new uno.Any( (Double) 5.0 ) },
            new uno.Any [] { new uno.Any( "Bob" ),
              new uno.Any( (Double) 2001 ),
              new uno.Any( (Double) 6.0 ) }
        };
        xData.setDataArray( aValues );


        // --- filter for second column >= 1998 ---
        unoidl.com.sun.star.sheet.XSheetFilterable xFilter =
            (unoidl.com.sun.star.sheet.XSheetFilterable) xRange;
        unoidl.com.sun.star.sheet.XSheetFilterDescriptor xFilterDesc =
            xFilter.createFilterDescriptor( true );
        unoidl.com.sun.star.sheet.TableFilterField[] aFilterFields =
            new unoidl.com.sun.star.sheet.TableFilterField[1];
        aFilterFields[0] = new unoidl.com.sun.star.sheet.TableFilterField();
        aFilterFields[0].Field        = 1;
        aFilterFields[0].IsNumeric    = true;
        aFilterFields[0].Operator =
            unoidl.com.sun.star.sheet.FilterOperator.GREATER_EQUAL;
        aFilterFields[0].NumericValue = 1998;
        xFilterDesc.setFilterFields( aFilterFields );
        unoidl.com.sun.star.beans.XPropertySet xFilterProp =
            (unoidl.com.sun.star.beans.XPropertySet) xFilterDesc;
        xFilterProp.setPropertyValue(
            "ContainsHeader", new uno.Any( true ) );
        xFilter.filter( xFilterDesc );


        // --- do the same filter as above, using criteria from a cell range ---
        unoidl.com.sun.star.table.XCellRange xCritRange =
            xSheet.getCellRangeByName( "B27:B28" );
        unoidl.com.sun.star.sheet.XCellRangeData xCritData =
            (unoidl.com.sun.star.sheet.XCellRangeData) xCritRange;
        uno.Any [][] aCritValues =
        {
            new uno.Any [] { new uno.Any( "Year" ) },
            new uno.Any [] { new uno.Any( ">= 1998" ) }
        };
        xCritData.setDataArray( aCritValues );
        unoidl.com.sun.star.sheet.XSheetFilterableEx xCriteria =
            (unoidl.com.sun.star.sheet.XSheetFilterableEx) xCritRange;
        xFilterDesc = xCriteria.createFilterDescriptorByObject( xFilter );
        if ( xFilterDesc != null )
            xFilter.filter( xFilterDesc );


        // --- sort by second column, ascending ---
        unoidl.com.sun.star.util.SortField[] aSortFields =
            new unoidl.com.sun.star.util.SortField[1];
        aSortFields[0] = new unoidl.com.sun.star.util.SortField();
        aSortFields[0].Field         = 1;
        aSortFields[0].SortAscending = true;

        unoidl.com.sun.star.beans.PropertyValue[] aSortDesc =
            new unoidl.com.sun.star.beans.PropertyValue[2];
        aSortDesc[0] = new unoidl.com.sun.star.beans.PropertyValue();
        aSortDesc[0].Name   = "SortFields";
        aSortDesc[0].Value  =
            new uno.Any(
                typeof (unoidl.com.sun.star.util.SortField []),
                aSortFields );
        aSortDesc[1] = new unoidl.com.sun.star.beans.PropertyValue();
        aSortDesc[1].Name   = "ContainsHeader";
        aSortDesc[1].Value  = new uno.Any( true );

        unoidl.com.sun.star.util.XSortable xSort =
            (unoidl.com.sun.star.util.XSortable) xRange;
        xSort.sort( aSortDesc );


        // --- insert subtotals ---
        unoidl.com.sun.star.sheet.XSubTotalCalculatable xSub =
            (unoidl.com.sun.star.sheet.XSubTotalCalculatable) xRange;
        unoidl.com.sun.star.sheet.XSubTotalDescriptor xSubDesc =
            xSub.createSubTotalDescriptor( true );
        unoidl.com.sun.star.sheet.SubTotalColumn[] aColumns =
            new unoidl.com.sun.star.sheet.SubTotalColumn[1];
        // calculate sum of third column
        aColumns[0] = new unoidl.com.sun.star.sheet.SubTotalColumn();
        aColumns[0].Column   = 2;
        aColumns[0].Function = unoidl.com.sun.star.sheet.GeneralFunction.SUM;
        // group by first column
        xSubDesc.addNew( aColumns, 0 );
        xSub.applySubTotals( xSubDesc, true );

        String aDatabase = getFirstDatabaseName();
        String aTableName = getFirstTableName( aDatabase );
        if ( aDatabase != null && aTableName != null )
        {
            // --- import from database ---
            unoidl.com.sun.star.beans.PropertyValue[] aImportDesc =
                new unoidl.com.sun.star.beans.PropertyValue[3];
            aImportDesc[0] = new unoidl.com.sun.star.beans.PropertyValue();
            aImportDesc[0].Name     = "DatabaseName";
            aImportDesc[0].Value    = new uno.Any( aDatabase );
            aImportDesc[1] = new unoidl.com.sun.star.beans.PropertyValue();
            aImportDesc[1].Name     = "SourceType";
            aImportDesc[1].Value    =
                new uno.Any(
                    typeof (unoidl.com.sun.star.sheet.DataImportMode),
                    unoidl.com.sun.star.sheet.DataImportMode.TABLE );
            aImportDesc[2] = new unoidl.com.sun.star.beans.PropertyValue();
            aImportDesc[2].Name     = "SourceObject";
            aImportDesc[2].Value    = new uno.Any( aTableName );

            unoidl.com.sun.star.table.XCellRange xImportRange =
                xSheet.getCellRangeByName( "B35:B35" );
            unoidl.com.sun.star.util.XImportable xImport =
                (unoidl.com.sun.star.util.XImportable) xImportRange;
            xImport.doImport( aImportDesc );


            // --- use the temporary database range to find the
            // imported data's size ---
            unoidl.com.sun.star.beans.XPropertySet xDocProp =
                (unoidl.com.sun.star.beans.XPropertySet) getDocument();
            uno.Any aRangesObj = xDocProp.getPropertyValue( "DatabaseRanges" );
            unoidl.com.sun.star.container.XNameAccess xRanges =
                (unoidl.com.sun.star.container.XNameAccess) aRangesObj.Value;
            String[] aNames = xRanges.getElementNames();
            for ( int i=0; i<aNames.Length; i++ )
            {
                uno.Any aRangeObj = xRanges.getByName( aNames[i] );
                unoidl.com.sun.star.beans.XPropertySet xRangeProp =
                    (unoidl.com.sun.star.beans.XPropertySet) aRangeObj.Value;
                bool bUser = (Boolean)
                    xRangeProp.getPropertyValue( "IsUserDefined" ).Value;
                if ( !bUser )
                {
                    // this is the temporary database range -
                    // get the cell range and format it
                    unoidl.com.sun.star.sheet.XCellRangeReferrer xRef =
                        (unoidl.com.sun.star.sheet.XCellRangeReferrer)
                          aRangeObj.Value;
                    unoidl.com.sun.star.table.XCellRange xResultRange =
                        xRef.getReferredCells();
                    unoidl.com.sun.star.beans.XPropertySet xResultProp =
                        (unoidl.com.sun.star.beans.XPropertySet) xResultRange;
                    xResultProp.setPropertyValue(
                        "IsCellBackgroundTransparent", new uno.Any( false ) );
                    xResultProp.setPropertyValue(
                        "CellBackColor", new uno.Any( (Int32) 0xFFFFCC ) );
                }
            }
        }
        else
            Console.WriteLine("can't get database");
    }



    private void doDataPilotSamples()
    {
        Console.WriteLine( "\n*** Samples for Data Pilot ***\n" );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );


        // --- Create a new DataPilot table ---
        prepareRange( xSheet, "A38:C38", "Data Pilot" );
        unoidl.com.sun.star.sheet.XDataPilotTablesSupplier xDPSupp =
            (unoidl.com.sun.star.sheet.XDataPilotTablesSupplier) xSheet;
        unoidl.com.sun.star.sheet.XDataPilotTables xDPTables =
            xDPSupp.getDataPilotTables();
        unoidl.com.sun.star.sheet.XDataPilotDescriptor xDPDesc =
            xDPTables.createDataPilotDescriptor();
        // set source range (use data range from CellRange test)
        unoidl.com.sun.star.table.CellRangeAddress aSourceAddress =
            createCellRangeAddress( xSheet, "A10:C30" );
        xDPDesc.setSourceRange( aSourceAddress );
        // settings for fields
        unoidl.com.sun.star.container.XIndexAccess xFields =
            xDPDesc.getDataPilotFields();
        uno.Any aFieldObj;
        unoidl.com.sun.star.beans.XPropertySet xFieldProp;
        // use first column as column field
        aFieldObj = xFields.getByIndex(0);
        xFieldProp = (unoidl.com.sun.star.beans.XPropertySet) aFieldObj.Value;
        xFieldProp.setPropertyValue(
            "Orientation",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.DataPilotFieldOrientation),
                unoidl.com.sun.star.sheet.DataPilotFieldOrientation.COLUMN ) );
        // use second column as row field
        aFieldObj = xFields.getByIndex(1);
        xFieldProp = (unoidl.com.sun.star.beans.XPropertySet) aFieldObj.Value;
        xFieldProp.setPropertyValue(
            "Orientation",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.DataPilotFieldOrientation),
                unoidl.com.sun.star.sheet.DataPilotFieldOrientation.ROW ) );
        // use third column as data field, calculating the sum
        aFieldObj = xFields.getByIndex(2);
        xFieldProp = (unoidl.com.sun.star.beans.XPropertySet) aFieldObj.Value;
        xFieldProp.setPropertyValue(
            "Orientation",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.DataPilotFieldOrientation),
                unoidl.com.sun.star.sheet.DataPilotFieldOrientation.DATA ) );
        xFieldProp.setPropertyValue(
            "Function",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.GeneralFunction),
                unoidl.com.sun.star.sheet.GeneralFunction.SUM ) );
        // select output position
        unoidl.com.sun.star.table.CellAddress aDestAddress =
            createCellAddress( xSheet, "A40" );
        xDPTables.insertNewByName( "DataPilotExample", aDestAddress, xDPDesc );


        // --- Modify the DataPilot table ---
        uno.Any aDPTableObj = xDPTables.getByName( "DataPilotExample" );
        xDPDesc =
            (unoidl.com.sun.star.sheet.XDataPilotDescriptor) aDPTableObj.Value;
        xFields = xDPDesc.getDataPilotFields();
        // add a second data field from the third column,
        // calculating the average
        aFieldObj = xFields.getByIndex(2);
        xFieldProp = (unoidl.com.sun.star.beans.XPropertySet) aFieldObj.Value;
        xFieldProp.setPropertyValue(
            "Orientation",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.DataPilotFieldOrientation),
                unoidl.com.sun.star.sheet.DataPilotFieldOrientation.DATA ) );
        xFieldProp.setPropertyValue(
            "Function",
            new uno.Any(
                typeof (unoidl.com.sun.star.sheet.GeneralFunction),
                unoidl.com.sun.star.sheet.GeneralFunction.AVERAGE ) );
    }



    private void doFunctionAccessSamples()
    {
        Console.WriteLine( "\n*** Samples for function handling ***\n" );
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceManager =
            getServiceManager();


        // --- Calculate a function ---
        Object aFuncInst = xServiceManager.createInstance(
            "com.sun.star.sheet.FunctionAccess" );
        unoidl.com.sun.star.sheet.XFunctionAccess xFuncAcc =
            (unoidl.com.sun.star.sheet.XFunctionAccess) aFuncInst;
        // put the data in a two-dimensional array
        Double [][] aData = { new Double [] { 1.0, 2.0, 3.0 } };
        // construct the array of function arguments
        uno.Any [] aArgs = new uno.Any [2];
        aArgs[0] = new uno.Any( typeof (Double [][]), aData );
        aArgs[1] = new uno.Any( (Double) 2.0 );
        uno.Any aResult = xFuncAcc.callFunction( "ZTEST", aArgs );
        Console.WriteLine(
            "ZTEST result for data {1,2,3} and value 2 is " + aResult.Value );


        // --- Get the list of recently used functions ---
        Object aRecInst = xServiceManager.createInstance(
            "com.sun.star.sheet.RecentFunctions" );
        unoidl.com.sun.star.sheet.XRecentFunctions xRecFunc =
            (unoidl.com.sun.star.sheet.XRecentFunctions) aRecInst;
        int[] nRecentIds = xRecFunc.getRecentFunctionIds();


        // --- Get the names for these functions ---
        Object aDescInst = xServiceManager.createInstance(
            "com.sun.star.sheet.FunctionDescriptions" );
        unoidl.com.sun.star.sheet.XFunctionDescriptions xFuncDesc =
            (unoidl.com.sun.star.sheet.XFunctionDescriptions) aDescInst;
        Console.Write("Recently used functions: ");
        for (int nFunction=0; nFunction<nRecentIds.Length; nFunction++)
        {
            unoidl.com.sun.star.beans.PropertyValue[] aProperties =
                xFuncDesc.getById( nRecentIds[nFunction] );
            for (int nProp=0; nProp<aProperties.Length; nProp++)
                if ( aProperties[nProp].Name.Equals( "Name" ) )
                    Console.Write( aProperties[nProp].Value + " " );
        }
        Console.WriteLine();
    }



    private void doApplicationSettingsSamples()
    {
        Console.WriteLine( "\n*** Samples for application settings ***\n" );
        unoidl.com.sun.star.lang.XMultiServiceFactory xServiceManager =
            getServiceManager();


        // --- Get the user defined sort lists ---
        Object aSettings = xServiceManager.createInstance(
            "com.sun.star.sheet.GlobalSheetSettings" );
        unoidl.com.sun.star.beans.XPropertySet xPropSet =
            (unoidl.com.sun.star.beans.XPropertySet) aSettings;
        String[] aEntries = (String [])
            xPropSet.getPropertyValue( "UserLists" ).Value;
        Console.WriteLine("User defined sort lists:");
        for ( int i=0; i<aEntries.Length; i++ )
            Console.WriteLine( aEntries[i] );
    }



}
