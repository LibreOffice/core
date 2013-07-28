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

#include "CommonConverters.hxx"
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <rtl/math.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <cstdarg>

namespace chart
{

using namespace ::com::sun::star;

//  diverse methods for class conversions; e.g. ::basegfx::B3DHomMatrix to HomogenMatrix

drawing::HomogenMatrix B3DHomMatrixToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM )
{
    drawing::HomogenMatrix aHM;
    aHM.Line1.Column1 = rM.get(0, 0);
    aHM.Line1.Column2 = rM.get(0, 1);
    aHM.Line1.Column3 = rM.get(0, 2);
    aHM.Line1.Column4 = rM.get(0, 3);
    aHM.Line2.Column1 = rM.get(1, 0);
    aHM.Line2.Column2 = rM.get(1, 1);
    aHM.Line2.Column3 = rM.get(1, 2);
    aHM.Line2.Column4 = rM.get(1, 3);
    aHM.Line3.Column1 = rM.get(2, 0);
    aHM.Line3.Column2 = rM.get(2, 1);
    aHM.Line3.Column3 = rM.get(2, 2);
    aHM.Line3.Column4 = rM.get(2, 3);
    aHM.Line4.Column1 = rM.get(3, 0);
    aHM.Line4.Column2 = rM.get(3, 1);
    aHM.Line4.Column3 = rM.get(3, 2);
    aHM.Line4.Column4 = rM.get(3, 3);
    return aHM;
}

::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix( const drawing::HomogenMatrix& rHM )
{
    ::basegfx::B3DHomMatrix aM;
    aM.set(0, 0, rHM.Line1.Column1);
    aM.set(0, 1, rHM.Line1.Column2);
    aM.set(0, 2, rHM.Line1.Column3);
    aM.set(0, 3, rHM.Line1.Column4);
    aM.set(1, 0, rHM.Line2.Column1);
    aM.set(1, 1, rHM.Line2.Column2);
    aM.set(1, 2, rHM.Line2.Column3);
    aM.set(1, 3, rHM.Line2.Column4);
    aM.set(2, 0, rHM.Line3.Column1);
    aM.set(2, 1, rHM.Line3.Column2);
    aM.set(2, 2, rHM.Line3.Column3);
    aM.set(2, 3, rHM.Line3.Column4);
    aM.set(3, 0, rHM.Line4.Column1);
    aM.set(3, 1, rHM.Line4.Column2);
    aM.set(3, 2, rHM.Line4.Column3);
    aM.set(3, 3, rHM.Line4.Column4);
    return aM;
}

::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM )
{
    ::basegfx::B2DHomMatrix aM;
    aM.set(0, 0, rM.get(0, 0));
    aM.set(0, 1, rM.get(0, 1));
    aM.set(0, 2, rM.get(0, 3));
    aM.set(1, 0, rM.get(1, 0));
    aM.set(1, 1, rM.get(1, 1));
    aM.set(1, 2, rM.get(1, 3));
    aM.set(2, 0, rM.get(3, 0));
    aM.set(2, 1, rM.get(3, 1));
    aM.set(2, 2, rM.get(3, 3));
    return aM;
}

drawing::HomogenMatrix3 B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM )
{
    drawing::HomogenMatrix3 aHM;
    aHM.Line1.Column1 = rM.get(0, 0);
    aHM.Line1.Column2 = rM.get(0, 1);
    aHM.Line1.Column3 = rM.get(0, 2);
    aHM.Line2.Column1 = rM.get(1, 0);
    aHM.Line2.Column2 = rM.get(1, 1);
    aHM.Line2.Column3 = rM.get(1, 2);
    aHM.Line3.Column1 = rM.get(2, 0);
    aHM.Line3.Column2 = rM.get(2, 1);
    aHM.Line3.Column3 = rM.get(2, 2);
    return aHM;
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

drawing::Position3D B3DPointToPosition3D( const ::basegfx::B3DPoint& rPoint)
{
        return drawing::Position3D(
          rPoint.getX()
        , rPoint.getY()
        , rPoint.getZ()
        );
}

::basegfx::B3DVector Direction3DToB3DVector( const drawing::Direction3D& rDirection)
{
    return ::basegfx::B3DVector(
          rDirection.DirectionX
        , rDirection.DirectionY
        , rDirection.DirectionZ
        );
}

void AddPointToPoly( drawing::PolyPolygonShape3D& rPoly, const drawing::Position3D& rPos, sal_Int32 nPolygonIndex )
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
            OSL_FAIL("polygon was accessed with a wrong index");
        }
    }
    else
    {
        OSL_FAIL("polygon was accessed with a wrong index");
    }
    return aRet;
}

