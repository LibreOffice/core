
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.awt.*;

public class java_Service
    extends com.sun.star.lib.uno.helper.WeakBase
    implements com.sun.star.lang.XServiceInfo, foo.XBar
{
    private XToolkit m_xToolkit;
    private XVclWindowPeer m_xPeer;
    private javax.swing.JFrame m_frame;

    public java_Service( XComponentContext xContext )
    {
        try
        {
            m_xToolkit =
                (XToolkit) UnoRuntime.queryInterface(
                    XToolkit.class,
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.awt.Toolkit", xContext ) );
        }
        catch (com.sun.star.uno.Exception exc)
        {
            throw new com.sun.star.uno.RuntimeException(
                "exception occured gettin toolkit: " + exc, this );
        }
    }

    // XBar impl
    public void raiseAndCloseWindows( foo.XBar [] seq )
        throws com.sun.star.uno.Exception
    {
        for ( int nPos = 0; nPos < seq.length; ++nPos )
            seq[ nPos ].raiseWindows( "called by Java code" );

        // modal dialog before closing
        javax.swing.JOptionPane.showMessageDialog(
            null, "[Java] all windows created." );

        for ( int nPos = 0; nPos < seq.length; ++nPos )
            seq[ nPos ].closeWindows();
    }

    public void raiseWindows( String msg )
        throws com.sun.star.uno.Exception
    {
        // create java frame
        m_frame = new javax.swing.JFrame(
            "[java frame created by Java code] " + msg );
        m_frame.setSize( 500, 50 );
        m_frame.setLocation( 200, 500 );
        m_frame.setVisible( true );

        // create office workwindow
        m_xPeer = (XVclWindowPeer) UnoRuntime.queryInterface(
            XVclWindowPeer.class,
            m_xToolkit.createWindow(
                new WindowDescriptor(
                    WindowClass.TOP, "workwindow", null, (short) -1,
                    new Rectangle( 800, 500, 500, 50 ),
                    WindowAttribute.SHOW |
                    WindowAttribute.BORDER |
                    WindowAttribute.SIZEABLE |
                    WindowAttribute.MOVEABLE |
                    WindowAttribute.CLOSEABLE ) ) );
        m_xPeer.setProperty(
            "Title", "[office window created by Java code] " + msg );
    }

    public void closeWindows()
        throws com.sun.star.uno.Exception
    {
        m_frame.dispose();
        m_xPeer.dispose();
    }


    private static final String s_implName = "foo.java.impl";
    private static final String s_serviceName = "foo.java";

    //  XServiceInfo impl
    public String getImplementationName()
    {
        return s_implName;
    }

    public String [] getSupportedServiceNames()
    {
        return new String [] { s_serviceName };
    }

    public boolean supportsService( String serviceName )
    {
        return serviceName.equals( s_serviceName );
    }

    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(
        String implName,
        com.sun.star.lang.XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey )
    {
        if (implName.equals( s_implName ))
        {
            return com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                java_Service.class, s_serviceName, multiFactory, regKey );
        }
        return null;
    }

    public static boolean __writeRegistryServiceInfo(
        com.sun.star.registry.XRegistryKey regKey )
    {
        return com.sun.star.comp.loader.FactoryHelper.writeRegistryServiceInfo(
            s_implName, s_serviceName, regKey );
    }
}
