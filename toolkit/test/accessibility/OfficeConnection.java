// base classes
import com.sun.star.uno.UnoRuntime;

// factory for creating components
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

// Exceptions
import com.sun.star.uno.RuntimeException;


/** @descr  This class establishes a connection to a StarOffice application.
 */
public class OfficeConnection
{
    public OfficeConnection (int nPortNumber)
    {
        mnDefaultPort = nPortNumber;
        connect ();
    }

    /** @descr Return the service manager that represents the connected
                StarOffice application
    */
    public XMultiServiceFactory getServiceManager ()
    {
        if ( ! mbInitialized)
            connect ();
        return maServiceManager;
    }

    /** @descr  Return a flag that indicates if the constructor has been able to
                establish a valid connection.
    */
    public boolean connectionIsValid ()
    {
        return getServiceManager() != null;
    }

    /** @descr  Connect to a already running StarOffice application.
    */
    private void connect ()
    {
        connect (msDefaultHost, mnDefaultPort);
    }

    private void connect (String hostname)
    {
        connect (hostname, mnDefaultPort);
    }

    /** @descr  Connect to a already running StarOffice application that has
                been started with a command line argument like
                "-accept=socket,host=localhost,port=5678;urp;"
    */
    private void connect (String hostname, int portnumber)
    {
        mbInitialized = true;
        //  Set up connection string.
        String sConnectString = "uno:socket,host=" + hostname + ",port=" + portnumber
            + ";urp;StarOffice.ServiceManager";


        // connect to a running office and get the ServiceManager
        try
        {
            //  Create a URL Resolver.
            XMultiServiceFactory aLocalServiceManager =
                com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();
            XUnoUrlResolver aURLResolver = (XUnoUrlResolver) UnoRuntime.queryInterface (
                XUnoUrlResolver.class,
                aLocalServiceManager.createInstance ("com.sun.star.bridge.UnoUrlResolver")
                );

            maServiceManager = (XMultiServiceFactory) UnoRuntime.queryInterface (
                    XMultiServiceFactory.class,
                    aURLResolver.resolve (sConnectString)
                    );
        }

        catch (Exception e)
        {
            MessageArea.println ("Could not connect with " + sConnectString + " : " + e);
            MessageArea.println ("Please start OpenOffice/StarOffice with "
                + "\"-accept=socket,host=localhost,port=5678;urp;\"");
        }
    }

    private int mnDefaultPort = 5678;
    private final String msDefaultHost = "localhost";
    private XMultiServiceFactory  maServiceManager = null;

    /** A value of true just indicates that it has been tried to establish a connection,
        not that that has been successfull.
    */
    private boolean mbInitialized = false;
}
