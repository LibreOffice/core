/*************************************************************************
 *
 *  $RCSfile: CanvasSprite.java,v $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
import drafts.com.sun.star.rendering.*;
import drafts.com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.geom.*;

// system-dependent stuff
import sun.awt.*;


public class CanvasSprite
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements drafts.com.sun.star.rendering.XAnimatedSprite,
                com.sun.star.lang.XServiceInfo,
                SpriteBase
{
    private XAnimation                      spriteAnimation;
    private JavaCanvas                      canvas;
    private Graphics2D                      referenceGraphics;
    private SpriteRunner                    runner;
    private ViewState                       viewState;
    private double                          alpha;
    private java.awt.geom.Point2D.Double    outputPosition;
    private SpriteRep                       spriteRep;

    //----------------------------------------------------------------------------------

    public CanvasSprite( XAnimation _animation, JavaCanvas _canvas, Graphics2D _referenceGraphics )
    {
        CanvasUtils.printLog( "CanvasSprite constructor called!" );

        spriteAnimation     = _animation;
        canvas              = _canvas;
        referenceGraphics   = _referenceGraphics;
        alpha               = 0.0;
        outputPosition      = new java.awt.geom.Point2D.Double(0.0,0.0);

        runner = new SpriteRunner( this, spriteAnimation, canvas );
    }

    public synchronized ViewState getViewState()
    {
        return viewState;
    }

    //----------------------------------------------------------------------------------

    //
    // SpriteBase
    // ==========
    //
    public synchronized SpriteRep getSpriteRep()
    {
        if( spriteRep == null )
        {
            spriteRep = new SpriteRep();

            setupSpriteBuffering( CanvasUtils.makeTransform( getViewState().AffineTransform ) );

            spriteRep.moveSprite( outputPosition );
            spriteRep.setSpriteAlpha( alpha );

            // render initial sprite content
            updateAnimation();
        }
        return spriteRep;
    }

    //----------------------------------------------------------------------------------

    //
    // XComponent
    // ==========
    //
    public void dispose()
    {
        // end the animation thread
        if( runner != null )
        {
            runner.quit();
            try
            {
                runner.join(0); // and wait until it's really over
            }
            catch( java.lang.InterruptedException e ) {}
        }

        if( spriteRep != null )
            spriteRep.dispose();

        canvas = null;
        spriteAnimation = null;
        runner = null;
        referenceGraphics = null;
        spriteRep = null;

        super.dispose();
    }

    //----------------------------------------------------------------------------------

    //
    // XSprite impl
    // ============
    //

    public synchronized void startAnimation( double speed )
    {
        runner.startAnimation( speed );
    }

    public synchronized void stopAnimation()
    {
        runner.stopAnimation();
    }

    public synchronized void resetAnimation()
    {
        runner.resetAnimation();
    }

    public synchronized void updateAnimation()
    {
        // only call render explicitely, if animation not
        // running. Otherwise, next animation render will update
        // anyway.
        if( spriteRep != null &&
            !runner.isAnimationActive() )
        {
            spriteRep.renderAnimation( spriteAnimation, getViewState(), runner.getCurrentT() );
        }
    }

    public synchronized void setPriority( double nPriority )
    {
        // TODO
    }

    public synchronized void setAlpha( double _alpha )
    {
        alpha = _alpha;

        if( spriteRep != null )
        {
            spriteRep.setSpriteAlpha( alpha );
        }
    }

    public synchronized void move( drafts.com.sun.star.geometry.RealPoint2D     _aNewPos,
                                   drafts.com.sun.star.rendering.ViewState      _viewState,
                                   drafts.com.sun.star.rendering.RenderState    _renderState )
    {
        // transform given point with concatenated transformation
        AffineTransform transform = CanvasUtils.ViewConcatRenderTransform( _viewState, _renderState );
        transform.transform( new java.awt.geom.Point2D.Double(_aNewPos.X,
                                                              _aNewPos.Y),
                             outputPosition );

        if( spriteRep != null )
        {
            spriteRep.moveSprite( outputPosition );
        }
    }

    public synchronized void transform( AffineMatrix2D aTransformation ) throws com.sun.star.lang.IllegalArgumentException
    {
        // TODO
    }

    public synchronized void clip( XPolyPolygon2D aClip )
    {
        // TODO
    }

    public synchronized void show()
    {
        canvas.showSprite( this );
        canvas.updateScreen( false );
    }

    public synchronized void hide()
    {
        canvas.hideSprite( this );

        // dispose and clear SpriteRep, animation content can be
        // regenerated at any time
        if( spriteRep != null )
            spriteRep.dispose();

        spriteRep = null;
    }

    public synchronized void setViewState( ViewState _viewState )
    {
        viewState = CanvasUtils.createAnimationViewState(_viewState,
                                                         getAnimationAttributes());

        CanvasUtils.printTransform( CanvasUtils.makeTransform( viewState.AffineTransform ),
                                    "CanvasSprite.setViewState" );

        if( spriteRep != null )
        {
            // calculate bounds of view-transformed animation output rectangle
            setupSpriteBuffering( CanvasUtils.makeTransform(getViewState().AffineTransform) );
            updateAnimation();
        }
    }

    public synchronized AnimationAttributes getAnimationAttributes()
    {
        return spriteAnimation.getAnimationAttributes();
    }

    public synchronized void setAll( RealPoint2D    _aNewPos,
                                     ViewState      _viewState,
                                     RenderState    _renderState,
                                     double         _alpha,
                                     boolean        bUpdateAnimation )
    {
        alpha = _alpha;

        // transform given point with concatenated transformation
        AffineTransform transform = CanvasUtils.ViewConcatRenderTransform( _viewState, _renderState );
        transform.transform( new java.awt.geom.Point2D.Double(_aNewPos.X,
                                                              _aNewPos.Y),
                             outputPosition );

        if( spriteRep != null )
        {
            spriteRep.setSpriteAlpha( alpha );
            spriteRep.moveSprite( outputPosition );

            if( bUpdateAnimation )
                updateAnimation();
        }
    }

    //----------------------------------------------------------------------------------

    private void setupSpriteBuffering( AffineTransform _viewTransform )
    {
        // determine bounds of view-transformed animation output rectangle
        drafts.com.sun.star.geometry.RealSize2D animSize = getAnimationAttributes().UntransformedSize;

        java.awt.geom.Rectangle2D.Double aTransformedBounds =
            CanvasUtils.calcTransformedRectBounds( new java.awt.geom.Rectangle2D.Double(0.0,0.0,
                                                                                        animSize.Width,
                                                                                        animSize.Height),
                                                   _viewTransform );
        CanvasUtils.printTransform( _viewTransform, "setupSpriteBuffering" );
        CanvasUtils.printLog( "setupSpriteBuffering: bounds are (" + aTransformedBounds.width + ", " + aTransformedBounds.height + ")" );

        // create a buffer of the appropriate size
        spriteRep.setupBuffer(referenceGraphics, (int)(aTransformedBounds.width+.5),
                              (int)(aTransformedBounds.height+.5) );
    }

    //----------------------------------------------------------------------------------

    private static final String s_implName = "XSprite.java.impl";
    private static final String s_serviceName = "drafts.com.sun.star.rendering.Sprite";

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //
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
