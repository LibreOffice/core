import java.awt.*;
import java.awt.event.*;

import com.sun.star.comp.servicemanager.ServiceManager;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.lang.*;


public class EmbedContFrame extends Frame
{
    WindowListener m_aCloser = new WindowAdapter()
    {
        public void windowClosing( WindowEvent e )
        {
            dispose();
            System.exit( 0 );
        }
    };

    public EmbedContFrame( String sName )
    {
        super( sName );
        addWindowListener( m_aCloser );
    }

    public static void start()
    {
        EmbedContFrame aFrame = new EmbedContFrame( "Testing container." );

        // connect to the office
        XMultiServiceFactory aServiceFactory = null;
        try {
            aServiceFactory = connectOfficeGetServiceFactory();
        }
        catch( Exception e )
        {}

        if ( aServiceFactory == null )
        {
            System.out.println( "Can't get service manager!\n" );
            System.exit( 1 );
        }

        EmbedContApp aApp = new EmbedContApp( aFrame, aServiceFactory );
        aApp.init();
        aApp.start();

        Dimension aSize = aApp.getSize();

        aFrame.add( "Center", aApp );
        aFrame.pack();
        aFrame.setSize( aSize );

        aFrame.setVisible( true );
    }

    public static void main( String args[] )
    {
        EmbedContFrame.start();
    }

    public static XMultiServiceFactory connectOfficeGetServiceFactory()
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException,
    Exception
    {
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // Get component context
        XComponentContext xComponentContext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xComponentContext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  oUrlResolver  = xLocalServiceManager.createInstanceWithContext( "com.sun.star.bridge.UnoUrlResolver",
                                                                                xComponentContext );
        XUnoUrlResolver xUrlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface( XUnoUrlResolver.class, oUrlResolver );

        Object oInitialObject = xUrlResolver.resolve( sConnectionString );
        XNamingService xName = (XNamingService)UnoRuntime.queryInterface( XNamingService.class, oInitialObject );

        XMultiServiceFactory xMSF = null;
        if( xName != null ) {
            Object oMSF = xName.getRegisteredObject( "StarOffice.ServiceManager" );
            xMSF = (XMultiServiceFactory)UnoRuntime.queryInterface( XMultiServiceFactory.class, oMSF );
        }
        else
            System.out.println( "Error: Can't get XNamingService interface from url resolver!" );

        return xMSF;
    }
}

