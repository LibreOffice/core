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

// __________ Imports __________

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


// __________ Implementation __________

/** Create a spreadsheet document and provide access to a sheet framework that
    is then used to modify some number formats.
 */
public class Number_Formats
{
    // __________ public members and methods __________


    // ____________________

    public static void main( String args[] )
    {
        try
        {
            Number_Formats aSample = new Number_Formats( args );
            aSample.doFunction();
        }
        catch( Exception ex )
        {
            System.err.println( "Sample caught exception! " + ex );
            ex.printStackTrace();
            System.exit(1);
        }

        System.out.println( "Sample done." );
        System.exit(0);
    }

    // ____________________

    public void doFunction() throws RuntimeException, Exception
    {
        // Assume:
        // com.sun.star.sheet.XSpreadsheetDocument maSpreadsheetDoc;
        // com.sun.star.sheet.XSpreadsheet maSheet;

        // Query the number formats supplier of the spreadsheet document
        com.sun.star.util.XNumberFormatsSupplier xNumberFormatsSupplier =
            (com.sun.star.util.XNumberFormatsSupplier)
            UnoRuntime.queryInterface(
            com.sun.star.util.XNumberFormatsSupplier.class, maSpreadsheetDoc );

        // Get the number formats from the supplier
        com.sun.star.util.XNumberFormats xNumberFormats =
            xNumberFormatsSupplier.getNumberFormats();

        // Query the XNumberFormatTypes interface
        com.sun.star.util.XNumberFormatTypes xNumberFormatTypes =
            (com.sun.star.util.XNumberFormatTypes)
            UnoRuntime.queryInterface(
            com.sun.star.util.XNumberFormatTypes.class, xNumberFormats );

        // Get the number format index key of the default currency format,
        // note the empty locale for default locale
        com.sun.star.lang.Locale aLocale = new com.sun.star.lang.Locale();
        int nCurrencyKey = xNumberFormatTypes.getStandardFormat(
            com.sun.star.util.NumberFormat.CURRENCY, aLocale );

        // Get cell range B3:B11
        com.sun.star.table.XCellRange xCellRange =
            maSheet.getCellRangeByPosition( 1, 2, 1, 10 );

        // Query the property set of the cell range
        com.sun.star.beans.XPropertySet xCellProp =
            (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, xCellRange );

        // Set number format to default currency
        xCellProp.setPropertyValue( "NumberFormat", new Integer(nCurrencyKey) );

        // Get cell B3
        com.sun.star.table.XCell xCell = maSheet.getCellByPosition( 1, 2 );

        // Query the property set of the cell
        xCellProp = (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface(
            com.sun.star.beans.XPropertySet.class, xCell );

        // Get the number format index key of the cell's properties
        int nIndexKey = ((Integer) xCellProp.getPropertyValue( "NumberFormat" )).intValue();
        if ( nIndexKey != nCurrencyKey )
            System.out.println( "Number format doesn't match!" );

        // Get the properties of the number format
        com.sun.star.beans.XPropertySet xProp = xNumberFormats.getByKey( nIndexKey );

        // Get the format code string of the number format's properties
        String aFormatCode = (String) xProp.getPropertyValue( "FormatString" );
        System.out.println( "FormatString: `" + aFormatCode + "'" );

        // Create an arbitrary format code
        aFormatCode = "\"wonderful \"" + aFormatCode;

        // Test if it's already present
        nIndexKey = xNumberFormats.queryKey( aFormatCode, aLocale, false );

        // If not, add to number formats collection
        if ( nIndexKey == -1 )
        {
            try
            {
                nIndexKey = xNumberFormats.addNew( aFormatCode, aLocale );
            }
            catch( com.sun.star.util.MalformedNumberFormatException ex )
            {
                System.err.println( "Bad number format code: " + ex );
                ex.printStackTrace();
                nIndexKey = -1;
            }
        }

        // Set the new format at the cell
        if ( nIndexKey != -1 )
            xCellProp.setPropertyValue( "NumberFormat", new Integer(nIndexKey) );


        // Set column containing the example values to optimal width to show
        // the new format of cell B3
        com.sun.star.table.XColumnRowRange xColRowRange =
            (com.sun.star.table.XColumnRowRange)
            UnoRuntime.queryInterface(com.sun.star.table.XColumnRowRange.class,
                                      maSheet);

        com.sun.star.container.XIndexAccess xIndexAccess =
            (com.sun.star.container.XIndexAccess)
            UnoRuntime.queryInterface(com.sun.star.container.XIndexAccess.class,
                                      xColRowRange.getColumns());

        com.sun.star.beans.XPropertySet xColPropSet =
            (com.sun.star.beans.XPropertySet)
            UnoRuntime.queryInterface(com.sun.star.beans.XPropertySet.class,
                                      xIndexAccess.getByIndex(1));

        xColPropSet.setPropertyValue( "OptimalWidth", new Boolean(true) );
    }

    // ____________________

    public Number_Formats( String[] args ) throws java.lang.Exception
    {
        // get the remote office context. If necessary a new office
        // process is started
        maOfficeContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
        System.out.println("Connected to a running office ...");
        maServiceManager = maOfficeContext.getServiceManager();

        // create a new spreadsheet document
        XComponentLoader aLoader = (XComponentLoader) UnoRuntime.queryInterface(
            XComponentLoader.class, maServiceManager.createInstanceWithContext(
                "com.sun.star.frame.Desktop", maOfficeContext) );

        maSpreadsheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
            XSpreadsheetDocument.class,
            aLoader.loadComponentFromURL( "private:factory/scalc",
                                          "_blank",
                                          0,
                                          new PropertyValue[ 0 ] ) );

        if ( !initSpreadsheet() )
            System.exit( 0 );
    }


    // __________ private members and methods __________
    private final String  msDataSheetName  = "Data";

    private XComponentContext      maOfficeContext;
    private XMultiComponentFactory maServiceManager;
    private XSpreadsheetDocument   maSpreadsheetDoc;
    private XSpreadsheet           maSheet;  // the first sheet


    // ____________________

    /** init the first sheet
     */
    private boolean initSpreadsheet()
    {
        boolean bOk = true;
        XSpreadsheets aSheets = maSpreadsheetDoc.getSheets();
        try
        {
            XIndexAccess aSheetsIA = (XIndexAccess) UnoRuntime.queryInterface( XIndexAccess.class, aSheets );
            maSheet = (XSpreadsheet) UnoRuntime.queryInterface(XSpreadsheet.class, aSheetsIA.getByIndex( 0 ));

            // enter some values in B3:B11
            for( int iCounter=1; iCounter < 10; iCounter++ )
            {
                XCell aCell = maSheet.getCellByPosition( 1, 1 + iCounter );
                aCell.setValue( (double) iCounter );
            }
        }
        catch( Exception ex )
        {
            System.err.println( "Couldn't initialize Spreadsheet Document: " + ex );
            ex.printStackTrace();
            bOk = false;
        }
        return bOk;
    }
}
