/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_BASEGFXHELPER_HXX
#define CHART2_BASEGFXHELPER_HXX

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/range/b3drange.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{
namespace BaseGFXHelper
{

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DRange getBoundVolume( const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPoly );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B2IRectangle makeRectangle(
            const com::sun::star::awt::Point& rPosition,
            const com::sun::star::awt::Size& rSize );

OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::awt::Point B2IRectangleToAWTPoint(
            const ::basegfx::B2IRectangle& rB2IRectangle );

OOO_DLLPUBLIC_CHARTTOOLS com::sun::star::awt::Size B2IRectangleToAWTSize(
            const ::basegfx::B2IRectangle& rB2IRectangle );

OOO_DLLPUBLIC_CHARTTOOLS
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

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix(
    const ::com::sun::star::drawing::HomogenMatrix & rHomogenMatrix );

OOO_DLLPUBLIC_CHARTTOOLS
::com::sun::star::drawing::HomogenMatrix B3DHomMatrixToHomogenMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DTuple GetRotationFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DTuple GetScaleFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

void ReduceToRotationMatrix( ::basegfx::B3DHomMatrix & rB3DMatrix );

OOO_DLLPUBLIC_CHARTTOOLS double Deg2Rad( double fDegrees );
OOO_DLLPUBLIC_CHARTTOOLS double Rad2Deg( double fRadians );

} //  namespace BaseGFXHelper
} //  namespace chart

// CHART2_BASEGFXHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
