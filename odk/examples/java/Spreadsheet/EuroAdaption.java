/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



//***************************************************************************
// comment: Step 1: get the Desktop object from the office
//          Step 2: open an empty Calc document
//          Step 3: enter a example text, set the numberformat to DM
//          Step 4: change the numberformat to EUR (Euro)
//          Step 5: use the DM/EUR factor on each cell with a content
//***************************************************************************

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XEnumerationAccess;

import com.sun.star.document.XActionLockable;

import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;

import com.sun.star.lang.Locale;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XCellFormatRangesSupplier;
import com.sun.star.sheet.XCellRangesQuery;
import com.sun.star.sheet.XSpreadsheetDocument;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XComponentContext;

import com.sun.star.util.NumberFormat;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;


public class EuroAdaption {

    public static void main(String args[]) {
        // You need the desktop to create a document
        // The getDesktop method does the UNO bootstrapping, gets the
        // remote servie manager and the desktop object.
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop();

        // create a sheet document
        XSpreadsheetDocument xSheetdocument = null;
        xSheetdocument = ( XSpreadsheetDocument ) createSheetdocument( xDesktop );
        System.out.println( "Create a new Spreadsheet" );

        // get the collection of all sheets from the document
        XSpreadsheets xSheets = null;
        xSheets = (XSpreadsheets) xSheetdocument.getSheets();

        // the Action Interface provides methods to hide actions,
        // like inserting data, on a sheet, that increase the performance
        XActionLockable xActionInterface = null;
        xActionInterface = (XActionLockable) UnoRuntime.queryInterface(
            XActionLockable.class, xSheetdocument );

        // lock all actions
        xActionInterface.addActionLock();

        com.sun.star.sheet.XSpreadsheet xSheet = null;
        try {
            // get via the index access the first sheet
            XIndexAccess xElements = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xSheets );

            // specify the first sheet from the spreadsheet
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xElements.getByIndex( 0 ));
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        // get the interface to apply and create new numberformats
        XNumberFormatsSupplier xNumberFormatSupplier = null;
        xNumberFormatSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(
            XNumberFormatsSupplier.class, xSheetdocument );
        XNumberFormats xNumberFormats = null;
        xNumberFormats = xNumberFormatSupplier.getNumberFormats();

        // insert some example data in a sheet
        createExampleData( xSheet, xNumberFormats );
        System.out.println( "Insert example data and use the number format with the currency 'DM'" );

        // Change the currency from the cells from DM to Euro
        Convert(  xSheet, xNumberFormats, "DM", "EUR", 1.95583f );
        System.out.println( "Change the number format to EUR and divide the values with the factor 1.95583" );

        // remove all locks, the user see all changes
        xActionInterface.removeActionLock();

