/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasBase.java,v $
 * $Revision: 1.8 $
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
import com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.geom.*;

public abstract class CanvasBase
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.rendering.XCanvas
{
    // to be overridden
    public abstract Graphics2D getGraphics();

    //----------------------------------------------------------------------------------

    //
    // XCanvas interface
    // =================
    //
    public synchronized void clear()
    {
        Graphics2D graphics = getGraphics();
        // TODO(F3): retrieve true dimensions of the Graphics
        graphics.clearRect(0,0,1000,1000);
    }

    public synchronized void drawPoint( RealPoint2D aPoint,
                                        ViewState   viewState,
                                        RenderState renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );

        // calculate the domain value for a single device pixel. we're
        // using delta mapping here, to avoid later subtraction of two
        // mapped values (as we really only need a transformed size,
        // not a transformed point).
        AffineTransform transform = graphics.getTransform();
        AffineTransform inverse;
        try
        {
            inverse = transform.createInverse();
        }
        catch( NoninvertibleTransformException e )
        {
            // transformation not invertible. Nothing to render then.
            return;
        }

        java.awt.geom.Point2D.Double pointSize = new java.awt.geom.Point2D.Double(1.0,1.0);
        java.awt.geom.Point2D.Double domainPointSize = new java.awt.geom.Point2D.Double();
        inverse.deltaTransform( pointSize, domainPointSize );

        // render a circle one device pixel wide
        Ellipse2D.Double ellipse = new Ellipse2D.Double(aPoint.X, aPoint.Y, domainPointSize.x, domainPointSize.y);

        // render, at last
        graphics.fill( ellipse );

        CanvasUtils.printLog( "XCanvas: drawPoint called" );
    }

    public synchronized void drawLine( RealPoint2D  aStartPoint,
                                       RealPoint2D  aEndPoint,
                                       ViewState    viewState,
                                       RenderState  renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        graphics.setStroke( new BasicStroke() );

        // setup line object
        Line2D.Double line = new Line2D.Double(aStartPoint.X, aStartPoint.Y, aEndPoint.X, aEndPoint.Y);

        // render, at last
        graphics.draw( line );

        CanvasUtils.printLog( "XCanvas: drawLine called" );
    }

    public synchronized void drawBezier( RealBezierSegment2D    aBezierSegment,
                                         RealPoint2D            aEndPoint,
                                         ViewState              viewState,
                                         RenderState            renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        graphics.setStroke( new BasicStroke() );

        // setup bezier object
        CubicCurve2D.Double curve = new CubicCurve2D.Double(aBezierSegment.Px, aBezierSegment.Py,
                                                            aBezierSegment.C1x, aBezierSegment.C1y,
                                                            aBezierSegment.C2x, aBezierSegment.C2y,
                                                            aEndPoint.X, aEndPoint.Y);

        // render, at last
        graphics.draw( curve );

        CanvasUtils.printLog( "XCanvas: drawbezier called" );
    }

    public synchronized XCachedPrimitive drawPolyPolygon( XPolyPolygon2D    xPolyPolygon,
                                                          ViewState         viewState,
                                                          RenderState       renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.drawPolyPolygon() called" );

        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        graphics.setStroke( new BasicStroke() );

        // render the polygon
        // TODO: maybe use Graphics.drawPolyline here!
        graphics.draw( CanvasUtils.makeGeneralPath(xPolyPolygon) );

        CanvasUtils.printLog( "XCanvas: drawPolyPolygon called" );

        return null;
    }

    public synchronized XCachedPrimitive strokePolyPolygon( XPolyPolygon2D      xPolyPolygon,
                                                            ViewState           viewState,
                                                            RenderState         renderState,
                                                            StrokeAttributes    strokeAttributes ) throws com.sun.star.lang.IllegalArgumentException
    {
        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        CanvasUtils.applyStrokeAttributes( graphics, strokeAttributes );

        // stroke the polygon
        graphics.draw( CanvasUtils.makeGeneralPath(xPolyPolygon) );

        CanvasUtils.printLog( "XCanvas: strokePolyPolygon called" );

        return null;
    }

    public synchronized XCachedPrimitive strokeTexturedPolyPolygon( XPolyPolygon2D      xPolyPolygon,
                                                                    ViewState           viewState,
                                                                    RenderState         renderState,
                                                                    Texture[]           textures,
                                                                    StrokeAttributes    strokeAttributes ) throws com.sun.star.lang.IllegalArgumentException, VolatileContentDestroyedException
    {
        return null;
    }

    public synchronized XCachedPrimitive strokeTextureMappedPolyPolygon( XPolyPolygon2D                             xPolyPolygon,
                                                                         ViewState                                  viewState,
                                                                         RenderState                                renderState,
                                                                         Texture[]                                  textures,
                                                                         com.sun.star.geometry.XMapping2D   xMapping,
                                                                         StrokeAttributes                           strokeAttributes ) throws com.sun.star.lang.IllegalArgumentException, VolatileContentDestroyedException
    {
        return null;
    }

    public synchronized XPolyPolygon2D queryStrokeShapes( com.sun.star.rendering.XPolyPolygon2D   xPolyPolygon,
                                                          com.sun.star.rendering.ViewState        viewState,
                                                          com.sun.star.rendering.RenderState      renderState,
                                                          com.sun.star.rendering.StrokeAttributes strokeAttributes ) throws com.sun.star.lang.IllegalArgumentException
    {
        return null;
    }

    public synchronized XCachedPrimitive fillPolyPolygon( com.sun.star.rendering.XPolyPolygon2D xPolyPolygon,
                                                          com.sun.star.rendering.ViewState      viewState,
                                                          com.sun.star.rendering.RenderState    renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.fillPolyPolygon() called" );

        // cache
        Graphics2D graphics = getGraphics();

        // initialize the Graphics2D
        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );

        // fill the polygon
        graphics.fill( CanvasUtils.makeGeneralPath(xPolyPolygon) );

        CanvasUtils.printLog( "XCanvas: fillPolyPolygon called" );

        return null;
    }

    public synchronized XCachedPrimitive fillTexturedPolyPolygon( com.sun.star.rendering.XPolyPolygon2D    xPolyPolygon,
                                                                  com.sun.star.rendering.ViewState         viewState,
                                                                  com.sun.star.rendering.RenderState       renderState,
                                                                  com.sun.star.rendering.Texture []        textures ) throws com.sun.star.lang.IllegalArgumentException
    {
        return null;
    }

    public synchronized XCachedPrimitive fillTextureMappedPolyPolygon( XPolyPolygon2D                           xPolyPolygon,
                                                                       ViewState                                viewState,
                                                                       RenderState                              renderState,
                                                                       Texture[]                                textures,
                                                                       com.sun.star.geometry.XMapping2D     xMapping ) throws com.sun.star.lang.IllegalArgumentException, VolatileContentDestroyedException
    {
        return null;
    }

    public synchronized XCanvasFont createFont( FontRequest fontRequest, com.sun.star.beans.PropertyValue[] extraFontProperties, com.sun.star.geometry.Matrix2D fontMatrix ) throws com.sun.star.lang.IllegalArgumentException
    {
        // TODO: support extra arguments
        return new CanvasFont( fontRequest, this );
    }

    public FontInfo[] queryAvailableFonts( FontInfo aFilter, com.sun.star.beans.PropertyValue[] aFontProperties ) throws com.sun.star.lang.IllegalArgumentException
    {
        // TODO
        return null;
    }

    public XCachedPrimitive drawText( StringContext text, XCanvasFont xFont, ViewState viewState, RenderState renderState, byte textDirection ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.drawText() called" );

        // cache
        Graphics2D graphics = getGraphics();

        CanvasUtils.printLog( "XCanvas: drawText called" );

        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        CanvasUtils.setupGraphicsFont( graphics, viewState, renderState, xFont );

        CanvasUtils.printLog( "XCanvas: drawText rendering \""+ text.Text.substring(text.StartPosition, text.StartPosition+text.Length) + "\"" );

        graphics.drawString( text.Text.substring(text.StartPosition, text.StartPosition+text.Length), (float)0.0, (float)0.0 );
        return null;
    }

    public XCachedPrimitive drawTextLayout( XTextLayout layoutetText, ViewState viewState, RenderState renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.drawOffsettedText() called" );

        // cache
        Graphics2D graphics = getGraphics();

        CanvasUtils.printLog( "XCanvas: drawOffsettedText called" );

        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );
        CanvasUtils.setupGraphicsFont( graphics, viewState, renderState, layoutetText.getFont() );

        CanvasUtils.printLog( "XCanvas: drawOffsettedText canvas setup done" );

        if( layoutetText instanceof TextLayout )
        {
            ((TextLayout)layoutetText).draw( graphics );
        }
        else
        {
            CanvasUtils.printLog( "drawTextLayout: mismatching TextLayout object." );
            throw new com.sun.star.lang.IllegalArgumentException();
        }

        return null;
    }

    public synchronized XCachedPrimitive drawBitmap( com.sun.star.rendering.XBitmap     xBitmap,
                                                     com.sun.star.rendering.ViewState   viewState,
                                                     com.sun.star.rendering.RenderState     renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.drawBitmap() called" );

        // cache
        Graphics2D graphics = getGraphics();

        CanvasUtils.setupGraphicsState( graphics, viewState, renderState, CanvasUtils.alsoSetupPaint );

        java.awt.image.BufferedImage bitmap = CanvasUtils.getBufferedImage( xBitmap );

        if( !graphics.drawImage(bitmap, 0, 0, null) )
            CanvasUtils.printLog( "CanvasBase.drawBitmap: image paint incomplete" );

        CanvasUtils.postRenderImageTreatment( bitmap );

        return null;
    }

    public synchronized XCachedPrimitive drawBitmapModulated( com.sun.star.rendering.XBitmap    xBitmap,
                                                              com.sun.star.rendering.ViewState  viewState,
                                                              com.sun.star.rendering.RenderState renderState ) throws com.sun.star.lang.IllegalArgumentException
    {
        CanvasUtils.printLog( "CanvasBase.drawBitmapModulated() called" );

        // TODO(F3): Implement channel modulation
        return drawBitmap(xBitmap,
                          viewState,
                          renderState);
    }

    public synchronized XGraphicDevice getDevice()
    {
        CanvasUtils.printLog( "CanvasBase.getDevice() called" );
        return new CanvasGraphicDevice( getGraphics() );
    }
}
