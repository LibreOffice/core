import java.awt.*;
import java.awt.event.*;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.io.IOException;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.beans.XPropertySet;

import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XComponent;

import com.sun.star.bridge.XBridgeFactory;
import com.sun.star.bridge.XBridge;


public class ConnectionAwareClient extends java.awt.Frame
    implements ActionListener , com.sun.star.lang.XEventListener
{
    private Button _btnWriter,_btnCalc;
    private Label _txtLabel;
    private String _url;

    private XComponentContext _ctx;

    private com.sun.star.frame.XComponentLoader _officeComponentLoader;

    public ConnectionAwareClient( XComponentContext ctx , String url )
    {
        _url = url;
        _ctx = ctx;

        Panel p1 = new Panel();
        _btnWriter = new Button("New writer");
        _btnCalc = new Button("New calc");
        _txtLabel = new Label( "disconnected" );

        _btnWriter.addActionListener(this);
        _btnCalc.addActionListener(this);
        p1.add( _btnWriter );
        p1.add( _btnCalc );
        p1.add( _txtLabel );

        addWindowListener(
            new WindowAdapter()
            {
                public void windowClosing(WindowEvent event)
                {
                    System.exit(0);
                }
            }
            );

        add( p1 );
    }

    public void disposing( com.sun.star.lang.EventObject event )
    {
        // remote bridge has gone down, because the office crashed or was terminated.
        _officeComponentLoader = null;
        _txtLabel.setText( "disconnected" );
    }

    public void actionPerformed( ActionEvent event )
    {
        try
        {
            String sUrl;
            if( event.getSource() == _btnWriter )
            {
                sUrl = "private:factory/swriter";
            }
            else
            {
                sUrl = "private:factory/scalc";
            }
            getComponentLoader().loadComponentFromURL(
                sUrl, "_blank", 0,new com.sun.star.beans.PropertyValue[0] );
            _txtLabel.setText( "connected" );
        }
        catch ( com.sun.star.connection.NoConnectException exc )
        {
            _txtLabel.setText( exc.getMessage() );
        }
        catch ( com.sun.star.uno.Exception exc )
        {
            _txtLabel.setText( exc.getMessage() );
            exc.printStackTrace();
            throw new java.lang.RuntimeException( exc.getMessage()  );
        }
    }

    /** separtates the uno-url into 3 different parts.
     */
    protected static String[] parseUnoUrl(  String url )
    {
        String [] aRet = new String [3];

        if( ! url.startsWith( "uno:" ) )
        {
            return null;
        }

        int semicolon = url.indexOf( ';' );
        if( semicolon == -1 )
            return null;

        aRet[0] = url.substring( 4 , semicolon );
        int nextSemicolon = url.indexOf( ';' , semicolon+1);

        if( semicolon == -1 )
            return null;
        aRet[1] = url.substring( semicolon+1, nextSemicolon );

        aRet[2] = url.substring( nextSemicolon+1);
        return aRet;
    }



    protected com.sun.star.frame.XComponentLoader getComponentLoader()
        throws com.sun.star.uno.Exception
    {
        XComponentLoader officeComponentLoader = _officeComponentLoader;

        if( officeComponentLoader == null )
        {
            // instantiate connector service
            Object x = _ctx.getServiceManager().createInstanceWithContext(
                "com.sun.star.connection.Connector", _ctx );

            XConnector xConnector = (XConnector )
                UnoRuntime.queryInterface(XConnector.class, x);

            String a[] = parseUnoUrl( _url );
            if( null == a )
            {
                throw new com.sun.star.uno.Exception( "Couldn't parse uno-url "+ _url );
            }

            // connect using the connection string part of the uno-url only.
            XConnection connection = xConnector.connect( a[0] );

            x = _ctx.getServiceManager().createInstanceWithContext(
                "com.sun.star.bridge.BridgeFactory", _ctx );

            XBridgeFactory xBridgeFactory = (XBridgeFactory) UnoRuntime.queryInterface(
                XBridgeFactory.class , x );

            // create a nameless bridge with no instance provider
            // using the middle part of the uno-url
            XBridge bridge = xBridgeFactory.createBridge( "" , a[1] , connection , null );

            // query for the XComponent interface and add this as event listener
            XComponent xComponent = (XComponent) UnoRuntime.queryInterface(
                XComponent.class, bridge );
            xComponent.addEventListener( this );

            // get the remote instance
            x = bridge.getInstance( a[2] );

            // Did the remote server export this object ?
            if( null == x )
            {
                throw new com.sun.star.uno.Exception(
                    "Server didn't provide an instance for" + a[2], null );
            }

            // Query the initial object for its main factory interface
            XMultiComponentFactory xOfficeMultiComponentFactory = ( XMultiComponentFactory )
                UnoRuntime.queryInterface( XMultiComponentFactory.class, x );

            // retrieve the component context (it's not yet exported from the office)
            // Query for the XPropertySet interface.
            XPropertySet xProperySet = ( XPropertySet )
                UnoRuntime.queryInterface( XPropertySet.class, xOfficeMultiComponentFactory );

            // Get the default context from the office server.
            Object oDefaultContext =
                xProperySet.getPropertyValue( "DefaultContext" );

            // Query for the interface XComponentContext.
            XComponentContext xOfficeComponentContext =
                ( XComponentContext ) UnoRuntime.queryInterface(
                    XComponentContext.class, oDefaultContext );


            // now create the desktop service
            // NOTE: use the office component context here !
            Object oDesktop = xOfficeMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.frame.Desktop", xOfficeComponentContext );

            officeComponentLoader = ( XComponentLoader )
                UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );

            if( officeComponentLoader == null )
            {
                throw new com.sun.star.uno.Exception(
                    "Couldn't instantiate com.sun.star.frame.Desktop" , null );
            }
            _officeComponentLoader = officeComponentLoader;
        }
        return officeComponentLoader;
    }

    public static void main( String [] args ) throws java.lang.Exception
        {
            if( args.length != 1 )
            {
                System.out.println( "usage: ConnectionAwareClient uno-url" );
                return;
            }
            XComponentContext ctx =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

            ConnectionAwareClient connAware = new ConnectionAwareClient( ctx, args[0]);
            connAware.pack();
            connAware.setVisible( true );
        }
}

