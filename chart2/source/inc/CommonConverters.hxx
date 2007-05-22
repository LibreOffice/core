/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommonConverters.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:13:46 $
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
#ifndef __CHART_COMMON_CONVERTERS_HXX
#define __CHART_COMMON_CONVERTERS_HXX

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX3_HPP_
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

#include <vector>
#include <algorithm>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
diverse methods for class conversions; e.g. ::basegfx::B3DHomMatrix to HomogenMatrix
and operations e.g  drawing::Position3D + drawing::Direction3D
*/

//-----------------------------------------------------------------------------
/** ::basegfx::B3DHomMatrix -> HomogenMatrix
*/
com::sun::star::drawing::HomogenMatrix
                 B3DHomMatrixToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM );

//-----------------------------------------------------------------------------
/** HomogenMatrix -> ::basegfx::B3DHomMatrix
*/
::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix( const com::sun::star::drawing::HomogenMatrix& rHM );

//-----------------------------------------------------------------------------
/** ::basegfx::B3DHomMatrix -> B2DHomMatrix
*/
::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM );

//-----------------------------------------------------------------------------
/** B2DHomMatrix <-> HomogenMatrix3
*/
com::sun::star::drawing::HomogenMatrix3
                B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM );

::basegfx::B2DHomMatrix HomogenMatrix3ToB2DHomMatrix( const com::sun::star::drawing::HomogenMatrix3& rHM );

//-----------------------------------------------------------------------------
/** Position3D -> B3DPoint
*/
::basegfx::B3DPoint Position3DToB3DPoint( const com::sun::star::drawing::Position3D& rPosition );

//-----------------------------------------------------------------------------
/** B3DVector -> Direction3D
*/
com::sun::star::drawing::Direction3D B3DVectorToDirection3D( const ::basegfx::B3DVector& rVector);

//-----------------------------------------------------------------------------
/** B3DVector -> Position3D
*/
com::sun::star::drawing::Position3D B3DVectorToPosition3D( const ::basegfx::B3DVector& rVector);

//-----------------------------------------------------------------------------
/** B3DPoint -> Position3D
*/
com::sun::star::drawing::Position3D B3DPointToPosition3D( const ::basegfx::B3DPoint& rPoint);

//-----------------------------------------------------------------------------
/** Direction3D -> B3DPoint
*/
::basegfx::B3DPoint Direction3DToB3DPoint( const com::sun::star::drawing::Direction3D& rDirection);

//-----------------------------------------------------------------------------
/** two drawing::Position3D -> PolyPolygonShape3D
*/
::com::sun::star::drawing::PolyPolygonShape3D
    MakeLine3D( const ::com::sun::star::drawing::Position3D & rStart,
                const ::com::sun::star::drawing::Position3D & rEnd );

//-----------------------------------------------------------------------------
/** PolyPolygonShape3D + drawing::Position3D -> PolyPolygonShape3D
*/
void AddPointToPoly( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly
                , const com::sun::star::drawing::Position3D& rPos
                , sal_Int32 nSequenceIndex=0 );

//-----------------------------------------------------------------------------
/** get a single Point from a Polygon
*/
::com::sun::star::drawing::Position3D getPointFromPoly(
                  const ::com::sun::star::drawing::PolyPolygonShape3D& rPolygon
                , sal_Int32 nPointIndex, sal_Int32 nPolyIndex=0 );

//-----------------------------------------------------------------------------
/** PolyPolygonShape3D + PolyPolygonShape3D -> PolyPolygonShape3D
*/
void appendPoly( com::sun::star::drawing::PolyPolygonShape3D& rRet
                , const com::sun::star::drawing::PolyPolygonShape3D& rAdd );

//-----------------------------------------------------------------------------
/** PolyPolygonBezierCoords -> PolyPolygonShape3D
*/
com::sun::star::drawing::PolyPolygonShape3D BezierToPoly(
    const com::sun::star::drawing::PolyPolygonBezierCoords& rBezier );

//-----------------------------------------------------------------------------
/** drawing::PointSequenceSequence (2D) -> PolyPolygonShape3D
*/
com::sun::star::drawing::PolyPolygonShape3D PointSequenceToPoly(
                const com::sun::star::drawing::PointSequenceSequence& rPointSequence );

//-----------------------------------------------------------------------------
/** PolyPolygonShape3D -> drawing::PointSequenceSequence (2D)
*/
com::sun::star::drawing::PointSequenceSequence PolyToPointSequence(
                const com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon );

//-----------------------------------------------------------------------------
/** drawing::PointSequenceSequence + drawing::PointSequenceSequence
*/
void appendPointSequence( com::sun::star::drawing::PointSequenceSequence& rTarget
                        , com::sun::star::drawing::PointSequenceSequence& rAdd );

//-----------------------------------------------------------------------------
/** PolyPolygonShape3D -> Polygon (2D,integer!)
*/
Polygon PolyToToolsPoly(
                const com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon );

//-----------------------------------------------------------------------------
/** tools Polygon (2D,integer!) -> PolyPolygonShape3D
*/
com::sun::star::drawing::PolyPolygonShape3D ToolsPolyToPoly(
                const Polygon& rToolsPoly, double zValue  );


