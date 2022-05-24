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

#include <CommonConverters.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <limits>

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

void AddPointToPoly( std::vector<std::vector<css::drawing::Position3D>>& rPoly, const drawing::Position3D& rPos, sal_Int32 nPolygonIndex )
{
    if(nPolygonIndex<0)
    {
        OSL_FAIL( "The polygon index needs to be > 0");
        nPolygonIndex=0;
    }

    //make sure that we have enough polygons
    if(nPolygonIndex >= static_cast<sal_Int32>(rPoly.size()) )
    {
        rPoly.resize(nPolygonIndex+1);
    }

    std::vector<css::drawing::Position3D>* pOuterSequence = &rPoly[nPolygonIndex];
    pOuterSequence->push_back(rPos);
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

drawing::Position3D getPointFromPoly( const std::vector<std::vector<css::drawing::Position3D>>& rPolygon, sal_Int32 nPointIndex, sal_Int32 nPolyIndex )
{
    drawing::Position3D aRet(0.0,0.0,0.0);

    if( nPolyIndex>=0 && o3tl::make_unsigned(nPolyIndex)<rPolygon.size())
    {
        if(nPointIndex<static_cast<sal_Int32>(rPolygon[nPolyIndex].size()))
        {
            aRet = rPolygon[nPolyIndex][nPointIndex];
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

void addPolygon( std::vector<std::vector<css::drawing::Position3D>>& rRet, const std::vector<std::vector<css::drawing::Position3D>>& rAdd )
{
    sal_Int32 nAddOuterCount = rAdd.size();
    sal_Int32 nOuterCount = rRet.size() + nAddOuterCount;
    rRet.resize( nOuterCount );
    auto pSequence = rRet.data();

    sal_Int32 nIndex = 0;
    sal_Int32 nOuter = nOuterCount - nAddOuterCount;
    for( ; nOuter < nOuterCount; nOuter++ )
    {
        if( nIndex >= nAddOuterCount )
            break;

        pSequence[nOuter] = rAdd[nIndex];

        nIndex++;
    }
}

void appendPoly( std::vector<std::vector<css::drawing::Position3D>>& rRet, const std::vector<std::vector<css::drawing::Position3D>>& rAdd )
{
    sal_Int32 nOuterCount = std::max( rRet.size(), rAdd.size() );
    rRet.resize(nOuterCount);
    auto pSequence = rRet.data();

    for( sal_Int32 nOuter=0;nOuter<nOuterCount;nOuter++ )
    {
        sal_Int32 nOldPointCount = rRet[nOuter].size();
        sal_Int32 nAddPointCount = 0;
        if(nOuter<static_cast<sal_Int32>(rAdd.size()))
            nAddPointCount = rAdd[nOuter].size();
        if(!nAddPointCount)
            continue;

        sal_Int32 nNewPointCount = nOldPointCount + nAddPointCount;

        pSequence[nOuter].resize(nNewPointCount);
        auto pSequence_nOuter = pSequence[nOuter].data();

        sal_Int32 nPointTarget=nOldPointCount;
        sal_Int32 nPointSource=nAddPointCount;
        for( ; nPointSource-- ; nPointTarget++ )
        {
            pSequence_nOuter[nPointTarget] = rAdd[nOuter][nPointSource];
        }
    }
}

drawing::PolyPolygonShape3D BezierToPoly(
    const drawing::PolyPolygonBezierCoords& rBezier )
{
    const drawing::PointSequenceSequence& rPointSequence = rBezier.Coordinates;

    drawing::PolyPolygonShape3D aRet;
    aRet.SequenceX.realloc( rPointSequence.getLength() );
    auto pSequenceX = aRet.SequenceX.getArray();
    aRet.SequenceY.realloc( rPointSequence.getLength() );
    auto pSequenceY = aRet.SequenceY.getArray();
    aRet.SequenceZ.realloc( rPointSequence.getLength() );
    auto pSequenceZ = aRet.SequenceZ.getArray();

    sal_Int32 nRealOuter = 0;
    for(sal_Int32 nN = 0; nN < rPointSequence.getLength(); nN++)
    {
        sal_Int32 nInnerLength = rPointSequence[nN].getLength();
        pSequenceX[nRealOuter].realloc( nInnerLength );
        auto pSequenceX_nRealOuter = pSequenceX[nRealOuter].getArray();
        pSequenceY[nRealOuter].realloc( nInnerLength );
        auto pSequenceY_nRealOuter = pSequenceY[nRealOuter].getArray();
        pSequenceZ[nRealOuter].realloc( nInnerLength );
        auto pSequenceZ_nRealOuter = pSequenceZ[nRealOuter].getArray();

        bool bHasOuterFlags = nN < rBezier.Flags.getLength();

        sal_Int32 nRealInner = 0;
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            bool bHasInnerFlags = bHasOuterFlags && (nM < rBezier.Flags[nN].getLength());

            if( !bHasInnerFlags || (rBezier.Flags[nN][nM] == drawing::PolygonFlags_NORMAL) )
            {
                pSequenceX_nRealOuter[nRealInner] = rPointSequence[nN][nM].X;
                pSequenceY_nRealOuter[nRealInner] = rPointSequence[nN][nM].Y;
                pSequenceZ_nRealOuter[nRealInner] = 0.0;
                nRealInner++;
            }
        }

        pSequenceX[nRealOuter].realloc( nRealInner );
        pSequenceY[nRealOuter].realloc( nRealInner );
        pSequenceZ[nRealOuter].realloc( nRealInner );

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
    auto pRet = aRet.getArray();

    for(sal_Int32 nN = 0; nN < rPolyPolygon.SequenceX.getLength(); nN++)
    {
        sal_Int32 nInnerLength = rPolyPolygon.SequenceX[nN].getLength();
        pRet[nN].realloc( nInnerLength );
        auto pRet_nN = pRet[nN].getArray();
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            pRet_nN[nM].X = static_cast<sal_Int32>(rPolyPolygon.SequenceX[nN][nM]);
            pRet_nN[nM].Y = static_cast<sal_Int32>(rPolyPolygon.SequenceY[nN][nM]);
        }
    }
    return aRet;
}

drawing::PointSequenceSequence PolyToPointSequence(
                const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon )
{
    drawing::PointSequenceSequence aRet;
    aRet.realloc( rPolyPolygon.size() );
    auto pRet = aRet.getArray();

    for(sal_Int32 nN = 0; nN < static_cast<sal_Int32>(rPolyPolygon.size()); nN++)
    {
        sal_Int32 nInnerLength = rPolyPolygon[nN].size();
        pRet[nN].realloc( nInnerLength );
        auto pRet_nN = pRet[nN].getArray();
        for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
        {
            pRet_nN[nM].X = static_cast<sal_Int32>(rPolyPolygon[nN][nM].PositionX);
            pRet_nN[nM].Y = static_cast<sal_Int32>(rPolyPolygon[nN][nM].PositionY);
        }
    }
    return aRet;
}

basegfx::B2DPolyPolygon PolyToB2DPolyPolygon(
                const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon )
{
    basegfx::B2DPolyPolygon aRetval;

    for(sal_Int32 nN = 0; nN < static_cast<sal_Int32>(rPolyPolygon.size()); nN++)
    {
        basegfx::B2DPolygon aNewPolygon;
        sal_Int32 nInnerLength = rPolyPolygon[nN].size();
        if(nInnerLength)
        {
            aNewPolygon.reserve(nInnerLength);
            for( sal_Int32 nM = 0; nM < nInnerLength; nM++)
            {
                auto X = static_cast<sal_Int32>(rPolyPolygon[nN][nM].PositionX);
                auto Y = static_cast<sal_Int32>(rPolyPolygon[nN][nM].PositionY);
                aNewPolygon.append(basegfx::B2DPoint(X, Y));
            }
            // check for closed state flag
            basegfx::utils::checkClosed(aNewPolygon);
        }
        aRetval.append(std::move(aNewPolygon));
    }

    return aRetval;
}

void appendPointSequence( drawing::PointSequenceSequence& rTarget
                        , const drawing::PointSequenceSequence& rAdd )
{
    sal_Int32 nAddCount = rAdd.getLength();
    if(!nAddCount)
        return;
    sal_Int32 nOldCount = rTarget.getLength();

    rTarget.realloc(nOldCount+nAddCount);
    auto pTarget = rTarget.getArray();
    for(sal_Int32 nS=0; nS<nAddCount; nS++ )
        pTarget[nOldCount+nS]=rAdd[nS];
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

drawing::Position3D SequenceToPosition3D( const uno::Sequence< double >& rSeq )
{
    OSL_ENSURE(rSeq.getLength()==3,"The sequence needs to have length 3 for conversion into vector");

    drawing::Position3D aRet;
    aRet.PositionX = rSeq.getLength()>0?rSeq[0]:0.0;
    aRet.PositionY = rSeq.getLength()>1?rSeq[1]:0.0;
    aRet.PositionZ = rSeq.getLength()>2?rSeq[2]:0.0;
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
        auto pResult = aResult.getArray();
        for(sal_Int32 nN=aValues.getLength();nN--;)
        {
            if( !(aValues[nN] >>= pResult[nN]) )
                pResult[nN] = std::numeric_limits<double>::quiet_NaN();
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
        auto pResult = aResult.getArray();

        for(sal_Int32 nN=aValues.getLength();nN--;)
            aValues[nN] >>= pResult[nN];
    }

    return aResult;
}

bool hasDoubleValue( const uno::Any& rAny )
{
    bool bRet = false;
    double fValue = 0.0;
    if( rAny >>= fValue )
        bRet = true;
    return bRet;
}

bool hasLongOrShortValue( const uno::Any& rAny )
{
    bool bRet = false;
    sal_Int32 n32 = 0;
    if( rAny >>= n32 )
        bRet = true;
    else
    {
        sal_Int16 n16 = 0;
        if( rAny >>= n16 )
            bRet = true;
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
                            std::u16string_view rReplaceWith )
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
