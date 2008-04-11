/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BackBuffer.java,v $
 * $Revision: 1.6 $
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

// UNO
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.lang.XInitialization;
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import com.sun.star.rendering.*;

// Java AWT
import java.awt.*;
import java.awt.image.*;
import java.awt.geom.*;

public class BackBuffer
{
    private BufferedImage                   backBuffer;
    //private VolatileImage                     backBuffer;
    private Graphics2D                      backBufferGraphics;
    private Graphics2D                      referenceDevice;

    public BackBuffer( Graphics2D _referenceDevice,
                       int width,
                       int height )
    {
        referenceDevice = _referenceDevice;
        setSize( width, height );
    }

    public Graphics2D getGraphics()
    {
        return backBufferGraphics;
    }

    public void setSize( int width,
                         int height )
    {
        if( backBuffer != null &&
            width == backBuffer.getWidth() &&
            height == backBuffer.getHeight() )
        {
            return;
        }

        if( backBufferGraphics != null )
            backBufferGraphics.dispose();

        if( backBuffer != null )
            backBuffer.flush();

        // TODO: Maybe VolatileImage with another BufferedImage as a backup is
        // a tad faster here.
        backBuffer = referenceDevice.getDeviceConfiguration().createCompatibleImage(width,
                                                                                    height);
//        backBuffer = referenceDevice.getDeviceConfiguration().createCompatibleVolatileImage(width,
//                                                                                            height);

        backBufferGraphics = backBuffer.createGraphics();
        CanvasUtils.initGraphics( backBufferGraphics );

        // clear the buffer to white (to have a defined state here)
        backBufferGraphics.setColor( java.awt.Color.white );
        backBufferGraphics.fillRect( 0,0,width,height );
    }

    public void redraw( Graphics2D graph )
    {
        if( graph != null &&
            backBuffer != null )
        {
            CanvasUtils.printLog("BackBuffer.redraw(): using buffer of size (" +
                                  backBuffer.getWidth() + "," + backBuffer.getHeight() + ")" );

            graph.drawImage(backBuffer, 0, 0, null);

            // TODO: this is just twiddled to work. I cannot be sure
            // that this volatile backbuffer will survive in the first
            // place, nor that it wise to leave it in VRAM.

            // only flush non-volatile images
            // CanvasUtils.postRenderImageTreatment( backBuffer );
        }
    }

    public java.awt.Image getBackBuffer()
    {
        return backBuffer;
    }

    public void dispose()
    {
        backBufferGraphics.dispose();
        backBuffer.flush();
    }
}
