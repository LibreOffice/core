import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.datatransfer.*;
import com.sun.star.datatransfer.clipboard.*;
import com.sun.star.uno.AnyConverter;

//-------------------------------------------------
// Demonstrates the usage of the clipboard service
//-------------------------------------------------

public class Clipboard
{
    public static void main(String[] args)
    {
        try
        {
            String sConnection;
            if ( args.length >= 2 )
                sConnection = "uno:" + args[0] + ";urp;StarOffice.ServiceManager" ;
            else
                sConnection = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

            /* Bootstraps a component context with the jurt base components
               registered. Component context to be granted to a component for running.
               Arbitrary values can be retrieved from the context.
            */

            XComponentContext xComponentContext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

            /* Gets the service manager instance to be used (or null). This method has
               been added for convenience, because the service manager is a often used
               object.
            */

            XMultiComponentFactory xMultiComponentFactory =
                xComponentContext.getServiceManager();

            /* Creates an instance of the component UnoUrlResolver which
               supports the services specified by the factory.
            */

            Object objectUrlResolver =
                xMultiComponentFactory.createInstanceWithContext(
                    "com.sun.star.bridge.UnoUrlResolver",
                    xComponentContext );

            // Create a new url resolver

            XUnoUrlResolver xurlresolver = (XUnoUrlResolver)
                UnoRuntime.queryInterface(XUnoUrlResolver.class, objectUrlResolver);

            // Resolves an object that is specified as follow:
            // uno:<connection description>;<protocol description>;<initial object name>

            Object objectInitial = xurlresolver.resolve(sConnection);

            // Create a service manager from the initial object
            xMultiComponentFactory = (XMultiComponentFactory)
            UnoRuntime.queryInterface(XMultiComponentFactory.class, objectInitial);

            Object oClipboard =
            xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.datatransfer.clipboard.SystemClipboard",
                xComponentContext);

            XClipboard xClipboard = (XClipboard)
                UnoRuntime.queryInterface(XClipboard.class, oClipboard);

            //---------------------------------------------------
            // registering as clipboard listener
            //---------------------------------------------------

            XClipboardNotifier xClipNotifier = (XClipboardNotifier)
                UnoRuntime.queryInterface(XClipboardNotifier.class, oClipboard);

            ClipboardListener aClipListener= new ClipboardListener();

            xClipNotifier.addClipboardListener(aClipListener);

            //---------------------------------------------------
            // get a list of formats currently on the clipboard
            //---------------------------------------------------

            XTransferable xTransferable = xClipboard.getContents();

            DataFlavor[] aDflvArr = xTransferable.getTransferDataFlavors();

            // print all available formats

            System.out.println("Reading the clipboard...");
            System.out.println("Available clipboard formats:");

            DataFlavor aUniFlv = null;

            for (int i=0;i<aDflvArr.length;i++)
            {
                System.out.println( "MimeType: " +
                                    aDflvArr[i].MimeType +
                                    " HumanPresentableName: " +
                                    aDflvArr[i].HumanPresentableName );

                // if there is the format unicode text on the clipboard save the
                // corresponding DataFlavor so that we can later output the string

                if ( aDflvArr[i].MimeType.equals("text/plain;charset=utf-16") )
                {
                    aUniFlv = aDflvArr[i];
                }
            }

            System.out.println("");

            try
            {
                if (aUniFlv != null)
                {
                    System.out.println("Unicode text on the clipboard...");
                    Object aData = xTransferable.getTransferData(aUniFlv);
                    System.out.println(AnyConverter.toString(aData));
                }
            }
            catch( UnsupportedFlavorException ex )
            {
                System.err.println( "Requested format is not available on the clipboard!" );
            }

            //---------------------------------------------------
            // becoming a clipboard owner
            //---------------------------------------------------

            System.out.println("Becoming a clipboard owner...");
            System.out.println("");

            ClipboardOwner aClipOwner = new ClipboardOwner();
            xClipboard.setContents(new TextTransferable("Hello World!"), aClipOwner);
            int iFirst = 0;

            while (aClipOwner.isClipboardOwner())
            {
                if (iFirst != 2) {
                    if (iFirst == 1) {
                        System.out.println("Change clipboard ownership by putting something into the clipboard!\n");
                        System.out.print("Still clipboard owner...");
                    } else {
                        System.out.println("Still clipboard owner...");
                    }
                    ++iFirst;
                } else {
                    System.out.print(".");
                }
                Thread.sleep(1000);
            }

            //---------------------------------------------------
            // unregistering as clipboard listener
            //---------------------------------------------------

            xClipNotifier.removeClipboardListener(aClipListener);

            System.exit(0);
        }
        catch( Exception exception )
        {
            System.err.println(exception);
        }
    }
}
