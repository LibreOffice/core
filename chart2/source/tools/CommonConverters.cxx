/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommonConverters.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:32:42 $
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
#ifndef __CHART_COMMON_CONVERTERS_HXX
#include "CommonConverters.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE2_HPP_
#include <com/sun/star/text/WritingMode2.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XNUMERICALDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XNumericalDataSequence.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#include <cstdarg>


//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  diverse methods for class conversions; e.g. ::basegfx::B3DHomMatrix to HomogenMatrix
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

drawing::HomogenMatrix Matrix4DToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM4 )
{
    drawing::HomogenMatrix aHM;
    aHM.Line1.Column1 = rM4.get(0, 0);
    aHM.Line1.Column2 = rM4.get(0, 1);
    aHM.Line1.Column3 = rM4.get(0, 2);
    aHM.Line1.Column4 = rM4.get(0, 3);
    aHM.Line2.Column1 = rM4.get(1, 0);
    aHM.Line2.Column2 = rM4.get(1, 1);
    aHM.Line2.Column3 = rM4.get(1, 2);
    aHM.Line2.Column4 = rM4.get(1, 3);
    aHM.Line3.Column1 = rM4.get(2, 0);
    aHM.Line3.Column2 = rM4.get(2, 1);
    aHM.Line3.Column3 = rM4.get(2, 2);
    aHM.Line3.Column4 = rM4.get(2, 3);
    aHM.Line4.Column1 = rM4.get(3, 0);
    aHM.Line4.Column2 = rM4.get(3, 1);
    aHM.Line4.Column3 = rM4.get(3, 2);
    aHM.Line4.Column4 = rM4.get(3, 3);
    return aHM;
}

::basegfx::B3DHomMatrix HomogenMatrixToMatrix4D( const drawing::HomogenMatrix& rHM )
{
    ::basegfx::B3DHomMatrix aM4;
    aM4.set(0, 0, rHM.Line1.Column1);
    aM4.set(0, 1, rHM.Line1.Column2);
    aM4.set(0, 2, rHM.Line1.Column3);
    aM4.set(0, 3, rHM.Line1.Column4);
    aM4.set(1, 0, rHM.Line2.Column1);
    aM4.set(1, 1, rHM.Line2.Column2);
    aM4.set(1, 2, rHM.Line2.Column3);
    aM4.set(1, 3, rHM.Line2.Column4);
    aM4.set(2, 0, rHM.Line3.Column1);
    aM4.set(2, 1, rHM.Line3.Column2);
    aM4.set(2, 2, rHM.Line3.Column3);
    aM4.set(2, 3, rHM.Line3.Column4);
    aM4.set(3, 0, rHM.Line4.Column1);
    aM4.set(3, 1, rHM.Line4.Column2);
    aM4.set(3, 2, rHM.Line4.Column3);
    aM4.set(3, 3, rHM.Line4.Column4);
    return aM4;
}

::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM4 )
{
    ::basegfx::B2DHomMatrix aM3;
    aM3.set(0, 0, rM4.get(0, 0));
    aM3.set(0, 1, rM4.get(0, 1));
    aM3.set(0, 2, rM4.get(0, 3));
    aM3.set(1, 0, rM4.get(1, 0));
    aM3.set(1, 1, rM4.get(1, 1));
    aM3.set(1, 2, rM4.get(1, 3));
    aM3.set(2, 0, rM4.get(3, 0));
    aM3.set(2, 1, rM4.get(3, 1));
    aM3.set(2, 2, rM4.get(3, 3));
    return aM3;
}


drawing::HomogenMatrix3 B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM3 )
{
    drawing::HomogenMatrix3 aHM;
    aHM.Line1.Column1 = rM3.get(0, 0);
    aHM.Line1.Column2 = rM3.get(0, 1);
    aHM.Line1.Column3 = rM3.get(0, 2);
    aHM.Line2.Column1 = rM3.get(1, 0);
    aHM.Line2.Column2 = rM3.get(1, 1);
    aHM.Line2.Column3 = rM3.get(1, 2);
    aHM.Line3.Column1 = rM3.get(2, 0);
    aHM.Line3.Column2 = rM3.get(2, 1);
    aHM.Line3.Column3 = rM3.get(2, 2);
    return aHM;
}

