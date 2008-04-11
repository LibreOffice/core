/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasClonedSprite.java,v $
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
import java.awt.geom.*;

public class CanvasClonedSprite
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.rendering.XSprite,
                com.sun.star.lang.XServiceInfo,
                SpriteBase
{
    private JavaCanvas                      canvas;
    private double                          alpha;
    private java.awt.geom.Point2D.Double    outputPosition;
    private SpriteRep                       spriteRep;
    private SpriteBase                      original;

    //----------------------------------------------------------------------------------

    public CanvasClonedSprite( JavaCanvas   _canvas,
                               XSprite      _original )
    {
        CanvasUtils.printLog( "CanvasClonesSprite constructor called!" );

        canvas = _canvas;

        if( _original instanceof SpriteBase )
        {
            original = (SpriteBase)_original;
        }

        alpha          = 0.0;
        outputPosition = new java.awt.geom.Point2D.Double(0.0,0.0);

        // TODO: throw on error here!
    }

    //----------------------------------------------------------------------------------

    //
    // SpriteBase
    // ==========
    //
    public SpriteRep getSpriteRep()
    {
        if( spriteRep == null )
        {
            spriteRep = new SpriteRep( original.getSpriteRep() );

            spriteRep.moveSprite( outputPosition );
            spriteRep.setSpriteAlpha( alpha );

            // TODO: Check for spriteRep.buffer != null here, throw otherwise
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
        canvas = null;
        spriteRep = null;
        original = null;

        super.dispose();
    }

    //----------------------------------------------------------------------------------

    //
    // XSprite impl
    // ==================
    //

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

    public synchronized void move( RealPoint2D  _aNewPos,
                                   ViewState    _viewState,
                                   RenderState  _renderState )
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
    }
    //----------------------------------------------------------------------------------

    private static final String s_implName = "XSprite.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.Sprite";

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
