// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;

// exceptions
import com.sun.star.container.NoSuchElementException;

// interfaces
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;

// helper
import com.sun.star.uno.IBridge;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;

// others
import java.lang.String;

// __________ Implementation __________

/**
 * support ONE singleton uno connection to an running office installation!
 * Can be used to open/use/close connection to uno environment of an already running office.
 * ctor isn't available from outside. You should call static function "getConnection()"
 * to open or use internal set connection which is created one times only.
 *
 * @author      Andreas Schl&uuml;ns
 * @created     7. Februar 2002
 * @modified    05.02.2002 12:10
 */
public class OfficeConnect
{
    // ____________________

    /**
     * At first call we create static connection object and open connection to already runing office - if we can.
     * Then - and for all further requests we return these static connection member.
     *
     * @param  sHost  host on which office runs
     * @param  sPort  port on which office can be found
     * @return        Description of the Returned Value
     */
    public static synchronized OfficeConnect createConnection(String sHost, String sPort)
    {
        if (maConnection == null)
        {
            maConnection = new OfficeConnect(sHost, sPort);
        }
        return maConnection;
    }

    // ____________________

    public static synchronized OfficeConnect getConnection()
    {
        return maConnection;
    }

    // ____________________

    /**
     * ctor
     * We try to open the connection in our ctor ... transparently for user.
     * After it was successfully you will find an internal set member m_xFactory wich
     * means remote uno service manager of connected office.
     * We made it private to support singleton pattern of these implementation.
     * see getConnection() for further informations
     *
     * @param  sHost  host on which office runs
     * @param  sPort  port on which office can be found
     */
    private OfficeConnect(String sHost, String sPort)
    {
        try
        {
            String sConnectString = "uno:socket,host=" + sHost + ",port=" + sPort + ";urp;StarOffice.ServiceManager";

            com.sun.star.lang.XMultiServiceFactory xLocalServiceManager = com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();
            com.sun.star.bridge.XUnoUrlResolver xURLResolver = (com.sun.star.bridge.XUnoUrlResolver)UnoRuntime.queryInterface(
                    com.sun.star.bridge.XUnoUrlResolver.class,
                    xLocalServiceManager.createInstance("com.sun.star.bridge.UnoUrlResolver"));

            mxServiceManager = (com.sun.star.lang.XMultiServiceFactory)UnoRuntime.queryInterface(
                    com.sun.star.lang.XMultiServiceFactory.class,
                    xURLResolver.resolve(sConnectString));
        }
        catch (com.sun.star.uno.RuntimeException exUNO)
        {
            System.out.println("connection failed" + exUNO);
        }
        catch (com.sun.star.uno.Exception exRun)
        {
            System.out.println("connection failed" + exRun);
        }
        catch (java.lang.Exception exJava)
        {
            System.out.println("connection failed" + exJava);
        }
    }

    // ____________________

    /**
     * URL's must be parsed before they can be used for dispatch.
     * We use special service to do so.
     *
     * @param  sURL  Description of Parameter
     * @return       Description of the Returned Value
     */
    public static com.sun.star.util.URL parseURL(String sURL)
    {
        com.sun.star.util.XURLTransformer xParser = (com.sun.star.util.XURLTransformer)OfficeConnect.getConnection().createRemoteInstance(com.sun.star.util.XURLTransformer.class, "com.sun.star.util.URLTransformer");

        com.sun.star.util.URL[] aURL = new com.sun.star.util.URL[1];
        aURL[0] = new com.sun.star.util.URL();

        aURL[0].Complete = sURL;
        xParser.parseStrict(aURL);
        return aURL[0];
    }

    // ____________________

    /**
     * create uno components inside remote office process
     * After connection of these proccess to a running office we have access to remote service manager of it.
     * So we can use it to create all existing services. Use this method to create components by name and
     * get her interface. Casting of it to right target interface is part of your implementation.
     *
     * @param  aType              describe class type of created service
     *                              Returned object can be casted directly to this one.
     *                              Uno query was done by this method automaticly.
     * @param  sServiceSpecifier  name of service which should be created
     * @return                    Description of the Returned Value
     */
    public Object createRemoteInstance(Class aType, String sServiceSpecifier)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                    aType,
                    mxServiceManager.createInstance(sServiceSpecifier));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.out.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * same as "createRemoteInstance()" but supports additional parameter for initializing created object
     *
     * @param  lArguments         optional arguments
     *                      They are used to initialize new created service.
     * @param  aType              Description of Parameter
     * @param  sServiceSpecifier  Description of Parameter
     * @return                    Description of the Returned Value
     */
    public Object createRemoteInstanceWithArguments(Class aType, String sServiceSpecifier, Any[] lArguments)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                    aType,
                    mxServiceManager.createInstanceWithArguments(
                    sServiceSpecifier,
                    lArguments));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.out.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * member
     */
    private static OfficeConnect                    maConnection    ;    // singleton connection instance
    private com.sun.star.lang.XMultiServiceFactory  mxServiceManager;    // reference to remote service manager of singleton connection object
}

