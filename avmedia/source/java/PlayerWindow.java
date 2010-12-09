/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XEventListener;
import com.sun.star.awt.*;
import com.sun.star.media.*;

// -----------------
// - Player Window -
// -----------------

public class PlayerWindow implements java.awt.event.KeyListener,
                                     java.awt.event.MouseListener,
                                     java.awt.event.MouseMotionListener,
                                     java.awt.event.FocusListener,
                                     com.sun.star.lang.XServiceInfo,
                                     com.sun.star.media.XPlayerWindow
{
    private com.sun.star.lang.XMultiServiceFactory maFactory;
    private WindowAdapter maFrame;
    private javax.media.Player maPlayer;
    private com.sun.star.media.ZoomLevel meZoomLevel = com.sun.star.media.ZoomLevel.ORIGINAL;
    private boolean mbShowControls = false;


    // -------------------------------------------------------------------------

    public PlayerWindow( com.sun.star.lang.XMultiServiceFactory aFactory,
                         java.lang.Object[] aArgs, javax.media.Player aPlayer )
    {
        maFactory = aFactory;

        try
        {
            if( aArgs.length > 1 )
            {
                com.sun.star.awt.Rectangle aBoundRect = (com.sun.star.awt.Rectangle) aArgs[ 1 ];

                maFrame = new WindowAdapter( AnyConverter.toInt( aArgs[ 0 ] ) );
                maFrame.setPosSize( aBoundRect.X, aBoundRect.Y, aBoundRect.Width, aBoundRect.Height, (short) 0 );
                mbShowControls = false;

                java.awt.Panel aPanel = new java.awt.Panel( new java.awt.BorderLayout() );

                aPanel.setLayout( null );
                aPanel.setBackground( java.awt.Color.black );
                aPanel.addKeyListener( this );
                aPanel.addMouseListener( this );
                aPanel.addMouseMotionListener( this );

                if( mbShowControls )
                {
                    java.awt.Component aControlComponent = aPlayer.getControlPanelComponent();

                    if( aControlComponent != null )
                        aPanel.add( aControlComponent );
                    else
                        mbShowControls = false;
                }

                java.awt.Component aVisualComponent = aPlayer.getVisualComponent();

                if( aVisualComponent != null )
                {
                    aVisualComponent.addKeyListener( this );
                    aVisualComponent.addMouseListener( this );
                    aVisualComponent.addMouseMotionListener( this );
                    aVisualComponent.addFocusListener( this );
                    aPanel.add( aVisualComponent );
                }
                else
                    meZoomLevel = com.sun.star.media.ZoomLevel.NOT_AVAILABLE;

                if( maFrame.getJavaFrame() != null )
                    maFrame.getJavaFrame().add( aPanel );

                LayoutComponents();
            }
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
        }
    }

    // -------------------------------------------------------------------------

    protected synchronized void LayoutComponents()
    {
        if( maFrame.getJavaFrame() != null )
        {
            java.awt.Panel  aPanel = (java.awt.Panel) maFrame.getJavaFrame().getComponent( 0 );
            int             nW = maFrame.getJavaFrame().getWidth();
            int             nH = maFrame.getJavaFrame().getHeight();
            int             nControlH = 0;

            aPanel.setBounds( 0, 0, nW, nH );

            if( mbShowControls )
            {
                java.awt.Component aControlComponent = aPanel.getComponent( 0 );

                if( aControlComponent != null )
                {
                    java.awt.Dimension aControlDimension = aControlComponent.getPreferredSize();

                    nControlH = Math.min( nH, aControlDimension.height );
                    aControlComponent.setBounds( 0, nH - nControlH, nW, nControlH );
                }
            }

            if( com.sun.star.media.ZoomLevel.NOT_AVAILABLE != meZoomLevel )
            {
                java.awt.Component aVisualComponent = aPanel.getComponent( mbShowControls ? 1 : 0 );

                if( aVisualComponent != null )
                {
                    java.awt.Dimension  aPrefDim = aVisualComponent.getPreferredSize();
                    int                 nVideoW = nW, nVideoH = ( nH - nControlH );
                    int                 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
                    boolean             bDone = false, bZoom = false;

                    if( com.sun.star.media.ZoomLevel.ORIGINAL == meZoomLevel )
                    {
                        bZoom = true;
                    }
                    else if( com.sun.star.media.ZoomLevel.ZOOM_1_TO_4 == meZoomLevel )
                    {
                        aPrefDim.width >>= 2;
                        aPrefDim.height >>= 2;
                        bZoom = true;
                    }
                    else if( com.sun.star.media.ZoomLevel.ZOOM_1_TO_2 == meZoomLevel )
                    {
                        aPrefDim.width >>= 1;
                        aPrefDim.height >>= 1;
                        bZoom = true;
                    }
                    else if( com.sun.star.media.ZoomLevel.ZOOM_2_TO_1 == meZoomLevel )
                    {
                        aPrefDim.width <<= 1;
                        aPrefDim.height <<= 1;
                        bZoom = true;
                    }
                    else if( com.sun.star.media.ZoomLevel.ZOOM_4_TO_1 == meZoomLevel )
                    {
                        aPrefDim.width <<= 2;
                        aPrefDim.height <<= 2;
                        bZoom = true;
                    }
                    else if( com.sun.star.media.ZoomLevel.FIT_TO_WINDOW == meZoomLevel )
                    {
                        nWidth = nVideoW;
                        nHeight = nVideoH;
                        bDone = true;
                    }

                    if( bZoom )
                    {
                        if( ( aPrefDim.width <= nVideoW ) && ( aPrefDim.height <= nVideoH ) )
                        {
                            nX = ( nVideoW - aPrefDim.width ) >> 1;
                            nY = ( nVideoH - aPrefDim.height ) >> 1;
                            nWidth = aPrefDim.width;
                            nHeight = aPrefDim.height;
                            bDone = true;
                        }
                    }

                    if( !bDone )
                    {
                        if( aPrefDim.width > 0 && aPrefDim.height > 0 && nVideoW > 0 && nVideoH > 0 )
                        {
                            double fPrefWH = (double) aPrefDim.width / aPrefDim.height;

                            if( fPrefWH < ( (double) nVideoW / nVideoH ) )
                                nVideoW = (int)( nVideoH * fPrefWH );
                            else
                                nVideoH = (int)( nVideoW / fPrefWH );

                            nX = ( nW - nVideoW ) >> 1;
                            nY = ( nH - nControlH - nVideoH ) >> 1;
                            nWidth = nVideoW;
                            nHeight = nVideoH;
                        }
                        else
                            nX = nY = nWidth = nHeight = 0;
                    }

                    aVisualComponent.setBounds( nX, nY, nWidth, nHeight );
                    aVisualComponent.requestFocus();
                }
                else
                    aPanel.requestFocus();
            }
            else
                aPanel.requestFocus();
        }
    }

    // -------------------------------------------------------------------------

    private void implFireMouseEvent( java.awt.event.MouseEvent aEvt )
    {
        if( aEvt.getSource() != null &&
            aEvt.getSource() instanceof java.awt.Component )
        {
            aEvt.translatePoint( ( (java.awt.Component) aEvt.getSource() ).getX(),
                                 ( (java.awt.Component) aEvt.getSource() ).getY() );
        }

        maFrame.fireMouseEvent( aEvt );
    }

    // ---------------
    // - KeyListener -
    // ---------------

    public void keyPressed( java.awt.event.KeyEvent aEvt )
    {
        maFrame.fireKeyEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void keyReleased( java.awt.event.KeyEvent aEvt )
    {
        maFrame.fireKeyEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void keyTyped( java.awt.event.KeyEvent aEvt )
    {
        maFrame.fireKeyEvent( aEvt );
    }

    // -----------------
    // - MouseListener -
    // -----------------

    public void mousePressed( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void mouseClicked( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void mouseEntered( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void mouseExited( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void mouseReleased( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -----------------------
    // - MouseMotionListener -
    // -----------------------

    public void mouseDragged( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void mouseMoved( java.awt.event.MouseEvent aEvt )
    {
        implFireMouseEvent( aEvt );
    }

    // -----------------------
    // - FocusListener -
    // -----------------------

    public void focusGained( java.awt.event.FocusEvent aEvt )
    {
        if( maFrame.getJavaFrame() != null )
            maFrame.fireFocusEvent( aEvt );
    }

    // -------------------------------------------------------------------------

    public void focusLost( java.awt.event.FocusEvent aEvt )
    {
        if( maFrame.getJavaFrame() != null )
            maFrame.fireFocusEvent( aEvt );
    }

    // -----------------
    // - XPlayerWindow -
    // -----------------

    public synchronized void update()
    {
        if( maFrame.getJavaFrame() != null )
            maFrame.getJavaFrame().repaint();
    }

    // -------------------------------------------------------------------------

    public synchronized boolean setZoomLevel( com.sun.star.media.ZoomLevel eZoomLevel )
    {
        boolean bRet = false;

        if( com.sun.star.media.ZoomLevel.NOT_AVAILABLE != meZoomLevel &&
            com.sun.star.media.ZoomLevel.NOT_AVAILABLE != eZoomLevel )
        {
            if( eZoomLevel != meZoomLevel )
            {
                meZoomLevel = eZoomLevel;
                LayoutComponents();
            }

            bRet = true;
        }

        return bRet;
    }

    // -------------------------------------------------------------------------

    public synchronized com.sun.star.media.ZoomLevel getZoomLevel()
    {
        return meZoomLevel;
    }

    // -------------------------------------------------------------------------

    public synchronized void setPointerType( int nPointerType )
    {
        if( maFrame.getJavaFrame() != null )
        {
            int nCursor;

            switch( nPointerType )
            {
                case( com.sun.star.awt.SystemPointer.CROSS ): nCursor = java.awt.Cursor.CROSSHAIR_CURSOR; break;
                case( com.sun.star.awt.SystemPointer.HAND ): nCursor = java.awt.Cursor.HAND_CURSOR; break;
                case( com.sun.star.awt.SystemPointer.MOVE ): nCursor = java.awt.Cursor.MOVE_CURSOR; break;
                case( com.sun.star.awt.SystemPointer.WAIT ): nCursor = java.awt.Cursor.WAIT_CURSOR; break;

                default: nCursor = java.awt.Cursor.DEFAULT_CURSOR; break;
            }

            maFrame.getJavaFrame().setCursor( java.awt.Cursor.getPredefinedCursor( nCursor ) );
        }
    }

    // --------------
    // - XComponent -
    // --------------

    public synchronized void dispose()
    {
        if( maFrame != null )
        {
            java.awt.Panel aPanel = (java.awt.Panel) maFrame.getJavaFrame().getComponent( 0 );

            if( aPanel != null && aPanel.getComponent( 0 ) != null )
                aPanel.getComponent( 0 ).removeFocusListener( this );

            if( maFrame.getJavaFrame() != null )
                maFrame.getJavaFrame().dispose();

            maFrame.fireDisposingEvent();
        }

        maFrame = null;
    }

    // -----------
    // - XWindow -
    // -----------

    public synchronized void setPosSize( int X, int Y, int Width, int Height, short Flags )
    {
        if( maFrame != null )
        {
            maFrame.setPosSize( X, Y, Width, Height, Flags );
            LayoutComponents();
        }
    }

    // -------------------------------------------------------------------------

    public synchronized com.sun.star.awt.Rectangle getPosSize()
    {
        return( ( maFrame != null ) ? maFrame.getPosSize() : new com.sun.star.awt.Rectangle() );
    }

    // -------------------------------------------------------------------------

    public synchronized void setVisible( boolean visible )
    {
        if( maFrame != null )
            maFrame.setVisible( visible );
    }

    // -------------------------------------------------------------------------

    public synchronized void setEnable( boolean enable )
    {
        if( maFrame != null )
            maFrame.setEnable( enable );
    }

    // -------------------------------------------------------------------------

    public synchronized void setFocus()
    {
        if( maFrame != null )
            maFrame.setFocus();
    }

    // -------------------------------------------------------------------------

    public synchronized void addEventListener( com.sun.star.lang.XEventListener xListener )
    {
        if( maFrame != null )
            maFrame.addEventListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeEventListener( com.sun.star.lang.XEventListener xListener )
    {
        if( maFrame != null )
            maFrame.removeEventListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addWindowListener( XWindowListener xListener )
    {
        if( maFrame != null )
            maFrame.addWindowListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeWindowListener( XWindowListener xListener )
    {
        if( maFrame != null )
            maFrame.removeWindowListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addFocusListener( XFocusListener xListener )
    {
        if( maFrame != null )
            maFrame.addFocusListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeFocusListener( XFocusListener xListener )
    {
        if( maFrame != null )
            maFrame.removeFocusListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addKeyListener( XKeyListener xListener )
    {
        if( maFrame != null )
            maFrame.addKeyListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeKeyListener( XKeyListener xListener )
    {
        if( maFrame != null )
            maFrame.removeKeyListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addMouseListener( XMouseListener xListener )
    {
        if( maFrame != null )
            maFrame.addMouseListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeMouseListener( XMouseListener xListener )
    {
        if( maFrame != null )
            maFrame.removeMouseListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addMouseMotionListener( XMouseMotionListener xListener )
    {
        if( maFrame != null )
            maFrame.addMouseMotionListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removeMouseMotionListener( XMouseMotionListener xListener )
    {
        if( maFrame != null )
            maFrame.removeMouseMotionListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void addPaintListener( XPaintListener xListener )
    {
        if( maFrame != null )
            maFrame.addPaintListener( xListener );
    }

    // -------------------------------------------------------------------------

    public synchronized void removePaintListener( XPaintListener xListener )
    {
        if( maFrame != null )
            maFrame.removePaintListener( xListener );
    }

    // ----------------
    // - XServiceInfo -
    // ----------------

    private static final String s_implName = "com.sun.star.comp.PlayerWindow_Java";
    private static final String s_serviceName = "com.sun.star.media.PlayerWindow_Java";

    public synchronized String getImplementationName()
    {
        return s_implName;
    }

    // -------------------------------------------------------------------------

    public synchronized String [] getSupportedServiceNames()
    {
        return new String [] { s_serviceName };
    }

    // -------------------------------------------------------------------------

    public synchronized boolean supportsService( String serviceName )
    {
        return serviceName.equals( s_serviceName );
    }
}
