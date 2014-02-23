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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>


// predefines

namespace basegfx {
    class B2DPolyPolygon;
    class B2DHomMatrix;
}



namespace drawinglayer
{
    namespace primitive2d
    {
        // helpers to create HitTestPrimitives Line
        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DHomMatrix& rMatrix);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolygon);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DRange& rRange,
            const basegfx::B2DHomMatrix& rMatrix);

        Primitive2DReference DRAWINGLAYER_DLLPUBLIC createHiddenGeometryPrimitives2D(
            bool bFilled,
            const basegfx::B2DPolyPolygon& rPolygon,
            const basegfx::B2DHomMatrix& rMatrix);

    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SDRDECOMPOSITIONTOOLS2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
