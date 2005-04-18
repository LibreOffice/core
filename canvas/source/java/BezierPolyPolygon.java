/*************************************************************************
 *
 *  $RCSfile: BezierPolyPolygon.java,v $
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
import com.sun.star.rendering.*;
import com.sun.star.geometry.*;

// system-dependent stuff
import sun.awt.*;


public class BezierPolyPolygon
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.rendering.XBezierPolyPolygon2D
{
    private java.awt.geom.GeneralPath path;

    //----------------------------------------------------------------------------------

    public BezierPolyPolygon( RealBezierSegment2D[][] points )
    {
        setBezierSegments( points, 0 );
    }

    public java.awt.geom.GeneralPath getJavaPath()
    {
        return path;
    }

    //----------------------------------------------------------------------------------

    //
    // XPolyPolygon implementation
    // ===========================
    //
    public void addPolyPolygon( RealPoint2D position, XPolyPolygon2D polyPolygon )
    {
    }

    public int getNumberOfPolygons(  )
    {
        return 0;
    }

    public int getNumberOfPolygonPoints( int polygon )
    {
        return 0;
    }

    public FillRule getFillRule(  )
    {
        if( path.getWindingRule() == java.awt.geom.GeneralPath.WIND_EVEN_ODD )
            return FillRule.EVEN_ODD;
        else
            return FillRule.NON_ZERO;
    }

    public void setFillRule( FillRule fillRule )
    {
        if( fillRule == FillRule.EVEN_ODD )
            path.setWindingRule( java.awt.geom.GeneralPath.WIND_EVEN_ODD );
        else
            path.setWindingRule( java.awt.geom.GeneralPath.WIND_NON_ZERO );
    }

    public boolean isClosed( int index )
    {
        // TODO
        return false;
    }

    public void setClosed( int index, boolean closedState )
    {
        // TODO
    }

    //----------------------------------------------------------------------------------

    //
    // XBezierPolyPolygon implementation
    // =================================
    //
    public RealBezierSegment2D[][] getBezierSegments( int nPolygonIndex, int nNumberOfPolygons, int nPointIndex, int nNumberOfPoints )
    {
        return null;
    }

    public void setBezierSegments( RealBezierSegment2D[][] points, int nPolygonIndex )
    {
        if( nPolygonIndex != 0 )
            CanvasUtils.printLog( "LinePolyPolygon.setPoints: subset not yet implemented!" );

        path = CanvasUtils.makeGenPathFromBezierPoints( points );
    }

    public RealBezierSegment2D getBezierSegment( int nPolygonIndex, int nPointIndex )
    {
        return null;
    }

    public void setBezierSegment( RealBezierSegment2D point, int nPolygonIndex, int nPointIndex )
    {
        CanvasUtils.printLog( "LinePolyPolygon.setPoint: not yet implemented!" );
    }

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //

    private static final String s_implName = "XBezierPolyPolygon2D.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.BezierPolyPolygon2D";

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
