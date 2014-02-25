/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package embeddedobj.test;

import java.awt.*;
import java.applet.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;

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

import com.sun.star.awt.XPaintListener;
import com.sun.star.awt.PaintEvent;
import com.sun.star.awt.XMouseListener;
import com.sun.star.awt.XMouseMotionListener;
import com.sun.star.awt.MouseEvent;
import com.sun.star.awt.Point;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.task.XJob;
import com.sun.star.beans.NamedValue;


class BitmapPainter implements XPaintListener, XMouseListener, XMouseMotionListener, XJob
{
    private XWindow m_xWindow;
    private XBitmap m_xBitmap;

    private com.sun.star.awt.Rectangle m_aDrawRect;

    private Object m_oImageLock;

    private PaintThread m_aPaintThread;

    // private XJob m_xMainThreadExecutor;
    // private NamedValue[] m_pValuesForExecutor;

    private boolean m_bFree = true;

    private boolean m_bProceedWithPainting = true;

// Methods

    public BitmapPainter( XJob xJob, XWindow xWindow, XBitmap xBitmap, com.sun.star.awt.Rectangle aDrawRect )
    {
        if ( xJob == null )
        {
            System.out.println( "No mainthreadexecutor is provided to BimapPainter on init!" );
            throw new com.sun.star.uno.RuntimeException();
        }

        if ( xWindow == null )
        {
            System.out.println( "No window is provided to BimapPainter on init!" );
            throw new com.sun.star.uno.RuntimeException();
        }

        // m_xMainThreadExecutor = xJob;
        // m_pValuesForExecutor = new NamedValue[1];
        // m_pValuesForExecutor[0] = new NamedValue( "JobToExecute", (Object)this );

        m_xWindow = xWindow;
        m_xBitmap = xBitmap;

        m_aDrawRect = aDrawRect;

        m_oImageLock = new Object();

        m_aPaintThread = new PaintThread( m_xWindow );
        m_aPaintThread.start();

        m_xWindow.addPaintListener( this );
        m_xWindow.addMouseListener( this );
        m_xWindow.addMouseMotionListener( this );
    }


    public void disconnectListener()
    {
        m_aPaintThread.disposeThread();
        m_xWindow.removePaintListener( this );
        m_xWindow.removeMouseListener( this );
        m_xWindow.removeMouseMotionListener( this );
    }


    public void setBitmap( XBitmap xBitmap )
    {
        synchronized( m_oImageLock )
        {
            m_xBitmap = xBitmap;
        }
    }


    public void setPos( com.sun.star.awt.Point aPoint )
    {
        synchronized( m_oImageLock )
        {
            m_aDrawRect.X = aPoint.X;
            m_aDrawRect.Y = aPoint.Y;
        }
    }


    public void setRect( com.sun.star.awt.Rectangle aRect )
    {
        synchronized( m_oImageLock )
        {
            m_aDrawRect = aRect;
        }
    }


    public void setSize( com.sun.star.awt.Size aSize )
    {
        synchronized( m_oImageLock )
        {
            m_aDrawRect.Width = aSize.Width;
            m_aDrawRect.Height = aSize.Height;
        }
    }


    public void stopPainting()
    {
        m_bProceedWithPainting = false;
    }


    public void startPainting()
    {
        m_bProceedWithPainting = true;
    }

    // XPaintListener

    public void windowPaint( PaintEvent e )
    {
        if ( !m_bProceedWithPainting )
            return;

        XBitmap xBitmap = null;
        com.sun.star.awt.Rectangle aRect = null;
        // boolean bFree = false;

        synchronized( m_oImageLock )
        {
            xBitmap = m_xBitmap;
            aRect = m_aDrawRect;
            // if ( m_bFree )
            // {
                // bFree = true;
                // m_bFree = false;
            // }
        }

        m_aPaintThread.setPaintRequest( xBitmap, aRect, e.UpdateRect );
        // if ( bFree )
        // {
            // try {
                // m_xMainThreadExecutor.execute( m_pValuesForExecutor );
            // } catch( Exception ex )
            // {
                // m_bFree = true;
            // }
        // }

        System.out.println( "VCL window paint event!" );
    }

    // XMouseListener

    public void mousePressed( MouseEvent e )
    {
    }


    public void mouseReleased( MouseEvent e )
    {
    }


    public void mouseEntered( MouseEvent e )
    {
    }


    public void mouseExited( MouseEvent e )
    {
    }

    // XMouseMotionListener

    public void mouseDragged( MouseEvent e )
    {
        // TODO: react to resizing of object bitmap
        // if the object is inplace active the object must control resizing
    }


    public void mouseMoved( MouseEvent e )
    {

    }

    // XEventListener

    public void disposing( com.sun.star.lang.EventObject e )
    {
        // do nothing, the window can die only when the application is closed
    }

    // XJob

    public Object execute( NamedValue[] pValues )
    {
/*
        // means request for painting

        XBitmap xBitmap = null;
        com.sun.star.awt.Rectangle aRect = null;

        synchronized( m_oImageLock )
        {
            xBitmap = m_xBitmap;
            aRect = m_aDrawRect;
        }

        System.out.println( "The bitmap is going to be painted!" );

        try {
            XDevice xDevice = (XDevice)UnoRuntime.queryInterface( XDevice.class, m_xWindow );
            if ( xDevice != null )
            {
                System.out.println( "Step1" );
                XGraphics xGraphics = xDevice.createGraphics();
                if ( xBitmap != null )
                {
                    System.out.println( "Step2" );
                    XDisplayBitmap xDisplayBitmap = xDevice.createDisplayBitmap( xBitmap );

                    com.sun.star.awt.Size aSize = xBitmap.getSize();
                    xGraphics.draw( xDisplayBitmap, 0, 0, aSize.Width, aSize.Height,
                                                aRect.X, aRect.Y, aRect.Width, aRect.Height );
                }

                System.out.println( "Step3" );
                xGraphics.drawRect( aRect.X - 1, aRect.Y - 1, aRect.Width + 2, aRect.Height + 2 );

                // draw resize squares
                System.out.println( "Step4" );
                xGraphics.drawRect( aRect.X - 2, aRect.Y - 2, 4, 4 );
                xGraphics.drawRect( aRect.X + aRect.Width - 2, aRect.Y - 2, 4, 4 );
                xGraphics.drawRect( aRect.X - 2, aRect.Y + aRect.Height - 2, 4, 4 );
                xGraphics.drawRect( aRect.X + aRect.Width - 2, aRect.Y + aRect.Height - 2, 4, 4 );

                System.out.println( "Step5" );

                System.out.println( "The bitmap is painted by BitmapPainter!" );
            }
        }
        catch ( Exception e )
        {
        }

        m_bFree = true;

*/
        return Any.VOID;
    }

};

