package embeddedobj.test;

import java.awt.*;
import java.applet.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.lang.Thread;

import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XDisplayBitmap;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XSystemChildFactory;
import com.sun.star.awt.WindowDescriptor;
import com.sun.star.awt.WindowClass;
import com.sun.star.awt.WindowAttribute;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;

class PaintThread extends java.lang.Thread
{
    private XWindow m_xWindow;
    private XBitmap m_xBitmap;
    private com.sun.star.awt.Rectangle m_aRect;

    private Object m_oRequestsLock;
    private boolean m_bToPaint = false;

    private boolean m_bDisposed = false;

    public static boolean interceptedRects( com.sun.star.awt.Rectangle aRect1, com.sun.star.awt.Rectangle aRect2 )
    {
        return ( ( aRect1.X <= aRect2.X && aRect2.X <= aRect1.X + aRect1.Width
                || aRect1.X <= aRect2.X + aRect2.Width && aRect2.X + aRect2.Width <= aRect1.X + aRect1.Width
                || aRect2.X <= aRect1.X && aRect1.X <= aRect2.X + aRect2.Width
                || aRect2.X <= aRect1.X + aRect1.Width && aRect1.X + aRect1.Width <= aRect2.X + aRect2.Width )
              && ( aRect1.Y <= aRect2.Y && aRect2.Y <= aRect1.Y + aRect1.Height
                || aRect1.Y <= aRect2.Y + aRect2.Height && aRect2.Y + aRect2.Height <= aRect1.Y + aRect1.Height
                || aRect2.Y <= aRect1.Y && aRect1.Y <= aRect2.Y + aRect2.Height
                || aRect2.Y <= aRect1.Y + aRect1.Height && aRect1.Y + aRect1.Height <= aRect2.Y + aRect2.Height ) );
    }

    public PaintThread( XWindow xWindow )
    {
        m_oRequestsLock = new Object();
        m_xWindow = xWindow;
    }

    public void setPaintRequest( XBitmap xBitmap, com.sun.star.awt.Rectangle aRect, com.sun.star.awt.Rectangle aClip )
    {
        synchronized( m_oRequestsLock )
        {
        /*
            System.out.println( "Paint request Pos( "
                                                    + aRect.X + ", "
                                                    + aRect.Y + ", "
                                                    + aRect.Width + ", "
                                                    + aRect.Height + " ), Clip ( "
                                                    + aClip.X + ", "
                                                    + aClip.Y + ", "
                                                    + aClip.Width + ", "
                                                    + aClip.Height + " )" );
        */

            if ( PaintThread.interceptedRects( aRect, aClip ) )
            {
                m_xBitmap = xBitmap;
                m_aRect = aRect;
                m_bToPaint = true;
            }
        }

        // System.out.println( "Paint request to paint thread is done! xBitmap = " + xBitmap );
    }

    public void disposeThread()
    {
        m_bDisposed = true;
    }

    public void run()
    {
        while( !m_bDisposed )
        {
            try {
                Thread.sleep( 200 );
            } catch( Exception e ) {}

            XBitmap xBitmap = null;
            com.sun.star.awt.Rectangle aRect = null;
            boolean bPaint = false;

            synchronized( m_oRequestsLock )
            {
                if ( m_bToPaint )
                {
                    xBitmap = m_xBitmap;
                    aRect = m_aRect;
                    m_bToPaint = false;
                    bPaint = true;
                }
            }

            if ( bPaint )
            {
                // System.out.println( "The bitmap is going to be painted!" );
                XDevice xDevice = (XDevice)UnoRuntime.queryInterface( XDevice.class, m_xWindow );
                if ( xDevice != null )
                {
                     // System.out.println( "Step1" );
                    XGraphics xGraphics = xDevice.createGraphics();
                    if ( xBitmap != null )
                    {
                        // System.out.println( "Step2" );
                        XDisplayBitmap xDisplayBitmap = xDevice.createDisplayBitmap( xBitmap );

                        com.sun.star.awt.Size aSize = xBitmap.getSize();
                        xGraphics.draw( xDisplayBitmap, 0, 0, aSize.Width, aSize.Height,
                                                    aRect.X, aRect.Y, aRect.Width, aRect.Height );
                    }

                    // System.out.println( "Step3" );
                    // xGraphics.drawRect( aRect.X - 1, aRect.Y - 1, aRect.Width + 2, aRect.Height + 2 );
                    xGraphics.drawLine( aRect.X - 1, aRect.Y - 1,
                                        aRect.X + aRect.Width + 1, aRect.Y - 1 );
                    xGraphics.drawLine( aRect.X + aRect.Width + 1, aRect.Y - 1,
                                        aRect.X + aRect.Width + 1, aRect.Y + aRect.Height + 1 );
                    xGraphics.drawLine( aRect.X + aRect.Width + 1, aRect.Y + aRect.Height + 1,
                                        aRect.X - 1, aRect.Y + aRect.Height + 1 );
                    xGraphics.drawLine( aRect.X - 1, aRect.Y + aRect.Height + 1,
                                        aRect.X - 1, aRect.Y - 1 );

                    // draw resize squares
                    // System.out.println( "Step4" );
                    // xGraphics.drawRect( aRect.X - 2, aRect.Y - 2, 4, 4 );
                    // xGraphics.drawRect( aRect.X + aRect.Width - 2, aRect.Y - 2, 4, 4 );
                    // xGraphics.drawRect( aRect.X - 2, aRect.Y + aRect.Height - 2, 4, 4 );
                    // xGraphics.drawRect( aRect.X + aRect.Width - 2, aRect.Y + aRect.Height - 2, 4, 4 );

                    // System.out.println( "Step5" );

                    // System.out.println( "The bitmap is painted by paint thread!" );
                }
            }
        }
    }
};

