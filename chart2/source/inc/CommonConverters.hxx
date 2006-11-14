/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommonConverters.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:32:16 $
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

//#ifndef _SVX_VECTOR3D_HXX
//#include <goodies/vector3d.hxx>
//#endif
//#ifndef _B3D_HMATRIX_HXX
//#include <goodies/hmatrix.hxx>
//#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
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

#ifndef _COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XDataSequence.hpp>
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
                 Matrix4DToHomogenMatrix( const ::basegfx::B3DHomMatrix& rM4 );

//-----------------------------------------------------------------------------
/** HomogenMatrix -> ::basegfx::B3DHomMatrix
*/
::basegfx::B3DHomMatrix HomogenMatrixToMatrix4D( const com::sun::star::drawing::HomogenMatrix& rHM );

//-----------------------------------------------------------------------------
/** ::basegfx::B3DHomMatrix -> B2DHomMatrix
*/
::basegfx::B2DHomMatrix IgnoreZ( const ::basegfx::B3DHomMatrix& rM4 );

//-----------------------------------------------------------------------------
/** B2DHomMatrix <-> HomogenMatrix3
*/
com::sun::star::drawing::HomogenMatrix3
B2DHomMatrixToHomogenMatrix3( const ::basegfx::B2DHomMatrix& rM3 );

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
        ::com::sun::star::chart2::XDataSequence > & xDataSequence );

//.............................................................................
} //namespace chart
//.............................................................................
#endif
