/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpriteRunner.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:12:41 $
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
