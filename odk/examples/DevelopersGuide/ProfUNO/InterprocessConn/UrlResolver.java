import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;


class UrlResolver
{
    public static void main( String [] args ) throws java.lang.Exception
    {
        if( args.length != 1 )
        {
            System.out.println( "usage: UrlResolver uno-url\n" +
                                "       uno-url  The uno-url identifying the object to\n" +
                                "                be imported, for instance\n" +
                                "                uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" +
                                "                (use \" on unix shells to avoid ;-problems" );
            System.exit( 1 );
        }

        // create default local component context
        XComponentContext xLocalContext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

        // create a urlresolver
        Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", xLocalContext );

        // query for the XUnoUrlResolver interface
        XUnoUrlResolver xUrlResolver =
            (XUnoUrlResolver) UnoRuntime.queryInterface( XUnoUrlResolver.class, urlResolver );

        try
        {
            // Import the object
            Object rInitialObject = xUrlResolver.resolve( args[0] );

            // XComponentContext
            if( null != rInitialObject )
            {
                System.out.println( "initial object successfully retrieved" );
            }
            else
            {
                System.out.println( "given initial-object name unknown at server side" );
            }
        }
        catch( com.sun.star.connection.NoConnectException e )
        {
            System.out.println( "Couldn't connect to remote server" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.connection.ConnectionSetupException e )
        {
            System.out.println( "Couldn't access necessary local resource to establish the interprocess connection" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
            System.out.println( "uno-url is syntactical illegal ( " + args[0] + " )" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.uno.RuntimeException e )
        {
            System.out.println( "RuntimeException" );
            System.out.println( e.getMessage() );
        }
        finally {
            System.exit(0);
        }
    }
}
