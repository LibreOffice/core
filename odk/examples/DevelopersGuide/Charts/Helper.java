// __________ Imports __________

import java.util.Random;

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

// application specific classes
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;

import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;

// __________ Implementation __________

/** Helper for creating a calc document adding cell values and charts
    @author Bj&ouml;rn Milcke
 */
public class Helper
{
    public Helper( String[] args )
    {
        // connect to a running office and get the ServiceManager
        try
        {
            String sConnectString;
            if( args.length > 0 )
                sConnectString = args[ 0 ];
            else
                sConnectString = new String( "socket,host=localhost,port=8100" );

            maMSFactory = connect( "uno:" + sConnectString + ";urp;StarOffice.ServiceManager" );
        }
        catch( Exception ex )
        {
            System.out.println( "Couldn't get ServiceManager: " + ex );
            System.exit( 0 );
        }
    }

    // ____________________

    public XSpreadsheetDocument createSpreadsheetDocument()
    {
        return (XSpreadsheetDocument) UnoRuntime.queryInterface(
            XSpreadsheetDocument.class, createDocument( "scalc" ));
    }

    // ____________________

    public XModel createPresentationDocument()
    {
        return createDocument( "simpress" );
    }

    // ____________________

    public XModel createDrawingDocument()
    {
        return createDocument( "sdraw" );
    }

    // ____________________

    public XModel createTextDocument()
    {
        return createDocument( "swriter" );
    }

    // ____________________

    public XModel createDocument( String sDocType )
    {
        XModel aResult = null;
        try
        {
            XComponentLoader aLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class,
                maMSFactory.createInstance( "com.sun.star.frame.Desktop" ) );

            aResult = (XModel) UnoRuntime.queryInterface(
                XModel.class,
                aLoader.loadComponentFromURL( "private:factory/" + sDocType,
                                              "_blank",
                                              0,
                                              new PropertyValue[ 0 ] ) );
        }
        catch( Exception ex )
        {
            System.out.println( "Couldn't create Document of type " + sDocType + ": " + ex );
            System.exit( 0 );
        }

        return aResult;
    }

    // __________ private members and methods __________

    private final String  msDataSheetName  = "Data";
    private final String  msChartSheetName = "Chart";
    private final String  msChartName      = "SampleChart";

    private XMultiServiceFactory   maMSFactory;
    private XSpreadsheetDocument   maSpreadSheetDoc;


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
}