::basegfx::B2DHomMatrix HomogenMatrix3ToB2DHomMatrix( const drawing::HomogenMatrix3& rHM )
{
    ::basegfx::B2DHomMatrix aM3;
    aM3.set(0, 0, rHM.Line1.Column1);
    aM3.set(0, 1, rHM.Line1.Column2);
    aM3.set(0, 2, rHM.Line1.Column3);
    aM3.set(1, 0, rHM.Line2.Column1);
    aM3.set(1, 1, rHM.Line2.Column2);
    aM3.set(1, 2, rHM.Line2.Column3);
    aM3.set(2, 0, rHM.Line3.Column1);
    aM3.set(2, 1, rHM.Line3.Column2);
    aM3.set(2, 2, rHM.Line3.Column3);
    return aM3;
}

::basegfx::B3DPoint Position3DToB3DPoint( const drawing::Position3D& rPosition )
{
    return ::basegfx::B3DPoint(
        rPosition.PositionX ,
        rPosition.PositionY ,
        rPosition.PositionZ );
}

drawing::Direction3D B3DVectorToDirection3D( const ::basegfx::B3DVector& rVector)
{
    return drawing::Direction3D(
          rVector.getX()
        , rVector.getY()
        , rVector.getZ()
        );
}

drawing::PolyPolygonShape3D MakeLine3D(
    const drawing::Position3D & rStart,
    const drawing::Position3D & rEnd )
{
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    *pInnerSequenceX++ = rStart.PositionX;
    *pInnerSequenceY++ = rStart.PositionY;
    *pInnerSequenceZ++ = rStart.PositionZ;

    *pInnerSequenceX++ = rEnd.PositionX;
    *pInnerSequenceY++ = rEnd.PositionY;
    *pInnerSequenceZ++ = rEnd.PositionZ;

    return aPP;
}

void AddPointToPoly( drawing::PolyPolygonShape3D& rPoly, const drawing::Position3D& rPos, sal_Int32 nPolygonIndex )
{
    if(nPolygonIndex<0)
    {
        OSL_ENSURE( false, "The polygon index needs to be > 0");
        nPolygonIndex=0;
    }

    //make sure that we have enough polygons
    if(nPolygonIndex >= rPoly.SequenceX.getLength() )
    {
        rPoly.SequenceX.realloc(nPolygonIndex+1);
        rPoly.SequenceY.realloc(nPolygonIndex+1);
        rPoly.SequenceZ.realloc(nPolygonIndex+1);
    }

    drawing::DoubleSequence* pOuterSequenceX = &rPoly.SequenceX.getArray()[nPolygonIndex];
    drawing::DoubleSequence* pOuterSequenceY = &rPoly.SequenceY.getArray()[nPolygonIndex];
    drawing::DoubleSequence* pOuterSequenceZ = &rPoly.SequenceZ.getArray()[nPolygonIndex];

    sal_Int32 nOldPointCount = pOuterSequenceX->getLength();

    pOuterSequenceX->realloc(nOldPointCount+1);
    pOuterSequenceY->realloc(nOldPointCount+1);
    pOuterSequenceZ->realloc(nOldPointCount+1);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    pInnerSequenceX[nOldPointCount] = rPos.PositionX;
    pInnerSequenceY[nOldPointCount] = rPos.PositionY;
    pInnerSequenceZ[nOldPointCount] = rPos.PositionZ;
}

