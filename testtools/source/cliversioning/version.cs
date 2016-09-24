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
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.frame;
using unoidl.com.sun.star.util;

namespace cliversion
{
public class Version
{
    public Version()
    {
		try
		{	
//			System.Diagnostics.Debugger.Launch();

			//link with cli_ure.dll
			uno.util.WeakBase wb = new uno.util.WeakBase();
			using ( SpreadsheetSample aSample = new SpreadsheetSample() )
			{
				aSample.doCellRangeSamples();
				aSample.terminate();
			}
		}
		catch (System.Exception )
		{
			//This exception is thrown if we link with a library which is not 
			//available
			throw;
		}
    }
}

class SpreadsheetSample: SpreadsheetDocHelper
{
    public SpreadsheetSample()
    {
    }
        /** All samples regarding the service com.sun.star.sheet.SheetCellRange. */
    public void doCellRangeSamples()
    {
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

        // --- Merge cells. ---
        xCellRange = xSheet.getCellRangeByName( "F3:G6" );
        prepareRange( xSheet, "E1:H7", "XMergeable" );
        unoidl.com.sun.star.util.XMergeable xMerge =
            (unoidl.com.sun.star.util.XMergeable) xCellRange;
        xMerge.merge( true );


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

        // --- Sheet operation. ---
        // uses the range filled with XCellRangeData
        unoidl.com.sun.star.sheet.XSheetOperation xSheetOp =
            (unoidl.com.sun.star.sheet.XSheetOperation) xData;
        double fResult = xSheetOp.computeFunction(
            unoidl.com.sun.star.sheet.GeneralFunction.AVERAGE );

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

}

/** This is a helper class for the spreadsheet and table samples.
    It connects to a running office and creates a spreadsheet document.
    Additionally it contains various helper functions.
 */
class SpreadsheetDocHelper : System.IDisposable
{

// __  private members  ___________________________________________

    private const String  msDataSheetName  = "Data";

    private unoidl.com.sun.star.uno.XComponentContext m_xContext;
    private unoidl.com.sun.star.lang.XMultiServiceFactory  mxMSFactory;
    private unoidl.com.sun.star.sheet.XSpreadsheetDocument mxDocument;



    public SpreadsheetDocHelper()
    {
//        System.Diagnostics.Debugger.Launch();
        // Connect to a running office and get the service manager
        mxMSFactory = connect();
        // Create a new spreadsheet document
        mxDocument = initDocument();
    }

// __  helper methods  ____________________________________________

    /** Returns the service manager.
        @return  XMultiServiceFactory interface of the service manager. */
    public unoidl.com.sun.star.lang.XMultiServiceFactory getServiceManager()
    {
        return mxMSFactory;
    }

    /** Returns the whole spreadsheet document.
        @return  XSpreadsheetDocument interface of the document. */
    public unoidl.com.sun.star.sheet.XSpreadsheetDocument getDocument()
    {
        return mxDocument;
    }

    /** Returns the spreadsheet with the specified index (0-based).
        @param nIndex  The index of the sheet.
        @return  XSpreadsheet interface of the sheet. */
    public unoidl.com.sun.star.sheet.XSpreadsheet getSpreadsheet( int nIndex )
    {
        // Collection of sheets
        unoidl.com.sun.star.sheet.XSpreadsheets xSheets =
            mxDocument.getSheets();
        
        unoidl.com.sun.star.container.XIndexAccess xSheetsIA =
            (unoidl.com.sun.star.container.XIndexAccess) xSheets;
        
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
            (unoidl.com.sun.star.sheet.XSpreadsheet)
              xSheetsIA.getByIndex( nIndex ).Value;
        
        return xSheet;
    }

    /** Inserts a new empty spreadsheet with the specified name.
        @param aName  The name of the new sheet.
        @param nIndex  The insertion index.
        @return  The XSpreadsheet interface of the new sheet. */
    public unoidl.com.sun.star.sheet.XSpreadsheet insertSpreadsheet(
        String aName, short nIndex )
    {
        // Collection of sheets
        unoidl.com.sun.star.sheet.XSpreadsheets xSheets =
            mxDocument.getSheets();
        
        xSheets.insertNewByName( aName, nIndex );
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
            (unoidl.com.sun.star.sheet.XSpreadsheet)
              xSheets.getByName( aName ).Value;
        
        return xSheet;
    }


// Methods to fill values into cells.

    /** Writes a double value into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param fValue  The value to write into the cell. */
    public void setValue(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
        String aCellName,
        double fValue )
    {
        xSheet.getCellRangeByName( aCellName ).getCellByPosition(
            0, 0 ).setValue( fValue );
    }

