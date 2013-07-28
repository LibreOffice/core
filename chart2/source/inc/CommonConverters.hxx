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
#ifndef __CHART_COMMON_CONVERTERS_HXX
#define __CHART_COMMON_CONVERTERS_HXX

#include <tools/poly.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include "charttoolsdllapi.hxx"

#include <vector>
#include <algorithm>

namespace chart
{

/**
diverse methods for class conversions; e.g. ::basegfx::B3DHomMatrix to HomogenMatrix
and operations e.g  drawing::Position3D + drawing::Direction3D
*/

/** ::basegfx::B3DHomMatrix -> HomogenMatrix
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::HomogenMatrix
                 B3DHomMatrixToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM );

/** HomogenMatrix -> ::basegfx::B3DHomMatrix
*/
OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix( const com::sun::star::drawing::HomogenMatrix& rHM );

/** ::basegfx::B3DHomMatrix -> B2DHomMatrix
*/
OOO_DLLPUBLIC_CHARTTOOLS
::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM );

/** B2DHomMatrix <-> HomogenMatrix3
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::HomogenMatrix3
                B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM );

/** Position3D -> B3DPoint
*/
OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DPoint Position3DToB3DPoint( const com::sun::star::drawing::Position3D& rPosition );

/** B3DVector -> Direction3D
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::Direction3D B3DVectorToDirection3D( const ::basegfx::B3DVector& rVector);

/** B3DPoint -> Position3D
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::Position3D B3DPointToPosition3D( const ::basegfx::B3DPoint& rPoint);

/** Direction3D -> B3DVector
*/
OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DVector Direction3DToB3DVector( const com::sun::star::drawing::Direction3D& rDirection);

/** PolyPolygonShape3D + drawing::Position3D -> PolyPolygonShape3D
*/
OOO_DLLPUBLIC_CHARTTOOLS
void AddPointToPoly( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly
                , const com::sun::star::drawing::Position3D& rPos
                , sal_Int32 nSequenceIndex=0 );

/** get a single Point from a Polygon
*/
OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::drawing::Position3D getPointFromPoly(
                  const ::com::sun::star::drawing::PolyPolygonShape3D& rPolygon
                , sal_Int32 nPointIndex, sal_Int32 nPolyIndex=0 );

OOO_DLLPUBLIC_CHARTTOOLS
void addPolygon( com::sun::star::drawing::PolyPolygonShape3D& rRet
                , const com::sun::star::drawing::PolyPolygonShape3D& rAdd );
/** PolyPolygonShape3D + PolyPolygonShape3D -> PolyPolygonShape3D
*/
OOO_DLLPUBLIC_CHARTTOOLS
void appendPoly( com::sun::star::drawing::PolyPolygonShape3D& rRet
                , const com::sun::star::drawing::PolyPolygonShape3D& rAdd );

/** PolyPolygonBezierCoords -> PolyPolygonShape3D
*/
OOO_DLLPUBLIC_CHARTTOOLS
com::sun::star::drawing::PolyPolygonShape3D BezierToPoly(
    const com::sun::star::drawing::PolyPolygonBezierCoords& rBezier );

/** PolyPolygonShape3D -> drawing::PointSequenceSequence (2D)
*/
OOO_DLLPUBLIC_CHARTTOOLS
com::sun::star::drawing::PointSequenceSequence PolyToPointSequence(
                const com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon );

/** drawing::PointSequenceSequence + drawing::PointSequenceSequence
*/
OOO_DLLPUBLIC_CHARTTOOLS
void appendPointSequence( com::sun::star::drawing::PointSequenceSequence& rTarget
                        , com::sun::star::drawing::PointSequenceSequence& rAdd );

/** Position3D + Direction3D == Position3D
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::Position3D
                operator+( const com::sun::star::drawing::Position3D& rPos
                           , const com::sun::star::drawing::Direction3D& rDirection);

/** Position3D - Position3D == Direction3D
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::Direction3D
                operator-( const com::sun::star::drawing::Position3D& rPos1
                           , const com::sun::star::drawing::Position3D& rPos2);

/** Position3D == Position3D ?
*/
OOO_DLLPUBLIC_CHARTTOOLS
bool            operator==( const com::sun::star::drawing::Position3D& rPos1
                           , const com::sun::star::drawing::Position3D& rPos2);