drawing::Position3D getPointFromPoly( const drawing::PolyPolygonShape3D& rPolygon, sal_Int32 nPointIndex, sal_Int32 nPolyIndex )
{
    drawing::Position3D aRet(0.0,0.0,0.0);

    if( nPolyIndex>=0 && nPolyIndex<rPolygon.SequenceX.getLength())
    {
        if(nPointIndex<rPolygon.SequenceX[nPolyIndex].getLength())
        {
            aRet.PositionX = rPolygon.SequenceX[nPolyIndex][nPointIndex];
            aRet.PositionY = rPolygon.SequenceY[nPolyIndex][nPointIndex];
            aRet.PositionZ = rPolygon.SequenceZ[nPolyIndex][nPointIndex];
        }
        else
        {
            ;DBG_ERROR("polygon was accessed with a wrong index");
        }
    }
    else
    {
        ;DBG_ERROR("polygon was accessed with a wrong index");
    }
    return aRet;
}

void appendPoly( drawing::PolyPolygonShape3D& rRet, const drawing::PolyPolygonShape3D& rAdd )
{
    sal_Int32 nOuterCount = Max( rRet.SequenceX.getLength(), rAdd.SequenceX.getLength() );
    rRet.SequenceX.realloc(nOuterCount);
    rRet.SequenceY.realloc(nOuterCount);
    rRet.SequenceZ.realloc(nOuterCount);

    for( sal_Int32 nOuter=0;nOuter<nOuterCount;nOuter++ )
    {
        sal_Int32 nOldPointCount = rRet.SequenceX[nOuter].getLength();
        sal_Int32 nAddPointCount = 0;
        if(nOuter<rAdd.SequenceX.getLength())
            nAddPointCount = rAdd.SequenceX[nOuter].getLength();
        if(!nAddPointCount)
            continue;

        sal_Int32 nNewPointCount = nOldPointCount + nAddPointCount;

        rRet.SequenceX[nOuter].realloc(nNewPointCount);
        rRet.SequenceY[nOuter].realloc(nNewPointCount);
        rRet.SequenceZ[nOuter].realloc(nNewPointCount);

        sal_Int32 nPointTarget=nOldPointCount;
        sal_Int32 nPointSource=nAddPointCount;
        for( ; nPointSource-- ; nPointTarget++ )
        {
            rRet.SequenceX[nOuter][nPointTarget] = rAdd.SequenceX[nOuter][nPointSource];
            rRet.SequenceY[nOuter][nPointTarget] = rAdd.SequenceY[nOuter][nPointSource];
            rRet.SequenceZ[nOuter][nPointTarget] = rAdd.SequenceZ[nOuter][nPointSource];
        }
    }
}

