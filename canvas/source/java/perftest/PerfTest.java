/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PerfTest.java,v $
 * $Revision: 1.4 $
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

import java.awt.*;
import java.awt.image.*;
import java.awt.geom.*;


public class PerfTest
{
    // the frame object we're generating. TODO: Remove public access.
    private Frame   frame;
    private boolean fullscreen;

    public PerfTest()
    {
        fullscreen = false;

        // create a normal Java frame, and set it into fullscreen mode
        frame = new javax.swing.JFrame( "PerformanceTest" );
        frame.setBounds( new Rectangle(0,0,1600,1200) );
        frame.setUndecorated( true );
        frame.setVisible( true );

        Graphics2D graphics = (Graphics2D)frame.getGraphics();
        if( graphics.getDeviceConfiguration().getDevice().isFullScreenSupported() )
        {
            System.err.println( "entering fullscreen mode" );
            graphics.getDeviceConfiguration().getDevice().setFullScreenWindow( frame );
            fullscreen = true;
        }
        else
        {
            System.err.println( "fullscreen not supported" );
        }

        graphics.dispose();
    }

    //----------------------------------------------------------------------------------

    public void dispose()
    {
        if( fullscreen )
        {
            Graphics2D graphics = (Graphics2D)frame.getGraphics();
            if( graphics.getDeviceConfiguration().getDevice().isFullScreenSupported() )
            {
                System.err.println( "leaving fullscreen mode" );
                graphics.getDeviceConfiguration().getDevice().setFullScreenWindow( null );
            }
            graphics.dispose();
        }

        if( frame != null )
            frame.dispose();
    }

    //----------------------------------------------------------------------------------

    public static void initGraphics( Graphics2D graphics )
    {
        if( graphics != null )
        {
            RenderingHints hints = new RenderingHints(null);
            boolean hq = true;

            if( hq )
            {
                hints.add( new RenderingHints( RenderingHints.KEY_FRACTIONALMETRICS,
                                               RenderingHints.VALUE_FRACTIONALMETRICS_ON ) );
//                 hints.add( new RenderingHints( RenderingHints.KEY_ALPHA_INTERPOLATION,
//                                                         RenderingHints.VALUE_ALPHA_INTERPOLATION_QUALITY) );
                hints.add( new RenderingHints( RenderingHints.KEY_ALPHA_INTERPOLATION,
                                               RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED) );
//                 hints.add( new RenderingHints( RenderingHints.KEY_INTERPOLATION,
//                                                         RenderingHints.VALUE_INTERPOLATION_BICUBIC) );
                hints.add( new RenderingHints( RenderingHints.KEY_INTERPOLATION,
                                               RenderingHints.VALUE_INTERPOLATION_BILINEAR) );
//                 hints.add( new RenderingHints( RenderingHints.KEY_RENDERING,
//                                                         RenderingHints.VALUE_RENDER_QUALITY) );
                hints.add( new RenderingHints( RenderingHints.KEY_RENDERING,
                                               RenderingHints.VALUE_RENDER_SPEED) );
//                 hints.add( new RenderingHints( RenderingHints.KEY_STROKE_CONTROL,
//                                                         RenderingHints.VALUE_STROKE_NORMALIZE) );
                hints.add( new RenderingHints( RenderingHints.KEY_STROKE_CONTROL,
                                               RenderingHints.VALUE_STROKE_DEFAULT) );
                hints.add( new RenderingHints( RenderingHints.KEY_ANTIALIASING,
                                               RenderingHints.VALUE_ANTIALIAS_ON) );
            }
            else
            {
                hints.add( new RenderingHints( RenderingHints.KEY_ALPHA_INTERPOLATION,
                                               RenderingHints.VALUE_ALPHA_INTERPOLATION_SPEED) );
                hints.add( new RenderingHints( RenderingHints.KEY_INTERPOLATION,
                                               RenderingHints.VALUE_INTERPOLATION_BILINEAR) );
                hints.add( new RenderingHints( RenderingHints.KEY_RENDERING,
                                               RenderingHints.VALUE_RENDER_SPEED) );
                hints.add( new RenderingHints( RenderingHints.KEY_STROKE_CONTROL,
                                               RenderingHints.VALUE_STROKE_DEFAULT) );
                hints.add( new RenderingHints( RenderingHints.KEY_ANTIALIASING,
                                               RenderingHints.VALUE_ANTIALIAS_OFF) );
            }

            // the least common denominator standard
            hints.add( new RenderingHints( RenderingHints.KEY_FRACTIONALMETRICS,
                                           RenderingHints.VALUE_FRACTIONALMETRICS_ON) );
            hints.add( new RenderingHints( RenderingHints.KEY_TEXT_ANTIALIASING,
                                           RenderingHints.VALUE_TEXT_ANTIALIAS_ON) );

            graphics.setRenderingHints( hints );
        }
    }

