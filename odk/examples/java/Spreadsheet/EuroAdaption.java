//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              EuroAdaption
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty Calc document
//                    Step 3: enter a example text, set the numberformat to DM
//                    Step 4: change the numberformat to EUR (Euro)
//                    Step 5: use the DM/EUR factor on each cell with a content
//
//***************************************************************************
// date                 : Tue August 22  2000
//
//***************************************************************************


import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

// access the implementations via names
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

// access the implementations via names
import com.sun.star.comp.servicemanager.ServiceManager;

import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.sheet.XCellFormatRangesSupplier;
import com.sun.star.sheet.XCellRangesQuery;

import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.lang.Locale;
import com.sun.star.util.NumberFormat;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;

public class EuroAdaption {

    public static void main(String args[]) {
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

        XMultiServiceFactory xMSF = null;
        try {
            xMSF = connect( sConnectionString );
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
            System.exit( 0 );
        }

        if( xMSF != null )  System.out.println("Connected to "+ sConnectionString );

        // You need the desktop to create a document
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop( xMSF );

        // create a sheet document
        com.sun.star.sheet.XSpreadsheetDocument xSheetdocument = null;
        xSheetdocument = ( com.sun.star.sheet.XSpreadsheetDocument ) createSheetdocument( xDesktop );
        System.out.println( "Create a new Spreadsheet" );


        // get the collection of all sheets from the document
        com.sun.star.sheet.XSpreadsheets xSheets = null;
        xSheets = (com.sun.star.sheet.XSpreadsheets) xSheetdocument.getSheets();

        // the Action Interface provides methods to hide actions, like inserting data, on a sheet, that increase the performance
        com.sun.star.document.XActionLockable xActionInterface = null;
        xActionInterface = (com.sun.star.document.XActionLockable) UnoRuntime.queryInterface(
            com.sun.star.document.XActionLockable.class, xSheetdocument );

        // lock all actions
        xActionInterface.addActionLock();

        com.sun.star.sheet.XSpreadsheet xSheet = null;
        try {
            // get via the index access the first sheet
            com.sun.star.container.XIndexAccess xElements = (com.sun.star.container.XIndexAccess)
                UnoRuntime.queryInterface( com.sun.star.container.XIndexAccess.class, xSheets );

            // specify the first sheet from the spreadsheet
            xSheet = (com.sun.star.sheet.XSpreadsheet) UnoRuntime.queryInterface(
                com.sun.star.sheet.XSpreadsheet.class, xElements.getByIndex( 0 ));
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
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


    public static void Convert(  XSpreadsheet xSheet, XNumberFormats xNumberFormats, String sOldSymbol, String sNewSymbol, float fFactor ) {
        try {
            Locale xLanguage = new Locale();
            xLanguage.Country = "de";    // Germany -> DM
            xLanguage.Language = "de";   // German

            // Numberformat string with sNewSymbol
            String sSimple = "0 [$" + sNewSymbol + "]";
            // create a number format key with the sNewSymbol
            int iSimpleKey = NumberFormat( xNumberFormats, sSimple, xLanguage );

            // you have to use the FormatSupplier interface to get the CellFormat enumeration
            XCellFormatRangesSupplier xCellFormatSupplier = (XCellFormatRangesSupplier)
                UnoRuntime.queryInterface(XCellFormatRangesSupplier.class, xSheet );

            // getCellFormatRanges() has the interfaces for the enumeration
            XEnumerationAccess xEnumerationAccess = (XEnumerationAccess)
                UnoRuntime.queryInterface( XEnumerationAccess.class,
                                           xCellFormatSupplier.getCellFormatRanges() );
            XEnumeration xRanges = xEnumerationAccess.createEnumeration();

            while( xRanges.hasMoreElements() ) {
                // the enumeration returns a cellrange
                XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(
                    XCellRange.class, xRanges.nextElement());

                // the PropertySet the get and set the properties from the cellrange
                XPropertySet xCellProp = ( XPropertySet ) UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );

                // getPropertyValue returns an Object, you have to cast it to type that you need
                Object oNumberObject = xCellProp.getPropertyValue( "NumberFormat" );
                int iNumberFormat = ((Integer) oNumberObject).intValue();

                // get the properties from the cellrange numberformat
                XPropertySet xFormat = (XPropertySet) xNumberFormats.getByKey( iNumberFormat );

                short fType = ((Short) xFormat.getPropertyValue("Type")).shortValue();
                String sCurrencySymbol = ((String) xFormat.getPropertyValue("CurrencySymbol")).toString();

                // change the numberformat only on cellranges with a currency numberformat
                if( ( (fType & com.sun.star.util.NumberFormat.CURRENCY) > 0) &&
                    ( sCurrencySymbol.compareTo( sOldSymbol ) == 0 ) ) {
                    AnyConverter aAnyConv = new AnyConverter();
                    boolean bThousandSep = aAnyConv.toBoolean(
                        xFormat.getPropertyValue("ThousandsSeparator"));
                    boolean bNegativeRed = aAnyConv.toBoolean(xFormat.getPropertyValue("NegativeRed"));
                    short fDecimals = aAnyConv.toShort(xFormat.getPropertyValue("Decimals"));
                    short fLeadingZeros = aAnyConv.toShort(xFormat.getPropertyValue("LeadingZeros"));
                    Locale oLocale = (Locale) aAnyConv.toObject(
                       new com.sun.star.uno.Type(Locale.class),xFormat.getPropertyValue("Locale"));

                    // create a new numberformat string
                    String sNew = xNumberFormats.generateFormat( iSimpleKey, oLocale, bThousandSep, bNegativeRed, fDecimals, fLeadingZeros );
                    // get the NumberKey from the numberformat
                    int iNewNumberFormat = NumberFormat( xNumberFormats, sNew, oLocale );

                    // set the new numberformat to the cellrange DM->EUR
                    xCellProp.setPropertyValue( "NumberFormat", new Integer( iNewNumberFormat ) );

                    // interate over all cells from the cellrange with an content and use the DM/EUR factor
                    XCellRangesQuery xCellRangesQuery = (XCellRangesQuery) UnoRuntime.queryInterface(
                        com.sun.star.sheet.XCellRangesQuery.class, xCellRange );
                    XSheetCellRanges xSheetCellRanges = xCellRangesQuery.queryContentCells(
                        (short) com.sun.star.sheet.CellFlags.VALUE );

                    if( xSheetCellRanges.getCount() > 0 ) {
                        XEnumerationAccess xCellEnumerationAccess = xSheetCellRanges.getCells();
                        XEnumeration xCellEnumeration = xCellEnumerationAccess.createEnumeration();

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
            e.printStackTrace(System.out);
        }
    }


    public static int NumberFormat( XNumberFormats xNumberFormat, String sFormat, com.sun.star.lang.Locale xLanguage ) {
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
            e.printStackTrace(System.out);
        }

        return( nRetKey );
    }


    public static void createExampleData( XSpreadsheet xSheet, XNumberFormats xNumberFormat ) {

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
            int iNumberFormatKey = NumberFormat( xNumberFormat, sSimple, xLanguage );

            for( int iCounter=1; iCounter < 10; iCounter++ ) {
                // get one cell and insert a number
                xCell = xSheet.getCellByPosition( 2, 1 + iCounter );
                xCell.setValue( (double) iCounter * 2 );
                xCellRange = xSheet.getCellRangeByPosition( 2, 1 + iCounter, 2, 1 + iCounter );

                // get the ProperySet from the cell, to change the numberformat
                XPropertySet xCellProp = ( XPropertySet ) UnoRuntime.queryInterface(
                    XPropertySet.class, xCellRange );
                xCellProp.setPropertyValue( "NumberFormat", new Integer(iNumberFormatKey) );
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }
    }


    public static XMultiServiceFactory connect( String connectStr )
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException, Exception {
        // Get component context
        XComponentContext xcomponentcontext =
        com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
        null );

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager =
        xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xLocalServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );
        XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface( XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( connectStr );

        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
        }

        return ( xMSF );
    }


    public static com.sun.star.frame.XDesktop getDesktop( XMultiServiceFactory xMSF ) {
        XInterface xInterface = null;
        com.sun.star.frame.XDesktop xDesktop = null;

        if( xMSF != null ) {
            try {
                xInterface = (XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");
                xDesktop = (com.sun.star.frame.XDesktop) UnoRuntime.queryInterface(com.sun.star.frame.XDesktop.class, xInterface);
            }
            catch( Exception e) {
                e.printStackTrace(System.out);
            }
        }
        else
            System.out.println( "Can't create a desktop. null pointer !" );

        return xDesktop;
    }


    public static com.sun.star.sheet.XSpreadsheetDocument createSheetdocument( com.sun.star.frame.XDesktop xDesktop ) {
        com.sun.star.sheet.XSpreadsheetDocument aSheetDocument = null;

        try {
            com.sun.star.lang.XComponent xComponent = null;
            xComponent = CreateNewDocument( xDesktop, "scalc" );

            aSheetDocument = (com.sun.star.sheet.XSpreadsheetDocument) UnoRuntime.queryInterface(com.sun.star.sheet.XSpreadsheetDocument.class, xComponent);
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        return aSheetDocument;
    }

    protected static com.sun.star.lang.XComponent CreateNewDocument( com.sun.star.frame.XDesktop xDesktop, String sDocumentType ) {
        String sURL = "private:factory/" + sDocumentType;

        com.sun.star.lang.XComponent xComponent = null;
        PropertyValue xValues[] = new PropertyValue[1];

        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        XInterface xInterface = null;

        PropertyValue[] xEmptyArgs = new PropertyValue[0];

        xComponentLoader = (com.sun.star.frame.XComponentLoader) UnoRuntime.queryInterface(com.sun.star.frame.XComponentLoader.class, xDesktop );

        try {
            xComponent  = xComponentLoader.loadComponentFromURL( sURL, "_blank", 0, xEmptyArgs);
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        return xComponent ;
    }

}
