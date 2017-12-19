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


// comment: Step 1: get the Desktop object from the office
//          Step 2: open an empty Calc document
//          Step 3: enter a example text, set the numberformat to DM
//          Step 4: change the numberformat to EUR (Euro)
//          Step 5: use the DM/EUR factor on each cell with a content


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
import com.sun.star.sheet.XSpreadsheetDocument;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XComponentContext;

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
        xSheetdocument = createSheetdocument( xDesktop );
        System.out.println( "Create a new Spreadsheet" );

        // get the collection of all sheets from the document
        XSpreadsheets xSheets = null;
        xSheets = xSheetdocument.getSheets();

        // the Action Interface provides methods to hide actions,
        // like inserting data, on a sheet, that increase the performance
        XActionLockable xActionInterface = null;
        xActionInterface = UnoRuntime.queryInterface(
            XActionLockable.class, xSheetdocument );

        // lock all actions
        xActionInterface.addActionLock();

        com.sun.star.sheet.XSpreadsheet xSheet = null;
        try {
            // get via the index access the first sheet
            XIndexAccess xElements = UnoRuntime.queryInterface(
                XIndexAccess.class, xSheets );

            // specify the first sheet from the spreadsheet
            xSheet = UnoRuntime.queryInterface(
                XSpreadsheet.class, xElements.getByIndex( 0 ));
        }
        catch( Exception e) {
            e.printStackTrace(System.err);
        }

        // get the interface to apply and create new numberformats
        XNumberFormatsSupplier xNumberFormatSupplier = null;
        xNumberFormatSupplier = UnoRuntime.queryInterface(
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
                UnoRuntime.queryInterface(
                XCellFormatRangesSupplier.class, xSheet );

            // getCellFormatRanges() has the interfaces for the enumeration
            XEnumerationAccess xEnumerationAccess =
                UnoRuntime.queryInterface(
                XEnumerationAccess.class,
                xCellFormatSupplier.getCellFormatRanges() );

            XEnumeration xRanges = xEnumerationAccess.createEnumeration();

            while( xRanges.hasMoreElements() ) {
                // the enumeration returns a cellrange
                XCellRange xCellRange = UnoRuntime.queryInterface(
                    XCellRange.class, xRanges.nextElement());

                // the PropertySet the get and set the properties from the cellrange
                XPropertySet xCellProp = UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );

                // getPropertyValue returns an Object, you have to cast it to
                // type that you need
                Object oNumberObject = xCellProp.getPropertyValue( "NumberFormat" );
                int iNumberFormat = AnyConverter.toInt(oNumberObject);

                // get the properties from the cellrange numberformat
                XPropertySet xFormat = xNumberFormats.getByKey(iNumberFormat );

                short fType = AnyConverter.toShort(xFormat.getPropertyValue("Type"));
                String sCurrencySymbol = AnyConverter.toString(
                    xFormat.getPropertyValue("CurrencySymbol"));

                // change the numberformat only on cellranges with a
                // currency numberformat
                if( ( (fType & com.sun.star.util.NumberFormat.CURRENCY) > 0) &&
                    ( sCurrencySymbol.equals( sOldSymbol ) ) ) {
                    boolean bThousandSep = AnyConverter.toBoolean(
                        xFormat.getPropertyValue("ThousandsSeparator"));
                    boolean bNegativeRed = AnyConverter.toBoolean(
                        xFormat.getPropertyValue("NegativeRed"));
                    short fDecimals = AnyConverter.toShort(
                        xFormat.getPropertyValue("Decimals"));
                    short fLeadingZeros = AnyConverter.toShort(
                        xFormat.getPropertyValue("LeadingZeros"));
                    Locale oLocale = (Locale) AnyConverter.toObject(
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
                                                Integer.valueOf( iNewNumberFormat ) );

                    // iterate over all cells from the cellrange with an
                    // content and use the DM/EUR factor
                    XCellRangesQuery xCellRangesQuery = UnoRuntime.queryInterface(
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
                            XCell xCell = UnoRuntime.queryInterface(
                                XCell.class, xCellEnumeration.nextElement());
                            xCell.setValue( xCell.getValue() / fFactor );
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

        return nRetKey;
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
                XPropertySet xCellProp = UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );
                xCellProp.setPropertyValue( "NumberFormat",
                                            Integer.valueOf(iNumberFormatKey) );
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
                xDesktop = UnoRuntime.queryInterface(
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

            aSheetDocument = UnoRuntime.queryInterface(
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
        PropertyValue xEmptyArgs[] = new PropertyValue[0];

        try {
            xComponentLoader = UnoRuntime.queryInterface(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
