/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasUtils.java,v $
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
import java.awt.image.*;
import java.awt.geom.*;

public class CanvasUtils
{
    //
    // Canvas utilities
    // ================
    //
    public static java.awt.geom.AffineTransform makeTransform( AffineMatrix2D ooTransform )
    {
        return new AffineTransform( ooTransform.m00,
                                    ooTransform.m10,
                                    ooTransform.m01,
                                    ooTransform.m11,
                                    ooTransform.m02,
                                    ooTransform.m12 );
    }

    public static AffineMatrix2D makeAffineMatrix2D( java.awt.geom.AffineTransform transform )
    {
        double[] matrix = new double[6];
        transform.getMatrix( matrix );

        return new AffineMatrix2D( matrix[0], matrix[2], matrix[4],
                                   matrix[1], matrix[3], matrix[5] );
    }

    public static void initGraphics( Graphics2D graphics )
    {
        if( graphics != null )
        {
            java.awt.RenderingHints hints = new java.awt.RenderingHints(null);
            boolean hq = true;

            if( hq )
            {
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_FRACTIONALMETRICS,
                                                        java.awt.RenderingHints.VALUE_FRACTIONALMETRICS_ON ) );
//                 hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_ALPHA_INTERPOLATION,
//                                                         java.awt.RenderingHints.VALUE_ALPHA_INTERPOLATION_QUALITY) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_ALPHA_INTERPOLATION,
                                                        java.awt.RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED) );
//                 hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_INTERPOLATION,
//                                                         java.awt.RenderingHints.VALUE_INTERPOLATION_BICUBIC) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_INTERPOLATION,
                                                        java.awt.RenderingHints.VALUE_INTERPOLATION_BILINEAR) );
//                 hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_RENDERING,
//                                                         java.awt.RenderingHints.VALUE_RENDER_QUALITY) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_RENDERING,
                                                        java.awt.RenderingHints.VALUE_RENDER_SPEED) );
