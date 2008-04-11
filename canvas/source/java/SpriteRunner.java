/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SpriteRunner.java,v $
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

// Java AWT
import java.awt.*;
import java.awt.geom.*;

//
// HOWTO get a Graphics2D from a window
//

//     import javax.swing.*;
//     import java.awt.*;
//     import java.awt.geom.*;

//     public class Stroke1 extends JFrame {
//       Stroke drawingStroke = new BasicStroke(5);
//       Rectangle2D rect =
//           new Rectangle2D.Double(20, 40, 100, 40);

//       public void paint(Graphics g) {
//         Graphics2D g2d = (Graphics2D)g;
//         g2d.setStroke(drawingStroke);
//         g2d.draw(rect);
//       }

//       public static void main(String args[]) {
//         JFrame frame = new Stroke1();
//         frame.setDefaultCloseOperation(EXIT_ON_CLOSE);
//         frame.setSize(200, 100);
//         frame.show();
//       }
//     }


public class SpriteRunner
    extends Thread
{
    private CanvasSprite    sprite;
    private XAnimation      spriteAnimation;
    private JavaCanvas      canvas;
    private double          startTime;
    private double          currentSpeed;
    private double          currentT;
    private boolean         animationActive;
    private boolean         stayAlive;

    //----------------------------------------------------------------------------------

    public SpriteRunner( CanvasSprite _sprite, XAnimation _animation, JavaCanvas _canvas )
    {
        CanvasUtils.printLog( "SpriteRunner constructor called!" );

        sprite = _sprite;
        spriteAnimation = _animation;
        canvas = _canvas;
        startTime = 0.0;
        currentSpeed = 0.0;
        currentT = 0.0;
        animationActive = false;
        stayAlive = true;

        // Set priority to lower-than-normal, as this thread runs the
        // animation in a busy loop.
        setPriority( MIN_PRIORITY );
    }

    //----------------------------------------------------------------------------------

    //
    // Thread
    // ======
    //
    // Overriding
    //
    public void run()
    {
        // perform the animation rendering (as fast as possible, for now)

        while( stayAlive )
        {
            while( animationActive )
            {
                // to determine the current animation step to render, calc the
                // elapsed time since this animation was started
                double elapsedTime = getCurrentTime() - startTime;

                // the frame to render is determined by mapping the cycle
                // duration (currentSpeed) to the [0,1] range of the animation.
                currentT = (elapsedTime % currentSpeed) / currentSpeed;

                // delegate animation rendering to SpriteCanvas, as
                // only this instance can enforce consistency (on-time
                // saving of background etc.)
                canvas.renderAnimation( sprite, spriteAnimation, currentT );

                // TODO: Consolidate _all_ sprites per canvas into one
                // thread, call updateScreen() only after all sprites
                // have been processed.

                //Make changes visible
                canvas.updateScreen( false );

                // TODO: Evaluate vs. setPriority and other means
//                 try
//                 {
//                     // give other tasks some time to breathe (10ms - this is only meant symbolically)
//                     sleep(10);
//                 } catch( InterruptedException e ) { }
            }

            // wait until animation is activated again
            try
            {
                wait();
            }
            catch( InterruptedException e ) { }
            catch( IllegalMonitorStateException e ) { }
        }
    }

    public synchronized void startAnimation( double speed )
    {
        resetAnimation();
        currentSpeed = speed;

        // start us, if not done already
        if( !isAlive() )
            start();

        // enable animation thread
        animationActive = true;

        try
        {
            notify();
        } catch( IllegalMonitorStateException e ) { }
    }

    public synchronized void stopAnimation()
    {
        // stop animation after the current frame has been completely
        // rendered
        animationActive = false;
    }

    public synchronized void resetAnimation()
    {
        startTime = getCurrentTime();
    }

    public synchronized boolean isAnimationActive()
    {
        return animationActive;
    }

    public synchronized void quit()
    {
        stayAlive = false;
        stopAnimation();
    }

    public synchronized double getCurrentT()
    {
        return currentT;
    }

    //----------------------------------------------------------------------------------

    // helper
    static double getCurrentTime()
    {
        // determine current time in seconds
        java.util.Calendar cal = new java.util.GregorianCalendar();
        return cal.getTimeInMillis() / 1000.0;
    }
}
