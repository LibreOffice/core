/*************************************************************************
 *
 *  $RCSfile: CommonConverters.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-04 10:22:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __CHART_COMMON_CONVERTERS_HXX
#define __CHART_COMMON_CONVERTERS_HXX

#ifndef _SVX_VECTOR3D_HXX
#include <goodies/vector3d.hxx>
#endif
#ifndef _B3D_HMATRIX_HXX
#include <goodies/hmatrix.hxx>
#endif
#ifndef _B2D_MATRIX3D_HXX
#include <goodies/matrix3d.hxx>
#endif
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

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
diverse methods for class conversions; e.g. Matrix4D to HomogenMatrix
and operations e.g  drawing::Position3D + drawing::Direction3D
*/

//-----------------------------------------------------------------------------
/** Matrix4D -> HomogenMatrix
*/
com::sun::star::drawing::HomogenMatrix
                 Matrix4DToHomogenMatrix( const Matrix4D& rM4 );

//-----------------------------------------------------------------------------
/** HomogenMatrix -> Matrix4D
*/
Matrix4D        HomogenMatrixToMatrix4D( const com::sun::star::drawing::HomogenMatrix& rHM );

//-----------------------------------------------------------------------------
/** Matrix4D -> Matrix3D
*/
Matrix3D        IgnoreZ( const Matrix4D& rM4 );

//-----------------------------------------------------------------------------
/** Matrix3D <-> HomogenMatrix3
*/
com::sun::star::drawing::HomogenMatrix3
                 Matrix3DToHomogenMatrix3( const Matrix3D& rM3 );

Matrix3D         HomogenMatrix3ToMatrix3D( const com::sun::star::drawing::HomogenMatrix3& rHM );

//-----------------------------------------------------------------------------
/** Position3D -> Vector3D
*/
Vector3D        Position3DToVector3D( const com::sun::star::drawing::Position3D& rPosition );

//-----------------------------------------------------------------------------
/** Vector3D -> Direction3D
*/
com::sun::star::drawing::Direction3D
                Vector3DToDirection3D( const Vector3D& rVector);

//-----------------------------------------------------------------------------
/** two drawing::Position3D -> PolyPolygonShape3D
*/
::com::sun::star::drawing::PolyPolygonShape3D
    MakeLine3D( const ::com::sun::star::drawing::Position3D & rStart,
                const ::com::sun::star::drawing::Position3D & rEnd );

//-----------------------------------------------------------------------------
/** PolyPolygonShape3D + drawing::Position3D -> PolyPolygonShape3D
*/
void AddPointToPoly( com::sun::star::drawing::PolyPolygonShape3D& rPoly
                , const com::sun::star::drawing::Position3D& rPos
                , sal_Int32 nSequenceIndex=0 );

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
/** Sequence<double> -> Vector3D
*/
Vector3D        SequenceToVector3D( const com::sun::star::uno::Sequence< double >& rSeq );

//-----------------------------------------------------------------------------
/** Vector3D -> Sequence<double>
*/
com::sun::star::uno::Sequence< double >
                Vector3DToSequence( const Vector3D& rVector );

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

//.............................................................................
} //namespace chart
//.............................................................................
#endif
