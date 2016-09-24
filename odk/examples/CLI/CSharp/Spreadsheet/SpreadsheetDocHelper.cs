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
using unoidl.com.sun.star.bridge;
using unoidl.com.sun.star.frame;

// __________  implementation  ____________________________________

/** This is a helper class for the spreadsheet and table samples.
    It connects to a running office and creates a spreadsheet document.
    Additionally it contains various helper functions.
 */
public class SpreadsheetDocHelper : System.IDisposable
{

// __  private members  ___________________________________________

    private const String  msDataSheetName  = "Data";

    private unoidl.com.sun.star.uno.XComponentContext m_xContext;
    private unoidl.com.sun.star.lang.XMultiServiceFactory  mxMSFactory;
    private unoidl.com.sun.star.sheet.XSpreadsheetDocument mxDocument;



    public SpreadsheetDocHelper( String[] args )
    {
        // Connect to a running office and get the service manager
        mxMSFactory = connect( args );
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
    private XMultiServiceFactory connect( String [] args )
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


}
