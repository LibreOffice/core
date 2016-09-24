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
            if ( PaintThread.interceptedRects( aRect, aClip ) )
            {
                m_xBitmap = xBitmap;
                m_aRect = aRect;
                m_bToPaint = true;
            }
        }
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
                XDevice xDevice = (XDevice)UnoRuntime.queryInterface( XDevice.class, m_xWindow );
                if ( xDevice != null )
                {
                    XGraphics xGraphics = xDevice.createGraphics();
                    if ( xBitmap != null )
                    {
                        XDisplayBitmap xDisplayBitmap = xDevice.createDisplayBitmap( xBitmap );

                        com.sun.star.awt.Size aSize = xBitmap.getSize();
                        xGraphics.draw( xDisplayBitmap, 0, 0, aSize.Width, aSize.Height,
                                                    aRect.X, aRect.Y, aRect.Width, aRect.Height );
                    }

                    xGraphics.drawLine( aRect.X - 1, aRect.Y - 1,
                                        aRect.X + aRect.Width + 1, aRect.Y - 1 );
                    xGraphics.drawLine( aRect.X + aRect.Width + 1, aRect.Y - 1,
                                        aRect.X + aRect.Width + 1, aRect.Y + aRect.Height + 1 );
                    xGraphics.drawLine( aRect.X + aRect.Width + 1, aRect.Y + aRect.Height + 1,
                                        aRect.X - 1, aRect.Y + aRect.Height + 1 );
                    xGraphics.drawLine( aRect.X - 1, aRect.Y + aRect.Height + 1,
                                        aRect.X - 1, aRect.Y - 1 );
                }
            }
        }
    }
};

