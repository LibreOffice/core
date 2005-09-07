/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LinePolyPolygon.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:11:35 $
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

public class LinePolyPolygon
    extends com.sun.star.lib.uno.helper.ComponentBase
    implements com.sun.star.lang.XServiceInfo,
               com.sun.star.rendering.XLinePolyPolygon2D
{
    private java.awt.geom.GeneralPath path;

    //----------------------------------------------------------------------------------

    public LinePolyPolygon( RealPoint2D[][] points )
    {
        setPoints( points, 0 );
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
    public synchronized void addPolyPolygon( RealPoint2D position, XPolyPolygon2D polyPolygon )
    {
    }

    public synchronized int getNumberOfPolygons(  )
    {
        return 0;
    }

    public synchronized int getNumberOfPolygonPoints( int polygon )
    {
        return 0;
    }

    public synchronized FillRule getFillRule(  )
    {
        if( path.getWindingRule() == java.awt.geom.GeneralPath.WIND_EVEN_ODD )
            return FillRule.EVEN_ODD;
        else
            return FillRule.NON_ZERO;
    }

    public synchronized void setFillRule( FillRule fillRule )
    {
        if( fillRule == FillRule.EVEN_ODD )
            path.setWindingRule( java.awt.geom.GeneralPath.WIND_EVEN_ODD );
        else
            path.setWindingRule( java.awt.geom.GeneralPath.WIND_NON_ZERO );
    }

    public synchronized boolean isClosed( int index )
    {
        // TODO
        return false;
    }

    public synchronized void setClosed( int index, boolean closedState )
    {
        // TODO
    }

    //----------------------------------------------------------------------------------

    //
    // XLinePolyPolygon implementation
    // ===============================
    //
    public synchronized RealPoint2D[][] getPoints( int nPolygonIndex, int nNumberOfPolygons, int nPointIndex, int nNumberOfPoints )
    {
        // TODO: Implement subsetting

//         double [] points = new double[6];

//         // BAH! Use util.Vector here!

//         // find number of subpaths
//         PathIterator aIter = path.getPathIterator( new AffineTransform() );
//         int nNumSubPaths = 0;
//         while( !aIter.isDone() )
//         {
//             if( aIter.currentSegment(points) == SEG_MOVETO )
//                 ++nNumSubPaths;

//             aIter.next();
//         }

//         Point2D [][] aRes = new Point2D[nNumSubPaths][];
//         aIter = path.getPathIterator( new AffineTransform() );
//         while( !aIter.isDone() )
//         {
//             switch( aIter.currentSegment(points) )
//             {
//                 case SEG_MOVETO:
//                     break;

//                 case SEG_LINETO:
//                     break;

//                 case SEG_CLOSE:
//                     break;

//                 default:
//                     CanvasUtils.printLog( "LinePolyPolygon.getPoints(): unexpected path type" );
//                     break;
//             }

//             aIter.next();
//         }
//         double [] points = new double[6];

        return null;
    }

    public synchronized void setPoints( RealPoint2D[][] points, int nPolygonIndex )
    {
        if( nPolygonIndex != 0 )
            CanvasUtils.printLog( "LinePolyPolygon.setPoints: subset not yet implemented!" );

        path = CanvasUtils.makeGenPathFromLinePoints( points );
    }

    public synchronized RealPoint2D getPoint( int nPolygonIndex, int nPointIndex )
    {
        return null;
    }

    public synchronized void setPoint( RealPoint2D point, int nPolygonIndex, int nPointIndex )
    {
        CanvasUtils.printLog( "LinePolyPolygon.setPoint: not yet implemented!" );
    }

    //----------------------------------------------------------------------------------

    //
    // XServiceInfo impl
    // =================
    //

    private static final String s_implName = "XLinePolyPolygon2D.java.impl";
    private static final String s_serviceName = "com.sun.star.rendering.LinePolyPolygon2D";

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
