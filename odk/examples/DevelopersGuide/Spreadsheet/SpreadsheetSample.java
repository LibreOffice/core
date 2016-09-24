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
import com.sun.star.uno.AnyConverter;

// __________  implementation  ____________________________________

/** Create and modify a spreadsheet document.
 */
public class SpreadsheetSample extends SpreadsheetDocHelper
{



    public static void main( String args[] )
    {
        try
        {
            SpreadsheetSample aSample = new SpreadsheetSample( args );
            aSample.doSampleFunction();
        }
        catch (Exception ex)
        {
            System.out.println( "Error: Sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
            System.exit( 1 );
        }
        System.out.println( "\nSamples done." );
        System.exit( 0 );
    }



    public SpreadsheetSample( String[] args )
    {
        super( args );
    }



    /** This sample function performs all changes on the document. */
    public void doSampleFunction()
    {
        try
        {
            doCellSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Cell sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doCellRangeSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Cell range sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doCellRangesSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Cell range container sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doCellCursorSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Cell cursor sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doFormattingSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Formatting sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doDocumentSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Document sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doDatabaseSamples();
        }
        catch( Exception ex )
        {
            System.out.println( "\nError: Database sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doDataPilotSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Data pilot sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doNamedRangesSamples();
        }
        catch( Exception ex )
        {
            System.out.println( "\nError: Named ranges sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doFunctionAccessSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Function access sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }

        try
        {
            doApplicationSettingsSamples();
        }
        catch (Exception ex)
        {
            System.out.println( "\nError: Application settings sample caught exception!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }
    }



    /** All samples regarding the service com.sun.star.sheet.SheetCell. */
    private void doCellSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for service sheet.SheetCell ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        com.sun.star.table.XCell xCell = null;
        com.sun.star.beans.XPropertySet xPropSet = null;
        String aText;
        prepareRange( xSheet, "A1:C7", "Cells and Cell Ranges" );

        // --- Get cell B3 by position - (column, row) ---
        xCell = xSheet.getCellByPosition( 1, 2 );


        // --- Insert two text paragraphs into the cell. ---
        com.sun.star.text.XText xText = UnoRuntime.queryInterface( com.sun.star.text.XText.class, xCell );
        com.sun.star.text.XTextCursor xTextCursor = xText.createTextCursor();

        xText.insertString( xTextCursor, "Text in first line.", false );
        xText.insertControlCharacter( xTextCursor,
            com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );
        xText.insertString( xTextCursor, "And a ", false );

        // create a hyperlink
        com.sun.star.lang.XMultiServiceFactory xServiceMan = UnoRuntime.queryInterface( com.sun.star.lang.XMultiServiceFactory.class, getDocument() );
        Object aHyperlinkObj = xServiceMan.createInstance( "com.sun.star.text.TextField.URL" );
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aHyperlinkObj );
        xPropSet.setPropertyValue( "URL", "http://www.example.org" );
        xPropSet.setPropertyValue( "Representation", "hyperlink" );
        // ... and insert
        com.sun.star.text.XTextContent xContent = UnoRuntime.queryInterface( com.sun.star.text.XTextContent.class, aHyperlinkObj );
        xText.insertTextContent( xTextCursor, xContent, false );


        // --- Query the separate paragraphs. ---
        com.sun.star.container.XEnumerationAccess xParaEA =
            UnoRuntime.queryInterface(
            com.sun.star.container.XEnumerationAccess.class, xCell );
        com.sun.star.container.XEnumeration xParaEnum = xParaEA.createEnumeration();
        // Go through the paragraphs
        while( xParaEnum.hasMoreElements() )
        {
            Object aPortionObj = xParaEnum.nextElement();
            com.sun.star.container.XEnumerationAccess xPortionEA =
                UnoRuntime.queryInterface(
                com.sun.star.container.XEnumerationAccess.class, aPortionObj );
            com.sun.star.container.XEnumeration xPortionEnum = xPortionEA.createEnumeration();
            aText = "";
            // Go through all text portions of a paragraph and construct string.
            while( xPortionEnum.hasMoreElements() )
            {
                com.sun.star.text.XTextRange xRange = UnoRuntime.queryInterface(com.sun.star.text.XTextRange.class,
                                          xPortionEnum.nextElement());
                aText += xRange.getString();
            }
            System.out.println( "Paragraph text: " + aText );
        }


        // --- Change cell properties. ---
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCell );
        // from styles.CharacterProperties
        xPropSet.setPropertyValue( "CharColor", Integer.valueOf( 0x003399 ) );
        xPropSet.setPropertyValue( "CharHeight", new Float( 20.0 ) );
        // from styles.ParagraphProperties
        xPropSet.setPropertyValue( "ParaLeftMargin", Integer.valueOf( 500 ) );
        // from table.CellProperties
        xPropSet.setPropertyValue( "IsCellBackgroundTransparent", Boolean.FALSE );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( 0x99CCFF ) );


        // --- Get cell address. ---
        com.sun.star.sheet.XCellAddressable xCellAddr = UnoRuntime.queryInterface( com.sun.star.sheet.XCellAddressable.class, xCell );
        com.sun.star.table.CellAddress aAddress = xCellAddr.getCellAddress();
        aText = "Address of this cell:  Column=" + aAddress.Column;
        aText += ";  Row=" + aAddress.Row;
        aText += ";  Sheet=" + aAddress.Sheet;
        System.out.println( aText );


        // --- Insert an annotation ---
        com.sun.star.sheet.XSheetAnnotationsSupplier xAnnotationsSupp =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XSheetAnnotationsSupplier.class, xSheet );
        com.sun.star.sheet.XSheetAnnotations xAnnotations = xAnnotationsSupp.getAnnotations();
        xAnnotations.insertNew( aAddress, "This is an annotation" );

        com.sun.star.sheet.XSheetAnnotationAnchor xAnnotAnchor = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetAnnotationAnchor.class, xCell );
        com.sun.star.sheet.XSheetAnnotation xAnnotation = xAnnotAnchor.getAnnotation();
        xAnnotation.setIsVisible( true );
    }



