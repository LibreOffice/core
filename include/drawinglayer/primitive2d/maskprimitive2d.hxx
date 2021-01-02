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

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>


namespace drawinglayer::primitive2d
{
        /** MaskPrimitive2D class

            This is the central masking primitive. It's a grouping
            primitive and contains a tools::PolyPolygon which defines the visible
            area. Only visualisation parts of the Child primitive sequence
            inside of the mask tools::PolyPolygon is defined to be visible.

            This primitive should be handled by a renderer. If it is not handled,
            it decomposes to its Child content, and thus the visualisation would
            contain no clips.

            The geometry range of this primitive is completely defined by the Mask
            tools::PolyPolygon since by definition nothing outside of the mask is visible.
         */
        class DRAWINGLAYER_DLLPUBLIC MaskPrimitive2D final : public GroupPrimitive2D
        {
        private:
            /// the mask PolyPolygon
            basegfx::B2DPolyPolygon                 maMask;

        public:
            /// constructor
            MaskPrimitive2D(
                const basegfx::B2DPolyPolygon& rMask,
                const Primitive2DContainer& rChildren);

            /// data read access
            const basegfx::B2DPolyPolygon& getMask() const { return maMask; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
