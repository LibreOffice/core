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

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <vector>


// predefines

namespace basegfx {
    class B3DPolyPolygon;
    class B3DHomMatrix;
    class B2DVector;
}

namespace drawinglayer::attribute {
    class SdrLineAttribute;
    class SdrFillAttribute;
    class Sdr3DObjectAttribute;
    class FillGradientAttribute;
    class SdrShadowAttribute;
}


namespace drawinglayer::primitive3d
    {
        // #i98295#
        basegfx::B3DRange getRangeFrom3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);
        void applyNormalsKindSphereTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill, const basegfx::B3DRange& rRange);
        void applyNormalsKindFlatTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);
        void applyNormalsInvertTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);

        // #i98314#
        void applyTextureTo3DGeometry(
            css::drawing::TextureProjectionMode eModeX,
            css::drawing::TextureProjectionMode eModeY,
            ::std::vector< basegfx::B3DPolyPolygon >& rFill,
            const basegfx::B3DRange& rRange,
            const basegfx::B2DVector& rTextureSize);

        Primitive3DContainer create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine);

        Primitive3DContainer create3DPolyPolygonFillPrimitives(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient);

        Primitive3DContainer createShadowPrimitive3D(
            const Primitive3DContainer& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D);

        Primitive3DContainer createHiddenGeometryPrimitives3D(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute);

} // end of namespace drawinglayer::primitive3d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
