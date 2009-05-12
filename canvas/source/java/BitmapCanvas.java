/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BitmapCanvas.java,v $
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
import com.sun.star.geometry.*;

// Java AWT
import java.awt.*;
import java.awt.image.*;
import java.awt.geom.*;

public class BitmapCanvas
    extends CanvasBase
    implements com.sun.star.rendering.XBitmapCanvas,
               com.sun.star.lang.XServiceInfo
{
    private Graphics2D  graphics;

    public Graphics2D getGraphics()
    {
        return graphics;
    }

    //----------------------------------------------------------------------------------

    public BitmapCanvas( Graphics2D _graphics )
    {
        graphics = _graphics;
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
        // TODO: create temp image when transform is non-trivial

        if( sourceCanvas == this )
        {
            // copy rectangle within the canvas
            graphics.copyArea((int)sourceRect.X1,
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
                CanvasUtils.setupGraphicsState( graphics, destViewState, destRenderState, CanvasUtils.alsoSetupPaint );

                // TODO: really extract correct source rect here
                BufferedImage backBuffer = ((BufferedGraphics2D)((JavaCanvas)sourceCanvas).getGraphics()).getBackBuffer();
                graphics.drawImage( backBuffer, 0, 0, null );
                CanvasUtils.postRenderImageTreatment( backBuffer );

            }
            // TODO: foreign canvas
        }
    }

    //----------------------------------------------------------------------------------

    private static final String s_implName = "XBitmapCanvas.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.BitmapCanvas";

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
