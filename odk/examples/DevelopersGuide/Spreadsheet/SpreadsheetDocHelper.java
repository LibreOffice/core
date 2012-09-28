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

import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.RuntimeException;

// __________  implementation  ____________________________________

/** This is a helper class for the spreadsheet and table samples.
    It connects to a running office and creates a spreadsheet document.
    Additionally it contains various helper functions.
 */
public class SpreadsheetDocHelper
{

// __  private members  ___________________________________________

    private final String  msDataSheetName  = "Data";

    private com.sun.star.uno.XComponentContext  mxRemoteContext;
    private com.sun.star.lang.XMultiComponentFactory  mxRemoteServiceManager;
//    private com.sun.star.lang.XMultiServiceFactory  mxMSFactory;
    private com.sun.star.sheet.XSpreadsheetDocument mxDocument;

// ________________________________________________________________

    public SpreadsheetDocHelper( String[] args )
    {
        // Connect to a running office and get the service manager
        connect();

        // Create a new spreadsheet document
        try
        {
            mxDocument = initDocument();
        }
        catch (Exception ex)
        {
            System.err.println( "Couldn't create document: " + ex );
            System.err.println( "Error: Couldn't create Document\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
            System.exit( 1 );
        }
    }

// __  helper methods  ____________________________________________

    /** Returns the service manager of the connected office.
        @return  XMultiComponentFactory interface of the service manager. */
    public com.sun.star.lang.XMultiComponentFactory getServiceManager()
    {
        return mxRemoteServiceManager;
    }

    /** Returns the component context of the connected office
        @return  XComponentContext interface of the context. */
    public com.sun.star.uno.XComponentContext getContext()
    {
        return mxRemoteContext;
    }

    /** Returns the whole spreadsheet document.
        @return  XSpreadsheetDocument interface of the document. */
    public com.sun.star.sheet.XSpreadsheetDocument getDocument()
    {
        return mxDocument;
    }

    /** Returns the spreadsheet with the specified index (0-based).
        @param nIndex  The index of the sheet.
        @return  XSpreadsheet interface of the sheet. */
    public com.sun.star.sheet.XSpreadsheet getSpreadsheet( int nIndex )
    {
        // Collection of sheets
        com.sun.star.sheet.XSpreadsheets xSheets = mxDocument.getSheets();
        com.sun.star.sheet.XSpreadsheet xSheet = null;
        try
        {
            com.sun.star.container.XIndexAccess xSheetsIA =
                (com.sun.star.container.XIndexAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XIndexAccess.class, xSheets );
            xSheet = (com.sun.star.sheet.XSpreadsheet) UnoRuntime.queryInterface(
               com.sun.star.sheet.XSpreadsheet.class, xSheetsIA.getByIndex(nIndex));
        }
        catch (Exception ex)
        {
            System.err.println( "Error: caught exception in getSpreadsheet()!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }
        return xSheet;
    }

    /** Inserts a new empty spreadsheet with the specified name.
        @param aName  The name of the new sheet.
        @param nIndex  The insertion index.
        @return  The XSpreadsheet interface of the new sheet. */
    public com.sun.star.sheet.XSpreadsheet insertSpreadsheet(
        String aName, short nIndex )
    {
        // Collection of sheets
        com.sun.star.sheet.XSpreadsheets xSheets = mxDocument.getSheets();
        com.sun.star.sheet.XSpreadsheet xSheet = null;
        try
        {
            xSheets.insertNewByName( aName, nIndex );
            xSheet = (com.sun.star.sheet.XSpreadsheet)
                UnoRuntime.queryInterface(com.sun.star.sheet.XSpreadsheet.class,
                                          xSheets.getByName( aName ));
        }
        catch (Exception ex)
        {
            System.err.println( "Error: caught exception in insertSpreadsheet()!\nException Message = "
                                + ex.getMessage());
            ex.printStackTrace();
        }
        return xSheet;
    }

// ________________________________________________________________
// Methods to fill values into cells.

    /** Writes a double value into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param fValue  The value to write into the cell. */
    public void setValue(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aCellName,
            double fValue ) throws RuntimeException, Exception
    {
        xSheet.getCellRangeByName( aCellName ).getCellByPosition( 0, 0 ).setValue( fValue );
    }

    /** Writes a formula into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param aFormula  The formula to write into the cell. */
    public void setFormula(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aCellName,
            String aFormula ) throws RuntimeException, Exception
    {
        xSheet.getCellRangeByName( aCellName ).getCellByPosition( 0, 0 ).setFormula( aFormula );
    }

    /** Writes a date with standard date format into a spreadsheet.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCellName  The address of the cell (or a named range).
        @param nDay  The day of the date.
        @param nMonth  The month of the date.
        @param nYear  The year of the date. */
    public void setDate(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aCellName,
            int nDay, int nMonth, int nYear ) throws RuntimeException, Exception
    {
        // Set the date value.
        com.sun.star.table.XCell xCell = xSheet.getCellRangeByName( aCellName ).getCellByPosition( 0, 0 );
        String aDateStr = nMonth + "/" + nDay + "/" + nYear;
        xCell.setFormula( aDateStr );

        // Set standard date format.
        com.sun.star.util.XNumberFormatsSupplier xFormatsSupplier =
            (com.sun.star.util.XNumberFormatsSupplier) UnoRuntime.queryInterface(
                com.sun.star.util.XNumberFormatsSupplier.class, getDocument() );
        com.sun.star.util.XNumberFormatTypes xFormatTypes =
            (com.sun.star.util.XNumberFormatTypes) UnoRuntime.queryInterface(
                com.sun.star.util.XNumberFormatTypes.class, xFormatsSupplier.getNumberFormats() );
        int nFormat = xFormatTypes.getStandardFormat(
            com.sun.star.util.NumberFormat.DATE, new com.sun.star.lang.Locale() );

        com.sun.star.beans.XPropertySet xPropSet = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCell );
        xPropSet.setPropertyValue( "NumberFormat", new Integer( nFormat ) );
    }

    /** Draws a colored border around the range and writes the headline in the
        first cell.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aRange  The address of the cell range (or a named range).
        @param aHeadline  The headline text. */
    public void prepareRange(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aRange, String aHeadline ) throws RuntimeException, Exception
    {
        com.sun.star.beans.XPropertySet xPropSet = null;
        com.sun.star.table.XCellRange xCellRange = null;

        // draw border
        xCellRange = xSheet.getCellRangeByName( aRange );
        xPropSet = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        com.sun.star.table.BorderLine aLine = new com.sun.star.table.BorderLine();
        aLine.Color = 0x99CCFF;
        aLine.InnerLineWidth = aLine.LineDistance = 0;
        aLine.OuterLineWidth = 100;
        com.sun.star.table.TableBorder aBorder = new com.sun.star.table.TableBorder();
        aBorder.TopLine = aBorder.BottomLine = aBorder.LeftLine = aBorder.RightLine = aLine;
        aBorder.IsTopLineValid = aBorder.IsBottomLineValid = true;
        aBorder.IsLeftLineValid = aBorder.IsRightLineValid = true;
        xPropSet.setPropertyValue( "TableBorder", aBorder );

        // draw headline
        com.sun.star.sheet.XCellRangeAddressable xAddr = (com.sun.star.sheet.XCellRangeAddressable)
            UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeAddressable.class, xCellRange );
        com.sun.star.table.CellRangeAddress aAddr = xAddr.getRangeAddress();

        xCellRange = xSheet.getCellRangeByPosition(
            aAddr.StartColumn, aAddr.StartRow, aAddr.EndColumn, aAddr.StartRow );
        xPropSet = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCellRange );
        xPropSet.setPropertyValue( "CellBackColor", new Integer( 0x99CCFF ) );
        // write headline
        com.sun.star.table.XCell xCell = xCellRange.getCellByPosition( 0, 0 );
        xCell.setFormula( aHeadline );
        xPropSet = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface( com.sun.star.beans.XPropertySet.class, xCell );
        xPropSet.setPropertyValue( "CharColor", new Integer( 0x003399 ) );
        xPropSet.setPropertyValue( "CharWeight", new Float( com.sun.star.awt.FontWeight.BOLD ) );
    }

// ________________________________________________________________
// Methods to create cell addresses and range addresses.