    /** All samples regarding the service com.sun.star.sheet.SheetCellRange. */
    private void doCellRangeSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for service sheet.SheetCellRange ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );
        com.sun.star.table.XCellRange xCellRange = null;
        com.sun.star.beans.XPropertySet xPropSet = null;
        com.sun.star.table.CellRangeAddress aRangeAddress = null;

        // Preparation
        setFormula( xSheet, "B5", "First cell" );
        setFormula( xSheet, "B6", "Second cell" );
        // Get cell range B5:B6 by position - (column, row, column, row)
        xCellRange = xSheet.getCellRangeByPosition( 1, 4, 1, 5 );


        // --- Change cell range properties. ---
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        // from com.sun.star.styles.CharacterProperties
        xPropSet.setPropertyValue( "CharColor", Integer.valueOf( 0x003399 ) );
        xPropSet.setPropertyValue( "CharHeight", new Float( 20.0 ) );
        // from com.sun.star.styles.ParagraphProperties
        xPropSet.setPropertyValue( "ParaLeftMargin", Integer.valueOf( 500 ) );
        // from com.sun.star.table.CellProperties
        xPropSet.setPropertyValue( "IsCellBackgroundTransparent", Boolean.FALSE );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( 0x99CCFF ) );


        // --- Replace text in all cells. ---
        com.sun.star.util.XReplaceable xReplace = UnoRuntime.queryInterface( com.sun.star.util.XReplaceable.class, xCellRange );
        com.sun.star.util.XReplaceDescriptor xReplaceDesc = xReplace.createReplaceDescriptor();
        xReplaceDesc.setSearchString( "cell" );
        xReplaceDesc.setReplaceString( "text" );
        // property SearchWords searches for whole cells!
        xReplaceDesc.setPropertyValue( "SearchWords", Boolean.FALSE );
        int nCount = xReplace.replaceAll( xReplaceDesc );
        System.out.println( "Search text replaced " + nCount + " times." );


        // --- Merge cells. ---
        xCellRange = xSheet.getCellRangeByName( "F3:G6" );
        prepareRange( xSheet, "E1:H7", "XMergeable" );
        com.sun.star.util.XMergeable xMerge = UnoRuntime.queryInterface( com.sun.star.util.XMergeable.class, xCellRange );
        xMerge.merge( true );


        // --- Change indentation. ---
