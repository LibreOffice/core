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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class B3DPolygon;
    class B3DPolyPolygon;
    class B3DHomMatrix;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class SdrLineAttribute;
    class SdrFillAttribute;
    class Sdr3DObjectAttribute;
    class FillGradientAttribute;
    class SdrShadowAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        // #i98295#
        basegfx::B3DRange DRAWINGLAYER_DLLPUBLIC getRangeFrom3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);
        void DRAWINGLAYER_DLLPUBLIC applyNormalsKindSphereTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill, const basegfx::B3DRange& rRange);
        void DRAWINGLAYER_DLLPUBLIC applyNormalsKindFlatTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);
        void DRAWINGLAYER_DLLPUBLIC applyNormalsInvertTo3DGeometry(::std::vector< basegfx::B3DPolyPolygon >& rFill);

        // #i98314#
        void DRAWINGLAYER_DLLPUBLIC applyTextureTo3DGeometry(
            ::com::sun::star::drawing::TextureProjectionMode eModeX,
            ::com::sun::star::drawing::TextureProjectionMode eModeY,
            ::std::vector< basegfx::B3DPolyPolygon >& rFill,
            const basegfx::B3DRange& rRange,
            const basegfx::B2DVector& rTextureSize);

        Primitive3DSequence DRAWINGLAYER_DLLPUBLIC create3DPolyPolygonLinePrimitives(
            const basegfx::B3DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const attribute::SdrLineAttribute& rLine);

        Primitive3DSequence DRAWINGLAYER_DLLPUBLIC create3DPolyPolygonFillPrimitives(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient);

        Primitive3DSequence DRAWINGLAYER_DLLPUBLIC createShadowPrimitive3D(
            const Primitive3DSequence& rSource,
            const attribute::SdrShadowAttribute& rShadow,
            bool bShadow3D);

        Primitive3DSequence DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives3D(
            const ::std::vector< basegfx::B3DPolyPolygon >& r3DPolyPolygonVector,
            const basegfx::B3DHomMatrix& rObjectTransform,
            const basegfx::B2DVector& rTextureSize,
            const attribute::Sdr3DObjectAttribute& aSdr3DObjectAttribute);

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_PRIMITIVE3D_SDRDECOMPOSITIONTOOLS3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
