/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CanvasCustomSprite.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:09:12 $
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
import com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.geom.*;

public class CanvasCustomSprite
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.rendering.XCustomSprite,
                com.sun.star.lang.XServiceInfo,
                SpriteBase
{
    private JavaCanvas                      canvas;
    private RealSize2D                      spriteSize;
    private Graphics2D                      referenceGraphics;
    private double                          alpha;
    private java.awt.geom.Point2D.Double    outputPosition;
    private SpriteRep                       spriteRep;

    //----------------------------------------------------------------------------------

    public CanvasCustomSprite( RealSize2D   _spriteSize,
                               JavaCanvas   _canvas,
                               Graphics2D   _referenceGraphics )
    {
        CanvasUtils.printLog( "CanvasCustomSprite constructor called, size is (" + _spriteSize.Width + ", " + _spriteSize.Height + ")" );

        canvas              = _canvas;
        spriteSize          = _spriteSize;
        referenceGraphics   = _referenceGraphics;
        alpha               = 0.0;
        outputPosition      = new java.awt.geom.Point2D.Double(0.0,0.0);
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

            spriteRep.setupBuffer(referenceGraphics, (int)(spriteSize.Width+.5), (int)(spriteSize.Height+.5) );

            spriteRep.moveSprite( outputPosition );
            spriteRep.setSpriteAlpha( alpha );
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
        if( spriteRep != null )
            spriteRep.dispose();

        canvas = null;
        referenceGraphics = null;
        spriteRep = null;

        super.dispose();
    }

    //----------------------------------------------------------------------------------

    //
    // XCustomSprite impl
    // ==================
    //

    public synchronized void setPriority( double nPriority )
    {
        // TODO
    }

    public synchronized void setAlpha( double _alpha )
    {
        CanvasUtils.printLog( "CanvasCustomSprite.setAlpha() called" );

        alpha = _alpha;

        if( spriteRep != null )
        {
            spriteRep.setSpriteAlpha( alpha );
        }
    }

    public synchronized void move( RealPoint2D  _aNewPos,
                                   ViewState    _viewState,
                                   RenderState  _renderState )
    {
        CanvasUtils.printLog( "CanvasCustomSprite.move() called" );

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

    public synchronized void clip( XPolyPolygon2D   aClip )
    {
        // TODO
    }

    public synchronized void show()
    {
        CanvasUtils.printLog( "CanvasCustomSprite.show() called" );

        canvas.showSprite( this );
        canvas.updateScreen( false );
    }

    public synchronized void hide()
    {
        CanvasUtils.printLog( "CanvasCustomSprite.hide() called" );

        canvas.hideSprite( this );

        // do _not_ dispose clear SpriteRep, since we cannot actively
        // repaint ourselves
    }

    public synchronized com.sun.star.rendering.XCanvas getContentCanvas()
    {
        CanvasUtils.printLog( "CanvasCustomSprite.getContentCanvas() called" );

        return getSpriteRep().getContentCanvas();
    }

    //----------------------------------------------------------------------------------

    private static final String s_implName = "XCustomSprite.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.CustomSprite";

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
