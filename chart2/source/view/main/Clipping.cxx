/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Clipping.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:35:45 $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "Clipping.hxx"
#include "CommonConverters.hxx"

#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace{
/** @descr  This is a supporting function for lcl_clip2d.  It computes a new parametric
            value for an entering (dTE) or leaving (dTL) intersection point with one
            of the edges bounding the clipping area.
            For explanation of the parameters please refer to :

            Liang-Biarsky parametric line-clipping algorithm as described in:
            Computer Graphics: principles and practice, 2nd ed.,
            James D. Foley et al.,
            Section 3.12.4 on page 117.
*/
bool lcl_CLIPt(double fDenom,double fNum, double & fTE, double & fTL)
{
    double  fT;

    if (fDenom > 0)             //  Intersection enters: PE
    {
        fT = fNum / fDenom;     //  Parametric value at the intersection.
        if (fT > fTL)           //  fTE and fTL crossover
            return false;       //    therefore reject the line.
        else if (fT > fTE)      //  A new fTE has been found.
            fTE = fT;
    }
    else if (fDenom < 0)        //  Intersection leaves: PL
    {
        fT = fNum / fDenom;     //  Parametric Value at the intersection.
        if (fT < fTE)           //  fTE and fTL crossover
            return false;       //    therefore reject the line.
        else if (fT < fTL)      //  A new fTL has been found.
            fTL = fT;
    }
    else if (fNum > 0)
        return false;           //  Line lies on the outside of the edge.

    return true;
}

/** @descr  The line given by it's two endpoints rP0 and rP1 is clipped at the rectangle
            rRectangle.  If there is at least a part of it visible then TRUE is returned and
            the endpoints of that part are stored in rP0 and rP1.  The points rP0 and rP1
            may have the same coordinates.
        @param  rP0 Start point of the line to clip.  Modified to contain a start point inside
            the clipping area if possible.
        @param  rP1 End point of the line to clip.  Modified to contain an end point inside
            the clipping area if possible.
        @param  rRectangle Clipping area.
        @return If the line lies completely or partly inside the clipping area then TRUE
            is returned.  If the line lies completely outside then FALSE is returned and rP0 and
            rP1 are left unmodified.
*/
bool lcl_clip2d(DoublePoint& rPoint0, DoublePoint& rPoint1, const DoubleRectangle& rRectangle)
{
    //Direction vector of the line.
    DoublePoint aDirection = rPoint1 - rPoint0;

    if( aDirection.X==0 && aDirection.Y==0 && rRectangle.isInside(rPoint0) )
    {
        //  Degenerate case of a zero length line.
        return true;
    }
    else
    {
        //  Values of the line parameter where the line enters resp. leaves the rectangle.
        double fTE = 0,
               fTL = 1;

        //  Test wether at least a part lies in the four half-planes with respect to
        //  the rectangles four edges.
        if( lcl_CLIPt(aDirection.X, rRectangle.Left - rPoint0.X, fTE, fTL) )
            if( lcl_CLIPt(-aDirection.X, rPoint0.X - rRectangle.Right, fTE, fTL) )
                if( lcl_CLIPt(aDirection.Y, rRectangle.Top - rPoint0.Y, fTE, fTL) )
                    if( lcl_CLIPt(-aDirection.Y, rPoint0.Y - rRectangle.Bottom, fTE, fTL) )
                    {
                        //  At least a part is visible.
                        if (fTL < 1)
                        {
                            //  Compute the new end point.
                            rPoint1.X = rPoint0.X + fTL * aDirection.X;
                            rPoint1.Y = rPoint0.Y + fTL * aDirection.Y;
                        }
                        if (fTE > 0)
                        {
                            //  Compute the new starting point.
                            rPoint0.X = rPoint0.X + fTE * aDirection.X;
                            rPoint0.Y = rPoint0.Y + fTE * aDirection.Y;
                        }
                        return true;
                    }

        //  Line is not visible.
        return false;
    }
}

bool lcl_clip2d_(drawing::Position3D& rPoint0, drawing::Position3D& rPoint1, const DoubleRectangle& rRectangle)
{
    DoublePoint aP0(rPoint0.PositionX,rPoint0.PositionY);
    DoublePoint aP1(rPoint1.PositionX,rPoint1.PositionY);
    bool bRet = lcl_clip2d( aP0, aP1, rRectangle );

    rPoint0.PositionX = aP0.X;
    rPoint0.PositionY = aP0.Y;
    rPoint1.PositionX = aP1.X;
    rPoint1.PositionY = aP1.Y;

    return bRet;
}

}//end anonymous namespace

void Clipping::clipPolygonAtRectangle( const drawing::PolyPolygonShape3D& rPolygon
                                      , const DoubleRectangle& rRectangle
                                      , drawing::PolyPolygonShape3D& aResult
                                      , bool bSplitPiecesToDifferentPolygons )
{
    aResult.SequenceX.realloc(0);
    aResult.SequenceY.realloc(0);
    aResult.SequenceZ.realloc(0);

    if(!rPolygon.SequenceX.getLength())
        return;

    drawing::Position3D aFrom;
    drawing::Position3D aTo;
    sal_Int32 nOldPointCount = rPolygon.SequenceX[0].getLength();
    sal_Int32 nNewPoly = -1;

    // set last point to a position outside the rectangle, such that the first
    // time lcl_clip2d returns true, the comparison to last will always yield false
    drawing::Position3D aLast(rRectangle.Left-1.0,rRectangle.Top-1.0, 0.0 );

    for(sal_Int32 nOldPoint=1; nOldPoint<nOldPointCount; nOldPoint++)
    {
        aFrom = getPointFromPoly(rPolygon,nOldPoint-1);
        aTo = getPointFromPoly(rPolygon,nOldPoint);
        if( lcl_clip2d_(aFrom, aTo, rRectangle) )
        {
            // compose an Polygon of as many consecutive points as possible
            if(aFrom == aLast)
            {
                if( !(aTo==aFrom) )
                    AddPointToPoly( aResult, aTo, nNewPoly );
            }
            else
            {
                if( bSplitPiecesToDifferentPolygons || nNewPoly<0)
                    nNewPoly++;
                AddPointToPoly( aResult, aFrom, nNewPoly );
                if( !(aTo==aFrom) )
                    AddPointToPoly( aResult, aTo, nNewPoly );
            }
            aLast = aTo;
        }
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
