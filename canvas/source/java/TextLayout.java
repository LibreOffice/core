/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextLayout.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:12:57 $
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
import com.sun.star.lib.uno.helper.WeakBase;

// OOo AWT
import com.sun.star.awt.*;

// Canvas
import com.sun.star.rendering.*;
import com.sun.star.geometry.*;

public class TextLayout
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.rendering.XTextLayout
{
    private double[]            advancements;
    private StringContext       text;
    private byte                direction;
    private CanvasBase          associatedCanvas;
    private CanvasFont          font;

    //----------------------------------------------------------------------------------

    public TextLayout( StringContext     aText,
                       byte              nDirection,
                       long              nRandomSeed,
                       CanvasFont       _font,
                       CanvasBase       _canvas )
    {
        text = aText;
        direction = nDirection;
        associatedCanvas = _canvas;
        font = _font;
    }

    //
    // XTextLayout implementation
    // ==========================
    //

    public XPolyPolygon2D[] queryTextShapes(  )
    {
        // TODO
        return null;
    }

    public com.sun.star.geometry.RealRectangle2D[] queryInkMeasures(  )
    {
        // TODO
        return null;
    }

    public com.sun.star.geometry.RealRectangle2D[] queryMeasures(  )
    {
        // TODO
        return null;
    }

    public double[] queryLogicalAdvancements(  )
    {
        // TODO
        return null;
    }

    public void applyLogicalAdvancements( double[] aAdvancements ) throws com.sun.star.lang.IllegalArgumentException
    {
        if( aAdvancements.length != text.Length )
            throw new com.sun.star.lang.IllegalArgumentException();

        advancements = aAdvancements;
    }

    public com.sun.star.geometry.RealRectangle2D queryTextBounds(  )
    {
        // TODO
        return null;
    }

    public double justify( double nSize ) throws com.sun.star.lang.IllegalArgumentException
    {
        // TODO
        return 0;
    }

    public double combinedJustify( XTextLayout[] aNextLayouts, double nSize ) throws com.sun.star.lang.IllegalArgumentException
    {
        // TODO
        return 0;
    }

    public TextHit getTextHit( /*IN*/com.sun.star.geometry.RealPoint2D aHitPoint )
    {
        // TODO
        return null;
    }

    public Caret getCaret( int nInsertionIndex, boolean bExcludeLigatures ) throws com.sun.star.lang.IndexOutOfBoundsException
    {
        // TODO
        return null;
    }

    public int getNextInsertionIndex( int nStartIndex, int nCaretAdvancement, boolean bExcludeLigatures ) throws com.sun.star.lang.IndexOutOfBoundsException
    {
        // TODO
        return 0;
    }

    public XPolyPolygon2D queryVisualHighlighting( int nStartIndex, int nEndIndex ) throws com.sun.star.lang.IndexOutOfBoundsException
    {
        // TODO
        return null;
    }

    public XPolyPolygon2D queryLogicalHighlighting( int nStartIndex, int nEndIndex ) throws com.sun.star.lang.IndexOutOfBoundsException
    {
        // TODO
        return null;
    }

    public double getBaselineOffset(  )
    {
        // TODO
        return 0.0;
    }

    public byte getMainTextDirection(  )
    {
        return direction;
    }

    public XCanvasFont getFont(  )
    {
        return font;
    }

    public StringContext getText(  )
    {
        return text;
    }

    public boolean draw( java.awt.Graphics2D graphics )
    {
        // TODO: use proper advancement. Text direction need not be horizontal!
        // TODO: given text string need not have a one-to-one relationship between code point and glyph (offset)!
        graphics.drawString( text.Text.substring(text.StartPosition, text.StartPosition + 1), (float)0.0, (float)0.0 );
        for( int i=1; i<advancements.length && i<text.Length; ++i )
        {
            CanvasUtils.printLog( "XCanvas: drawOffsettedText rendering a \"" +
                                  text.Text.substring(text.StartPosition + i,
                                                      text.StartPosition + i + 1) +
                                  "\" (position " + (text.StartPosition + i) +
                                  " of " + text.Text + ", offset " + advancements[i] + ")" );

            graphics.drawString( text.Text.substring(text.StartPosition + i, text.StartPosition + i + 1), (float)advancements[i-1], (float)0.0 );
        }

        return true;
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