    /** Writes a formula into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param aFormula  The formula to write into the cell. */
    public void setFormula(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
        String aCellName,
        String aFormula )
    {
        xSheet.getCellRangeByName( aCellName ).getCellByPosition(
            0, 0 ).setFormula( aFormula );
    }

    /** Writes a date with standard date format into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param nDay  The day of the date.
        @param nMonth  The month of the date.
        @param nYear  The year of the date. */
    public void setDate(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
        String aCellName,
        int nDay, int nMonth, int nYear )
    {
        // Set the date value.
        unoidl.com.sun.star.table.XCell xCell =
            xSheet.getCellRangeByName( aCellName ).getCellByPosition( 0, 0 );
        String aDateStr = nMonth + "/" + nDay + "/" + nYear;
        xCell.setFormula( aDateStr );

        // Set standard date format.
        unoidl.com.sun.star.util.XNumberFormatsSupplier xFormatsSupplier =
            (unoidl.com.sun.star.util.XNumberFormatsSupplier) getDocument();
        unoidl.com.sun.star.util.XNumberFormatTypes xFormatTypes =
            (unoidl.com.sun.star.util.XNumberFormatTypes)
              xFormatsSupplier.getNumberFormats();
        int nFormat = xFormatTypes.getStandardFormat(
            unoidl.com.sun.star.util.NumberFormat.DATE,
            new unoidl.com.sun.star.lang.Locale() );

        unoidl.com.sun.star.beans.XPropertySet xPropSet =
            (unoidl.com.sun.star.beans.XPropertySet) xCell;
        xPropSet.setPropertyValue(
            "NumberFormat",
            new uno.Any( (Int32) nFormat ) );
    }

    /** Draws a colored border around the range and writes the headline
        in the first cell.
        
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aRange  The address of the cell range (or a named range).
        @param aHeadline  The headline text. */
    public void prepareRange(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
        String aRange, String aHeadline )
    {
        unoidl.com.sun.star.beans.XPropertySet xPropSet = null;
        unoidl.com.sun.star.table.XCellRange xCellRange = null;
        
        // draw border
        xCellRange = xSheet.getCellRangeByName( aRange );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        unoidl.com.sun.star.table.BorderLine aLine =
            new unoidl.com.sun.star.table.BorderLine();
        aLine.Color = 0x99CCFF;
        aLine.InnerLineWidth = aLine.LineDistance = 0;
        aLine.OuterLineWidth = 100;
        unoidl.com.sun.star.table.TableBorder aBorder =
            new unoidl.com.sun.star.table.TableBorder();
        aBorder.TopLine = aBorder.BottomLine = aBorder.LeftLine =
            aBorder.RightLine = aLine;
        aBorder.IsTopLineValid = aBorder.IsBottomLineValid = true;
        aBorder.IsLeftLineValid = aBorder.IsRightLineValid = true;
        xPropSet.setPropertyValue(
            "TableBorder",
            new uno.Any(
                typeof (unoidl.com.sun.star.table.TableBorder), aBorder ) );

        // draw headline
        unoidl.com.sun.star.sheet.XCellRangeAddressable xAddr =
            (unoidl.com.sun.star.sheet.XCellRangeAddressable) xCellRange;
        unoidl.com.sun.star.table.CellRangeAddress aAddr =
            xAddr.getRangeAddress();

        xCellRange = xSheet.getCellRangeByPosition(
            aAddr.StartColumn,
            aAddr.StartRow, aAddr.EndColumn, aAddr.StartRow );
        
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCellRange;
        xPropSet.setPropertyValue(
            "CellBackColor", new uno.Any( (Int32) 0x99CCFF ) );
        // write headline
        unoidl.com.sun.star.table.XCell xCell =
            xCellRange.getCellByPosition( 0, 0 );
        xCell.setFormula( aHeadline );
        xPropSet = (unoidl.com.sun.star.beans.XPropertySet) xCell;
        xPropSet.setPropertyValue(
            "CharColor", new uno.Any( (Int32) 0x003399 ) );
        xPropSet.setPropertyValue(
            "CharWeight",
            new uno.Any( (Single) unoidl.com.sun.star.awt.FontWeight.BOLD ) );
    }


// Methods to create cell addresses and range addresses.

    /** Creates a unoidl.com.sun.star.table.CellAddress and initializes it
        with the given range.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCell  The address of the cell (or a named cell). */
    public unoidl.com.sun.star.table.CellAddress createCellAddress(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet,
        String aCell )
    {
        unoidl.com.sun.star.sheet.XCellAddressable xAddr =
        (unoidl.com.sun.star.sheet.XCellAddressable)
            xSheet.getCellRangeByName( aCell ).getCellByPosition( 0, 0 );
        return xAddr.getCellAddress();
    }