/*
drawing::PolyPolygonShape3D operator+(
          const drawing::PolyPolygonShape3D& rPoly1
        , const drawing::PolyPolygonShape3D& rPoly2 )
{
    drawing::PolyPolygonShape3D aRet;

    sal_Int32 nOuterCount = Max( rPoly1.SequenceX.getLength(), rPoly2.SequenceX.getLength() );
    aRet.SequenceX.realloc(nOuterCount);
    aRet.SequenceY.realloc(nOuterCount);
    aRet.SequenceZ.realloc(nOuterCount);

    for( sal_Int32 nOuter=0;nOuter<nOuterCount;nOuter++ )
    {
        sal_Int32 nPointCount_1 = 0;
        sal_Int32 nPointCount_2 = 0;
        if(nOuter<rPoly1.SequenceX.getLength())
            nPointCount_1 = rPoly1.SequenceX[nOuter].getLength();
        if(nOuter<rPoly2.SequenceX.getLength())
            nPointCount_2 = rPoly2.SequenceX[nOuter].getLength();

        sal_Int32 nPointCount = nPointCount_1 + nPointCount_2;

        aRet.SequenceX[nOuter].realloc(nPointCount);
        aRet.SequenceY[nOuter].realloc(nPointCount);
        aRet.SequenceZ[nOuter].realloc(nPointCount);

        sal_Int32 nPointTarget=0;

        {
            sal_Int32 nPointSourceCount = nPointCount_1;
            const drawing::PolyPolygonShape3D* pPoly = &rPoly1;
            for( sal_Int32 nPointSource=0; nPointSource<nPointSourceCount ; nPointSource++,nPointTarget++ )
            {
                aRet.SequenceX[nOuter][nPointTarget] = pPoly->SequenceX[nOuter][nPointSource];
                aRet.SequenceY[nOuter][nPointTarget] = pPoly->SequenceY[nOuter][nPointSource];
                aRet.SequenceZ[nOuter][nPointTarget] = pPoly->SequenceZ[nOuter][nPointSource];
            }
        }
        {
            sal_Int32 nPointSourceCount = nPointCount_2;
            const drawing::PolyPolygonShape3D* pPoly = &rPoly2;
            for( sal_Int32 nPointSource=nPointSourceCount; nPointSource-- ; nPointTarget++ )
            {
                aRet.SequenceX[nOuter][nPointTarget] = pPoly->SequenceX[nOuter][nPointSource];
                aRet.SequenceY[nOuter][nPointTarget] = pPoly->SequenceY[nOuter][nPointSource];
                aRet.SequenceZ[nOuter][nPointTarget] = pPoly->SequenceZ[nOuter][nPointSource];
            }
        }
    }
    return aRet;
}
*/
/*
drawing::PolyPolygonShape3D operator+(
          const drawing::PolyPolygonShape3D& rPoly1
        , const drawing::PolyPolygonShape3D& rPoly2 )
{
    drawing::PolyPolygonShape3D aRet;

    sal_Int32 nOuterCount = rPoly1.SequenceX.getLength()+rPoly2.SequenceX.getLength();
    aRet.SequenceX.realloc(nOuterCount);
    aRet.SequenceY.realloc(nOuterCount);
    aRet.SequenceZ.realloc(nOuterCount);

    drawing::DoubleSequence* pOuterSequenceX = aRet.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aRet.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aRet.SequenceZ.getArray();

    for( sal_Int32 nOuterTarget=0;nOuterTarget<nOuterCount;nOuterTarget++ )
    {
        const drawing::PolyPolygonShape3D* pPoly = &rPoly1;
        sal_Int32 nOuterSource = nOuterTarget;
        if(nOuterTarget>=rPoly1.SequenceX.getLength())
        {
            pPoly = &rPoly2;
            nOuterSource -=  rPoly1.SequenceX.getLength();
        }

        sal_Int32 nPointCount = pPoly->SequenceX[nOuterSource].getLength();

        pOuterSequenceX->realloc(nPointCount);
        pOuterSequenceY->realloc(nPointCount);
        pOuterSequenceZ->realloc(nPointCount);

        double* pInnerSequenceX = pOuterSequenceX->getArray();
        double* pInnerSequenceY = pOuterSequenceY->getArray();
        double* pInnerSequenceZ = pOuterSequenceZ->getArray();

        for( sal_Int32 nPoint=0;nPoint<nPointCount;nPoint++ )
        {
            *pInnerSequenceX = pPoly->SequenceX[nOuterSource][nPoint];
            *pInnerSequenceY = pPoly->SequenceY[nOuterSource][nPoint];
            *pInnerSequenceZ = pPoly->SequenceZ[nOuterSource][nPoint];

            pInnerSequenceX++; pInnerSequenceY++; pInnerSequenceZ++;
        }

        pOuterSequenceX++; pOuterSequenceY++; pOuterSequenceZ++;
    }
    return aRet;
}
*/

drawing::PointSequenceSequence PolyToPointSequence(
                const drawing::PolyPolygonShape3D& rPolyPolygon )
{
    drawing::PointSequenceSequence aRet;
    aRet.realloc( rPolyPolygon.SequenceX.getLength() );

    for(sal_Int32 nN = 0; nN < rPolyPolygon.SequenceX.getLength(); nN++)
    {
        sal_Int32 nInnerLength = rPolyPolygon.SequenceX[nN].getLength();
        aRet[nN].realloc( nInnerLength );
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            aRet[nN][nM].X = static_cast<sal_Int32>(rPolyPolygon.SequenceX[nN][nM]);
            aRet[nN][nM].Y = static_cast<sal_Int32>(rPolyPolygon.SequenceY[nN][nM]);
        }
    }
    return aRet;
}