        System.out.println("done");
        System.exit(0);
    }


    public static void Convert( XSpreadsheet xSheet, XNumberFormats xNumberFormats,
                                String sOldSymbol, String sNewSymbol,
                                float fFactor ) {
        try {
            Locale xLanguage = new Locale();
            xLanguage.Country = "de";    // Germany -> DM
            xLanguage.Language = "de";   // German

            // Numberformat string with sNewSymbol
            String sSimple = "0 [$" + sNewSymbol + "]";
            // create a number format key with the sNewSymbol
            int iSimpleKey = NumberFormat( xNumberFormats, sSimple, xLanguage );

            // you have to use the FormatSupplier interface to get the
            // CellFormat enumeration
            XCellFormatRangesSupplier xCellFormatSupplier =
                (XCellFormatRangesSupplier)UnoRuntime.queryInterface(
                    XCellFormatRangesSupplier.class, xSheet );

            // getCellFormatRanges() has the interfaces for the enumeration
            XEnumerationAccess xEnumerationAccess =
                (XEnumerationAccess)UnoRuntime.queryInterface(
                    XEnumerationAccess.class,
                    xCellFormatSupplier.getCellFormatRanges() );

            XEnumeration xRanges = xEnumerationAccess.createEnumeration();

            // create an AnyConverter for later use
            AnyConverter aAnyConv = new AnyConverter();

            while( xRanges.hasMoreElements() ) {
                // the enumeration returns a cellrange
                XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(
                    XCellRange.class, xRanges.nextElement());

                // the PropertySet the get and set the properties from the cellrange
                XPropertySet xCellProp = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );

                // getPropertyValue returns an Object, you have to cast it to
                // type that you need
                Object oNumberObject = xCellProp.getPropertyValue( "NumberFormat" );
                int iNumberFormat = aAnyConv.toInt(oNumberObject);

                // get the properties from the cellrange numberformat
                XPropertySet xFormat = (XPropertySet)
                    xNumberFormats.getByKey(iNumberFormat );

                short fType = aAnyConv.toShort(xFormat.getPropertyValue("Type"));
                String sCurrencySymbol = aAnyConv.toString(
                    xFormat.getPropertyValue("CurrencySymbol"));

                // change the numberformat only on cellranges with a
                // currency numberformat
                if( ( (fType & com.sun.star.util.NumberFormat.CURRENCY) > 0) &&
                    ( sCurrencySymbol.compareTo( sOldSymbol ) == 0 ) ) {
                    boolean bThousandSep = aAnyConv.toBoolean(
                        xFormat.getPropertyValue("ThousandsSeparator"));
                    boolean bNegativeRed = aAnyConv.toBoolean(
                        xFormat.getPropertyValue("NegativeRed"));
                    short fDecimals = aAnyConv.toShort(
                        xFormat.getPropertyValue("Decimals"));
                    short fLeadingZeros = aAnyConv.toShort(
                        xFormat.getPropertyValue("LeadingZeros"));
                    Locale oLocale = (Locale) aAnyConv.toObject(
                       new com.sun.star.uno.Type(Locale.class),
                       xFormat.getPropertyValue("Locale"));

                    // create a new numberformat string
                    String sNew = xNumberFormats.generateFormat( iSimpleKey,
                                         oLocale, bThousandSep, bNegativeRed,
                                         fDecimals, fLeadingZeros );

                    // get the NumberKey from the numberformat
                    int iNewNumberFormat = NumberFormat( xNumberFormats,
                                                         sNew, oLocale );

                    // set the new numberformat to the cellrange DM->EUR
                    xCellProp.setPropertyValue( "NumberFormat",
                                                new Integer( iNewNumberFormat ) );

                    // interate over all cells from the cellrange with an
                    // content and use the DM/EUR factor
                    XCellRangesQuery xCellRangesQuery = (XCellRangesQuery)
                        UnoRuntime.queryInterface(
                        XCellRangesQuery.class, xCellRange );

                    XSheetCellRanges xSheetCellRanges =
                        xCellRangesQuery.queryContentCells(
                            (short) com.sun.star.sheet.CellFlags.VALUE );

                    if( xSheetCellRanges.getCount() > 0 ) {
                        XEnumerationAccess xCellEnumerationAccess =
                            xSheetCellRanges.getCells();
                        XEnumeration xCellEnumeration =
                            xCellEnumerationAccess.createEnumeration();

                        while( xCellEnumeration.hasMoreElements() ) {
                            XCell xCell = (XCell) UnoRuntime.queryInterface(
                                XCell.class, xCellEnumeration.nextElement());
                            xCell.setValue( (double) xCell.getValue() / fFactor );
                        }
                    }
                }
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }
    }


    public static int NumberFormat( XNumberFormats xNumberFormat, String sFormat,
                                    com.sun.star.lang.Locale xLanguage ) {
        int nRetKey = 0;

        try {
            // exists the numberformat
            nRetKey = xNumberFormat.queryKey( sFormat, xLanguage, true );

            // if not, create a new one
            if( nRetKey == -1 ) {
                nRetKey = xNumberFormat.addNew( sFormat, xLanguage );
                if( nRetKey == -1 )
                    nRetKey = 0;
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        return( nRetKey );
    }


    public static void createExampleData( XSpreadsheet xSheet,
                                          XNumberFormats xNumberFormat ) {

        // enter in a cellrange numbers and change the numberformat to DM
        XCell xCell = null;
        XCellRange xCellRange = null;

        try {
            Locale xLanguage = new Locale();
            xLanguage.Country = "de";    // Germany -> DM
            xLanguage.Language = "de";   // German

            // Numberformat string from DM
            String sSimple = "0 [$DM]";

            // get the numberformat key
            int iNumberFormatKey = NumberFormat(xNumberFormat, sSimple, xLanguage);

            for( int iCounter=1; iCounter < 10; iCounter++ ) {
                // get one cell and insert a number
                xCell = xSheet.getCellByPosition( 2, 1 + iCounter );
                xCell.setValue( (double) iCounter * 2 );
                xCellRange = xSheet.getCellRangeByPosition( 2, 1 + iCounter,
                                                            2, 1 + iCounter );

                // get the ProperySet from the cell, to change the numberformat
                XPropertySet xCellProp = (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );
                xCellProp.setPropertyValue( "NumberFormat",
                                            new Integer(iNumberFormatKey) );
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }
    }

    public static XDesktop getDesktop() {
        XDesktop xDesktop = null;
        XMultiComponentFactory xMCF = null;

        try {
            XComponentContext xContext = null;

            // get the remote office component context
            xContext = com.sun.star.comp.helper.Bootstrap.bootstrap();

            // get the remote office service manager
            xMCF = xContext.getServiceManager();
            if( xMCF != null ) {
                System.out.println("Connected to a running office ...");

                Object oDesktop = xMCF.createInstanceWithContext(
                    "com.sun.star.frame.Desktop", xContext);
                xDesktop = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, oDesktop);
            }
            else
                System.out.println( "Can't create a desktop. No connection, no remote servicemanager available!" );
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }


        return xDesktop;
    }


    public static XSpreadsheetDocument createSheetdocument( XDesktop xDesktop ) {
        XSpreadsheetDocument aSheetDocument = null;

        try {
            XComponent xComponent = null;
            xComponent = CreateNewDocument( xDesktop, "scalc" );

            aSheetDocument = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, xComponent);
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        return aSheetDocument;
    }

    protected static XComponent CreateNewDocument( XDesktop xDesktop,
                                                   String sDocumentType ) {
        String sURL = "private:factory/" + sDocumentType;

        XComponent xComponent = null;
        XComponentLoader xComponentLoader = null;
        PropertyValue xValues[] = new PropertyValue[1];
        PropertyValue xEmptyArgs[] = new PropertyValue[0];

        try {
            xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class, xDesktop );

            xComponent  = xComponentLoader.loadComponentFromURL(
                sURL, "_blank", 0, xEmptyArgs);
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        return xComponent ;
    }

}
