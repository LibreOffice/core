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

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <vcl/bitmap.hxx>


// MarkerArrayPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** MarkerArrayPrimitive2D class

            This primitive defines an array of markers. Their size is defined
            in pixels and independent from the view transformation which makes
            this primitive highly view-dependent. It is also transformation
            invariant, so that the bitmap is always visualized unscaled and
            unrotated.
            It is used e.g. for grid position visualisation. The given Bitmap
            (with transparence) is defined to be visible centered at each of the given
            positions.
            It decomposes to the needed number of BitmapPrimitive2D's, so it would
            be efficient to handle it directly in a renderer.
         */
        class DRAWINGLAYER_DLLPUBLIC MarkerArrayPrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the positions for the marker
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the marker definition to visualize
            Bitmap                                          maMarker;

            /// create local decomposition
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            MarkerArrayPrimitive2D(
                std::vector< basegfx::B2DPoint >&& rPositions,
                const Bitmap& rMarker);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const Bitmap& getMarker() const { return maMarker; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