void addPolygon( drawing::PolyPolygonShape3D& rRet, const drawing::PolyPolygonShape3D& rAdd )
{
    sal_Int32 nAddOuterCount = rAdd.SequenceX.getLength();
    sal_Int32 nOuterCount = rRet.SequenceX.getLength() + nAddOuterCount;
    rRet.SequenceX.realloc( nOuterCount );
    rRet.SequenceY.realloc( nOuterCount );
    rRet.SequenceZ.realloc( nOuterCount );

    sal_Int32 nIndex = 0;
    sal_Int32 nOuter = nOuterCount - nAddOuterCount;
    for( ; nOuter < nOuterCount; nOuter++ )
    {
        if( nIndex >= nAddOuterCount )
            break;

        rRet.SequenceX[nOuter] = rAdd.SequenceX[nIndex];
        rRet.SequenceY[nOuter] = rAdd.SequenceY[nIndex];
        rRet.SequenceZ[nOuter] = rAdd.SequenceZ[nIndex];

        nIndex++;
    }
}

void appendPoly( drawing::PolyPolygonShape3D& rRet, const drawing::PolyPolygonShape3D& rAdd )
{
    sal_Int32 nOuterCount = std::max( rRet.SequenceX.getLength(), rAdd.SequenceX.getLength() );
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

drawing::PolyPolygonShape3D BezierToPoly(
    const drawing::PolyPolygonBezierCoords& rBezier )
{
    const drawing::PointSequenceSequence& rPointSequence = rBezier.Coordinates;

    drawing::PolyPolygonShape3D aRet;
    aRet.SequenceX.realloc( rPointSequence.getLength() );
    aRet.SequenceY.realloc( rPointSequence.getLength() );
    aRet.SequenceZ.realloc( rPointSequence.getLength() );

    sal_Int32 nRealOuter = 0;
    for(sal_Int32 nN = 0; nN < rPointSequence.getLength(); nN++)
    {
        sal_Int32 nInnerLength = rPointSequence[nN].getLength();
        aRet.SequenceX[nN].realloc( nInnerLength );
        aRet.SequenceY[nN].realloc( nInnerLength );
        aRet.SequenceZ[nN].realloc( nInnerLength );

        bool bHasOuterFlags = nN < rBezier.Flags.getLength();

        sal_Int32 nRealInner = 0;
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            bool bHasInnerFlags = bHasOuterFlags && (nM < rBezier.Flags[nN].getLength());

            if( !bHasInnerFlags || (rBezier.Flags[nN][nM] == drawing::PolygonFlags_NORMAL) )
            {
                aRet.SequenceX[nRealOuter][nRealInner] = rPointSequence[nN][nM].X;
                aRet.SequenceY[nRealOuter][nRealInner] = rPointSequence[nN][nM].Y;
                aRet.SequenceZ[nRealOuter][nRealInner] = 0.0;
                nRealInner++;
            }
        }

        aRet.SequenceX[nRealOuter].realloc( nRealInner );
        aRet.SequenceY[nRealOuter].realloc( nRealInner );
        aRet.SequenceZ[nRealOuter].realloc( nRealInner );

        if( nRealInner>0 )
            nRealOuter++;
    }

    aRet.SequenceX.realloc( nRealOuter );
    aRet.SequenceY.realloc( nRealOuter );
    aRet.SequenceZ.realloc( nRealOuter );

    return aRet;
}

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

