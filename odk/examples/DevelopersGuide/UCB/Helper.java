// base classes
import java.util.Vector;
import java.io.File;
import java.io.FileOutputStream;
import java.net.URL;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.ucb.*;
import com.sun.star.lang.*;

// factory for creating components
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.bridge.XUnoUrlResolver;

// Exceptions
import com.sun.star.uno.RuntimeException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;

/**
 * Helper for creating a new connection with the specific args to a running office.
 */
public class Helper {

    /**
     * Member properties
     */
    private XInterface m_ucb              = null;
    private String     m_connectString    = null;
    private String     m_contenturl       = null;

    /**
     *  Constructor, create a new connection (ucb) with the specific arguments
     *  to a running office.
     *
     *@param  String   Connect string. Example : -connect=socket,host=localhost,port=8100
     *@param  String   Connect URL.    Example : -url=file:///
     *@exception  java.lang.Exception
     */
    public Helper( String connect, String url ) throws java.lang.Exception {

        m_connectString = connect;
        m_contenturl    = url;
        if ( m_connectString == null || m_connectString.equals( "" )) {
                throw new Exception( "ERROR : Connect String not found. See Help " +
                "( Arguments: -? or -help )." );
        }

        // Create a new xXCB
        m_ucb = createUCB(
            "uno:" + m_connectString + ";urp;StarOffice.ServiceManager" );
    }

    /**
     * Returns created identifier object for given URL..
     *
     *@return     XContent       Created identifier object for given URL
     *@exception  java.lang.Exception
     */
    public XContent createUCBContent() throws java.lang.Exception {
        return createUCBContent( getContentURL() );
    }

    /**
     * Returned created identifier object for given URL.
     *
     *@param      String         Connect URL. Example : -url=file:///
     *@return     XContent       Created identifier object for given URL
     *@exception  java.lang.Exception
     */
    public XContent createUCBContent( String connectURL ) throws java.lang.Exception {
        XContent content = null;
        if ( connectURL != null && !connectURL.equals( "" )) {

            // Obtain required UCB interfaces...
            XContentIdentifierFactory idFactory
                = ( XContentIdentifierFactory )UnoRuntime.queryInterface(
                    XContentIdentifierFactory.class, m_ucb );
            XContentProvider provider
                = ( XContentProvider )UnoRuntime.queryInterface(
                    XContentProvider.class, m_ucb );

            // Create identifier object for given URL.
            XContentIdentifier id = idFactory.createContentIdentifier( connectURL );
            content = provider.queryContent( id );
        }
        return content;
    }

    /**
     * Connect to a running office that is accepting a connection
     * then return the ServiceManager to instantiate office components
     *
     *@param      String    Connect string. Example : -connect=socket,host=localhost,port=8100
     *@return     XMultiServiceFactory
     *@exception  com.sun.star.uno.Exception
     *@exception  java.lang.Exception
     */
    public XInterface createUCB( String connectString )
        throws com.sun.star.uno.Exception, java.lang.Exception {

        // Get component context
        XComponentContext xcomponentcontext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
            null );

        // Initializing serviceManager
        XMultiComponentFactory localServiceManager = xcomponentcontext.getServiceManager();
        XUnoUrlResolver URLResolver = (XUnoUrlResolver) UnoRuntime.queryInterface(
            XUnoUrlResolver.class,
            localServiceManager.createInstanceWithContext(
                "com.sun.star.bridge.UnoUrlResolver",
                xcomponentcontext ) );
        XMultiServiceFactory serviceManager = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            URLResolver.resolve( connectString  ) );
        XInterface ucb = ( XInterface )UnoRuntime.queryInterface(
            XInterface.class,
            serviceManager.createInstance( "com.sun.star.ucb.UniversalContentBroker" ));
        return ucb;
    }

    /**
     *  Get ucb instance.
     *
     *@return   XInterface  That contains the ucb  instance
     */
    public XInterface getUCB() {
        return m_ucb;
    }

    /**
     *  Get connect string.
     *
     *@return   String  That contains the connect string
     */
    public String getConnectString() {
        return m_connectString;
    }

    /**
     *  Get connect URL.
     *
     *@return   String  That contains the connect URL
     */
    public String getContentURL() {
        return m_contenturl;
    }

    /**
     *  Executes a command.
     *
     *param       XInterface
     *param       String
     *param       Object
     *@return     Object     The result according to the specification of the command.
     *@exception  com.sun.star.ucb.CommandAbortedException
     *@exception  com.sun.star.uno.Exception
     */
    Object executeCommand( XInterface ifc, String commandName, Object argument )
        throws com.sun.star.ucb.CommandAbortedException, com.sun.star.uno.Exception  {

        /////////////////////////////////////////////////////////////////////
        // Obtain command processor interface from given content.
        /////////////////////////////////////////////////////////////////////

        XCommandProcessor cmdProcessor
            = (XCommandProcessor)UnoRuntime.queryInterface(
                XCommandProcessor.class, ifc );

        /////////////////////////////////////////////////////////////////////
        // Assemble command to execute.
        /////////////////////////////////////////////////////////////////////

        Command command = new Command();
        command.Name     = commandName;
        command.Handle   = -1; // not available
        command.Argument = argument;

        // Note: throws CommandAbortedException, Exception
        return cmdProcessor.execute( command, 0, null );
    }

    public static String getCurrentDirAsAbsoluteFileURL()
    {
        try
        {
            File file = new File( "" );
            String url = file.toURL().toString();
            if ( url.charAt( 6 ) != '/' ) { // file:/xxx vs. file:///xxxx
                StringBuffer buf = new StringBuffer( "file:///" );
                buf.append( url.substring( 6 ) );
                if ( !url.endsWith( "/" ) )
                    buf.append( '/' );

                url = buf.toString();
            }
            return url;
        }
        catch ( java.net.MalformedURLException e )
        {
            e.printStackTrace();
        }

        return new String();
    }

    public static String getAbsoluteFileURL( String relativeURL )
    {
        StringBuffer buf = new StringBuffer( getCurrentDirAsAbsoluteFileURL() );
        buf.append( relativeURL );
        return buf.toString();
    }

    public static String createTargetDataFile()
    {
        try
        {
            StringBuffer buf = new StringBuffer( "data-" );
            buf.append( System.currentTimeMillis() );
            File file = new File( buf.toString() );
            String url = file.toURL().toString();
            if ( url.charAt( 6 ) != '/' ) { // file:/xxx vs. file:///xxxx
                StringBuffer buf1 = new StringBuffer( "file:///" );
                buf1.append( url.substring( 6 ) );
                url = buf1.toString();
            }

            try
            {
                file.createNewFile();
                String content = new String(
                    "This is the content of a sample data file." );
                FileOutputStream stream = new FileOutputStream( file );
                stream.write( content.getBytes() );
                stream.close();
            }
//            catch ( java.io.FileNotFoundException e ) {}
            catch ( java.io.IOException e )
            {
                e.printStackTrace();
            }

            return url;
        }
        catch ( java.net.MalformedURLException e )
        {
            e.printStackTrace();
        }

        return new String();
    }
}
