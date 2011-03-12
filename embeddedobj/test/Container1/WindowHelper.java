package embeddedobj.test;

import java.awt.*;
import java.applet.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;

import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XSystemChildFactory;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.WindowAttribute;
import com.sun.star.awt.VclWindowPeerAttribute;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Any;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

class WindowHelper {

    public static XWindow createWindow( XMultiServiceFactory xFactory, NativeView aParent, java.awt.Rectangle aBounds )
    {
        XWindow  xWindow = null;
        XToolkit    xToolkit = null;

        // get access to toolkit of remote office to create the container window of new target frame
        try{
            xToolkit = (XToolkit)UnoRuntime.queryInterface( XToolkit.class,
                                                            xFactory.createInstance("com.sun.star.awt.Toolkit") );
        }
        catch( Exception ex )
        {
            return null;
        }

        XSystemChildFactory xChildFactory = (XSystemChildFactory)UnoRuntime.queryInterface(
                XSystemChildFactory.class,
                xToolkit);

        try
        {
            XWindowPeer xPeer = null;
            Integer nHandle = aParent.getHWND();
            short   nSystem = (short)aParent.getNativeWindowSystemType();
            byte[]  lProcID = new byte[0];
/*
            try {
                xPeer = xChildFactory.createSystemChild((Object)nHandle, lProcID, nSystem);
            }
            catch( Exception e )
            {}
*/
            if (xPeer==null)
            {
                JavaWindowPeerFake aWrapper = new JavaWindowPeerFake(aParent);

                XWindowPeer xParentPeer = (XWindowPeer)UnoRuntime.queryInterface(
                        XWindowPeer.class,
                        aWrapper);

                WindowDescriptor aDescriptor = new WindowDescriptor();
                aDescriptor.Type = WindowClass.TOP;
                aDescriptor.WindowServiceName = "workwindow";
                aDescriptor.ParentIndex = 1;
                aDescriptor.Parent = xParentPeer;
                aDescriptor.Bounds = new com.sun.star.awt.Rectangle( (int)aBounds.getX(),
                                                                     (int)aBounds.getY(),
                                                                     (int)aBounds.getWidth(),
                                                                     (int)aBounds.getHeight() );

                System.out.println( "The rectangle for vcl window is:\nx = " + (int)aBounds.getX()
                                                                + "; y = " + (int)aBounds.getY()
                                                                + "; width = " + (int)aBounds.getWidth()
                                                                + "; height = " + (int)aBounds.getHeight() );

                if (nSystem == com.sun.star.lang.SystemDependent.SYSTEM_WIN32)
                    aDescriptor.WindowAttributes = WindowAttribute.SHOW;
                else
                    aDescriptor.WindowAttributes = WindowAttribute.SYSTEMDEPENDENT;

                aDescriptor.WindowAttributes |= VclWindowPeerAttribute.CLIPCHILDREN;

                xPeer = xToolkit.createWindow( aDescriptor );
            }

            xWindow = (XWindow)UnoRuntime.queryInterface( XWindow.class, xPeer);
            if ( xWindow != null )
                xWindow.setPosSize( (int)aBounds.getX(),
                                    (int)aBounds.getY(),
                                    (int)aBounds.getWidth(),
                                    (int)aBounds.getHeight(),
                                    com.sun.star.awt.PosSize.POSSIZE );
        }
        catch( Exception ex1 )
        {
            System.out.println( "Exception on VCL window creation: " + ex1 );
            xWindow = null;
        }

        return xWindow;
    }

    public static XBitmap getVCLBitmapFromBytes( XMultiServiceFactory xFactory, Object aAny )
    {
        if ( !AnyConverter.isArray( aAny ) )
            throw new com.sun.star.uno.RuntimeException();

        Object[] aArgs = new Object[1];
        aArgs[0] = aAny;
        XBitmap xResult = null;

        try {
            XSingleServiceFactory xBitmapFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(
                    XSingleServiceFactory.class,
                    xFactory.createInstance( "com.sun.star.embed.BitmapCreator" ) );

            xResult = (XBitmap)UnoRuntime.queryInterface(
                    XBitmap.class,
                    xBitmapFactory.createInstanceWithArguments( aArgs ) );
        }
        catch( Exception e )
        {
            System.out.println( "Could not create VCL bitmap based on sequence," );
            System.out.println( "exception: " + e );
        }

        return xResult;
    }
};