void appendPointSequence( com::sun::star::drawing::PointSequenceSequence& rTarget
                        , com::sun::star::drawing::PointSequenceSequence& rAdd )
{
    sal_Int32 nAddCount = rAdd.getLength();
    if(!nAddCount)
        return;
    sal_Int32 nOldCount = rTarget.getLength();

    rTarget.realloc(nOldCount+nAddCount);
    for(sal_Int32 nS=0; nS<nAddCount; nS++ )
        rTarget[nOldCount+nS]=rAdd[nS];
}

Polygon PolyToToolsPoly( const drawing::PolyPolygonShape3D& rPolyPolygon )
{
    sal_Int32 nOuterLength = rPolyPolygon.SequenceX.getLength();
    if(!nOuterLength)
        return Polygon();

    sal_Int32 nNewSize = nOuterLength;
    sal_Int32 nNewIndex = 0;
    Polygon aRet(static_cast<USHORT>(nNewSize));

    for(sal_Int32 nN = 0; nN < nOuterLength; nN++)
    {
        sal_Int32 nInnerLength = rPolyPolygon.SequenceX[nN].getLength();
        nNewSize += nInnerLength-1,
        aRet.SetSize(static_cast<USHORT>(nNewSize));
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            aRet.SetPoint( Point( static_cast<long>(rPolyPolygon.SequenceX[nN][nM])
                                , static_cast<long>(rPolyPolygon.SequenceY[nN][nM])
                                )
                                , static_cast<USHORT>(nNewIndex) );
            nNewIndex++;
        }
    }
    return aRet;
}

drawing::PolyPolygonShape3D ToolsPolyToPoly( const Polygon& rToolsPoly, double zValue )
{
    sal_Int32 nPointCount = rToolsPoly.GetSize();

    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for( sal_Int32 nN = 0; nN<nPointCount; nN++ )
    {
        const Point& rPos = rToolsPoly.GetPoint( static_cast<USHORT>(nN) );
        *pInnerSequenceX++ = rPos.X();
        *pInnerSequenceY++ = rPos.Y();
        *pInnerSequenceZ++ = zValue;
    }

    return aPP;
}

drawing::Position3D  operator+( const drawing::Position3D& rPos
                           , const drawing::Direction3D& rDirection)
{
    return drawing::Position3D(
          rPos.PositionX + rDirection.DirectionX
        , rPos.PositionY + rDirection.DirectionY
        , rPos.PositionZ + rDirection.DirectionZ
        );
}

drawing::Position3D  operator-( const drawing::Position3D& rPos
                           , const drawing::Direction3D& rDirection)
{
    return drawing::Position3D(
          rPos.PositionX - rDirection.DirectionX
        , rPos.PositionY - rDirection.DirectionY
        , rPos.PositionZ - rDirection.DirectionZ
        );
}

drawing::Direction3D  operator-( const drawing::Position3D& rPos1
                           , const drawing::Position3D& rPos2)
{
    return drawing::Direction3D(
          rPos1.PositionX - rPos2.PositionX
        , rPos1.PositionY - rPos2.PositionY
        , rPos1.PositionZ - rPos2.PositionZ
        );
}

drawing::Direction3D  operator*( const drawing::Direction3D& rDirection
                           , double fFraction)
{
    return drawing::Direction3D(
          fFraction*rDirection.DirectionX
        , fFraction*rDirection.DirectionY
        , fFraction*rDirection.DirectionZ
        );
}

bool operator==( const drawing::Position3D& rPos1
                           , const drawing::Position3D& rPos2)
{
    return rPos1.PositionX == rPos2.PositionX
        && rPos1.PositionY == rPos2.PositionY
        && rPos1.PositionZ == rPos2.PositionZ;
}

