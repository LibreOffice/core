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

#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b3dtuple.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <basegfx/range/b3drange.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include "charttoolsdllapi.hxx"

namespace com::sun::star::awt { struct Rectangle; }
namespace com::sun::star::drawing { struct PolyPolygonShape3D; }

namespace chart::BaseGFXHelper
{

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DRange getBoundVolume( const css::drawing::PolyPolygonShape3D& rPolyPoly );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B2IRectangle makeRectangle(
            const css::awt::Point& rPosition,
            const css::awt::Size& rSize );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B2IRectangle makeRectangle( const css::awt::Rectangle& rRect );

OOO_DLLPUBLIC_CHARTTOOLS css::awt::Point B2IRectangleToAWTPoint(
            const ::basegfx::B2IRectangle& rB2IRectangle );

OOO_DLLPUBLIC_CHARTTOOLS css::awt::Size B2IRectangleToAWTSize(
            const ::basegfx::B2IRectangle& rB2IRectangle );

::basegfx::B3DVector Direction3DToB3DVector(
    const css::drawing::Direction3D& rDirection );

css::drawing::Direction3D B3DVectorToDirection3D(
    const ::basegfx::B3DVector& rB3DVector );

::basegfx::B3DVector Position3DToB3DVector(
    const css::drawing::Position3D& rPosition );

css::drawing::Position3D B3DVectorToPosition3D(
    const ::basegfx::B3DVector& rB3DVector );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DHomMatrix HomogenMatrixToB3DHomMatrix(
    const css::drawing::HomogenMatrix & rHomogenMatrix );

OOO_DLLPUBLIC_CHARTTOOLS
css::drawing::HomogenMatrix B3DHomMatrixToHomogenMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DTuple GetRotationFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

OOO_DLLPUBLIC_CHARTTOOLS ::basegfx::B3DTuple GetScaleFromMatrix(
    const ::basegfx::B3DHomMatrix & rB3DMatrix );

void ReduceToRotationMatrix( ::basegfx::B3DHomMatrix & rB3DMatrix );

} //  namespace chart::BaseGFXHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
