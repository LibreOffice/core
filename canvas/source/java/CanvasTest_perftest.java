/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasTest_perftest.java,v $
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

public class CanvasTest
    extends CanvasBase
    implements com.sun.star.awt.XWindow,
               com.sun.star.rendering.XSpriteCanvas,
               com.sun.star.lang.XServiceInfo,
               com.sun.star.lang.XInitialization
{
    private WindowAdapter                   dummyFrame;
    private BackBuffer                      backBuffer;
    private java.awt.image.BufferStrategy   bufferStrategy;

    private java.awt.Font                   fpsFont;
    private long                            lastTime;


    // TEST ONLY {
    private static int testWidth = 1600;
    private static int testHeight = 1200;
    private BufferedImage backgroundBuffer;
    private BufferedImage buffer;
    private BufferedImage buffer2;
    private Graphics2D backBufGraphics;
    private Graphics2D bufferGraphics;
    private Graphics2D buffer2Graphics;
    private float currAlpha;
    // TEST ONLY }



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
        CanvasUtils.printLog( "CanvasTest.initialize called!" );

        // Do that as the very first thing. The Java2D internal
        // classes choose their render path at initialization time,
        // thus this must happen before doing _any_ GUI.

        // TODO: Put those flags into javarc/java.ini, we're maybe
        // coming late into a running JVM!

        // now, we're getting slightly system dependent here.
        String os = (String) System.getProperty("os.name");

        CanvasUtils.printLog( "System detected: " + os );

        // tweak some speed knobs...
        if( os.startsWith("Windows") )
        {
            System.setProperty("sun.java2d.translaccel", "true");
            System.setProperty("sun.java2d.ddforcevram", "true");
            //System.setProperty("sun.java2d.accthreshold", "0");

            CanvasUtils.printLog( "Optimizing for Windows" );
        }
        else
        {
            System.setProperty("sun.java2d.opengl", "true");

            CanvasUtils.printLog( "Optimizing for Unix" );
        }

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
            com.sun.star.awt.Rectangle boundRect = (com.sun.star.awt.Rectangle) arguments[2];

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

            // TEST
            if( true )
            {
                backBuffer = new BackBuffer( (Graphics2D) dummyFrame.frame.getGraphics(),
                                             Math.max(1,boundRect.Width),
                                             Math.max(1,boundRect.Height) );
            }
            else
            {
                backBuffer = new BackBuffer( (Graphics2D) dummyFrame.frame.getGraphics(),
                                             10, 10 );
            }

            // TODO: Maybe delay double buffer init until first sprite creation
            dummyFrame.frame.createBufferStrategy(2);
            bufferStrategy = dummyFrame.frame.getBufferStrategy();

            if( bufferStrategy.getCapabilities().isPageFlipping() )
                CanvasUtils.printLog( "CanvasTest.initialize double buffering is using page flipping!" );
            else
                CanvasUtils.printLog( "CanvasTest.initialize double buffering is using blitting!" );

            lastTime = System.currentTimeMillis();
            fpsFont = new java.awt.Font( "Times", Font.PLAIN, 20 );


            if( false )
            {
            // TEST ONLY {
            Graphics2D frameGraphics= (Graphics2D) dummyFrame.frame.getGraphics();
            backgroundBuffer        = frameGraphics.getDeviceConfiguration().createCompatibleImage(testWidth,testHeight); // TODO: size dynamic
            buffer                  = frameGraphics.getDeviceConfiguration().createCompatibleImage(testWidth,testHeight,
                                                                                                   Transparency.TRANSLUCENT);
            buffer2                 = frameGraphics.getDeviceConfiguration().createCompatibleImage(testWidth,testHeight,
                                                                                                   Transparency.TRANSLUCENT);
            bufferGraphics          = (Graphics2D)buffer.getGraphics();
            buffer2Graphics         = (Graphics2D)buffer2.getGraphics();
            backBufGraphics         = (Graphics2D)backgroundBuffer.getGraphics();
            currAlpha               = 0.1f;

            // init content
            Font font = new Font( "Times", Font.PLAIN, 100 );

            bufferGraphics.setComposite( AlphaComposite.getInstance(AlphaComposite.CLEAR));
            bufferGraphics.fillRect( 0,0,testWidth,testHeight );

            bufferGraphics.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER));
            bufferGraphics.setColor( Color.red );
            bufferGraphics.fillRect( 0,0,testWidth/2,testHeight/2 );
            bufferGraphics.setColor( Color.green );
            bufferGraphics.fillRect( testWidth/2,0,testWidth,testHeight/2 );
            bufferGraphics.setColor( Color.blue );
            bufferGraphics.fillRect( 0,testHeight/2,testWidth/2,testHeight );

            buffer2Graphics.setColor( Color.red );
            buffer2Graphics.fillRect( 0,0,testWidth/2,testHeight/2 );
            buffer2Graphics.setColor( Color.green );
            buffer2Graphics.fillRect( testWidth/2,0,testWidth,testHeight/2 );
            buffer2Graphics.setColor( Color.blue );
            buffer2Graphics.fillRect( 0,testHeight/2,testWidth/2,testHeight );

            backBufGraphics.setColor( Color.white );
            backBufGraphics.fillRect(0,0,testWidth,testHeight);
            backBufGraphics.setColor( Color.red );
            backBufGraphics.setFont( font );
            int i, turns=15;
            for(i=0; i<turns; ++i)
            {
                backBufGraphics.drawString( "Crossfade test", testWidth*i/turns, testHeight*i/turns );
            }
            // TEST ONLY }
            }

            CanvasUtils.printLog( "CanvasTest.initialize finished!" );
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
        CanvasUtils.printLog( "CanvasTest: disposed!" );

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

    public CanvasTest( XComponentContext xContext )
    {
        CanvasUtils.printLog( "CanvasTest constructor called!" );
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

            CanvasUtils.printLog( "CanvasTest graphics set to " + Width + "," + Height );
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
                                       com.sun.star.rendering.Rectangle2D   sourceRect,
                                       com.sun.star.rendering.ViewState         sourceViewState,
                                       com.sun.star.rendering.RenderState   sourceRenderState,
                                       com.sun.star.rendering.Rectangle2D   destRect,
                                       com.sun.star.rendering.ViewState         destViewState,
                                       com.sun.star.rendering.RenderState   destRenderState )
    {
        CanvasUtils.printLog( "CanvasTest.copyRect() called" );

        // TODO: create temp image when transform is non-trivial

        if( sourceCanvas == this )
        {
            // copy rectangle within the canvas
            getGraphics().copyArea((int)sourceRect.x1,
                                   (int)sourceRect.y1,
                                   (int)(sourceRect.x2 - sourceRect.x1),
                                   (int)(sourceRect.y2 - sourceRect.y1),
                                   (int)(destRect.x1 - sourceRect.x1),
                                   (int)(destRect.y1 - sourceRect.y1) );
        }
        else
        {
            if( sourceCanvas instanceof CanvasTest )
            {
                // cache
                CanvasUtils.setupGraphicsState( getGraphics(), destViewState, destRenderState, CanvasUtils.alsoSetupPaint );

                java.awt.Image backBuffer = ((CanvasTest)sourceCanvas).backBuffer.getBackBuffer();

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
        CanvasUtils.printLog( "CanvasTest.createSpriteFromAnimation called" );

        return new CanvasSprite( animation, this, (Graphics2D)dummyFrame.frame.getGraphics() );
    }

    public synchronized XAnimatedSprite createSpriteFromBitmaps( com.sun.star.rendering.XBitmap[] animationBitmaps,
                                                                 short interpolationMode )
    {
        return null;
    }

    public synchronized XCustomSprite createCustomSprite( Size2D spriteSize )
    {
        CanvasUtils.printLog( "CanvasTest.createCustomSprite called" );

        return new CanvasCustomSprite( spriteSize, this, (Graphics2D)dummyFrame.frame.getGraphics() );
    }

    public synchronized XSprite createClonedSprite( XSprite original )
    {
        return new CanvasClonedSprite( this, original );
    }

    public synchronized void updateScreen()
    {
        redrawAllLayers();
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
            CanvasUtils.printLog( "CanvasTest.renderAnimation sprite not active!" );
        }
    }

    public synchronized void showSprite( SpriteBase sprite )
    {
        CanvasUtils.printLog( "CanvasTest.showSprite() called" );

        SpriteRep spriteRep = (SpriteRep)activeSprites.get( sprite );
        if( spriteRep != null )
        {
            CanvasUtils.printLog( "CanvasTest.showSprite sprite already active!" );
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
        CanvasUtils.printLog( "CanvasTest.hideSprite() called" );

        SpriteRep spriteRep = (SpriteRep)activeSprites.get( sprite );
        if( spriteRep != null )
        {
            activeSprites.put( sprite, null );
            redrawAllLayers();
        }
        else
        {
            CanvasUtils.printLog( "CanvasTest.hideSprite sprite not active!" );
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

            if( false )
            {
            // TEST ONLY {
                    // repaint background
                    graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER) );
                    graph.drawImage(backgroundBuffer, 0, 0, null);
                    //backgroundBuffer.flush();

                    // alpha-composite foreground on top of that
                    graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER, currAlpha) );

                    graph.drawImage(buffer, 0, 0, null);
                    //buffer.flush();

                    currAlpha += 0.1f; if( currAlpha > 1.0 ) currAlpha = 0.1f;
            // TEST ONLY }
            }

            if( true )
            {
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
        finally
        {
            if( graph != null )
                graph.dispose();
        }

        bufferStrategy.show();
    }

    //----------------------------------------------------------------------------------

    private static final String s_implName = "XCanvas.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.Canvas";

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
                CanvasTest.class, s_serviceName, multiFactory, regKey );
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