    /** Creates a com.sun.star.table.CellAddress and initializes it
        with the given range.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aCell  The address of the cell (or a named cell). */
    public com.sun.star.table.CellAddress createCellAddress(
            com.sun.star.sheet.XSpreadsheet xSheet,
            String aCell ) throws RuntimeException, Exception
    {
        com.sun.star.sheet.XCellAddressable xAddr = (com.sun.star.sheet.XCellAddressable)
            UnoRuntime.queryInterface( com.sun.star.sheet.XCellAddressable.class,
                xSheet.getCellRangeByName( aCell ).getCellByPosition( 0, 0 ) );
        return xAddr.getCellAddress();
    }

    /** Creates a com.sun.star.table.CellRangeAddress and initializes
        it with the given range.
        @param xSheet  The XSpreadsheet interface of the spreadsheet.
        @param aRange  The address of the cell range (or a named range). */
    public com.sun.star.table.CellRangeAddress createCellRangeAddress(
            com.sun.star.sheet.XSpreadsheet xSheet, String aRange )
    {
        com.sun.star.sheet.XCellRangeAddressable xAddr = (com.sun.star.sheet.XCellRangeAddressable)
            UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeAddressable.class,
                xSheet.getCellRangeByName( aRange ) );
        return xAddr.getRangeAddress();
    }