    public void run()
    {
        frame.createBufferStrategy(2);
        BufferStrategy bufferStrategy = frame.getBufferStrategy();

        if( bufferStrategy.getCapabilities().isPageFlipping() )
            System.err.println( "double buffering is using page flipping" );
        else
            System.err.println( "double buffering is using blitting" );

        int width = 1600;
        int height = 1200;
        Graphics2D graphics             = (Graphics2D)frame.getGraphics();
        VolatileImage backBuffer        = graphics.getDeviceConfiguration().createCompatibleVolatileImage(width,height); // TODO: size dynamic
        BufferedImage buffer            = graphics.getDeviceConfiguration().createCompatibleImage(width,height,
                                                                                                  Transparency.TRANSLUCENT);
        BufferedImage buffer2           = graphics.getDeviceConfiguration().createCompatibleImage(width,height,
                                                                                                  Transparency.TRANSLUCENT);
        BufferedImage buffer3           = graphics.getDeviceConfiguration().createCompatibleImage(width,height,
                                                                                                  Transparency.TRANSLUCENT);

        GraphicsDevice device = graphics.getDeviceConfiguration().getDevice();
        System.err.println( "Images generated. Available vram: " + device.getAvailableAcceleratedMemory() );

        Graphics2D bufferGraphics       = (Graphics2D)buffer.getGraphics();
        Graphics2D buffer2Graphics      = (Graphics2D)buffer2.getGraphics();
        Graphics2D buffer3Graphics      = (Graphics2D)buffer3.getGraphics();
        Graphics2D backBufGraphics      = (Graphics2D)backBuffer.getGraphics();

        System.err.println( "Image graphics generated. Available vram: " + device.getAvailableAcceleratedMemory() );

        // init Graphics
        initGraphics( graphics );
        initGraphics( bufferGraphics );
        initGraphics( buffer2Graphics );
        initGraphics( backBufGraphics );

        // init content
        Font font = new Font( "Times", Font.PLAIN, 100 );
        Font fpsFont = new Font( "Times", Font.PLAIN, 20 );

        bufferGraphics.setComposite( AlphaComposite.getInstance(AlphaComposite.CLEAR));
        bufferGraphics.fillRect( 0,0,width,height );

        bufferGraphics.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER));
        bufferGraphics.setColor( Color.red );
        bufferGraphics.fillRect( 0,0,width/2,height/2 );
        bufferGraphics.setColor( Color.green );
        bufferGraphics.fillRect( width/2,0,width,height/2 );
        bufferGraphics.setColor( Color.blue );
        bufferGraphics.fillRect( 0,height/2,width/2,height );

        buffer2Graphics.setColor( Color.red );
        buffer2Graphics.fillRect( 0,0,width/2,height/2 );
        buffer2Graphics.setColor( Color.green );
        buffer2Graphics.fillRect( width/2,0,width,height/2 );
        buffer2Graphics.setColor( Color.blue );
        buffer2Graphics.fillRect( 0,height/2,width/2,height );

        buffer3Graphics.setColor( Color.blue );
        buffer3Graphics.fillRect( 0,0,width/2,height/2 );
        buffer3Graphics.setColor( Color.white );
        buffer3Graphics.fillRect( width/2,0,width,height/2 );
        buffer3Graphics.setColor( Color.gray );
        buffer3Graphics.fillRect( 0,height/2,width/2,height );

        backBufGraphics.setColor( Color.white );
        backBufGraphics.fillRect(0,0,width,height);
        backBufGraphics.setColor( Color.red );
        backBufGraphics.setFont( font );
        int i, turns=15;
        for(i=0; i<turns; ++i)
        {
            backBufGraphics.drawString( "Crossfade test", width*i/turns, height*i/turns );
        }

        System.err.println( "Images filled with content. Available vram: " + device.getAvailableAcceleratedMemory() );

        long lastTime = System.currentTimeMillis();
        int  turn, numTurns = 100;
        for(turn=0; turn<numTurns; ++turn)
        {
            // fetch the Graphics object to draw into (we're doing double
            // buffering here, the content is later shown via
            // bufferStrategy.show().
            Graphics2D graph = null;

            try
            {
                graph = (Graphics2D)bufferStrategy.getDrawGraphics();

                try
                {
                    // repaint background
                    graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER) );
                    graph.drawImage(backBuffer, 0, 0, null);

                    // alpha-composite foreground on top of that
                    float alpha = turn/(float)numTurns;
                    graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER, alpha) );

                    graph.drawImage(buffer, 0, 0, null);
                    buffer.flush();
                    graph.drawImage(buffer2, 100, 100, null);
                    buffer2.flush();

                    long currTime = System.currentTimeMillis();
                    graph.setComposite( AlphaComposite.getInstance(AlphaComposite.SRC_OVER) );
                    graph.setFont( fpsFont );
                    graph.setColor( Color.black );
                    String fps = new String( String.valueOf(1000.0/(currTime-lastTime)) );
                    System.err.println( "Images composited. Framerate: " + fps + " fps" );
                    graph.drawString( fps.substring(0,5) + " fps", 0, 20);
                    lastTime = currTime;

                    System.err.println( "Available vram: " + device.getAvailableAcceleratedMemory() );
                }
                catch( Exception e )
                {
                }

                System.err.println( "Turn: " + turn );
            }
            finally
            {
                if( graph != null )
                    graph.dispose();
            }

            bufferStrategy.show();
        }

        try
        {
            Thread.sleep(2000);
        }
        catch( Exception e )
        {
        }
    }

    //----------------------------------------------------------------------------------

    public static void main(String[] args)
    {
        // now, we're getting slightly system dependent here.
        String os = (String) System.getProperty("os.name");

        System.err.println( "System detected: " + os );

        // tweak some speed knobs...
        if( os.startsWith("Windows") )
        {
            System.setProperty("sun.java2d.translaccel", "true");
            System.setProperty("sun.java2d.ddforcevram", "true");
            //System.setProperty("sun.java2d.accthreshold", "0");

            System.err.println( "Optimizing for Windows" );
        }
        else
        {
            System.setProperty("sun.java2d.opengl", "true");

            System.err.println( "Optimizing for Unix" );
        }

        PerfTest test = new PerfTest();

        test.run();

        test.dispose();
    }

    //----------------------------------------------------------------------------------

}
