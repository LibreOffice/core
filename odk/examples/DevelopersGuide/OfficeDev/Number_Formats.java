// __________ Imports __________

// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// factory for creating components
import com.sun.star.comp.servicemanager.ServiceManager;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XNamingService;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;

// property access
import com.sun.star.beans.*;

// container access
import com.sun.star.container.*;

// utilities
import com.sun.star.util.*;

// internationalization
import com.sun.star.i18n.*;

// application specific classes
import com.sun.star.sheet.*;
import com.sun.star.table.*;
import com.sun.star.chart.*;

// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;


// __________ Implementation __________

/** Create a spreadsheet document and provide access to a sheet framework that
    is then used to modify some number formats.
    @author Eike Rathke
 */
public class Number_Formats
{
    // __________ public members and methods __________


    // ____________________

    public static void main( String args[] )
    {
        Number_Formats aSample = new Number_Formats( args );

        try
        {
            aSample.doFunction();
        }
        catch( Exception ex )
        {
            System.out.println( "Sample caught exception! " + ex );
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
                System.out.println( "Bad number format code: " + ex );
                nIndexKey = -1;
            }
        }

        // Set the new format at the cell
        if ( nIndexKey != -1 )
            xCellProp.setPropertyValue( "NumberFormat", new Integer(nIndexKey) );
    }

    // ____________________

    public Number_Formats( String[] args )
    {
        boolean bOk = true;

        // connect to a running office and get the ServiceManager
        try
        {
            String sConn = "uno:" + msConnectString + ";urp;StarOffice.ServiceManager";
            System.out.println( "connecting: " + sConn );
            maMSFactory = connect( sConn );
        }
        catch( Exception ex )
        {
            System.out.println( "Couldn't get ServiceManager: " + ex );
            System.exit( 0 );
        }

        // create a new spreadsheet document
        try
        {
            XComponentLoader aLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class,
                maMSFactory.createInstance( "com.sun.star.frame.Desktop" ) );

            maSpreadsheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                XSpreadsheetDocument.class,
                aLoader.loadComponentFromURL( "private:factory/scalc",
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );

            bOk = initSpreadsheet();
        }
        catch( Exception ex )
        {
            System.out.println( "Couldn't create SpreadsheetDocument: " + ex );
            bOk = false;
        }

        if ( !bOk )
            System.exit( 0 );
    }


    // __________ private members and methods __________

    private final String  msConnectString  = "socket,host=localhost,port=8100";

    private final String  msDataSheetName  = "Data";

    private XMultiServiceFactory    maMSFactory;
    private XSpreadsheetDocument    maSpreadsheetDoc;
    private XSpreadsheet            maSheet;  // the first sheet


    // ____________________

    /** Connect to a running office that is accepting connections
        and return the ServiceManager to instantiate office components
     */
    private XMultiServiceFactory connect( String sConnectString )
        throws RuntimeException, Exception
    {
        XMultiServiceFactory aLocalServiceManager =
            com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

        XUnoUrlResolver aURLResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
            XUnoUrlResolver.class,
            aLocalServiceManager.createInstance( "com.sun.star.bridge.UnoUrlResolver" ) );

        XMultiServiceFactory aServiceManager = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            aURLResolver.resolve( sConnectString ) );

        return aServiceManager;
    }

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
            System.out.println( "Couldn't initialize Spreadsheet Document: " + ex );
            bOk = false;
        }
        return bOk;
    }
}
