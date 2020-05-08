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

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>


// PointArrayPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** PointArrayPrimitive2D class

            This primitive defines single,discrete 'pixels' for the given
            positions in the given color. This makes it view-dependent since
            the logic size of a 'pixel' depends on the view transformation.

            This is one of the non-decomposable primitives, so a renderer
            should process it (Currently it is only used for grid visualisation,
            but this may change).
         */
        class DRAWINGLAYER_DLLPUBLIC PointArrayPrimitive2D final : public BasePrimitive2D
        {
        private:
            /// the array of positions
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the color to use
            basegfx::BColor                                 maRGBColor;

            /// #i96669# add simple range buffering for this primitive
            basegfx::B2DRange                               maB2DRange;

        public:
            /// constructor
            PointArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                const basegfx::BColor& rRGBColor);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
