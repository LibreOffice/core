/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JavaCanvas.java,v $
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
import java.awt.image.*;
import java.awt.geom.*;

public class JavaCanvas
    extends CanvasBase
    implements com.sun.star.awt.XWindow,
               com.sun.star.rendering.XSpriteCanvas,
               com.sun.star.rendering.XIntegerBitmap,
               com.sun.star.lang.XServiceInfo,
               com.sun.star.lang.XInitialization
{
    private WindowAdapter                   dummyFrame;
    public BackBuffer                       backBuffer;
    private java.awt.image.BufferStrategy   bufferStrategy;

    private java.awt.Font                   fpsFont;
    private long                            lastTime;
    private com.sun.star.awt.Rectangle      boundRect;

    public Graphics2D getGraphics()
    {
        return backBuffer.getGraphics();
    }

    //----------------------------------------------------------------------------------

    //
    // XInitialization
    // ===============
    //
    public void initialize( java.lang.Object[] arguments )
    {
        CanvasUtils.printLog( "JavaCanvas.initialize called!" );

        // Do that as the very first thing. The Java2D internal
        // classes choose their render path at initialization time,
        // thus this must happen before doing _any_ GUI.

        // TODO: Put those flags into javarc/java.ini, we're maybe
        // coming late into a running JVM!

        // now, we're getting slightly system dependent here.
        String os = (String) System.getProperty("os.name");

        CanvasUtils.printLog( "System detected: " + os );

        // tweak some speed knobs...
//         if( os.startsWith("Windows") )
//         {
//             System.setProperty("sun.java2d.translaccel", "true");
//             System.setProperty("sun.java2d.ddforcevram", "true");
//             //System.setProperty("sun.java2d.accthreshold", "0");

//             CanvasUtils.printLog( "Optimizing for Windows" );
//         }
//         else
//         {
//             System.setProperty("sun.java2d.opengl", "true");

//             CanvasUtils.printLog( "Optimizing for Unix" );
//         }

        /* we're initialized with the following array of anys:

           arguments[0] = pointer to VCL window
           arguments[1] = Integer (operating system window handle)
           arguments[2] = com.sun.star.awt.Rectangle (position and size within that OS window)
           arguments[3] = boolean (fullsize window or not)

           We then generate a dummy Java frame with that window as the
           parent, to fake a root window for the Java implementation.
        */
        try
        {
            boundRect = (com.sun.star.awt.Rectangle) arguments[2];

            //boolean isFullScreen = arguments[1];
            boolean isFullScreen = true;
            //AnyConverter.toBoolean( arguments[3] ) );

            // fake a root for Java in VCL window. Pass the flag
            // whether we shall run fullscreen, too.
            dummyFrame = new WindowAdapter( AnyConverter.toInt( arguments[1] ), isFullScreen );

            if( isFullScreen )
            {
                // blow window up to fullscreen. Otherwise, we cannot clear the whole screen,
                // which results in ugly flickering
                Dimension screenSize = dummyFrame.frame.getToolkit().getScreenSize();
                boundRect.X      = 0;
                boundRect.Y      = 0;
                boundRect.Width  = screenSize.width-1;
                boundRect.Height = screenSize.height-1;
            }

            dummyFrame.setPosSize( boundRect.X, boundRect.Y, boundRect.Width, boundRect.Height, (short)0 );
            CanvasUtils.printLog( "Window size: " + boundRect.Width + ", " + boundRect.Height );

            backBuffer = new BackBuffer( (Graphics2D) dummyFrame.frame.getGraphics(),
                                         Math.max(1,boundRect.Width),
                                         Math.max(1,boundRect.Height) );

            // TODO: Maybe delay double buffer init until first sprite creation
            dummyFrame.frame.createBufferStrategy(2);
            bufferStrategy = dummyFrame.frame.getBufferStrategy();

            if( bufferStrategy.getCapabilities().isPageFlipping() )
                CanvasUtils.printLog( "JavaCanvas.initialize double buffering is using page flipping!" );
            else
                CanvasUtils.printLog( "JavaCanvas.initialize double buffering is using blitting!" );

            lastTime = System.currentTimeMillis();
            fpsFont = new java.awt.Font( "Times", Font.PLAIN, 20 );

            CanvasUtils.printLog( "JavaCanvas.initialize finished!" );
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
            CanvasUtils.printLog( "Cannot create EmbeddedFrame within VCL window hierarchy!" );
        }
    }

    //----------------------------------------------------------------------------------

    //
    // XComponent
    // ==========
    //
    public void dispose()
    {
        CanvasUtils.printLog( "JavaCanvas: disposed!" );

        // destroy all active sprites
        java.util.Set entries = activeSprites.entrySet();
        java.util.Iterator iter = entries.iterator();

        while( iter.hasNext() )
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)iter.next();
            if( entry.getValue() != null )
                ((SpriteRep)entry.getValue()).dispose();
        }

        if( bufferStrategy != null )
            bufferStrategy.getDrawGraphics().dispose(); // really necessary?

        if( dummyFrame != null )
            dummyFrame.dispose();

        if( backBuffer != null)
            backBuffer.dispose();

        bufferStrategy = null;
        dummyFrame = null;
        backBuffer = null;

        super.dispose();
    }

    //----------------------------------------------------------------------------------

    public JavaCanvas( XComponentContext xContext )
    {
        CanvasUtils.printLog( "JavaCanvas constructor called!" );
        activeSprites = new java.util.HashMap( 33 );
    }

    //----------------------------------------------------------------------------------

    //
    // XWindow interface
    // =================
    //
    // This is delegated to WindowAdapter!
    //
    public synchronized void setPosSize( int X, int Y, int Width, int Height, short Flags )
    {
        if( dummyFrame != null )
        {
            dummyFrame.setPosSize( X, Y, Width, Height, Flags );

            Width = Math.max(1,Width);
            Height= Math.max(1,Height);

            CanvasUtils.printLog( "JavaCanvas graphics set to " + Width + "," + Height );
            backBuffer.setSize(Width,Height);
        }
    }

    public synchronized com.sun.star.awt.Rectangle getPosSize(  )
    {
        if( dummyFrame != null )
            return dummyFrame.getPosSize();

        return new com.sun.star.awt.Rectangle();
    }

    public synchronized void setVisible( boolean visible )
    {
        if( dummyFrame != null )
            dummyFrame.setVisible( visible );
    }

    public synchronized void setEnable( boolean enable )
    {
        if( dummyFrame != null )
            dummyFrame.setEnable( enable );
    }

    public synchronized void setFocus()
    {
        if( dummyFrame != null )
            dummyFrame.setFocus();
    }

    public synchronized void addWindowListener( XWindowListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addWindowListener( xListener );
    }

    public synchronized void removeWindowListener( XWindowListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removeWindowListener( xListener );
    }

    public synchronized void addFocusListener( XFocusListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addFocusListener( xListener );
    }

    public synchronized void removeFocusListener( XFocusListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removeFocusListener( xListener );
    }

    public synchronized void addKeyListener( XKeyListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addKeyListener( xListener );
    }

    public synchronized void removeKeyListener( XKeyListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removeKeyListener( xListener );
    }

    public synchronized void addMouseListener( XMouseListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addMouseListener( xListener );
    }

    public synchronized void removeMouseListener( XMouseListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removeMouseListener( xListener );
    }

    public synchronized void addMouseMotionListener( XMouseMotionListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addMouseMotionListener( xListener );
    }

    public synchronized void removeMouseMotionListener( XMouseMotionListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removeMouseMotionListener( xListener );
    }

    public synchronized void addPaintListener( XPaintListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.addPaintListener( xListener );
    }

    public synchronized void removePaintListener( XPaintListener xListener )
    {
        if( dummyFrame != null )
            dummyFrame.removePaintListener( xListener );
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

    // a map of SpriteReps, with Sprite object as keys. Contains all
    // active (i.e. visible) sprites, the SpriteReps are used to
    // repaint the sprite content at any time.
    private java.util.HashMap activeSprites;

    //
    // XSpriteCanvas impl
    // ==================
    //

    public synchronized com.sun.star.rendering.XAnimatedSprite createSpriteFromAnimation( XAnimation animation )
    {
        CanvasUtils.printLog( "JavaCanvas.createSpriteFromAnimation called" );

        return new CanvasSprite( animation, this, (Graphics2D)dummyFrame.frame.getGraphics() );
    }

    public synchronized XAnimatedSprite createSpriteFromBitmaps( com.sun.star.rendering.XBitmap[] animationBitmaps,
                                                                 byte interpolationMode )
    {
        return null;
    }

    public synchronized XCustomSprite createCustomSprite( RealSize2D spriteSize )
    {
        CanvasUtils.printLog( "JavaCanvas.createCustomSprite called" );

        return new CanvasCustomSprite( spriteSize, this, (Graphics2D)dummyFrame.frame.getGraphics() );
    }

    public synchronized XSprite createClonedSprite( XSprite original )
    {
        return new CanvasClonedSprite( this, original );
    }

    public synchronized boolean updateScreen( boolean bUpdateAll )
    {
        redrawAllLayers();

        return true;
    }

    //
    // XBitmap implementation
    // ======================
    //

    public synchronized IntegerSize2D getSize()
    {
        return new IntegerSize2D( boundRect.Width,
                                  boundRect.Height );
    }

    //----------------------------------------------------------------------------------

    public synchronized XBitmapCanvas queryBitmapCanvas()
    {
        return this;
    }

    //----------------------------------------------------------------------------------

    public synchronized com.sun.star.rendering.XBitmap getScaledBitmap( RealSize2D newSize, boolean beFast ) throws com.sun.star.lang.IllegalArgumentException, VolatileContentDestroyedException
    {
        // TODO
        return null;
    }

    //----------------------------------------------------------------------------------

    public synchronized boolean hasAlpha()
    {
        // TODO
        return false;
    }

    //----------------------------------------------------------------------------------

    //
    // XIntegerBitmap implementation
    // =============================
    //

    public synchronized byte[] getData( IntegerBitmapLayout[] bitmapLayout,
                                        IntegerRectangle2D    rect )
    {
        // TODO
        return null;
    }

    //----------------------------------------------------------------------------------

    public synchronized void setData( byte[] data, IntegerBitmapLayout bitmapLayout, com.sun.star.geometry.IntegerRectangle2D rect )
    {
        // TODO
    }

    //----------------------------------------------------------------------------------

    public synchronized void setPixel( byte[] color, IntegerBitmapLayout bitmapLayout, com.sun.star.geometry.IntegerPoint2D pos )
    {
        // TODO
    }

    //----------------------------------------------------------------------------------

    public synchronized byte[] getPixel( IntegerBitmapLayout[] bitmapLayout,
                                         IntegerPoint2D        pos )
    {
        // TODO
        return null;
    }

    //----------------------------------------------------------------------------------

    public synchronized XBitmapPalette getPalette()
    {
        // TODO
        return null;
    }

    //----------------------------------------------------------------------------------

    public synchronized IntegerBitmapLayout getMemoryLayout()
    {
        // TODO: finish that one
        IntegerBitmapLayout layout = new IntegerBitmapLayout();

        layout.ScanLines = boundRect.Width;
        layout.ScanLineBytes = boundRect.Width*4;
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
    // XSpriteCanvas helper
    // ====================
    //
    public synchronized void renderAnimation( CanvasSprite sprite, XAnimation animation, double t )
    {
        SpriteRep spriteRep = (SpriteRep)activeSprites.get( sprite );
        if( spriteRep != null )
        {
            //Graphics2D graph = getWindowGraphics();

            // TODO: ensure update of graphics object externally, e.g. when
            // VCL moves the toplevel window.
            //java.awt.Rectangle bounds = dummyFrame.frame.getBounds();
            //graphics.setGraphics(graph, bounds.width, bounds.height);

            spriteRep.renderAnimation( animation, sprite.getViewState(), t );
        }
        else
        {
            CanvasUtils.printLog( "JavaCanvas.renderAnimation sprite not active!" );
        }
    }

    public synchronized void showSprite( SpriteBase sprite )
    {
        CanvasUtils.printLog( "JavaCanvas.showSprite() called" );

        SpriteRep spriteRep = (SpriteRep)activeSprites.get( sprite );
        if( spriteRep != null )
        {
            CanvasUtils.printLog( "JavaCanvas.showSprite sprite already active!" );
        }
        else
        {
            spriteRep = sprite.getSpriteRep();

            // a valid SpriteRep for a given Sprite in the
            // activeSprites hash denotes 'sprite active'
            activeSprites.put( sprite, spriteRep );

            // TODO: TEMP! Just for testing! Set empty cursor for presentation
//             dummyFrame.frame.setCursor( dummyFrame.frame.getToolkit().createCustomCursor(new java.awt.image.BufferedImage(0,0,
//                                                                                                                           java.awt.image.BufferedImage.TYPE_INT_RGB),
//                                                                                          new java.awt.Point(0,0),
//                                                                                          "") );
        }
    }

    public synchronized void hideSprite( SpriteBase sprite )
    {
        CanvasUtils.printLog( "JavaCanvas.hideSprite() called" );

        SpriteRep spriteRep = (SpriteRep)activeSprites.get( sprite );
        if( spriteRep != null )
        {
            activeSprites.put( sprite, null );
            redrawAllLayers();
        }
        else
        {
            CanvasUtils.printLog( "JavaCanvas.hideSprite sprite not active!" );
        }
    }

    private void redrawAllLayers()
    {
        // fetch the Graphics object to draw into (we're doing double
        // buffering here, the content is later shown via
        // bufferStrategy.show().
        Graphics2D graph = null;

        try
        {
            graph = (Graphics2D)bufferStrategy.getDrawGraphics();

            GraphicsDevice device = graph.getDeviceConfiguration().getDevice();
            CanvasUtils.printLog( "Available vram: " + device.getAvailableAcceleratedMemory() );

            // repaint background
            backBuffer.redraw( graph );

            // repaint all active sprites
            java.util.Set entries = activeSprites.entrySet();
            java.util.Iterator iter = entries.iterator();
            while( iter.hasNext() )
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)iter.next();
                if( entry.getValue() != null )
                    ((SpriteRep)entry.getValue()).redraw(graph);
            }

            long currTime = System.currentTimeMillis();
            graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER) );
            graph.setFont( fpsFont );
            graph.setColor( java.awt.Color.black );

            try
            {
                String fps = new String( String.valueOf(1000.0/(currTime-lastTime)) );
                graph.drawString( fps.substring(0,5) + " fps", 0, 20);
                CanvasUtils.printLog( fps.substring(0,5) + " fps" );
            }
            catch( Exception e )
            {
                graph.drawString( "0 fps", 0, 20);
            }

            lastTime = currTime;
        }
        catch( Exception e )
        {
            CanvasUtils.printLog( "Exception thrown in redrawAllLayers" );
        }
        finally
        {
            if( graph != null )
                graph.dispose();
        }

        bufferStrategy.show();
    }

    //----------------------------------------------------------------------------------

    private static final String s_implName = "XCanvas.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.JavaCanvas";

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //
    public synchronized String getImplementationName()
    {
        return s_implName;
    }

    public synchronized String [] getSupportedServiceNames()
    {
        return new String [] { s_serviceName };
    }

    public synchronized boolean supportsService( String serviceName )
    {
        return serviceName.equals( s_serviceName );
    }

    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(
        String implName,
        com.sun.star.lang.XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey )
    {
        if (implName.equals( s_implName ))
        {
            return com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                JavaCanvas.class, s_serviceName, multiFactory, regKey );
        }
        return null;
    }

    public static boolean __writeRegistryServiceInfo(
        com.sun.star.registry.XRegistryKey regKey )
    {
        return com.sun.star.comp.loader.FactoryHelper.writeRegistryServiceInfo(
            s_implName, s_serviceName, regKey );
    }
}