//                 hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_STROKE_CONTROL,
//                                                         java.awt.RenderingHints.VALUE_STROKE_NORMALIZE) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_STROKE_CONTROL,
                                                        java.awt.RenderingHints.VALUE_STROKE_DEFAULT) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_ANTIALIASING,
                                                        java.awt.RenderingHints.VALUE_ANTIALIAS_ON) );
            }
            else
            {
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_ALPHA_INTERPOLATION,
                                                        java.awt.RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_INTERPOLATION,
                                                        java.awt.RenderingHints.VALUE_INTERPOLATION_BILINEAR) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_RENDERING,
                                                        java.awt.RenderingHints.VALUE_RENDER_SPEED) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_STROKE_CONTROL,
                                                        java.awt.RenderingHints.VALUE_STROKE_DEFAULT) );
                hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_ANTIALIASING,
                                                        java.awt.RenderingHints.VALUE_ANTIALIAS_OFF) );
            }

            // the least common denominator standard
            hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_FRACTIONALMETRICS,
                                                    java.awt.RenderingHints.VALUE_FRACTIONALMETRICS_ON) );
            hints.add( new java.awt.RenderingHints( java.awt.RenderingHints.KEY_TEXT_ANTIALIASING,
                                                    java.awt.RenderingHints.VALUE_TEXT_ANTIALIAS_ON) );

            graphics.setRenderingHints( hints );
        }
    }

    //----------------------------------------------------------------------------------

    public static java.awt.geom.GeneralPath makeGenPathFromBezierPoints( RealBezierSegment2D [][] points )
    {
        java.awt.geom.GeneralPath path = new java.awt.geom.GeneralPath();

        // extract every polygon into GeneralPath object
        for( int i=0; i<points.length; ++i )
        {
            if( points[i].length > 0 )
                path.moveTo((float) points[i][0].Px, (float) points[i][0].Py);

            for( int j=1; j<points[i].length; ++j )
            {
                CanvasUtils.printLog( "makeGenPathFromBezierPoints: point added." );
                path.curveTo((float)(points[i][j-1].C1x), (float)(points[i][j-1].C1y),
                             (float)(points[i][j-1].C2x), (float)(points[i][j-1].C2y),
                             (float) points[i][j].Px, (float) points[i][j].Py );
            }

            // TODO: closePath?
        }

        return path;
    }

    public static java.awt.geom.GeneralPath makeGenPathFromBezierPoly( com.sun.star.rendering.XBezierPolyPolygon2D poly )
    {
        try
        {
            com.sun.star.geometry.RealBezierSegment2D [][] points = poly.getBezierSegments(0,-1,0,-1);

            return makeGenPathFromBezierPoints( points );
        }
        catch( com.sun.star.lang.IndexOutOfBoundsException e )
        {
        }

        return new java.awt.geom.GeneralPath();
    }

    public static java.awt.geom.GeneralPath makeGenPathFromLinePoints( RealPoint2D [][] points )
    {
        java.awt.geom.GeneralPath path = new java.awt.geom.GeneralPath();

        // extract every polygon into GeneralPath object
        for( int i=0; i<points.length; ++i )
        {
            if( points[i].length > 0 )
                path.moveTo((float) points[i][0].X, (float) points[i][0].Y);

            for( int j=1; j<points[i].length; ++j )
            {
                CanvasUtils.printLog( "makeGenPathFromLinePoints: point (" +
                                      points[i][j].X + "," + points[i][j].Y + ") added." );
                path.lineTo((float) points[i][j].X, (float) points[i][j].Y );
            }

            // TODO: closePath?
        }

        return path;
    }

    public static java.awt.geom.GeneralPath makeGenPathFromLinePoly( com.sun.star.rendering.XLinePolyPolygon2D poly )
    {
        try
        {
            com.sun.star.geometry.RealPoint2D [][] points = poly.getPoints(0,-1,0,-1);

            return makeGenPathFromLinePoints( points );
        }
        catch( com.sun.star.lang.IndexOutOfBoundsException e )
        {
        }

        return new java.awt.geom.GeneralPath();
    }

    public static java.awt.geom.GeneralPath makeGeneralPath( com.sun.star.rendering.XPolyPolygon2D poly )
    {
        if( poly instanceof BezierPolyPolygon )
        {
            CanvasUtils.printLog( "makeGeneralPath: bezier impl used." );
            return ((BezierPolyPolygon)poly).getJavaPath();
        }

        if( poly instanceof LinePolyPolygon )
        {
            CanvasUtils.printLog( "makeGeneralPath: line impl used." );
            return ((LinePolyPolygon)poly).getJavaPath();
        }

        XBezierPolyPolygon2D bezierPoly = (XBezierPolyPolygon2D) UnoRuntime.queryInterface(XBezierPolyPolygon2D.class, poly);

        if( bezierPoly != null )
        {
            // extract polygon data. Prefer bezier interface, because
            // that's the more high-level data.
            return makeGenPathFromBezierPoly( bezierPoly );
        }

        XLinePolyPolygon2D linePoly = (XLinePolyPolygon2D) UnoRuntime.queryInterface(XLinePolyPolygon2D.class, poly);

        if( linePoly != null )
        {
            // extract polygon data. Fallback to line polygon, if no
            // curves are available.
            return makeGenPathFromLinePoly( linePoly );
        }

        // Only opaque general interface. No chance to get to the
        // data. Empty path, then
        CanvasUtils.printLog( "makeGeneralPath: Cannot access polygon data, given interface has class" + poly.getClass().getName() );
        return new GeneralPath();
    }

    public static java.awt.image.BufferedImage getBufferedImage( com.sun.star.rendering.XBitmap bitmap )
    {
        if( bitmap instanceof CanvasBitmap )
        {
            CanvasUtils.printLog( "getBufferedImage: CanvasBitmap impl used." );
            return ((CanvasBitmap)bitmap).getBufferedImage();
        }

        XIntegerBitmap integerBitmap = (XIntegerBitmap) UnoRuntime.queryInterface(XIntegerBitmap.class, bitmap);

        if( integerBitmap != null )
        {
            // extract bitmap data. TODO.
            return null;
        }

        // check other types. TODO.
        return null;
    }

    public static byte [] int2byte( int [] input )
    {
        byte [] output = new byte[4*input.length];

        int i, j;
        for( i=0, j=0; i<input.length; ++i )
        {
            output[j]   = (byte)(input[i] & 255);
            output[j+1] = (byte)((input[i]/256) & 255);
            output[j+2] = (byte)((input[i]/256/256) & 255);
            output[j+3] = (byte)((input[i]/256/256/256) & 255);
            j += 4;
        }

        return output;
    }

    public static int [] byte2int( byte [] input )
    {
        int [] output = new int[(input.length+3)/4];

        int i, j;
        for( i=0,j=0; j<output.length; ++j )
        {
            output[j] = input[i] + (input[i+1] + (input[i+2] + input[i+3]*256)*256)*256;
            i += 4;
        }

        return output;
    }

    public static int javaRuleFromCompositeOp( byte compositeOp )
    {
        // TODO: Finish mapping of Canvas and Java compositing magics
        int rule = java.awt.AlphaComposite.SRC_OVER;
        switch( compositeOp )
        {
            case com.sun.star.rendering.CompositeOperation.CLEAR:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: clear selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.SOURCE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: src selected" );
                rule = java.awt.AlphaComposite.SRC;
                break;

            case com.sun.star.rendering.CompositeOperation.DESTINATION:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: dst selected" );
                rule = java.awt.AlphaComposite.DST;
                break;

            case com.sun.star.rendering.CompositeOperation.OVER:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: over selected" );
                rule = java.awt.AlphaComposite.SRC_OVER;
                break;

            case com.sun.star.rendering.CompositeOperation.UNDER:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: under selected" );
                rule = java.awt.AlphaComposite.DST_OVER;
                break;

            case com.sun.star.rendering.CompositeOperation.INSIDE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: inside selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.INSIDE_REVERSE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: inReverse selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.OUTSIDE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: outside selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.OUTSIDE_REVERSE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: outReverse selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.XOR:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: xor selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.ADD:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: add selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            case com.sun.star.rendering.CompositeOperation.SATURATE:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: saturate selected" );
                rule = java.awt.AlphaComposite.CLEAR;
                break;

            default:
                CanvasUtils.printLog( "javaRuleFromCompositeOp: Unexpected compositing rule" );
                break;
        }

        return rule;
    }

    public static java.awt.AlphaComposite makeAlphaComposite( byte compositeOp )
    {
        return java.awt.AlphaComposite.getInstance( javaRuleFromCompositeOp( compositeOp ) );
    }

    public static java.awt.AlphaComposite makeAlphaCompositeAlpha( byte compositeOp, double alpha )
    {
        return java.awt.AlphaComposite.getInstance( javaRuleFromCompositeOp( compositeOp ), (float)alpha );
    }

    // when given to setupGraphicsState, makes that method to also
    // setup the Paint with the color specified in the render state.
    public static final byte    alsoSetupPaint=0;

    // when given to setupGraphicsState, makes that method to _not_
    // setup the Paint with the color specified in the render state.
    public static final byte    dontSetupPaint=1;

    public static java.awt.geom.AffineTransform ViewConcatRenderTransform(  ViewState   viewState,
                                                                            RenderState renderState )
    {
        // calculate overall affine transform
        AffineTransform transform = makeTransform( viewState.AffineTransform );
        transform.concatenate( makeTransform( renderState.AffineTransform ) );

        printTransform( transform, "ViewConcatRenderTransform" );

        return transform;
    }

    public static void setupGraphicsState( java.awt.Graphics2D  graphics,
                                           ViewState            viewState,
                                           RenderState          renderState,
                                           byte                 paintTouchMode )
    {
        // calculate overall affine transform
        graphics.setTransform( ViewConcatRenderTransform(viewState, renderState ) );

        // setup overall clip polyPolygon
        if( viewState.Clip != null )
        {
            Area clipArea = new Area( makeGeneralPath( viewState.Clip ) );

            if( renderState.Clip != null )
                clipArea.intersect( new Area( makeGeneralPath( renderState.Clip ) ) );

            graphics.setClip( clipArea );
        }
        else if( renderState.Clip != null )
        {
            Area clipArea = new Area( makeGeneralPath( renderState.Clip ) );
            graphics.setClip( clipArea );
        }
        else
        {
            // TODO: HACK! Use true visible area here!
            graphics.setClip( new java.awt.Rectangle(-1000000,-1000000,2000000,2000000) );
        }

        // setup current output color
        // TODO: Complete color handling here
        if( paintTouchMode == alsoSetupPaint )
        {
            switch( renderState.DeviceColor.length )
            {
                case 3:
                    CanvasUtils.printLog( "setupGraphicsState: Color(" +
                                          renderState.DeviceColor[0] + "," +
                                          renderState.DeviceColor[1] + "," +
                                          renderState.DeviceColor[2] + ") set." );
                    graphics.setColor( new Color( (float)renderState.DeviceColor[0],
                                                  (float)renderState.DeviceColor[1],
                                                  (float)renderState.DeviceColor[2] ) );
                    break;

                case 4:
                    CanvasUtils.printLog( "setupGraphicsState: Color(" +
                                          renderState.DeviceColor[0] + "," +
                                          renderState.DeviceColor[1] + "," +
                                          renderState.DeviceColor[2] + "," +
                                          renderState.DeviceColor[3] + ") set." );
                    graphics.setColor( new Color( (float)renderState.DeviceColor[0],
                                                  (float)renderState.DeviceColor[1],
                                                  (float)renderState.DeviceColor[2],
                                                  (float)renderState.DeviceColor[3] ) );
                    break;

                default:
                    CanvasUtils.printLog( "setupGraphicsState: unexpected number of " +
                                          renderState.DeviceColor.length + " color components!" );
                    break;
            }
        }

        // setup current composite mode
        graphics.setComposite( makeAlphaComposite( renderState.CompositeOperation ) );
    }

    public static void applyStrokeAttributes( java.awt.Graphics2D   graphics,
                                              StrokeAttributes      attributes )
    {
        int cap = java.awt.BasicStroke.CAP_BUTT;

        if( attributes.StartCapType != attributes.EndCapType )
            CanvasUtils.printLog( "applyStrokeAttributes: different start and end caps are not yet supported!" );

        if( attributes.LineArray.length != 0 )
            CanvasUtils.printLog( "applyStrokeAttributes: multi-strokes are not yet supported!" );

        if( attributes.StartCapType == PathCapType.BUTT  )
            cap = java.awt.BasicStroke.CAP_BUTT;
        else if( attributes.StartCapType == PathCapType.ROUND )
            cap = java.awt.BasicStroke.CAP_ROUND;
        else if( attributes.StartCapType == PathCapType.SQUARE )
            cap = java.awt.BasicStroke.CAP_SQUARE;

        int join = java.awt.BasicStroke.JOIN_MITER;

        if( attributes.JoinType == PathJoinType.MITER )
            cap = java.awt.BasicStroke.JOIN_MITER;
        else if( attributes.JoinType == PathJoinType.ROUND )
            cap = java.awt.BasicStroke.JOIN_ROUND;
        else if( attributes.JoinType == PathJoinType.BEVEL )
            cap = java.awt.BasicStroke.JOIN_BEVEL;
        else
            CanvasUtils.printLog( "applyStrokeAttributes: current join type not yet supported!" );

        float [] dashArray = null;

        if( attributes.DashArray.length != 0 )
        {
            dashArray = new float [attributes.DashArray.length];

            for( int i=0; i<attributes.DashArray.length; ++i )
                dashArray[i] = (float)attributes.DashArray[i];
        }

        graphics.setStroke( new java.awt.BasicStroke( (float)attributes.StrokeWidth,
                                                      cap,
                                                      join,
                                                      (float)attributes.MiterLimit,
                                                      dashArray,
                                                      0) );
    }

    public static void setupGraphicsFont( java.awt.Graphics2D                           graphics,
                                          ViewState                                     viewState,
                                          RenderState                                   renderState,
                                          com.sun.star.rendering.XCanvasFont        xFont           )
    {
        if( xFont instanceof CanvasFont )
        {
            CanvasUtils.printLog( "setupGraphicsFont: font impl used." );
            graphics.setFont( ((CanvasFont)xFont).getFont() );
        }
        else
        {
            CanvasUtils.printLog( "setupGraphicsFont: creating Java font anew." );
            CanvasFont canvasFont;
            canvasFont = new CanvasFont( xFont.getFontRequest(), null );
            graphics.setFont( canvasFont.getFont() );
        }
    }

    static java.awt.geom.Rectangle2D.Double calcTransformedRectBounds( java.awt.geom.Rectangle2D.Double aRect,
                                                                       AffineTransform                  aTransform )
    {
        // transform rect by given transformation
        java.awt.geom.Point2D.Double aPointTopLeft = new java.awt.geom.Point2D.Double(aRect.x, aRect.y);
        aTransform.transform(aPointTopLeft, aPointTopLeft);

        java.awt.geom.Point2D.Double aPointTopRight = new java.awt.geom.Point2D.Double(aRect.x + aRect.width,
                                                                                       aRect.y);
        aTransform.transform(aPointTopRight, aPointTopRight);

        java.awt.geom.Point2D.Double aPointBottomLeft = new java.awt.geom.Point2D.Double(aRect.x,
                                                                                         aRect.y + aRect.height);
        aTransform.transform(aPointBottomLeft, aPointBottomLeft);

        java.awt.geom.Point2D.Double aPointBottomRight = new java.awt.geom.Point2D.Double(aRect.x + aRect.width,
                                                                                          aRect.y + aRect.height);
        aTransform.transform(aPointBottomRight, aPointBottomRight);

        // calc bounding rect of those four points
        java.awt.geom.Point2D.Double aResTopLeft = new java.awt.geom.Point2D.Double( Math.min(aPointTopLeft.x,
                                                                                              Math.min(aPointTopRight.x,
                                                                                                       Math.min(aPointBottomLeft.x,aPointBottomRight.x))),
                                                                                     Math.min(aPointTopLeft.y,
                                                                                              Math.min(aPointTopRight.y,
                                                                                                       Math.min(aPointBottomLeft.y,aPointBottomRight.y))) );

        java.awt.geom.Point2D.Double aResBottomRight = new java.awt.geom.Point2D.Double( Math.max(aPointTopLeft.x,
                                                                                                  Math.max(aPointTopRight.x,
                                                                                                           Math.max(aPointBottomLeft.x,aPointBottomRight.x))),
                                                                                         Math.max(aPointTopLeft.y,
                                                                                                  Math.max(aPointTopRight.y,
                                                                                                           Math.max(aPointBottomLeft.y,aPointBottomRight.y))) );
        return new java.awt.geom.Rectangle2D.Double( aResTopLeft.x, aResTopLeft.y,
                                                     aResBottomRight.x - aResTopLeft.x,
                                                     aResBottomRight.y - aResTopLeft.y );
    }

    // Create a corrected view transformation out of the give one,
    // which ensures that the rectangle given by (0,0) and
    // attributes.untransformedSize is mapped with its left,top corner
    // to (0,0) again. This is required to properly render sprite
    // animations to buffer bitmaps.
    public static ViewState createAnimationViewState( ViewState             inputViewState,
                                                      AnimationAttributes   attributes      )
    {
        // TODO: Properly respect clip here. Might have to be transformed, too.

        AffineTransform aViewTransform = makeTransform( inputViewState.AffineTransform );

        // transform Rect(0,0,attributes.untransformedSize) by
        // viewTransform
        java.awt.geom.Rectangle2D.Double aTransformedRect =
            calcTransformedRectBounds( new java.awt.geom.Rectangle2D.Double(0.0, 0.0,
                                                                            attributes.UntransformedSize.Width,
                                                                            attributes.UntransformedSize.Height),
                                       aViewTransform );

        printTransform( aViewTransform, "createAnimationViewState" );

        CanvasUtils.printLog( "createAnimationViewState: transformed origin is: (" + aTransformedRect.x + ", " + aTransformedRect.y + ")" );

        // now move resulting left,top point of bounds to (0,0)
        AffineTransform animationViewTransform = new AffineTransform();
        animationViewTransform.translate( -aTransformedRect.x, -aTransformedRect.y );
        animationViewTransform.concatenate( aViewTransform );

        printTransform( animationViewTransform, "createAnimationViewState" );

        return new ViewState( makeAffineMatrix2D( animationViewTransform ), inputViewState.Clip );
    }

    public static void postRenderImageTreatment( Image buffer )
    {
        // TODO: This is specific to Sun's JREs 1.4 and upwards. Make this more portable
        buffer.flush(); // as long as we force images to VRAM,
                        // we need to flush them afterwards, to
                        // avoid eating up all VRAM.
    }

    public static void printTransform( AffineTransform  transform,
                                       String           stringPrefix )
    {
        CanvasUtils.printLog( stringPrefix + ": Transform is" );
        double [] matrix = new double[6];
        transform.getMatrix(matrix);
        int i;
        for( i=0; i<6; ++i )
            System.err.print( matrix[i] + ", " );
        CanvasUtils.printLog( "" );
    }

    public static void preCondition( boolean    bCondition,
                                     String     methodName  )
    {
        if( !bCondition )
            printLog("Precondition violated: " + methodName);
    }

    public static void printLog( String s )
    {
        System.err.println( s );
    }
}