    /** Creates a unoidl.com.sun.star.table.CellRangeAddress and initializes
        it with the given range.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aRange  The address of the cell range (or a named range). */
    public unoidl.com.sun.star.table.CellRangeAddress createCellRangeAddress(
        unoidl.com.sun.star.sheet.XSpreadsheet xSheet, String aRange )
    {
        unoidl.com.sun.star.sheet.XCellRangeAddressable xAddr =
            (unoidl.com.sun.star.sheet.XCellRangeAddressable)
            xSheet.getCellRangeByName( aRange );
        return xAddr.getRangeAddress();
    }


// Methods to convert cell addresses and range addresses to strings.

    /** Returns the text address of the cell.
        @param nColumn  The column index.
        @param nRow  The row index.
        @return  A string containing the cell address. */
    public String getCellAddressString( int nColumn, int nRow )
    {
        String aStr = "";
        if (nColumn > 25)
            aStr += (char) ('A' + nColumn / 26 - 1);
        aStr += (char) ('A' + nColumn % 26);
        aStr += (nRow + 1);
        return aStr;
    }

    /** Returns the text address of the cell range.
        @param aCellRange  The cell range address.
        @return  A string containing the cell range address. */
    public String getCellRangeAddressString(
        unoidl.com.sun.star.table.CellRangeAddress aCellRange )
    {
        return
            getCellAddressString( aCellRange.StartColumn, aCellRange.StartRow )
            + ":"
            + getCellAddressString( aCellRange.EndColumn, aCellRange.EndRow );
    }

    /** Returns the text address of the cell range.
        @param xCellRange  The XSheetCellRange interface of the cell range.
        @param bWithSheet  true = Include sheet name.
        @return  A string containing the cell range address. */
    public String getCellRangeAddressString(
        unoidl.com.sun.star.sheet.XSheetCellRange xCellRange, bool bWithSheet )
    {
        String aStr = "";
        if (bWithSheet)
        {
            unoidl.com.sun.star.sheet.XSpreadsheet xSheet =
                xCellRange.getSpreadsheet();
            unoidl.com.sun.star.container.XNamed xNamed =
                (unoidl.com.sun.star.container.XNamed) xSheet;
            aStr += xNamed.getName() + ".";
        }
        unoidl.com.sun.star.sheet.XCellRangeAddressable xAddr =
            (unoidl.com.sun.star.sheet.XCellRangeAddressable) xCellRange;
        aStr += getCellRangeAddressString( xAddr.getRangeAddress() );
        return aStr;
    }

    /** Returns a list of addresses of all cell ranges contained in the
        collection.
        
        @param xRangesIA  The XIndexAccess interface of the collection.
        @return  A string containing the cell range address list. */
    public String getCellRangeListString(
        unoidl.com.sun.star.container.XIndexAccess xRangesIA )
    {
        String aStr = "";
        int nCount = xRangesIA.getCount();
        for (int nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (nIndex > 0)
                aStr += " ";
            uno.Any aRangeObj = xRangesIA.getByIndex( nIndex );
            unoidl.com.sun.star.sheet.XSheetCellRange xCellRange =
                (unoidl.com.sun.star.sheet.XSheetCellRange) aRangeObj.Value;
            aStr += getCellRangeAddressString( xCellRange, false );
        }
        return aStr;
    }



    /** Connect to a running office that is accepting connections.
        @return  The ServiceManager to instantiate office components. */
    private XMultiServiceFactory connect()
    {
        
        m_xContext = uno.util.Bootstrap.bootstrap();
        
        return (XMultiServiceFactory) m_xContext.getServiceManager();
    }

    public void Dispose()
    {

    }

    /** Creates an empty spreadsheet document.
        @return  The XSpreadsheetDocument interface of the document. */
    private unoidl.com.sun.star.sheet.XSpreadsheetDocument initDocument()
    {
        XComponentLoader aLoader = (XComponentLoader)
            mxMSFactory.createInstance( "com.sun.star.frame.Desktop" );
        
        XComponent xComponent = aLoader.loadComponentFromURL(
            "private:factory/scalc", "_blank", 0,
            new unoidl.com.sun.star.beans.PropertyValue[0] );

        return (unoidl.com.sun.star.sheet.XSpreadsheetDocument) xComponent;
    }


    public void terminate()
    {
        XModifiable xMod = (XModifiable) mxDocument;
        if (xMod != null)
            xMod.setModified(false);
        XDesktop aDesktop = (XDesktop)
            mxMSFactory.createInstance( "com.sun.star.frame.Desktop" );
        if (aDesktop != null)
        {
            try
            {
                aDesktop.terminate();
            }
            catch (DisposedException d)
            {
                //This exception may be thrown because shutting down OOo using
                //XDesktop terminate does not really work. In the case of the
                //Exception OOo will still terminate. 
            }
        }
    }


}

}