// ________________________________________________________________
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
            com.sun.star.table.CellRangeAddress aCellRange )
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
            com.sun.star.sheet.XSheetCellRange xCellRange,
            boolean bWithSheet )
    {
        String aStr = "";
        if (bWithSheet)
        {
            com.sun.star.sheet.XSpreadsheet xSheet = xCellRange.getSpreadsheet();
            com.sun.star.container.XNamed xNamed = (com.sun.star.container.XNamed)
                UnoRuntime.queryInterface( com.sun.star.container.XNamed.class, xSheet );
            aStr += xNamed.getName() + ".";
        }
        com.sun.star.sheet.XCellRangeAddressable xAddr = (com.sun.star.sheet.XCellRangeAddressable)
            UnoRuntime.queryInterface( com.sun.star.sheet.XCellRangeAddressable.class, xCellRange );
        aStr += getCellRangeAddressString( xAddr.getRangeAddress() );
        return aStr;
    }

    /** Returns a list of addresses of all cell ranges contained in the collection.
        @param xRangesIA  The XIndexAccess interface of the collection.
        @return  A string containing the cell range address list. */
    public String getCellRangeListString(
            com.sun.star.container.XIndexAccess xRangesIA ) throws RuntimeException, Exception
    {
        String aStr = "";
        int nCount = xRangesIA.getCount();
        for (int nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (nIndex > 0)
                aStr += " ";
            Object aRangeObj = xRangesIA.getByIndex( nIndex );
            com.sun.star.sheet.XSheetCellRange xCellRange = (com.sun.star.sheet.XSheetCellRange)
                UnoRuntime.queryInterface( com.sun.star.sheet.XSheetCellRange.class, aRangeObj );
            aStr += getCellRangeAddressString( xCellRange, false );
        }
        return aStr;
    }

// ________________________________________________________________

    // Connect to a running office that is accepting connections.
    private void connect()
    {
        if (mxRemoteContext == null && mxRemoteServiceManager == null) {
            try {
                // First step: get the remote office component context
                mxRemoteContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
                System.out.println("Connected to a running office ...");

                mxRemoteServiceManager = mxRemoteContext.getServiceManager();
            }
            catch( Exception e) {
                System.err.println("ERROR: can't get a component context from a running office ...");
                e.printStackTrace();
                System.exit(1);
            }
        }
    }

    /** Creates an empty spreadsheet document.
        @return  The XSpreadsheetDocument interface of the document. */
    private com.sun.star.sheet.XSpreadsheetDocument initDocument()
            throws RuntimeException, Exception
    {
        XComponentLoader aLoader = (XComponentLoader)
            UnoRuntime.queryInterface(
                XComponentLoader.class,
                mxRemoteServiceManager.createInstanceWithContext(
                    "com.sun.star.frame.Desktop", mxRemoteContext));

        XComponent xComponent = aLoader.loadComponentFromURL(
            "private:factory/scalc", "_blank", 0,
            new com.sun.star.beans.PropertyValue[0] );

        return (com.sun.star.sheet.XSpreadsheetDocument)UnoRuntime.queryInterface(
            com.sun.star.sheet.XSpreadsheetDocument.class, xComponent );
    }

// ________________________________________________________________
}
