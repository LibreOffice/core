/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasBitmap.java,v $
 * $Revision: 1.7 $
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
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import com.sun.star.rendering.*;
import com.sun.star.geometry.*;

public class CanvasBitmap
    extends CanvasBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.rendering.XBitmapCanvas,
               com.sun.star.rendering.XIntegerBitmap
{
    private java.awt.image.BufferedImage    bitmap;
    private java.awt.Graphics2D             graphics;

    public CanvasBitmap( java.awt.image.BufferedImage _bitmap )
    {
        bitmap = _bitmap;
        graphics = bitmap.createGraphics();
    }

    public CanvasBitmap( IntegerSize2D mySize )
    {
        bitmap = new java.awt.image.BufferedImage(mySize.Width, mySize.Height,
                                                  java.awt.image.BufferedImage.TYPE_4BYTE_ABGR);
        graphics = bitmap.createGraphics();
    }

    public CanvasBitmap( RealSize2D newSize, boolean beFast, CanvasBitmap source )
    {
//         java.awt.geom.AffineTransform transform = new java.awt.geom.AffineTransform();
//         transform.scale( newSize.width/size.Width, newSize.height/size.Height );

//         // TODO: Maybe keep the image returned via
//         // bitmap.getScaledInstance, and do scaling lazy.
//         bitmap = new java.awt.image.BufferedImage((int)(newSize.width+.5),
//                                                   (int)(newSize.height+.5),
//                                                   java.awt.image.BufferedImage.TYPE_4BYTE_ABGR);

//         java.awt.image.AffineTransformOp transformer =
//             new java.awt.image.AffineTransformOp( transform,
//                                                   java.awt.image.AffineTransformOp.TYPE_BILINEAR);

//         transformer.filter(source.getBufferedImage(), bitmap);
    }

    public synchronized java.awt.image.BufferedImage getBufferedImage()
    {
        return bitmap;
    }

    public java.awt.Graphics2D getGraphics()
    {
        return graphics;
    }

    //
    // XBitmap implementation
    // ======================
    //

    public synchronized IntegerSize2D getSize()
    {
        return new IntegerSize2D( bitmap.getWidth(),
                                  bitmap.getHeight() );
    }

    //----------------------------------------------------------------------------------

    public synchronized XBitmapCanvas queryBitmapCanvas()
    {
        return this;
    }

    //----------------------------------------------------------------------------------

    public synchronized com.sun.star.rendering.XBitmap getScaledBitmap( RealSize2D newSize, boolean beFast ) throws com.sun.star.lang.IllegalArgumentException, VolatileContentDestroyedException
    {
        return new CanvasBitmap( newSize, beFast, this );
    }

    //----------------------------------------------------------------------------------

    public synchronized boolean hasAlpha()
    {
        // TODO
        return false;
    }

    //----------------------------------------------------------------------------------

    //
    // XBitmapCanvas impl
    // ==================
    //

    public synchronized void copyRect( com.sun.star.rendering.XBitmapCanvas sourceCanvas,
                                       com.sun.star.geometry.RealRectangle2D sourceRect,
                                       com.sun.star.rendering.ViewState         sourceViewState,
                                       com.sun.star.rendering.RenderState   sourceRenderState,
                                       com.sun.star.geometry.RealRectangle2D    destRect,
                                       com.sun.star.rendering.ViewState         destViewState,
                                       com.sun.star.rendering.RenderState   destRenderState )
    {
        CanvasUtils.printLog( "JavaCanvas.copyRect() called" );

        // TODO: create temp image when transform is non-trivial

        if( sourceCanvas == this )
        {
            // copy rectangle within the canvas
            getGraphics().copyArea((int)sourceRect.X1,
                                   (int)sourceRect.Y1,
                                   (int)(sourceRect.X2 - sourceRect.X1),
                                   (int)(sourceRect.Y2 - sourceRect.Y1),
                                   (int)(destRect.X1 - sourceRect.X1),
                                   (int)(destRect.Y1 - sourceRect.Y1) );
        }
        else
        {
            if( sourceCanvas instanceof JavaCanvas )
            {
                // cache
                CanvasUtils.setupGraphicsState( getGraphics(), destViewState, destRenderState, CanvasUtils.alsoSetupPaint );

                java.awt.Image backBuffer = ((JavaCanvas)sourceCanvas).backBuffer.getBackBuffer();

                // TODO: really extract correct source rect here
                getGraphics().drawImage( backBuffer, 0, 0, null);
                CanvasUtils.postRenderImageTreatment( backBuffer );
            }
            // TODO: foreign canvas
        }
    }

    //----------------------------------------------------------------------------------

    //
    // XIntegerBitmap implementation
    // =============================
    //

    public synchronized byte[] getData( IntegerBitmapLayout[] bitmapLayout,
                                        IntegerRectangle2D    rect )
    {
        int [] pixelData = bitmap.getRGB( rect.X1, rect.Y1, rect.X2 - rect.X1, rect.Y1 - rect.Y2, null, 0, 0 );

        return CanvasUtils.int2byte( pixelData );
    }

    //----------------------------------------------------------------------------------

    public synchronized void setData( byte[] data, IntegerBitmapLayout bitmapLayout, com.sun.star.geometry.IntegerRectangle2D rect )
    {
        int [] pixelData = CanvasUtils.byte2int( data );
        bitmap.setRGB( rect.X1, rect.Y1, rect.X2 - rect.X1, rect.Y2 - rect.Y1, pixelData, 0, bitmap.getWidth() );
    }

    //----------------------------------------------------------------------------------

    public synchronized void setPixel( byte[] color, IntegerBitmapLayout bitmapLayout, com.sun.star.geometry.IntegerPoint2D pos )
    {
        if( color.length != 4 )
            CanvasUtils.printLog( "CanvasBitmap.setPixel: Wrong color format" );

        int pixel = color[0] + (color[1] + (color[2] + color[3]*256)*256)*256;
        bitmap.setRGB( pos.X, pos.Y, pixel );
    }

    //----------------------------------------------------------------------------------

    public synchronized byte[] getPixel( IntegerBitmapLayout[]  bitmapLayout,
                                         IntegerPoint2D         pos )
    {
        int pixel = bitmap.getRGB( pos.X, pos.Y );

        byte[] res = new byte[4];
        res[0] = (byte)(pixel & 255);
        res[1] = (byte)((pixel/256) & 255);
        res[2] = (byte)((pixel/256/256) & 255);
        res[3] = (byte)((pixel/256/256/256) & 255);

        return res;
    }

    //----------------------------------------------------------------------------------

    public synchronized XBitmapPalette getPalette()
    {
        return null;
    }

    //----------------------------------------------------------------------------------

    public synchronized IntegerBitmapLayout getMemoryLayout()
    {
        // TODO: finish that one
        IntegerBitmapLayout layout = new IntegerBitmapLayout();

        layout.ScanLines = bitmap.getWidth();
        layout.ScanLineBytes = bitmap.getWidth()*4;
        layout.ScanLineStride = layout.ScanLineBytes;
        layout.Format = 0;
        layout.NumComponents = 4;
        layout.ComponentMasks = null;
        layout.Endianness = 0;
        layout.IsMsbFirst = false;

        return layout;
    }

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //

    private static final String s_implName = "XIntegerBitmap.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.IntegerBitmap";

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
}
