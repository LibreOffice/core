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
#pragma once

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace com::sun::star::awt { struct Rectangle; }
namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::drawing { struct PolyPolygonBezierCoords; }

namespace chart
{

/**
diverse methods for class conversions; e.g. ::basegfx::B3DHomMatrix to HomogenMatrix
and operations e.g  drawing::Position3D + drawing::Direction3D
*/

/** ::basegfx::B3DHomMatrix -> HomogenMatrix
*/
css::drawing::HomogenMatrix
                 B3DHomMatrixToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM );

/** HomogenMatrix -> ::basegfx::B3DHomMatrix
*/
::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix( const css::drawing::HomogenMatrix& rHM );

/** ::basegfx::B3DHomMatrix -> B2DHomMatrix
*/

::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM );

/** B2DHomMatrix <-> HomogenMatrix3
*/
css::drawing::HomogenMatrix3
                B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM );

/** Position3D -> B3DPoint
*/
::basegfx::B3DPoint Position3DToB3DPoint( const css::drawing::Position3D& rPosition );

/** B3DVector -> Direction3D
*/
css::drawing::Direction3D B3DVectorToDirection3D( const ::basegfx::B3DVector& rVector);

/** B3DPoint -> Position3D
*/
css::drawing::Position3D B3DPointToPosition3D( const ::basegfx::B3DPoint& rPoint);

/** Direction3D -> B3DVector
*/
::basegfx::B3DVector Direction3DToB3DVector( const css::drawing::Direction3D& rDirection);

/** PolyPolygonShape3D + drawing::Position3D -> PolyPolygonShape3D
*/

void AddPointToPoly( css::drawing::PolyPolygonShape3D& rPoly
                , const css::drawing::Position3D& rPos
                , sal_Int32 nSequenceIndex=0 );

void AddPointToPoly( std::vector<std::vector<css::drawing::Position3D>>& rPoly
                , const css::drawing::Position3D& rPos
                , sal_Int32 nSequenceIndex=0 );

/** get a single Point from a Polygon
*/
css::drawing::Position3D getPointFromPoly(
                  const std::vector<std::vector<css::drawing::Position3D>>& rPolygon
                , sal_Int32 nPointIndex, sal_Int32 nPolyIndex );
css::drawing::Position3D getPointFromPoly(
                  const css::drawing::PolyPolygonShape3D& rPolygon
                , sal_Int32 nPointIndex, sal_Int32 nPolyIndex );


void addPolygon( std::vector<std::vector<css::drawing::Position3D>>& rRet
                , const std::vector<std::vector<css::drawing::Position3D>>& rAdd );
/** PolyPolygonShape3D + PolyPolygonShape3D -> PolyPolygonShape3D
*/

void appendPoly( std::vector<std::vector<css::drawing::Position3D>>& rRet
                , const std::vector<std::vector<css::drawing::Position3D>>& rAdd );

/** PolyPolygonBezierCoords -> PolyPolygonShape3D
*/

css::drawing::PolyPolygonShape3D BezierToPoly(
    const css::drawing::PolyPolygonBezierCoords& rBezier );

/** PolyPolygonShape3D -> drawing::PointSequenceSequence (2D)
*/

css::drawing::PointSequenceSequence PolyToPointSequence(
                const css::drawing::PolyPolygonShape3D& rPolyPolygon );

css::drawing::PointSequenceSequence PolyToPointSequence(
                const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon );

/** PolyPolygonShape3D ->  basegfx::B2DPolyPolygon (2D)
*/

basegfx::B2DPolyPolygon PolyToB2DPolyPolygon(
                const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon );

/** drawing::PointSequenceSequence + drawing::PointSequenceSequence
*/

void appendPointSequence( css::drawing::PointSequenceSequence& rTarget
                        , const css::drawing::PointSequenceSequence& rAdd );

/** Position3D + Direction3D == Position3D
*/
css::drawing::Position3D
                operator+( const css::drawing::Position3D& rPos
                           , const css::drawing::Direction3D& rDirection);

/** Position3D - Position3D == Direction3D
*/
css::drawing::Direction3D
                operator-( const css::drawing::Position3D& rPos1
                           , const css::drawing::Position3D& rPos2);

/** awt::Rect --> awt::Point (2D)
*/
css::awt::Point ToPoint( const css::awt::Rectangle& rRectangle );

/** awt::Rect --> awt::Size (2D)
*/
css::awt::Size ToSize( const css::awt::Rectangle& rRectangle );

/** Position3D --> awt::Point (2D)
*/
css::awt::Point Position3DToAWTPoint( const css::drawing::Position3D& rPos );

/** Direction3D --> awt::Size (2D)
*/
css::awt::Size Direction3DToAWTSize( const css::drawing::Direction3D& rDirection );

/** Sequence<double> -> drawing::Position3D
*/
css::drawing::Position3D
                SequenceToPosition3D( const css::uno::Sequence< double >& rSeq );

/** chart2::XDataSequence -> uno::Sequence< double >
*/


css::uno::Sequence< double > DataSequenceToDoubleSequence(
    const css::uno::Reference< css::chart2::data::XDataSequence > & xDataSequence );


css::uno::Sequence< OUString > DataSequenceToStringSequence(
    const css::uno::Reference< css::chart2::data::XDataSequence > & xDataSequence );

/** std::vector< std::vector< T > > -> std::vector< T >
 */
template< typename T >
std::vector< T >
    FlattenSequence( const std::vector< std::vector< T > > & aSeqSeq )
{
    sal_Int32 nOuter, nInner, nCount = 0,
        nResultSize = 0;
    const sal_Int32 nOuterSize = aSeqSeq.size();
    for( nOuter=0; nOuter<nOuterSize; ++nOuter )
        nResultSize += aSeqSeq[nOuter].size();
    std::vector< T > aResult( nResultSize );

    for( nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        const sal_Int32 nInnerSize = aSeqSeq[nOuter].size();
        for( nInner=0; nInner<nInnerSize; ++nInner, ++nCount )
            aResult[nCount] = aSeqSeq[nOuter][nInner];
    }
    return aResult;
}


bool hasDoubleValue( const css::uno::Any& rAny );


bool  hasLongOrShortValue( const css::uno::Any& rAny );

sal_Int16 getShortForLongAlso( const css::uno::Any& rAny );


bool replaceParamterInString( OUString & rInOutResourceString,
                            std::u16string_view rParamToReplace,
                            std::u16string_view rReplaceWith );

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
