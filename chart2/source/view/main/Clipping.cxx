/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "Clipping.hxx"
#include "CommonConverters.hxx"
#include "BaseGFXHelper.hxx"

#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>

namespace chart
{
using namespace ::com::sun::star;
using ::basegfx::B2DRectangle;
using ::basegfx::B2DTuple;

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
            rRectangle.  If there is at least a part of it visible then sal_True is returned and
            the endpoints of that part are stored in rP0 and rP1.  The points rP0 and rP1
            may have the same coordinates.
        @param  rP0 Start point of the line to clip.  Modified to contain a start point inside
            the clipping area if possible.
        @param  rP1 End point of the line to clip.  Modified to contain an end point inside
            the clipping area if possible.
        @param  rRectangle Clipping area.
        @return If the line lies completely or partly inside the clipping area then TRUE
            is returned.  If the line lies completely outside then sal_False is returned and rP0 and
            rP1 are left unmodified.
*/
bool lcl_clip2d(B2DTuple& rPoint0, B2DTuple& rPoint1, const B2DRectangle& rRectangle)
{
    //Direction vector of the line.
    B2DTuple aDirection = rPoint1 - rPoint0;

    if( aDirection.getX()==0 && aDirection.getY()==0 && rRectangle.isInside(rPoint0) )
    {
        //  Degenerate case of a zero length line.
        return true;
    }
    else
    {
        //  Values of the line parameter where the line enters resp. leaves the rectangle.
        double fTE = 0,
               fTL = 1;

        //  Test whether at least a part lies in the four half-planes with respect to
        //  the rectangles four edges.
        if( lcl_CLIPt(aDirection.getX(), rRectangle.getMinX() - rPoint0.getX(), fTE, fTL) )
            if( lcl_CLIPt(-aDirection.getX(), rPoint0.getX() - rRectangle.getMaxX(), fTE, fTL) )
                if( lcl_CLIPt(aDirection.getY(), rRectangle.getMinY() - rPoint0.getY(), fTE, fTL) )
                    if( lcl_CLIPt(-aDirection.getY(), rPoint0.getY() - rRectangle.getMaxY(), fTE, fTL) )
                    {
                        //  At least a part is visible.
                        if (fTL < 1)
                        {
                            //  Compute the new end point.
                            rPoint1.setX( rPoint0.getX() + fTL * aDirection.getX() );
                            rPoint1.setY( rPoint0.getY() + fTL * aDirection.getY() );
                        }
                        if (fTE > 0)
                        {
                            //  Compute the new starting point.
                            rPoint0.setX( rPoint0.getX() + fTE * aDirection.getX() );
                            rPoint0.setY( rPoint0.getY() + fTE * aDirection.getY() );
                        }
                        return true;
                    }

        //  Line is not visible.
        return false;
    }
}

bool lcl_clip2d_(drawing::Position3D& rPoint0, drawing::Position3D& rPoint1, const B2DRectangle& rRectangle)
{
    B2DTuple aP0(rPoint0.PositionX,rPoint0.PositionY);
    B2DTuple aP1(rPoint1.PositionX,rPoint1.PositionY);
    bool bRet = lcl_clip2d( aP0, aP1, rRectangle );

    rPoint0.PositionX = aP0.getX();
    rPoint0.PositionY = aP0.getY();
    rPoint1.PositionX = aP1.getX();
    rPoint1.PositionY = aP1.getY();

    return bRet;
}

void lcl_addPointToPoly( drawing::PolyPolygonShape3D& rPoly
        , const drawing::Position3D& rPos
        , sal_Int32 nPolygonIndex
        , std::vector< sal_Int32 >& rResultPointCount
        , sal_Int32 nReservePointCount )
{
    if(nPolygonIndex<0)
    {
        OSL_FAIL( "The polygon index needs to be > 0");
        nPolygonIndex=0;
    }

    //make sure that we have enough polygons
    if(nPolygonIndex >= rPoly.SequenceX.getLength() )
    {
        rPoly.SequenceX.realloc(nPolygonIndex+1);
        rPoly.SequenceY.realloc(nPolygonIndex+1);
        rPoly.SequenceZ.realloc(nPolygonIndex+1);
        rResultPointCount.resize(nPolygonIndex+1,0);
    }

    drawing::DoubleSequence* pOuterSequenceX = &rPoly.SequenceX.getArray()[nPolygonIndex];
    drawing::DoubleSequence* pOuterSequenceY = &rPoly.SequenceY.getArray()[nPolygonIndex];
    drawing::DoubleSequence* pOuterSequenceZ = &rPoly.SequenceZ.getArray()[nPolygonIndex];

    sal_Int32 nNewResultPointCount = rResultPointCount[nPolygonIndex]+1;
    sal_Int32 nSeqLength = pOuterSequenceX->getLength();

    if( nSeqLength <= nNewResultPointCount )
    {
        sal_Int32 nReallocLength = nReservePointCount;
        if( nNewResultPointCount > nReallocLength )
        {
            nReallocLength = nNewResultPointCount;
            OSL_FAIL("this should not be the case to avoid performance problems");
        }
        pOuterSequenceX->realloc(nReallocLength);
        pOuterSequenceY->realloc(nReallocLength);
        pOuterSequenceZ->realloc(nReallocLength);
    }

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    pInnerSequenceX[nNewResultPointCount-1] = rPos.PositionX;
    pInnerSequenceY[nNewResultPointCount-1] = rPos.PositionY;
    pInnerSequenceZ[nNewResultPointCount-1] = rPos.PositionZ;
    rResultPointCount[nPolygonIndex]=nNewResultPointCount;
}

}//end anonymous namespace

