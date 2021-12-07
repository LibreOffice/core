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
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <vcl/bitmapex.hxx>


namespace drawinglayer::primitive2d
{
        /** PatternFillPrimitive2D class

            This primitive allows filling a given tools::PolyPolygon with a pattern
            defined by a sequence of primitives which are mapped to  the unit range.
            The pattern is defined using a reference range which defines a rectangular
            area relative to the tools::PolyPolygon (in unit coordinates) which is virtually
            infinitely repeated.
         */
        class DRAWINGLAYER_DLLPUBLIC PatternFillPrimitive2D final : public BufferedDecompositionPrimitive2D
        {
        private:
            const basegfx::B2DPolyPolygon       maMask;
            const Primitive2DContainer          maChildren;
            const basegfx::B2DRange             maReferenceRange;

            /// values holding the discrete buffer size
            sal_uInt32                          mnDiscreteWidth;
            sal_uInt32                          mnDiscreteHeight;

            /// helper that is capable to calculate the needed discrete buffer size for
            /// eventually buffered content
            void calculateNeededDiscreteBufferSize(
                sal_uInt32& rWidth,
                sal_uInt32& rHeight,
                const geometry::ViewInformation2D& rViewInformation) const;

            /// helper which creates the content - checks if clipping is needed and eventually
            /// creates buffered content to speed up rendering
            Primitive2DContainer createContent(const geometry::ViewInformation2D& rViewInformation) const;

            /// create local decomposition
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PatternFillPrimitive2D(
                const basegfx::B2DPolyPolygon& rMask,
                Primitive2DContainer&& rChildren,
                const basegfx::B2DRange& rReferenceRange);

            /// data read access
            const basegfx::B2DPolyPolygon& getMask() const { return maMask; }
            const Primitive2DContainer& getChildren() const { return maChildren; }
            const basegfx::B2DRange& getReferenceRange() const { return maReferenceRange; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// overload to react on evtl. buffered content
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;

            // XAccounting
            virtual sal_Int64 estimateUsage() override;

            /// helper which creates the content - checks if clipping is needed and eventually
            /// creates buffered content to speed up rendering
            BitmapEx createTileImage(sal_uInt32 nWidth, sal_uInt32 nHeight) const;

            /// helper that is capable to calculate the needed discrete buffer size for
            /// eventually buffered content
            void getTileSize(
                sal_uInt32& rWidth,
                sal_uInt32& rHeight,
                const geometry::ViewInformation2D& rViewInformation) const;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