void appendPointSequence( drawing::PointSequenceSequence& rTarget
                        , drawing::PointSequenceSequence& rAdd )
{
    sal_Int32 nAddCount = rAdd.getLength();
    if(!nAddCount)
        return;
    sal_Int32 nOldCount = rTarget.getLength();

    rTarget.realloc(nOldCount+nAddCount);
    for(sal_Int32 nS=0; nS<nAddCount; nS++ )
        rTarget[nOldCount+nS]=rAdd[nS];
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

drawing::Direction3D  operator-( const drawing::Position3D& rPos1
                           , const drawing::Position3D& rPos2)
{
    return drawing::Direction3D(
          rPos1.PositionX - rPos2.PositionX
        , rPos1.PositionY - rPos2.PositionY
        , rPos1.PositionZ - rPos2.PositionZ
        );
}

bool operator==( const drawing::Position3D& rPos1
                           , const drawing::Position3D& rPos2)
{
    return rPos1.PositionX == rPos2.PositionX
        && rPos1.PositionY == rPos2.PositionY
        && rPos1.PositionZ == rPos2.PositionZ;
}

awt::Point Position3DToAWTPoint( const drawing::Position3D& rPos )
{
    awt::Point aRet;
    aRet.X = static_cast<sal_Int32>(rPos.PositionX);
    aRet.Y = static_cast<sal_Int32>(rPos.PositionY);
    return aRet;
}

awt::Point ToPoint( const awt::Rectangle& rRectangle )
{
    return awt::Point( rRectangle.X, rRectangle.Y );
}

awt::Size ToSize( const awt::Rectangle& rRectangle )
{
    return awt::Size( rRectangle.Width, rRectangle.Height );
}

awt::Size Direction3DToAWTSize( const drawing::Direction3D& rDirection )
{
    awt::Size aRet;
    aRet.Width = static_cast<sal_Int32>(rDirection.DirectionX);
    aRet.Height = static_cast<sal_Int32>(rDirection.DirectionY);
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

using namespace ::com::sun::star::chart2;

uno::Sequence< double > DataSequenceToDoubleSequence(
    const uno::Reference< data::XDataSequence >& xDataSequence )
{
    uno::Sequence< double > aResult;
    OSL_ASSERT( xDataSequence.is());
    if(!xDataSequence.is())
        return aResult;

    uno::Reference< data::XNumericalDataSequence > xNumericalDataSequence( xDataSequence, uno::UNO_QUERY );
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
                ::rtl::math::setNan( &aResult[nN] );
        }
    }

    return aResult;
}

uno::Sequence< OUString > DataSequenceToStringSequence(
    const uno::Reference< data::XDataSequence >& xDataSequence )
{
    uno::Sequence< OUString > aResult;
    if(!xDataSequence.is())
        return aResult;

    uno::Reference< data::XTextualDataSequence > xTextualDataSequence( xDataSequence, uno::UNO_QUERY );
    if( xTextualDataSequence.is() )
    {
        aResult = xTextualDataSequence->getTextualData();
    }
    else
    {
        uno::Sequence< uno::Any > aValues = xDataSequence->getData();
        aResult.realloc(aValues.getLength());

        for(sal_Int32 nN=aValues.getLength();nN--;)
            aValues[nN] >>= aResult[nN];
    }

    return aResult;
}

sal_Bool hasDoubleValue( const uno::Any& rAny )
{
    sal_Bool bRet = sal_False;
    double fValue = 0.0;
    if( rAny >>= fValue )
        bRet = sal_True;
    return bRet;
}

sal_Bool hasLongOrShortValue( const uno::Any& rAny )
{
    sal_Bool bRet = sal_False;
    sal_Int32 n32 = 0;
    if( rAny >>= n32 )
        bRet = sal_True;
    else
    {
        sal_Int16 n16 = 0;
        if( rAny >>= n16 )
            bRet = sal_True;
    }
    return bRet;
}
sal_Int16 getShortForLongAlso( const uno::Any& rAny )
{
    sal_Int16 nRet = 0;

    if( !(rAny >>= nRet) )
    {
        sal_Int32 n32 = 0;
        if( rAny >>= n32 )
            nRet = static_cast<sal_Int16>(n32);
    }
    return nRet;
}

bool replaceParamterInString( OUString & rInOutResourceString,
                            const OUString & rParamToReplace,
                            const OUString & rReplaceWith )
{
    sal_Int32 nPos = rInOutResourceString.indexOf( rParamToReplace );
    if( nPos == -1 )
        return false;

    rInOutResourceString = rInOutResourceString.replaceAt( nPos
                        , rParamToReplace.getLength(), rReplaceWith );
    return true;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