/* does not work (bug in XIndent implementation)
        prepareRange( xSheet, "I20:I23", "XIndent" );
        setValue( xSheet, "I21", 1 );
        setValue( xSheet, "I22", 1 );
        setValue( xSheet, "I23", 1 );

        xCellRange = xSheet.getCellRangeByName( "I21:I22" );
        com.sun.star.util.XIndent xIndent = (com.sun.star.util.XIndent)
            UnoRuntime.queryInterface( com.sun.star.util.XIndent.class, xCellRange );
        xIndent.incrementIndent();

        xCellRange = xSheet.getCellRangeByName( "I22:I23" );
        xIndent = (com.sun.star.util.XIndent)
            UnoRuntime.queryInterface( com.sun.star.util.XIndent.class, xCellRange );
        xIndent.incrementIndent();
*/


        // --- Column properties. ---
        xCellRange = xSheet.getCellRangeByName( "B1" );
        com.sun.star.table.XColumnRowRange xColRowRange = UnoRuntime.queryInterface( com.sun.star.table.XColumnRowRange.class, xCellRange );
        com.sun.star.table.XTableColumns xColumns = xColRowRange.getColumns();

        Object aColumnObj = xColumns.getByIndex( 0 );
        xPropSet = UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, aColumnObj );
        xPropSet.setPropertyValue( "Width", Integer.valueOf( 6000 ) );

        com.sun.star.container.XNamed xNamed = UnoRuntime.queryInterface( com.sun.star.container.XNamed.class, aColumnObj );
        System.out.println( "The name of the wide column is " + xNamed.getName() + "." );


        // --- Cell range data ---
        prepareRange( xSheet, "A9:C30", "XCellRangeData" );

        xCellRange = xSheet.getCellRangeByName( "A10:C30" );
        com.sun.star.sheet.XCellRangeData xData = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeData.class, xCellRange );
        Object[][] aValues =
        {
            { "Name",   "Fruit",    "Quantity" },
            { "Alice",  "Apples",   new Double( 3.0 ) },
            { "Alice",  "Oranges",  new Double( 7.0 ) },
            { "Bob",    "Apples",   new Double( 3.0 ) },
            { "Alice",  "Apples",   new Double( 9.0 ) },
            { "Bob",    "Apples",   new Double( 5.0 ) },
            { "Bob",    "Oranges",  new Double( 6.0 ) },
            { "Alice",  "Oranges",  new Double( 3.0 ) },
            { "Alice",  "Apples",   new Double( 8.0 ) },
            { "Alice",  "Oranges",  new Double( 1.0 ) },
            { "Bob",    "Oranges",  new Double( 2.0 ) },
            { "Bob",    "Oranges",  new Double( 7.0 ) },
            { "Bob",    "Apples",   new Double( 1.0 ) },
            { "Alice",  "Apples",   new Double( 8.0 ) },
            { "Alice",  "Oranges",  new Double( 8.0 ) },
            { "Alice",  "Apples",   new Double( 7.0 ) },
            { "Bob",    "Apples",   new Double( 1.0 ) },
            { "Bob",    "Oranges",  new Double( 9.0 ) },
            { "Bob",    "Oranges",  new Double( 3.0 ) },
            { "Alice",  "Oranges",  new Double( 4.0 ) },
            { "Alice",  "Apples",   new Double( 9.0 ) }
        };
        xData.setDataArray( aValues );


        // --- Get cell range address. ---
        com.sun.star.sheet.XCellRangeAddressable xRangeAddr = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeAddressable.class, xCellRange );
        aRangeAddress = xRangeAddr.getRangeAddress();
        System.out.println( "Address of this range:  Sheet=" + aRangeAddress.Sheet );
        System.out.println( "Start column=" + aRangeAddress.StartColumn + ";  Start row=" + aRangeAddress.StartRow );
        System.out.println( "End column  =" + aRangeAddress.EndColumn   + ";  End row  =" + aRangeAddress.EndRow );


        // --- Sheet operation. ---
        // uses the range filled with XCellRangeData
        com.sun.star.sheet.XSheetOperation xSheetOp = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetOperation.class, xData );
        double fResult = xSheetOp.computeFunction( com.sun.star.sheet.GeneralFunction.AVERAGE );
        System.out.println( "Average value of the data table A10:C30: " + fResult );


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

        com.sun.star.sheet.XCellSeries xSeries = null;
        // Fill 2 rows linear with end value -> 2nd series is not filled completely
        xSeries = getCellSeries( xSheet, "E10:I11" );
        xSeries.fillSeries( com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.LINEAR,
            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 9 );
        // Add months to a date
        xSeries = getCellSeries( xSheet, "E12:I12" );
        xSeries.fillSeries( com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.DATE,
            com.sun.star.sheet.FillDateMode.FILL_DATE_MONTH, 1, 0x7FFFFFFF );
        // Fill right to left with a text containing a value
        xSeries = getCellSeries( xSheet, "E13:I13" );
        xSeries.fillSeries( com.sun.star.sheet.FillDirection.TO_LEFT, com.sun.star.sheet.FillMode.LINEAR,
            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 10, 0x7FFFFFFF );
        // Fill with an user defined list
        xSeries = getCellSeries( xSheet, "E14:I14" );
        xSeries.fillSeries( com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.AUTO,
            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 1, 0x7FFFFFFF );
        // Fill bottom to top with a geometric series
        xSeries = getCellSeries( xSheet, "K10:K14" );
        xSeries.fillSeries( com.sun.star.sheet.FillDirection.TO_TOP, com.sun.star.sheet.FillMode.GROWTH,
            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 0x7FFFFFFF );
        // Auto fill
        xSeries = getCellSeries( xSheet, "E16:K18" );
        xSeries.fillAuto( com.sun.star.sheet.FillDirection.TO_RIGHT, 2 );
        // Fill series copies cell formats -> draw border here
        prepareRange( xSheet, "E9:K18", "XCellSeries" );


        // --- Array formulas ---
        xCellRange = xSheet.getCellRangeByName( "E21:G23" );
        prepareRange( xSheet, "E20:G23", "XArrayFormulaRange" );
        com.sun.star.sheet.XArrayFormulaRange xArrayFormula = UnoRuntime.queryInterface( com.sun.star.sheet.XArrayFormulaRange.class, xCellRange );
        // Insert a 3x3 unit matrix.
        xArrayFormula.setArrayFormula( "=A10:C12" );
        System.out.println( "Array formula is: " + xArrayFormula.getArrayFormula() );


        // --- Multiple operations ---
        setFormula( xSheet, "E26", "=E27^F26" );
        setValue( xSheet, "E27", 1 );
        setValue( xSheet, "F26", 1 );
        getCellSeries( xSheet, "E27:E31" ).fillAuto( com.sun.star.sheet.FillDirection.TO_BOTTOM, 1 );
        getCellSeries( xSheet, "F26:J26" ).fillAuto( com.sun.star.sheet.FillDirection.TO_RIGHT, 1 );
        setFormula( xSheet, "F33", "=SIN(E33)" );
        setFormula( xSheet, "G33", "=COS(E33)" );
        setFormula( xSheet, "H33", "=TAN(E33)" );
        setValue( xSheet, "E34", 0 );
        setValue( xSheet, "E35", 0.2 );
        getCellSeries( xSheet, "E34:E38" ).fillAuto( com.sun.star.sheet.FillDirection.TO_BOTTOM, 2 );
        prepareRange( xSheet, "E25:J38", "XMultipleOperation" );

        com.sun.star.table.CellRangeAddress aFormulaRange = createCellRangeAddress( xSheet, "E26" );
        com.sun.star.table.CellAddress aColCell = createCellAddress( xSheet, "E27" );
        com.sun.star.table.CellAddress aRowCell = createCellAddress( xSheet, "F26" );

        xCellRange = xSheet.getCellRangeByName( "E26:J31" );
        com.sun.star.sheet.XMultipleOperation xMultOp = UnoRuntime.queryInterface( com.sun.star.sheet.XMultipleOperation.class, xCellRange );
        xMultOp.setTableOperation(
            aFormulaRange, com.sun.star.sheet.TableOperationMode.BOTH, aColCell, aRowCell );

        aFormulaRange = createCellRangeAddress( xSheet, "F33:H33" );
        aColCell = createCellAddress( xSheet, "E33" );
        // Row cell not needed

        xCellRange = xSheet.getCellRangeByName( "E34:H38" );
        xMultOp = UnoRuntime.queryInterface( com.sun.star.sheet.XMultipleOperation.class, xCellRange );
        xMultOp.setTableOperation(
            aFormulaRange, com.sun.star.sheet.TableOperationMode.COLUMN, aColCell, aRowCell );


        // --- Cell Ranges Query ---
        xCellRange = xSheet.getCellRangeByName( "A10:C30" );
        com.sun.star.sheet.XCellRangesQuery xRangesQuery = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangesQuery.class, xCellRange );
        com.sun.star.sheet.XSheetCellRanges xCellRanges =
            xRangesQuery.queryContentCells( (short)com.sun.star.sheet.CellFlags.STRING );
        System.out.println(
            "Cells in A10:C30 containing text: "
            + xCellRanges.getRangeAddressesAsString() );
    }

    /** Returns the XCellSeries interface of a cell range.
        @param xSheet  The spreadsheet containing the cell range.
        @param aRange  The address of the cell range.
        @return  The XCellSeries interface. */
    private com.sun.star.sheet.XCellSeries getCellSeries(
            com.sun.star.sheet.XSpreadsheet xSheet, String aRange )
    {
        return UnoRuntime.queryInterface(
            com.sun.star.sheet.XCellSeries.class, xSheet.getCellRangeByName( aRange ) );
    }



    /** All samples regarding cell range collections. */
    private void doCellRangesSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for cell range collections ***\n" );

        // Create a new cell range container
        com.sun.star.lang.XMultiServiceFactory xDocFactory =
            UnoRuntime.queryInterface(
            com.sun.star.lang.XMultiServiceFactory.class, getDocument() );
        com.sun.star.sheet.XSheetCellRangeContainer xRangeCont =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XSheetCellRangeContainer.class,
            xDocFactory.createInstance( "com.sun.star.sheet.SheetCellRanges" ) );


        // --- Insert ranges ---
        insertRange( xRangeCont, 0, 0, 0, 0, 0, false );    // A1:A1
        insertRange( xRangeCont, 0, 0, 1, 0, 2, true );     // A2:A3
        insertRange( xRangeCont, 0, 1, 0, 1, 2, false );    // B1:B3


        // --- Query the list of filled cells ---
        System.out.print( "All filled cells: " );
        com.sun.star.container.XEnumerationAccess xCellsEA = xRangeCont.getCells();
        com.sun.star.container.XEnumeration xEnum = xCellsEA.createEnumeration();
        while( xEnum.hasMoreElements() )
        {
            Object aCellObj = xEnum.nextElement();
            com.sun.star.sheet.XCellAddressable xAddr = UnoRuntime.queryInterface( com.sun.star.sheet.XCellAddressable.class, aCellObj );
            com.sun.star.table.CellAddress aAddr = xAddr.getCellAddress();
            System.out.print( getCellAddressString( aAddr.Column, aAddr.Row ) + " " );
        }
        System.out.println();
    }

    /** Inserts a cell range address into a cell range container and prints
        a message.
        @param xContainer  The com.sun.star.sheet.XSheetCellRangeContainer interface of the container.
        @param nSheet  Index of sheet of the range.
        @param nStartCol  Index of first column of the range.
        @param nStartRow  Index of first row of the range.
        @param nEndCol  Index of last column of the range.
        @param nEndRow  Index of last row of the range.
        @param bMerge  Determines whether the new range should be merged with the existing ranges. */
    private void insertRange(
            com.sun.star.sheet.XSheetCellRangeContainer xContainer,
            int nSheet, int nStartCol, int nStartRow, int nEndCol, int nEndRow,
            boolean bMerge ) throws RuntimeException, Exception
    {
        com.sun.star.table.CellRangeAddress aAddress = new com.sun.star.table.CellRangeAddress();
        aAddress.Sheet = (short)nSheet;
        aAddress.StartColumn = nStartCol;
        aAddress.StartRow = nStartRow;
        aAddress.EndColumn = nEndCol;
        aAddress.EndRow = nEndRow;
        xContainer.addRangeAddress( aAddress, bMerge );
        System.out.println(
            "Inserting " + getCellRangeAddressString( aAddress )
            + " " + (bMerge ? "   with" : "without") + " merge,"
            + " resulting list: " + xContainer.getRangeAddressesAsString() );
    }



    /** All samples regarding cell cursors. */
    private void doCellCursorSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for cell cursor ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );


        // --- Find the array formula using a cell cursor ---
        com.sun.star.table.XCellRange xRange = xSheet.getCellRangeByName( "F22" );
        com.sun.star.sheet.XSheetCellRange xCellRange = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetCellRange.class, xRange );
        com.sun.star.sheet.XSheetCellCursor xCursor = xSheet.createCursorByRange( xCellRange );

        xCursor.collapseToCurrentArray();
        com.sun.star.sheet.XArrayFormulaRange xArray = UnoRuntime.queryInterface( com.sun.star.sheet.XArrayFormulaRange.class, xCursor );
        System.out.println(
            "Array formula in " + getCellRangeAddressString( xCursor, false )
            + " contains formula " + xArray.getArrayFormula() );


        // --- Find the used area ---
        com.sun.star.sheet.XUsedAreaCursor xUsedCursor = UnoRuntime.queryInterface( com.sun.star.sheet.XUsedAreaCursor.class, xCursor );
        xUsedCursor.gotoStartOfUsedArea( false );
        xUsedCursor.gotoEndOfUsedArea( true );
        // xUsedCursor and xCursor are interfaces of the same object -
        // so modifying xUsedCursor takes effect on xCursor:
        System.out.println( "The used area is: " + getCellRangeAddressString( xCursor, true ) );
    }



    /** All samples regarding the formatting of cells and ranges. */
    private void doFormattingSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Formatting samples ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 1 );
        com.sun.star.table.XCellRange xCellRange;
        com.sun.star.beans.XPropertySet xPropSet = null;
        com.sun.star.container.XIndexAccess xRangeIA = null;
        com.sun.star.lang.XMultiServiceFactory xDocServiceManager;


        // --- Cell styles ---
        // get the cell style container
        com.sun.star.style.XStyleFamiliesSupplier xFamiliesSupplier = UnoRuntime.queryInterface( com.sun.star.style.XStyleFamiliesSupplier.class, getDocument() );
        com.sun.star.container.XNameAccess xFamiliesNA = xFamiliesSupplier.getStyleFamilies();
        Object aCellStylesObj = xFamiliesNA.getByName( "CellStyles" );
        com.sun.star.container.XNameContainer xCellStylesNA = UnoRuntime.queryInterface( com.sun.star.container.XNameContainer.class, aCellStylesObj );

        // create a new cell style
        xDocServiceManager = UnoRuntime.queryInterface( com.sun.star.lang.XMultiServiceFactory.class, getDocument() );
        Object aCellStyle = xDocServiceManager.createInstance( "com.sun.star.style.CellStyle" );
        String aStyleName = "MyNewCellStyle";
        xCellStylesNA.insertByName( aStyleName, aCellStyle );

        // modify properties of the new style
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aCellStyle );
        xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( 0x888888 ) );
        xPropSet.setPropertyValue( "IsCellBackgroundTransparent", Boolean.FALSE );



        // --- Query equal-formatted cell ranges ---
        // prepare example, use the new cell style
        xCellRange = xSheet.getCellRangeByName( "D2:F2" );
        xPropSet = UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, xCellRange );
        xPropSet.setPropertyValue( "CellStyle", aStyleName );

        xCellRange = xSheet.getCellRangeByName( "A3:G3" );
        xPropSet = UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, xCellRange );
        xPropSet.setPropertyValue( "CellStyle", aStyleName );

        // All ranges in one container
        xCellRange = xSheet.getCellRangeByName( "A1:G3" );
        System.out.println( "Service CellFormatRanges:" );
        com.sun.star.sheet.XCellFormatRangesSupplier xFormatSupp =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XCellFormatRangesSupplier.class, xCellRange );
        xRangeIA = xFormatSupp.getCellFormatRanges();
        System.out.println( getCellRangeListString( xRangeIA ) );

        // Ranges sorted in SheetCellRanges containers
        System.out.println( "\nService UniqueCellFormatRanges:" );
        com.sun.star.sheet.XUniqueCellFormatRangesSupplier xUniqueFormatSupp =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XUniqueCellFormatRangesSupplier.class, xCellRange );
        com.sun.star.container.XIndexAccess xRangesIA = xUniqueFormatSupp.getUniqueCellFormatRanges();
        int nCount = xRangesIA.getCount();
        for (int nIndex = 0; nIndex < nCount; ++nIndex)
        {
            Object aRangesObj = xRangesIA.getByIndex( nIndex );
            xRangeIA = UnoRuntime.queryInterface(
                com.sun.star.container.XIndexAccess.class, aRangesObj );
            System.out.println(
                "Container " + (nIndex + 1) + ": " + getCellRangeListString( xRangeIA ) );
        }


        // --- Table auto formats ---
        // get the global collection of table auto formats, use global service
        // manager
        com.sun.star.lang.XMultiComponentFactory xServiceManager = getServiceManager();

        Object aAutoFormatsObj = xServiceManager.createInstanceWithContext(
            "com.sun.star.sheet.TableAutoFormats", getContext());
        com.sun.star.container.XNameContainer xAutoFormatsNA =
            UnoRuntime.queryInterface(
            com.sun.star.container.XNameContainer.class, aAutoFormatsObj );

        // create a new table auto format and insert into the container
        String aAutoFormatName =  "Temp_Example";
        boolean bExistsAlready = xAutoFormatsNA.hasByName( aAutoFormatName );
        Object aAutoFormatObj = null;
        if (bExistsAlready)
            // auto format already exists -> use it
            aAutoFormatObj = xAutoFormatsNA.getByName( aAutoFormatName );
        else
        {
            aAutoFormatObj = xDocServiceManager.createInstance(
                "com.sun.star.sheet.TableAutoFormat" );
            xAutoFormatsNA.insertByName( aAutoFormatName, aAutoFormatObj );
        }
        // index access to the auto format fields
        com.sun.star.container.XIndexAccess xAutoFormatIA =
            UnoRuntime.queryInterface(
            com.sun.star.container.XIndexAccess.class, aAutoFormatObj );

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
                Object aFieldObj = xAutoFormatIA.getByIndex( 4 * nRow + nColumn );
                xPropSet = UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, aFieldObj );
                xPropSet.setPropertyValue( "CellBackColor", Integer.valueOf( nColor ) );
            }
        }

        // set the auto format to the spreadsheet
        xCellRange = xSheet.getCellRangeByName( "A5:H25" );
        com.sun.star.table.XAutoFormattable xAutoForm = UnoRuntime.queryInterface( com.sun.star.table.XAutoFormattable.class, xCellRange );
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
        xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        com.sun.star.sheet.XSheetConditionalEntries xEntries =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XSheetConditionalEntries.class,
            xPropSet.getPropertyValue( "ConditionalFormat" ));

        // create a condition and apply it to the range
        com.sun.star.beans.PropertyValue[] aCondition = new com.sun.star.beans.PropertyValue[3];
        aCondition[0] = new com.sun.star.beans.PropertyValue();
        aCondition[0].Name  = "Operator";
        aCondition[0].Value = com.sun.star.sheet.ConditionOperator.GREATER;
        aCondition[1] = new com.sun.star.beans.PropertyValue();
        aCondition[1].Name  = "Formula1";
        aCondition[1].Value = "1";
        aCondition[2] = new com.sun.star.beans.PropertyValue();
        aCondition[2].Name  = "StyleName";
        aCondition[2].Value = aStyleName;
        xEntries.addNew( aCondition );
        xPropSet.setPropertyValue( "ConditionalFormat", xEntries );
    }



    /** All samples regarding the spreadsheet document. */
    private void doDocumentSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for spreadsheet document ***\n" );


        // --- Insert a new spreadsheet ---
        com.sun.star.sheet.XSpreadsheet xSheet = insertSpreadsheet( "A new sheet", (short)0x7FFF );


        // --- Copy a cell range ---
        prepareRange( xSheet, "A1:B3", "Copy from" );
        prepareRange( xSheet, "D1:E3", "To" );
        setValue( xSheet, "A2", 123 );
        setValue( xSheet, "B2", 345 );
        setFormula( xSheet, "A3", "=SUM(A2:B2)" );
        setFormula( xSheet, "B3", "=FORMULA(A3)" );

        com.sun.star.sheet.XCellRangeMovement xMovement = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeMovement.class, xSheet );
        com.sun.star.table.CellRangeAddress aSourceRange = createCellRangeAddress( xSheet, "A2:B3" );
        com.sun.star.table.CellAddress aDestCell = createCellAddress( xSheet, "D2" );
        xMovement.copyRange( aDestCell, aSourceRange );


        // --- Print automatic column page breaks ---
        com.sun.star.sheet.XSheetPageBreak xPageBreak = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetPageBreak.class, xSheet );
        com.sun.star.sheet.TablePageBreakData[] aPageBreakArray = xPageBreak.getColumnPageBreaks();

        System.out.print( "Automatic column page breaks:" );
        for (int nIndex = 0; nIndex < aPageBreakArray.length; ++nIndex)
            if (!aPageBreakArray[nIndex].ManualBreak)
                System.out.print( " " + aPageBreakArray[nIndex].Position );
        System.out.println();


        // --- Document properties ---
        com.sun.star.beans.XPropertySet xPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, getDocument() );

        String aText = "Value of property IsIterationEnabled: ";
        aText += AnyConverter.toBoolean(xPropSet.getPropertyValue( "IsIterationEnabled" ));
        System.out.println( aText );
        aText = "Value of property IterationCount: ";
        aText += AnyConverter.toInt(xPropSet.getPropertyValue( "IterationCount" ));
        System.out.println( aText );
        aText = "Value of property NullDate: ";
        com.sun.star.util.Date aDate = (com.sun.star.util.Date)
                AnyConverter.toObject(com.sun.star.util.Date.class, xPropSet.getPropertyValue( "NullDate" ));
        aText += aDate.Year + "-" + aDate.Month + "-" + aDate.Day;
        System.out.println( aText );


        // --- Data validation ---
        prepareRange( xSheet, "A5:C7", "Validation" );
        setFormula( xSheet, "A6", "Insert values between 0.0 and 5.0 below:" );

        com.sun.star.table.XCellRange xCellRange = xSheet.getCellRangeByName( "A7:C7" );
        com.sun.star.beans.XPropertySet xCellPropSet = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        // validation properties
        com.sun.star.beans.XPropertySet xValidPropSet = UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class,
                                   xCellPropSet.getPropertyValue( "Validation" ));
        xValidPropSet.setPropertyValue( "Type", com.sun.star.sheet.ValidationType.DECIMAL );
        xValidPropSet.setPropertyValue( "ShowErrorMessage", Boolean.TRUE );
        xValidPropSet.setPropertyValue( "ErrorMessage", "This is an invalid value!" );
        xValidPropSet.setPropertyValue( "ErrorAlertStyle", com.sun.star.sheet.ValidationAlertStyle.STOP );
        // condition
        com.sun.star.sheet.XSheetCondition xCondition = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetCondition.class, xValidPropSet );
        xCondition.setOperator( com.sun.star.sheet.ConditionOperator.BETWEEN );
        xCondition.setFormula1( "0.0" );
        xCondition.setFormula2( "5.0" );
        // apply on cell range
        xCellPropSet.setPropertyValue( "Validation", xValidPropSet );

        // --- Scenarios ---
        Object[][] aValues = new Object[2][2];

        aValues[0][0] = new Double( 11 );
        aValues[0][1] = new Double( 12 );
        aValues[1][0] = "Test13";
        aValues[1][1] = "Test14";
        insertScenario( xSheet, "B10:C11", aValues, "First Scenario", "The first scenario." );

        aValues[0][0] = "Test21";
        aValues[0][1] = "Test22";
        aValues[1][0] = new Double( 23 );
        aValues[1][1] = new Double( 24 );
        insertScenario( xSheet, "B10:C11", aValues, "Second Scenario", "The visible scenario." );

        aValues[0][0] = new Double( 31 );
        aValues[0][1] = new Double( 32 );
        aValues[1][0] = "Test33";
        aValues[1][1] = "Test34";
        insertScenario( xSheet, "B10:C11", aValues, "Third Scenario", "The last scenario." );

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
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aRange,
            Object[][] aValueArray,
            String aScenarioName,
            String aScenarioComment ) throws RuntimeException, Exception
    {
        // get the cell range with the given address
        com.sun.star.table.XCellRange xCellRange = xSheet.getCellRangeByName( aRange );

        // create the range address sequence
        com.sun.star.sheet.XCellRangeAddressable xAddr = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeAddressable.class, xCellRange );
        com.sun.star.table.CellRangeAddress[] aRangesSeq = new com.sun.star.table.CellRangeAddress[1];
        aRangesSeq[0] = xAddr.getRangeAddress();

        // create the scenario
        com.sun.star.sheet.XScenariosSupplier xScenSupp = UnoRuntime.queryInterface( com.sun.star.sheet.XScenariosSupplier.class, xSheet );
        com.sun.star.sheet.XScenarios xScenarios = xScenSupp.getScenarios();
        xScenarios.addNewByName( aScenarioName, aRangesSeq, aScenarioComment );

        // insert the values into the range
        com.sun.star.sheet.XCellRangeData xData = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeData.class, xCellRange );
        xData.setDataArray( aValueArray );
    }

    /** Activates a scenario.
        @param xSheet           The XSpreadsheet interface of the spreadsheet.
        @param aScenarioName    The name of the scenario. */
    private void showScenario(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aScenarioName ) throws RuntimeException, Exception
    {
        // get the scenario set
        com.sun.star.sheet.XScenariosSupplier xScenSupp = UnoRuntime.queryInterface( com.sun.star.sheet.XScenariosSupplier.class, xSheet );
        com.sun.star.sheet.XScenarios xScenarios = xScenSupp.getScenarios();

        // get the scenario and activate it
        Object aScenarioObj = xScenarios.getByName( aScenarioName );
        com.sun.star.sheet.XScenario xScenario = UnoRuntime.queryInterface( com.sun.star.sheet.XScenario.class, aScenarioObj );
        xScenario.apply();
    }



    private void doNamedRangesSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for named ranges ***\n" );
        com.sun.star.sheet.XSpreadsheetDocument xDocument = getDocument();
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );


        // --- Named ranges ---
        prepareRange( xSheet, "G42:H45", "Named ranges" );
        xSheet.getCellByPosition( 6, 42 ).setValue( 1 );
        xSheet.getCellByPosition( 6, 43 ).setValue( 2 );
        xSheet.getCellByPosition( 7, 42 ).setValue( 3 );
        xSheet.getCellByPosition( 7, 43 ).setValue( 4 );

        // insert a named range
        com.sun.star.beans.XPropertySet xDocProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xDocument );
        Object aRangesObj = xDocProp.getPropertyValue( "NamedRanges" );
        com.sun.star.sheet.XNamedRanges xNamedRanges = UnoRuntime.queryInterface( com.sun.star.sheet.XNamedRanges.class, aRangesObj );
        com.sun.star.table.CellAddress aRefPos = new com.sun.star.table.CellAddress();
        aRefPos.Sheet  = 0;
        aRefPos.Column = 6;
        aRefPos.Row    = 44;
        xNamedRanges.addNewByName( "ExampleName", "SUM(G43:G44)", aRefPos, 0 );

        // use the named range in formulas
        xSheet.getCellByPosition( 6, 44 ).setFormula( "=ExampleName" );
        xSheet.getCellByPosition( 7, 44 ).setFormula( "=ExampleName" );


        // --- Label ranges ---
        prepareRange( xSheet, "G47:I50", "Label ranges" );
        com.sun.star.table.XCellRange xRange = xSheet.getCellRangeByPosition( 6, 47, 7, 49 );
        com.sun.star.sheet.XCellRangeData xData = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeData.class, xRange );
        Object[][] aValues =
        {
            { "Apples", "Oranges" },
            { new Double( 5 ), new Double( 7 ) },
            { new Double( 6 ), new Double( 8 ) }
        };
        xData.setDataArray( aValues );

        // insert a column label range
        Object aLabelsObj = xDocProp.getPropertyValue( "ColumnLabelRanges" );
        com.sun.star.sheet.XLabelRanges xLabelRanges = UnoRuntime.queryInterface( com.sun.star.sheet.XLabelRanges.class, aLabelsObj );
        com.sun.star.table.CellRangeAddress aLabelArea = new com.sun.star.table.CellRangeAddress();
        aLabelArea.Sheet       = 0;
        aLabelArea.StartColumn = 6;
        aLabelArea.StartRow    = 47;
        aLabelArea.EndColumn   = 7;
        aLabelArea.EndRow      = 47;
        com.sun.star.table.CellRangeAddress aDataArea = new com.sun.star.table.CellRangeAddress();
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
        try
        {
            com.sun.star.lang.XMultiComponentFactory xServiceManager = getServiceManager();
            com.sun.star.container.XNameAccess xContext =
                UnoRuntime.queryInterface(
                com.sun.star.container.XNameAccess.class,
                xServiceManager.createInstanceWithContext(
                    "com.sun.star.sdb.DatabaseContext", getContext()) );
            String[] aNames = xContext.getElementNames();
            if ( aNames.length > 0 )
                aDatabase = aNames[0];
        }
        catch ( Exception e )
        {
            System.out.println( "\nError: caught exception in getFirstDatabaseName()!\n" +
                                "Exception Message = "
                                + e.getMessage());
            e.printStackTrace();
        }
        return aDatabase;
    }

    /** Helper for doDatabaseSamples: get name of first table in a database. */
    private String getFirstTableName( String aDatabase )
    {
        if ( aDatabase == null )
            return null;

        String aTable = null;
        try
        {
            com.sun.star.lang.XMultiComponentFactory xServiceManager = getServiceManager();
            com.sun.star.container.XNameAccess xContext = UnoRuntime.queryInterface( com.sun.star.container.XNameAccess.class,
                xServiceManager.createInstanceWithContext(
                    "com.sun.star.sdb.DatabaseContext", getContext()) );
            com.sun.star.sdb.XCompletedConnection xSource =
                UnoRuntime.queryInterface(
                com.sun.star.sdb.XCompletedConnection.class,
                xContext.getByName( aDatabase ) );
            com.sun.star.task.XInteractionHandler xHandler =
                UnoRuntime.queryInterface(
                com.sun.star.task.XInteractionHandler.class,
                xServiceManager.createInstanceWithContext(
                    "com.sun.star.task.InteractionHandler", getContext()) );
            com.sun.star.sdbcx.XTablesSupplier xSupplier =
                UnoRuntime.queryInterface(
                com.sun.star.sdbcx.XTablesSupplier.class,
                xSource.connectWithCompletion( xHandler ) );
            com.sun.star.container.XNameAccess xTables = xSupplier.getTables();
            String[] aNames = xTables.getElementNames();
            if ( aNames.length > 0 )
                aTable = aNames[0];
        }
        catch ( Exception e )
        {
            System.out.println( "\nError: caught exception in getFirstTableName()!\n" +
                                "Exception Message = "
                                + e.getMessage());
            e.printStackTrace();
        }
        return aTable;
    }

    private void doDatabaseSamples() throws Exception
    {
        System.out.println( "\n*** Samples for database operations ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 2 );


        // --- put some example data into the sheet ---
        com.sun.star.table.XCellRange xRange = xSheet.getCellRangeByName( "B3:D24" );
        com.sun.star.sheet.XCellRangeData xData = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeData.class, xRange );
        Object[][] aValues =
        {
            { "Name",             "Year",            "Sales" },
            { "Alice", new Double( 2001 ), new Double( 4.0 ) },
            { "Carol", new Double( 1997 ), new Double( 3.0 ) },
            { "Carol", new Double( 1998 ), new Double( 8.0 ) },
            { "Bob",   new Double( 1997 ), new Double( 8.0 ) },
            { "Alice", new Double( 2002 ), new Double( 9.0 ) },
            { "Alice", new Double( 1999 ), new Double( 7.0 ) },
            { "Alice", new Double( 1996 ), new Double( 3.0 ) },
            { "Bob",   new Double( 2000 ), new Double( 1.0 ) },
            { "Carol", new Double( 1999 ), new Double( 5.0 ) },
            { "Bob",   new Double( 2002 ), new Double( 1.0 ) },
            { "Carol", new Double( 2001 ), new Double( 5.0 ) },
            { "Carol", new Double( 2000 ), new Double( 1.0 ) },
            { "Carol", new Double( 1996 ), new Double( 8.0 ) },
            { "Bob",   new Double( 1996 ), new Double( 7.0 ) },
            { "Alice", new Double( 1997 ), new Double( 3.0 ) },
            { "Alice", new Double( 2000 ), new Double( 9.0 ) },
            { "Bob",   new Double( 1998 ), new Double( 1.0 ) },
            { "Bob",   new Double( 1999 ), new Double( 6.0 ) },
            { "Carol", new Double( 2002 ), new Double( 8.0 ) },
            { "Alice", new Double( 1998 ), new Double( 5.0 ) },
            { "Bob",   new Double( 2001 ), new Double( 6.0 ) }
        };
        xData.setDataArray( aValues );


        // --- filter for second column >= 1998 ---
        com.sun.star.sheet.XSheetFilterable xFilter = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetFilterable.class, xRange );
        com.sun.star.sheet.XSheetFilterDescriptor xFilterDesc =
            xFilter.createFilterDescriptor( true );
        com.sun.star.sheet.TableFilterField[] aFilterFields =
            new com.sun.star.sheet.TableFilterField[1];
        aFilterFields[0] = new com.sun.star.sheet.TableFilterField();
        aFilterFields[0].Field        = 1;
        aFilterFields[0].IsNumeric    = true;
        aFilterFields[0].Operator     = com.sun.star.sheet.FilterOperator.GREATER_EQUAL;
        aFilterFields[0].NumericValue = 1998;
        xFilterDesc.setFilterFields( aFilterFields );
        com.sun.star.beans.XPropertySet xFilterProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xFilterDesc );
        xFilterProp.setPropertyValue( "ContainsHeader", Boolean.TRUE );
        xFilter.filter( xFilterDesc );


        // --- do the same filter as above, using criteria from a cell range ---
        com.sun.star.table.XCellRange xCritRange = xSheet.getCellRangeByName( "B27:B28" );
        com.sun.star.sheet.XCellRangeData xCritData = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeData.class, xCritRange );
        Object[][] aCritValues =
        {
            { "Year"    },
            { ">= 1998" }
        };
        xCritData.setDataArray( aCritValues );
        com.sun.star.sheet.XSheetFilterableEx xCriteria = UnoRuntime.queryInterface( com.sun.star.sheet.XSheetFilterableEx.class, xCritRange );
        xFilterDesc = xCriteria.createFilterDescriptorByObject( xFilter );
        if ( xFilterDesc != null )
            xFilter.filter( xFilterDesc );


        // --- sort by second column, ascending ---
        com.sun.star.table.TableSortField[] aSortFields = new com.sun.star.table.TableSortField[1];
        aSortFields[0] = new com.sun.star.table.TableSortField();
        aSortFields[0].Field         = 1;
        aSortFields[0].IsAscending = false;
        aSortFields[0].IsCaseSensitive = false;


        com.sun.star.beans.PropertyValue[] aSortDesc = new com.sun.star.beans.PropertyValue[2];
        aSortDesc[0] = new com.sun.star.beans.PropertyValue();
        aSortDesc[0].Name   = "SortFields";
        aSortDesc[0].Value  = aSortFields;
        aSortDesc[1] = new com.sun.star.beans.PropertyValue();
        aSortDesc[1].Name   = "ContainsHeader";
        aSortDesc[1].Value  = Boolean.TRUE;

        com.sun.star.util.XSortable xSort = UnoRuntime.queryInterface( com.sun.star.util.XSortable.class, xRange );
        xSort.sort( aSortDesc );


        // --- insert subtotals ---
        com.sun.star.sheet.XSubTotalCalculatable xSub = UnoRuntime.queryInterface( com.sun.star.sheet.XSubTotalCalculatable.class, xRange );
        com.sun.star.sheet.XSubTotalDescriptor xSubDesc = xSub.createSubTotalDescriptor( true );
        com.sun.star.sheet.SubTotalColumn[] aColumns = new com.sun.star.sheet.SubTotalColumn[1];
        // calculate sum of third column
        aColumns[0] = new com.sun.star.sheet.SubTotalColumn();
        aColumns[0].Column   = 2;
        aColumns[0].Function = com.sun.star.sheet.GeneralFunction.SUM;
        // group by first column
        xSubDesc.addNew( aColumns, 0 );
        xSub.applySubTotals( xSubDesc, true );

        String aDatabase = getFirstDatabaseName();
        String aTableName = getFirstTableName( aDatabase );
        if ( aDatabase != null && aTableName != null )
        {
            // --- import from database ---
            com.sun.star.beans.PropertyValue[] aImportDesc = new com.sun.star.beans.PropertyValue[3];
            aImportDesc[0] = new com.sun.star.beans.PropertyValue();
            aImportDesc[0].Name     = "DatabaseName";
            aImportDesc[0].Value    = aDatabase;
            aImportDesc[1] = new com.sun.star.beans.PropertyValue();
            aImportDesc[1].Name     = "SourceType";
            aImportDesc[1].Value    = com.sun.star.sheet.DataImportMode.TABLE;
            aImportDesc[2] = new com.sun.star.beans.PropertyValue();
            aImportDesc[2].Name     = "SourceObject";
            aImportDesc[2].Value    = aTableName;

            com.sun.star.table.XCellRange xImportRange = xSheet.getCellRangeByName( "B35:B35" );
            com.sun.star.util.XImportable xImport = UnoRuntime.queryInterface( com.sun.star.util.XImportable.class, xImportRange );
            xImport.doImport( aImportDesc );


            // --- use the temporary database range to find the imported data's size ---
            com.sun.star.beans.XPropertySet xDocProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, getDocument() );
            Object aRangesObj = xDocProp.getPropertyValue( "DatabaseRanges" );
            com.sun.star.container.XNameAccess xRanges =
                UnoRuntime.queryInterface(
                com.sun.star.container.XNameAccess.class, aRangesObj );
            String[] aNames = xRanges.getElementNames();
            for ( int i=0; i<aNames.length; i++ )
            {
                Object aRangeObj = xRanges.getByName( aNames[i] );
                com.sun.star.beans.XPropertySet xRangeProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aRangeObj );
                boolean bUser = AnyConverter.toBoolean(xRangeProp.getPropertyValue( "IsUserDefined" ));
                if ( !bUser )
                {
                    // this is the temporary database range - get the cell range and format it
                    com.sun.star.sheet.XCellRangeReferrer xRef = UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeReferrer.class, aRangeObj );
                    com.sun.star.table.XCellRange xResultRange = xRef.getReferredCells();
                    com.sun.star.beans.XPropertySet xResultProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xResultRange );
                    xResultProp.setPropertyValue( "IsCellBackgroundTransparent", Boolean.FALSE );
                    xResultProp.setPropertyValue( "CellBackColor", Integer.valueOf( 0xFFFFCC ) );
                }
            }
        }
        else
            System.out.println("can't get database");
    }



    private void doDataPilotSamples() throws Exception
    {
        System.out.println( "\n*** Samples for Data Pilot ***\n" );
        com.sun.star.sheet.XSpreadsheet xSheet = getSpreadsheet( 0 );


        // --- Create a new DataPilot table ---
        prepareRange( xSheet, "A38:C38", "Data Pilot" );
        com.sun.star.sheet.XDataPilotTablesSupplier xDPSupp = UnoRuntime.queryInterface( com.sun.star.sheet.XDataPilotTablesSupplier.class, xSheet );
        com.sun.star.sheet.XDataPilotTables xDPTables = xDPSupp.getDataPilotTables();
        com.sun.star.sheet.XDataPilotDescriptor xDPDesc = xDPTables.createDataPilotDescriptor();
        // set source range (use data range from CellRange test)
        com.sun.star.table.CellRangeAddress aSourceAddress = createCellRangeAddress( xSheet, "A10:C30" );
        xDPDesc.setSourceRange( aSourceAddress );
        // settings for fields
        com.sun.star.container.XIndexAccess xFields = xDPDesc.getDataPilotFields();
        Object aFieldObj;
        com.sun.star.beans.XPropertySet xFieldProp;
        // use first column as column field
        aFieldObj = xFields.getByIndex(0);
        xFieldProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aFieldObj );
        xFieldProp.setPropertyValue( "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.COLUMN );
        // use second column as row field
        aFieldObj = xFields.getByIndex(1);
        xFieldProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aFieldObj );
        xFieldProp.setPropertyValue( "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.ROW );
        // use third column as data field, calculating the sum
        aFieldObj = xFields.getByIndex(2);
        xFieldProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aFieldObj );
        xFieldProp.setPropertyValue( "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.DATA );
        xFieldProp.setPropertyValue( "Function", com.sun.star.sheet.GeneralFunction.SUM );
        // select output position
        com.sun.star.table.CellAddress aDestAddress = createCellAddress( xSheet, "A40" );
        xDPTables.insertNewByName( "DataPilotExample", aDestAddress, xDPDesc );


        // --- Modify the DataPilot table ---
        Object aDPTableObj = xDPTables.getByName( "DataPilotExample" );
        xDPDesc = UnoRuntime.queryInterface( com.sun.star.sheet.XDataPilotDescriptor.class, aDPTableObj );
        xFields = xDPDesc.getDataPilotFields();
        // add a second data field from the third column, calculating the average
        aFieldObj = xFields.getByIndex(2);
        xFieldProp = UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, aFieldObj );
        xFieldProp.setPropertyValue( "Orientation", com.sun.star.sheet.DataPilotFieldOrientation.DATA );
        xFieldProp.setPropertyValue( "Function", com.sun.star.sheet.GeneralFunction.AVERAGE );
    }



    private void doFunctionAccessSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for function handling ***\n" );
        com.sun.star.lang.XMultiComponentFactory xServiceManager = getServiceManager();


        // --- Calculate a function ---
        Object aFuncInst = xServiceManager.createInstanceWithContext(
            "com.sun.star.sheet.FunctionAccess", getContext());
        com.sun.star.sheet.XFunctionAccess xFuncAcc =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XFunctionAccess.class, aFuncInst );
        // put the data in a two-dimensional array
        double[][] aData = { { 1.0, 2.0, 3.0 } };
        // construct the array of function arguments
        Object[] aArgs = new Object[2];
        aArgs[0] = aData;
        aArgs[1] = new Double( 2.0 );
        Object aResult = xFuncAcc.callFunction( "ZTEST", aArgs );
        System.out.println("ZTEST result for data {1,2,3} and value 2 is "
                                        + ((Double)aResult).doubleValue() );


        // --- Get the list of recently used functions ---
        Object aRecInst = xServiceManager.createInstanceWithContext(
            "com.sun.star.sheet.RecentFunctions", getContext());
        com.sun.star.sheet.XRecentFunctions xRecFunc =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XRecentFunctions.class, aRecInst );
        int[] nRecentIds = xRecFunc.getRecentFunctionIds();


        // --- Get the names for these functions ---
        Object aDescInst = xServiceManager.createInstanceWithContext(
            "com.sun.star.sheet.FunctionDescriptions", getContext());
        com.sun.star.sheet.XFunctionDescriptions xFuncDesc =
            UnoRuntime.queryInterface(
            com.sun.star.sheet.XFunctionDescriptions.class, aDescInst );
        System.out.print("Recently used functions: ");
        for (int nFunction=0; nFunction<nRecentIds.length; nFunction++)
        {
            com.sun.star.beans.PropertyValue[] aProperties =
                xFuncDesc.getById( nRecentIds[nFunction] );
            for (int nProp=0; nProp<aProperties.length; nProp++)
                if ( aProperties[nProp].Name.equals( "Name" ) )
                    System.out.print( aProperties[nProp].Value + " " );
        }
        System.out.println();
    }



    private void doApplicationSettingsSamples() throws RuntimeException, Exception
    {
        System.out.println( "\n*** Samples for application settings ***\n" );
        com.sun.star.lang.XMultiComponentFactory xServiceManager = getServiceManager();


        // --- Get the user defined sort lists ---
        Object aSettings = xServiceManager.createInstanceWithContext(
            "com.sun.star.sheet.GlobalSheetSettings", getContext());
        com.sun.star.beans.XPropertySet xPropSet =
            UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, aSettings );
        String[] aEntries = (String[])
                AnyConverter.toObject(String[].class,
                              xPropSet.getPropertyValue( "UserLists" ));
        System.out.println("User defined sort lists:");
        for ( int i=0; i<aEntries.length; i++ )
            System.out.println( aEntries[i] );
    }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
