/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WindowAdapter.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

import com.sun.star.awt.*;


public class WindowAdapter
// defacto implementing the interface, but not deriving from it, since
// we're no real XInterface here
//    implements com.sun.star.awt.XWindow
{
    // the frame object we're generating. TODO: Remove public access.
    public java.awt.Frame   frame;
    private boolean         fullscreen;

    public WindowAdapter( int       windowHandle,
                          boolean   _fullscreen         )
    {
        CanvasUtils.printLog( "WindowAdapter(X11): constructor called" );
        fullscreen = false;

        if( _fullscreen )
        {
            // create a normal Java frame, and set it into fullscreen mode
            frame = new javax.swing.JFrame( "Presentation" );
            frame.setUndecorated( true );
            frame.setVisible( true );

            java.awt.Graphics2D graphics = (java.awt.Graphics2D)frame.getGraphics();
            if( graphics.getDeviceConfiguration().getDevice().isFullScreenSupported() )
            {
                CanvasUtils.printLog( "WindowAdapter(X11): entering fullscreen mode" );
                graphics.getDeviceConfiguration().getDevice().setFullScreenWindow( frame );
                fullscreen = true;
            }
            else
            {
                CanvasUtils.printLog( "WindowAdapter(X11): fullscreen not supported" );
            }

            graphics.dispose();
        }
        else
        {
            // we're initialized with the operating system window handle
            // as the parameter. We then generate a dummy Java frame with
            // that window as the parent, to fake a root window for the
            // Java implementation.

            // now, we're getting slightly system dependent here.
            String os = (String) System.getProperty("os.name");

            System.err.println("WindowAdapter created");

            // create the embedded frame
            if( os.startsWith("Linux") )
            {
                // create a java frame from that
                // TODO: Maybe that's the reason why we crash on Linux 1.5beta
                // immediately: Try XAWT here, or  naked X: sun.awt.X11.XEmbeddedFrame

                // TODO
                //frame = new sun.awt.motif.MEmbeddedFrame( windowHandle ); // Crashes 1.5, because class is unknown
                //frame = new sun.awt.X11.XEmbeddedFrame( windowHandle ); // cannot currently compile, because class is not in 1.4.
                CanvasUtils.printLog( "WindowAdapter(X11): no frame created for now" );
                frame = null; // disabled for now
            }
            else
            {
                throw new com.sun.star.uno.RuntimeException();
            }
        }
    }

    //----------------------------------------------------------------------------------

    public void dispose()
    {
        if( fullscreen )
        {
            java.awt.Graphics2D graphics = (java.awt.Graphics2D)frame.getGraphics();
            if( graphics.getDeviceConfiguration().getDevice().isFullScreenSupported() )
            {
                CanvasUtils.printLog( "WindowAdapter(X11): leaving fullscreen mode" );
                graphics.getDeviceConfiguration().getDevice().setFullScreenWindow( null );
            }
            graphics.dispose();
        }

        if( frame != null )
            frame.dispose();
    }

    //----------------------------------------------------------------------------------

    //
    // XWindow interface
    // =================
    //
    public void setPosSize( int X, int Y, int Width, int Height, short Flags )
    {
        frame.setBounds( new java.awt.Rectangle( X, Y, Width, Height ) );
    }

    public com.sun.star.awt.Rectangle getPosSize(  )
    {
        java.awt.Rectangle bounds = frame.getBounds();

        return new com.sun.star.awt.Rectangle( bounds.x, bounds.y, bounds.width, bounds.height );
    }

    public void setVisible( boolean visible )
    {
         frame.setVisible( visible );
    }

    public void setEnable( boolean enable )
    {
        frame.setEnabled( enable );
    }

    public void setFocus()
    {
    }

    public void addWindowListener( XWindowListener xListener )
    {
    }

    public void removeWindowListener( XWindowListener xListener )
    {
    }

    public void addFocusListener( XFocusListener xListener )
    {
    }

    public void removeFocusListener( XFocusListener xListener )
    {
    }

    public void addKeyListener( XKeyListener xListener )
    {
    }

    public void removeKeyListener( XKeyListener xListener )
    {
    }

    public void addMouseListener( XMouseListener xListener )
    {
    }

    public void removeMouseListener( XMouseListener xListener )
    {
    }

    public void addMouseMotionListener( XMouseMotionListener xListener )
    {
    }

    public void removeMouseMotionListener( XMouseMotionListener xListener )
    {
    }

    public void addPaintListener( XPaintListener xListener )
    {
    }

    public void removePaintListener( XPaintListener xListener )
    {
    }
}
