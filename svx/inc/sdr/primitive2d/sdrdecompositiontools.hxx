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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/attribute/sdrglowattribute.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <svx/svxdllapi.h>

// predefines
namespace basegfx {
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2DHomMatrix;
}

namespace drawinglayer::attribute {
    class SdrFillAttribute;
    class SdrLineAttribute;
    class FillGradientAttribute;
    class SdrShadowAttribute;
    class SdrLineStartEndAttribute;
    class SdrTextAttribute;
}


namespace drawinglayer::primitive2d
    {
        Primitive2DReference SVXCORE_DLLPUBLIC createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient);

        Primitive2DReference SVXCORE_DLLPUBLIC createPolyPolygonFillPrimitive(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::B2DRange& rDefinitionRange,
            const attribute::SdrFillAttribute& rFill,
            const attribute::FillGradientAttribute& rFillGradient);

        Primitive2DReference SVXCORE_DLLPUBLIC createPolygonLinePrimitive(
            const basegfx::B2DPolygon& rPolygon,
            const attribute::SdrLineAttribute& rLine,
            const attribute::SdrLineStartEndAttribute& rStroke);

        Primitive2DReference SVXCORE_DLLPUBLIC createTextPrimitive(
            const basegfx::B2DPolyPolygon& rUnitPolyPolygon,
            const basegfx::B2DHomMatrix& rObjectTransform,
            const attribute::SdrTextAttribute& rText,
            const attribute::SdrLineAttribute& rStroke,
            bool bCellText,
            bool bWordWrap);

        Primitive2DContainer SVXCORE_DLLPUBLIC createEmbeddedShadowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrShadowAttribute& rShadow,
            const basegfx::B2DHomMatrix& rObjectMatrix = basegfx::B2DHomMatrix(),
            const Primitive2DContainer* pContentForShadow = nullptr);

        Primitive2DContainer SVXCORE_DLLPUBLIC createEmbeddedGlowPrimitive(
            Primitive2DContainer&& rContent,
            const attribute::SdrGlowAttribute& rGlow);

        Primitive2DContainer SVXCORE_DLLPUBLIC createEmbeddedSoftEdgePrimitive(
            Primitive2DContainer&& aContent,
            sal_Int32 nRadius);

} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
