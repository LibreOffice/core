import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XController;
import com.sun.star.view.XSelectionSupplier;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextFrame;
import com.sun.star.text.XTextContent;
import com.sun.star.beans.XPropertySet;


/** This class gives you information on the selected objects (text range, text
 * frame, or graphics) at an OpenOffice.org Server. The Office must be started in
 * advance using the following command line option:
 * soffice "-accept=socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"
 */
public class WriterSelector {
    /**
     * @param args No arguments.
     */
    public static void main(String args[]) {
        try {
            //the connection string to connect the office
            String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

            // It is possible to use a different connection string, passed as argument
            if ( args.length == 1 ) {
                sConnectionString = args[0];
            }

            /* Bootstraps a component context with the jurt base components
               registered. Component context to be granted to a component for running.
               Arbitrary values can be retrieved from the context. */
            XComponentContext xcomponentcontext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
            null );

            /* Gets the service manager instance to be used (or null). This method has
               been added for convenience, because the service manager is a often used
               object. */
            XMultiComponentFactory xmulticomponentfactory =
            xcomponentcontext.getServiceManager();

            /* Creates an instance of the component UnoUrlResolver which
               supports the services specified by the factory. */
            Object objectUrlResolver =
            xmulticomponentfactory.createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );

            // Create a new url resolver
            XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
            UnoRuntime.queryInterface( XUnoUrlResolver.class,
            objectUrlResolver );

            // Resolves an object that is specified as follow:
            // uno:<connection description>;<protocol description>;<initial object name>
            Object objectInitial = xurlresolver.resolve( sConnectionString );

            // Create a service manager from the initial object
            xmulticomponentfactory = ( XMultiComponentFactory )
            UnoRuntime.queryInterface( XMultiComponentFactory.class,
            objectInitial );

            // Query for the XPropertySet interface.
            XPropertySet xpropertysetMultiComponentFactory = ( XPropertySet )
            UnoRuntime.queryInterface( XPropertySet.class,
            xmulticomponentfactory );

            // Get the default context from the office server.
            Object objectDefaultContext =
            xpropertysetMultiComponentFactory.getPropertyValue(
            "DefaultContext" );

            // Query for the interface XComponentContext.
            xcomponentcontext = ( XComponentContext ) UnoRuntime.queryInterface(
            XComponentContext.class, objectDefaultContext );

            // Querying for the interface XDesktop.
            XDesktop xdesktop = ( XDesktop ) UnoRuntime.queryInterface(
            XDesktop.class, xmulticomponentfactory.createInstanceWithContext(
            "com.sun.star.frame.Desktop", xcomponentcontext ) );

            // Getting the current frame from the OpenOffice.org Server.
            XFrame xframe = xdesktop.getCurrentFrame();

            // Getting the controller.
            XController xcontroller = xframe.getController();

            XSelectionSupplier xselectionsupplier =
            ( XSelectionSupplier ) UnoRuntime.queryInterface(
            XSelectionSupplier.class, xcontroller );

            Object objectSelection = xselectionsupplier.getSelection();

            XServiceInfo xserviceinfo = ( XServiceInfo )
            UnoRuntime.queryInterface( XServiceInfo.class,
            objectSelection );

            if ( xserviceinfo.supportsService( "com.sun.star.text.TextRanges" )
            ) {
                XIndexAccess xindexaccess = ( XIndexAccess )
                UnoRuntime.queryInterface(
                XIndexAccess.class, objectSelection );

                for ( int intCounter = 0; intCounter < xindexaccess.getCount();
                intCounter++ ) {
                    XTextRange xtextrange = ( XTextRange )
                    UnoRuntime.queryInterface(
                    XTextRange.class, xindexaccess.getByIndex( intCounter ) );

                    System.out.println( "You have selected a text range: \""
                    + xtextrange.getString() + "\"." );
                }
            }

            if ( xserviceinfo.supportsService(
            "com.sun.star.text.TextGraphicObject" )
            ) {
                System.out.println( "You have selected a graphics." );
            }

            if ( xserviceinfo.supportsService(
            "com.sun.star.text.TextTableCursor" )
            ) {
                System.out.println( "You have selected a text table." );
            }

            System.exit(0);
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }
}