void Clipping::clipPolygonAtRectangle( const drawing::PolyPolygonShape3D& rPolygon
                                      , const B2DRectangle& rRectangle
                                      , drawing::PolyPolygonShape3D& aResult
                                      , bool bSplitPiecesToDifferentPolygons )
{
    aResult.SequenceX.realloc(0);
    aResult.SequenceY.realloc(0);
    aResult.SequenceZ.realloc(0);

    if(!rPolygon.SequenceX.getLength())
        return;

    //need clipping?:
    {
        ::basegfx::B3DRange a3DRange( BaseGFXHelper::getBoundVolume( rPolygon ) );
        ::basegfx::B2DRange a2DRange( a3DRange.getMinX(), a3DRange.getMinY(), a3DRange.getMaxX(), a3DRange.getMaxY() );
        if( rRectangle.isInside( a2DRange ) )
        {
            aResult = rPolygon;
            return;
        }
        else
        {
            a2DRange.intersect( rRectangle );
            if( a2DRange.isEmpty() )
                return;
        }
    }

    //
    std::vector< sal_Int32 > aResultPointCount;//per polygon index

    //apply clipping:
    drawing::Position3D aFrom;
    drawing::Position3D aTo;

    sal_Int32 nNewPolyIndex = 0;
    sal_Int32 nOldPolyCount = rPolygon.SequenceX.getLength();
    for(sal_Int32 nOldPolyIndex=0; nOldPolyIndex<nOldPolyCount; nOldPolyIndex++, nNewPolyIndex++ )
    {
        sal_Int32 nOldPointCount = rPolygon.SequenceX[nOldPolyIndex].getLength();

        // set last point to a position outside the rectangle, such that the first
        // time lcl_clip2d returns true, the comparison to last will always yield false
        drawing::Position3D aLast(rRectangle.getMinX()-1.0,rRectangle.getMinY()-1.0, 0.0 );

        for(sal_Int32 nOldPoint=1; nOldPoint<nOldPointCount; nOldPoint++)
        {
            aFrom = getPointFromPoly(rPolygon,nOldPoint-1,nOldPolyIndex);
            aTo = getPointFromPoly(rPolygon,nOldPoint,nOldPolyIndex);
            if( lcl_clip2d_(aFrom, aTo, rRectangle) )
            {
                // compose an Polygon of as many consecutive points as possible
                if(aFrom == aLast)
                {
                    if( !(aTo==aFrom) )
                    {
                        lcl_addPointToPoly( aResult, aTo, nNewPolyIndex, aResultPointCount, nOldPointCount );
                    }
                }
                else
                {
                    if( bSplitPiecesToDifferentPolygons && nOldPoint!=1 )
                    {
                        if( nNewPolyIndex < aResult.SequenceX.getLength()
                                && aResultPointCount[nNewPolyIndex]>0 )
                            nNewPolyIndex++;
                    }
                    lcl_addPointToPoly( aResult, aFrom, nNewPolyIndex, aResultPointCount, nOldPointCount );
                    if( !(aTo==aFrom) )
                        lcl_addPointToPoly( aResult, aTo, nNewPolyIndex, aResultPointCount, nOldPointCount );
                }
                aLast = aTo;
            }
        }
    }
    //free unused space
    for( sal_Int32 nPolygonIndex = aResultPointCount.size(); nPolygonIndex--; )
    {
        drawing::DoubleSequence* pOuterSequenceX = &aResult.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSequenceY = &aResult.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSequenceZ = &aResult.SequenceZ.getArray()[nPolygonIndex];

        sal_Int32 nUsedPointCount = aResultPointCount[nPolygonIndex];
        pOuterSequenceX->realloc(nUsedPointCount);
        pOuterSequenceY->realloc(nUsedPointCount);
        pOuterSequenceZ->realloc(nUsedPointCount);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