/** awt::Rect --> awt::Point (2D)
*/
OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::awt::Point ToPoint( const com::sun::star::awt::Rectangle& rRectangle );

/** awt::Rect --> awt::Size (2D)
*/
OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::awt::Size ToSize( const com::sun::star::awt::Rectangle& rRectangle );

/** Position3D --> awt::Point (2D)
*/
OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::awt::Point Position3DToAWTPoint( const com::sun::star::drawing::Position3D& rPos );

/** Direction3D --> awt::Size (2D)
*/
OOO_DLLPUBLIC_CHARTTOOLS ::com::sun::star::awt::Size Direction3DToAWTSize( const com::sun::star::drawing::Direction3D& rDirection );

/** B3DPoint -> Sequence<double>
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::uno::Sequence< double > B3DPointToSequence( const ::basegfx::B3DPoint& rPoint );

/** Sequence<double> -> drawing::Position3D
*/
OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::drawing::Position3D
                SequenceToPosition3D( const com::sun::star::uno::Sequence< double >& rSeq );

/** drawing::Position3D -> Sequence<double>
*/

OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::uno::Sequence< double >
                Position3DToSequence( const com::sun::star::drawing::Position3D& rPosition );

/** chart2::XDataSequence -> uno::Sequence< double >
*/

OOO_DLLPUBLIC_CHARTTOOLS
::com::sun::star::uno::Sequence< double > DataSequenceToDoubleSequence(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > & xDataSequence );

OOO_DLLPUBLIC_CHARTTOOLS
::com::sun::star::uno::Sequence< OUString > DataSequenceToStringSequence(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > & xDataSequence );

/** uno::Sequence< uno::Sequence< T > > -> uno::Sequence< T >
 */
template< typename T >
::com::sun::star::uno::Sequence< T >
    FlattenSequence( const ::com::sun::star::uno::Sequence<
                         ::com::sun::star::uno::Sequence< T > > & aSeqSeq )
{
    sal_Int32 nOuter, nInner, nCount = 0,
        nResultSize = 0;
    const sal_Int32 nOuterSize = aSeqSeq.getLength();
    for( nOuter=0; nOuter<nOuterSize; ++nOuter )
        nResultSize += aSeqSeq[nOuter].getLength();
    ::com::sun::star::uno::Sequence< T > aResult( nResultSize );

    for( nOuter=0; nOuter<nOuterSize; ++nOuter )
    {
        const sal_Int32 nInnerSize = aSeqSeq[nOuter].getLength();
        for( nInner=0; nInner<nInnerSize; ++nInner, ++nCount )
            aResult[nCount] = aSeqSeq[nOuter][nInner];
    }
    return aResult;
}

template< typename T >
    ::std::vector< T >
    FlattenVector( const ::std::vector< ::std::vector< T > > & rVecVec )
{
    typedef ::std::vector< T > tFlatVec;
    typedef ::std::vector< tFlatVec > tVecVec;

    tFlatVec aResult;
    typename tVecVec::const_iterator aOuterEnd( rVecVec.end());
    for( typename tVecVec::const_iterator aOuterIt( rVecVec.begin()); aOuterIt != aOuterEnd; ++aOuterIt )
        ::std::copy( aOuterIt->begin(), aOuterIt->end(), back_inserter( aResult ));
    return aResult;
}

OOO_DLLPUBLIC_CHARTTOOLS
sal_Bool hasDoubleValue( const ::com::sun::star::uno::Any& rAny );

OOO_DLLPUBLIC_CHARTTOOLS
sal_Bool  hasLongOrShortValue( const ::com::sun::star::uno::Any& rAny );
OOO_DLLPUBLIC_CHARTTOOLS
sal_Int16 getShortForLongAlso( const ::com::sun::star::uno::Any& rAny );

OOO_DLLPUBLIC_CHARTTOOLS
bool replaceParamterInString( OUString & rInOutResourceString,
                            const OUString & rParamToReplace,
                            const OUString & rReplaceWith );

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
