/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpriteRep.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:12:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
import java.awt.geom.*;

public class SpriteRep
{
    private java.awt.image.BufferedImage    buffer;
    private CanvasBitmap                    canvasBitmap;
    private double                          alpha;
    private java.awt.geom.Point2D.Double    outputPosition;
    private boolean                         bufferOwned;

    //----------------------------------------------------------------------------------

    // TODO: Everything in this class
    // TODO: Implement lifetime control for buffer object, which is shared between SpriteReps
    public SpriteRep()
    {
        CanvasUtils.printLog( "SpriteRep constructor called!" );

        alpha          = 0.0;
        outputPosition = new java.awt.geom.Point2D.Double(0.0,0.0);
        bufferOwned    = true; // the buffer member is our own, and has to be disposed
    }

    public SpriteRep( SpriteRep original )
    {
        CanvasUtils.printLog( "SpriteRep clone constructor called!" );

        alpha          = 0.0;
        outputPosition = new java.awt.geom.Point2D.Double(0.0,0.0);
        cloneBuffer( original );
        bufferOwned    = false; // the buffer member is not our own, and must not be disposed
    }

    //----------------------------------------------------------------------------------

    public synchronized void renderAnimation( XAnimation animation, ViewState viewState, double t )
    {
        if( canvasBitmap != null )
        {
            // clear buffer with all transparent
            Graphics2D bitmapGraphics = canvasBitmap.getGraphics();

            // before that, setup _everything_ we might have changed in CanvasUtils.setupGraphicsState
            bitmapGraphics.setColor( new Color( 0.0f, 0.0f, 0.0f, 1.0f ) );
            bitmapGraphics.setComposite(
                java.awt.AlphaComposite.getInstance(java.awt.AlphaComposite.CLEAR));
            bitmapGraphics.setTransform( new AffineTransform() );
            bitmapGraphics.setClip( new java.awt.Rectangle(0,0,buffer.getWidth(),buffer.getHeight()) );
            bitmapGraphics.fillRect(0,0,buffer.getWidth(),buffer.getHeight());

            try
            {
                // now push the animation at time instance t into the
                // virginal graphics
                animation.render(canvasBitmap, viewState, t);
            }
            catch( com.sun.star.lang.IllegalArgumentException e )
            {
                CanvasUtils.printLog( "Cannot create EmbeddedFrame within VCL window hierarchy!" );
            }
        }
    }

    public synchronized void setSpriteAlpha( double _alpha )
    {
        CanvasUtils.printLog("SpriteRep.setSpriteAlpha called with alpha=" + alpha);
        alpha = _alpha;
    }

    public synchronized void moveSprite( java.awt.geom.Point2D.Double aNewPos )
    {
        outputPosition = aNewPos;
        CanvasUtils.printLog( "SpriteRep.moveSprite: moving to (" + outputPosition.x + ", " + outputPosition.y + ")" );
    }

    public synchronized void redraw( Graphics2D output )
    {
        if( buffer != null )
        {
            CanvasUtils.printLog( "SpriteRep.redraw: compositing with alpha=" + alpha );
            output.setComposite( java.awt.AlphaComposite.getInstance(java.awt.AlphaComposite.SRC_OVER, (float)alpha) );

            output.drawImage( buffer,
                              (int)(outputPosition.getX() + .5),
                              (int)(outputPosition.getY() + .5),
                              null );

            //CanvasUtils.postRenderImageTreatment( buffer );

            CanvasUtils.printLog( "SpriteRep.redraw called, output rect is (" +
                                  outputPosition.getX() + ", " +
                                  outputPosition.getY() + ", " +
                                  buffer.getWidth() + ", " +
                                  buffer.getHeight() + ")" );
        }
    }

    public synchronized void setupBuffer( java.awt.Graphics2D graphics, int width, int height )
    {
        if( canvasBitmap != null )
            canvasBitmap.dispose();

        if( buffer != null )
            buffer.flush();

        buffer         = graphics.getDeviceConfiguration().createCompatibleImage(Math.max(1,width),
                                                                                 Math.max(1,height),
                                                                                 Transparency.TRANSLUCENT);
        canvasBitmap   = new CanvasBitmap( buffer );
        CanvasUtils.initGraphics( canvasBitmap.getGraphics() );

        CanvasUtils.printLog( "SpriteRep.setupBuffer called, with dimensions (" + width + ", " + height + ")" );
    }

    public synchronized void cloneBuffer( SpriteRep original )
    {
        buffer         = original.buffer;
    }

    public synchronized com.sun.star.rendering.XCanvas getContentCanvas()
    {
        CanvasUtils.printLog( "SpriteRep.getContentCanvas() called" );

        Graphics2D graphics = canvasBitmap.getGraphics();
        graphics.setTransform( new AffineTransform() );
        graphics.setComposite( AlphaComposite.getInstance(AlphaComposite.CLEAR));
        graphics.fillRect( 0,0,buffer.getWidth(),buffer.getHeight() );

        return canvasBitmap;
    }

    public void dispose()
    {
        if( canvasBitmap != null  )
            canvasBitmap.dispose();

        if( buffer != null && bufferOwned )
            buffer.flush();

        canvasBitmap = null;
        buffer = null;
    }
}