::basegfx::B3DPoint SequenceToB3DPoint( const uno::Sequence< double >& rSeq )
{
    OSL_ENSURE(rSeq.getLength()==3,"The sequence needs to have length 3 for conversion into vector");

    double x=rSeq.getLength()>0?rSeq[0]:0.0;
    double y=rSeq.getLength()>1?rSeq[1]:0.0;
    double z=rSeq.getLength()>2?rSeq[2]:0.0;

    ::basegfx::B3DPoint aRet(x,y,z);
    return aRet;
}

uno::Sequence< double > B3DPointToSequence( const ::basegfx::B3DPoint& rPoint )
{
    uno::Sequence< double > aRet(3);
    aRet[0] = rPoint.getX();
    aRet[1] = rPoint.getY();
    aRet[2] = rPoint.getZ();
    return aRet;
}

drawing::Position3D SequenceToPosition3D( const uno::Sequence< double >& rSeq )
{
    OSL_ENSURE(rSeq.getLength()==3,"The sequence needs to have length 3 for conversion into vector");

    drawing::Position3D aRet;
    aRet.PositionX = rSeq.getLength()>0?rSeq[0]:0.0;
    aRet.PositionY = rSeq.getLength()>1?rSeq[1]:0.0;
    aRet.PositionZ = rSeq.getLength()>2?rSeq[2]:0.0;
    return aRet;
}

uno::Sequence< double > Position3DToSequence( const drawing::Position3D& rPosition )
{
    uno::Sequence< double > aRet(3);
    aRet[0] = rPosition.PositionX;
    aRet[1] = rPosition.PositionY;
    aRet[2] = rPosition.PositionZ;
    return aRet;
}

drawing::Direction3D SequenceToDirection3D( const uno::Sequence< double >& rSeq )
{
    drawing::Direction3D aRet;
    aRet.DirectionX = rSeq.getLength()>0?rSeq[0]:0.0;
    aRet.DirectionY = rSeq.getLength()>1?rSeq[1]:0.0;
    aRet.DirectionZ = rSeq.getLength()>2?rSeq[2]:0.0;
    return aRet;
}

uno::Sequence< double > Direction3DToSequence( const drawing::Direction3D& rDirection )
{
    uno::Sequence< double > aRet(3);
    aRet[0] = rDirection.DirectionX;
    aRet[1] = rDirection.DirectionY;
    aRet[2] = rDirection.DirectionZ;
    return aRet;
}

drawing::Direction3D operator/( const drawing::Direction3D& rDirection, double f )
{
    OSL_ENSURE(f,"a Direction3D is divided by NULL");
    return drawing::Direction3D(
        rDirection.DirectionX/f
        , rDirection.DirectionY/f
        , rDirection.DirectionZ/f
        );
}

text::WritingMode WritingMode2ToWritingMode1( sal_Int16 nWritingMode2 )
{
    switch(nWritingMode2)
    {
        case text::WritingMode2::RL_TB:
            return  text::WritingMode_RL_TB;
        case text::WritingMode2::TB_RL:
            return  text::WritingMode_TB_RL;
        case text::WritingMode2::LR_TB:
            return  text::WritingMode_LR_TB;
        default: // TL
            return  text::WritingMode_TB_RL;//there can no correct conversion be done here
    }
}

using namespace ::com::sun::star::chart2;

uno::Sequence< double > DataSequenceToDoubleSequence(
    const uno::Reference< XDataSequence > & xDataSequence )
{
    uno::Sequence< double > aResult;
    OSL_ASSERT( xDataSequence.is());
    if(!xDataSequence.is())
        return aResult;

    uno::Reference< XNumericalDataSequence > xNumericalDataSequence( xDataSequence, uno::UNO_QUERY );
    if( xNumericalDataSequence.is() )
    {
        aResult = xNumericalDataSequence->getNumericalData();
    }
    else
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        aResult.realloc(aValues.getLength());
        for(sal_Int32 nN=aValues.getLength();nN--;)
        {
            if( !(aValues[nN] >>= aResult[nN]) )
            {
                ::rtl::math::setNan( & aResult[nN] );
            }
        }
    }

    return aResult;
}

//.............................................................................
} //namespace chart
//.............................................................................
