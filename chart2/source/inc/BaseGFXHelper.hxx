/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BaseGFXHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:12:09 $
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
#ifndef CHART2_BASEGFXHELPER_HXX
#define CHART2_BASEGFXHELPER_HXX

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif
#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif
#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif
#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif
#ifndef _BGFX_RANGE_B2IRECTANGLE_HXX
#include <basegfx/range/b2irectangle.hxx>
#endif
#ifndef _BGFX_TUPLE_B3DTUPLE_HXX
#include <basegfx/tuple/b3dtuple.hxx>
#endif
#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif
#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
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
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

namespace chart
{
namespace BaseGFXHelper
{

::basegfx::B3DRange getBoundVolume( const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPoly );

::basegfx::B2IRectangle makeRectangle(
            const com::sun::star::awt::Point& rPosition,
            const com::sun::star::awt::Size& rSize );

com::sun::star::awt::Point B2IRectangleToAWTPoint(
            const ::basegfx::B2IRectangle& rB2IRectangle );

com::sun::star::awt::Size B2IRectangleToAWTSize(
            const ::basegfx::B2IRectangle& rB2IRectangle );

com::sun::star::awt::Rectangle B2IRectangleToAWTRectangle(
    const ::basegfx::B2IRectangle& rB2IRectangle );

::basegfx::B3DVector Direction3DToB3DVector(
    const com::sun::star::drawing::Direction3D& rDirection );

com::sun::star::drawing::Direction3D B3DVectorToDirection3D(
    const ::basegfx::B3DVector& rB3DVector );

::basegfx::B3DVector Position3DToB3DVector(
    const com::sun::star::drawing::Position3D& rPosition );

com::sun::star::drawing::Position3D B3DVectorToPosition3D(
    const ::basegfx::B3DVector& rB3DVector );

::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix(
    const ::com::sun::star::drawing::HomogenMatrix & rHomogenMatrix );

::com::sun::star::drawing::HomogenMatrix B3DHomMatrixToHomogenMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

::basegfx::B3DTuple GetRotationFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

::basegfx::B3DTuple GetScaleFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

void ReduceToRotationMatrix( ::basegfx::B3DHomMatrix & rB3DMatrix );

double Deg2Rad( double fDegrees );
double Rad2Deg( double fRadians );

double ShiftAngleToIntervalMinusPiToPi( double fAngleRad );

} //  namespace BaseGFXHelper
} //  namespace chart

// CHART2_BASEGFXHELPER_HXX
#endif
