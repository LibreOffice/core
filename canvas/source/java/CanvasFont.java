/*************************************************************************
 *
 *  $RCSfile: CanvasFont.java,v $
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
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import drafts.com.sun.star.rendering.*;
import drafts.com.sun.star.geometry.*;

// system-dependent stuff
import sun.awt.*;


public class CanvasFont
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               drafts.com.sun.star.rendering.XCanvasFont
{
    private CanvasBase                                  associatedCanvas;
    private drafts.com.sun.star.rendering.FontRequest   fontRequest;
    private java.awt.Font                               font;

    //----------------------------------------------------------------------------------

    public CanvasFont( drafts.com.sun.star.rendering.FontRequest    _fontRequest,
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
    private static final String s_serviceName = "drafts.com.sun.star.rendering.XCanvasFont";

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
