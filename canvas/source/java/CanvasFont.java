/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CanvasFont.java,v $
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
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import com.sun.star.rendering.*;
import com.sun.star.geometry.*;

public class CanvasFont
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.rendering.XCanvasFont
{
    private CanvasBase                                  associatedCanvas;
    private com.sun.star.rendering.FontRequest  fontRequest;
    private java.awt.Font                               font;

    //----------------------------------------------------------------------------------

    public CanvasFont( com.sun.star.rendering.FontRequest   _fontRequest,
                       CanvasBase                                   _canvas )
    {
        associatedCanvas = _canvas;
        fontRequest = _fontRequest;

        // TODO: Use proper attributes here, first of all, use fractional point font size
        font = new java.awt.Font( fontRequest.FontDescription.FamilyName, java.awt.Font.PLAIN, (int)(fontRequest.CellSize + .5) );
    }

    public java.awt.Font getFont()
    {
        return font;
    }

    //----------------------------------------------------------------------------------

    //
    // XCanvasFont implementation
    // ===========================
    //

    public XTextLayout createTextLayout( StringContext aText, byte nDirection, long nRandomSeed )
    {
        return new TextLayout( aText, nDirection, nRandomSeed, this, associatedCanvas );
    }

    public FontRequest getFontRequest(  )
    {
        return fontRequest;
    }

    public FontMetrics getFontMetrics(  )
    {
        // TODO
        return null;
    }

    public double[] getAvailableSizes(  )
    {
        // TODO
        return null;
    }

    public com.sun.star.beans.PropertyValue[] getExtraFontProperties(  )
    {
        // TODO
        return null;
    }

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //

    private static final String s_implName = "CanvasFont.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.XCanvasFont";

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