//-----------------------------------------------------------------------------
/** Position3D + Direction3D == Position3D
*/
com::sun::star::drawing::Position3D
                operator+( const com::sun::star::drawing::Position3D& rPos
                           , const com::sun::star::drawing::Direction3D& rDirection);

//-----------------------------------------------------------------------------
/** Direction3D + Direction3D == Direction3D
*/
com::sun::star::drawing::Direction3D
                operator+( const com::sun::star::drawing::Direction3D& rDirection
                           , const com::sun::star::drawing::Direction3D& rDirectionAdd);

//-----------------------------------------------------------------------------
/** Position3D - Direction3D == Position3D
*/
com::sun::star::drawing::Position3D
                operator-( const com::sun::star::drawing::Position3D& rPos
                           , const com::sun::star::drawing::Direction3D& rDirection);


//-----------------------------------------------------------------------------
/** Position3D - Position3D == Direction3D
*/
com::sun::star::drawing::Direction3D
                operator-( const com::sun::star::drawing::Position3D& rPos1
                           , const com::sun::star::drawing::Position3D& rPos2);

//-----------------------------------------------------------------------------
/** Direction3D * double == Direction3D
*/
com::sun::star::drawing::Direction3D
                operator*( const com::sun::star::drawing::Direction3D& rDirection
                           , double fFraction);

//-----------------------------------------------------------------------------
/** Position3D == Position3D ?
*/
bool            operator==( const com::sun::star::drawing::Position3D& rPos1
                           , const com::sun::star::drawing::Position3D& rPos2);

//-----------------------------------------------------------------------------
/** awt::Rect --> awt::Point (2D)
*/
::com::sun::star::awt::Point ToPoint( const com::sun::star::awt::Rectangle& rRectangle );

//-----------------------------------------------------------------------------
/** awt::Rect --> awt::Size (2D)
*/
::com::sun::star::awt::Size ToSize( const com::sun::star::awt::Rectangle& rRectangle );

//-----------------------------------------------------------------------------
/** Position3D --> awt::Point (2D)
*/
::com::sun::star::awt::Point Position3DToAWTPoint( const com::sun::star::drawing::Position3D& rPos );

//-----------------------------------------------------------------------------
/** Direction3D --> awt::Size (2D)
*/
::com::sun::star::awt::Size Direction3DToAWTSize( const com::sun::star::drawing::Direction3D& rDirection );

//-----------------------------------------------------------------------------
/** Sequence<double> -> B3DPoint
*/
::basegfx::B3DPoint SequenceToB3DPoint( const com::sun::star::uno::Sequence< double >& rSeq );

//-----------------------------------------------------------------------------
/** B3DPoint -> Sequence<double>
*/
com::sun::star::uno::Sequence< double > B3DPointToSequence( const ::basegfx::B3DPoint& rPoint );

//-----------------------------------------------------------------------------
/** Sequence<double> -> drawing::Position3D
*/
com::sun::star::drawing::Position3D
                SequenceToPosition3D( const com::sun::star::uno::Sequence< double >& rSeq );

//-----------------------------------------------------------------------------
/** drawing::Position3D -> Sequence<double>
*/

com::sun::star::uno::Sequence< double >
                Position3DToSequence( const com::sun::star::drawing::Position3D& rPosition );

//-----------------------------------------------------------------------------
/** Sequence<double> -> drawing::Direction3D
*/
com::sun::star::drawing::Direction3D
                SequenceToDirection3D( const com::sun::star::uno::Sequence< double >& rSeq );

//-----------------------------------------------------------------------------
/** drawing::Direction3D -> Sequence<double>
*/

com::sun::star::uno::Sequence< double >
                Direction3DToSequence( const com::sun::star::drawing::Direction3D& rDirection );

//-----------------------------------------------------------------------------
/** drawing::Direction3D / double
*/

com::sun::star::drawing::Direction3D
                operator/( const com::sun::star::drawing::Direction3D& rDirection, double f );

//-----------------------------------------------------------------------------
/** text::WritingMode2 -> text::WritingMode
*/

com::sun::star::text::WritingMode WritingMode2ToWritingMode1( sal_Int16 nWritingMode );

//-----------------------------------------------------------------------------
/** chart2::XDataSequence -> uno::Sequence< double >
*/

::com::sun::star::uno::Sequence< double > DataSequenceToDoubleSequence(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > & xDataSequence );


::com::sun::star::uno::Sequence< rtl::OUString > DataSequenceToStringSequence(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSequence > & xDataSequence );

//-----------------------------------------------------------------------------
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

sal_Bool hasDoubleValue( const ::com::sun::star::uno::Any& rAny );

sal_Bool  hasLongOrShortValue( const ::com::sun::star::uno::Any& rAny );
sal_Int16 getShortForLongAlso( const ::com::sun::star::uno::Any& rAny );

bool replaceParamterInString( rtl::OUString & rInOutResourceString,
                            const rtl::OUString & rParamToReplace,
                            const rtl::OUString & rReplaceWith );

//.............................................................................
} //namespace chart
//.............................................................................
#endif
