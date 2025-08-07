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

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmap.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>


// DiscreteShadowPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** DiscreteShadow data class

         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadow
        {
        private:
            /// the original shadow Bitmap in a special form
            Bitmap                    maBitmap;

            /// buffered extracted parts of CombinedShadow for easier usage
            Bitmap                    maTopLeft;
            Bitmap                    maTop;
            Bitmap                    maTopRight;
            Bitmap                    maRight;
            Bitmap                    maBottomRight;
            Bitmap                    maBottom;
            Bitmap                    maBottomLeft;
            Bitmap                    maLeft;

        public:
            /// constructor
            explicit DiscreteShadow(const Bitmap& rBitmap);

            /// data read access
            const Bitmap& getBitmap() const { return maBitmap; }

            /// compare operator
            bool operator==(const DiscreteShadow& rCompare) const
            {
                return getBitmap() == rCompare.getBitmap();
            }

            /// helper accesses which create on-demand needed segments
            const Bitmap& getTopLeft() const;
            const Bitmap& getTop() const;
            const Bitmap& getTopRight() const;
            const Bitmap& getRight() const;
            const Bitmap& getBottomRight() const;
            const Bitmap& getBottom() const;
            const Bitmap& getBottomLeft() const;
            const Bitmap& getLeft() const;
        };

        /** DiscreteShadowPrimitive2D class

         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadowPrimitive2D final : public DiscreteMetricDependentPrimitive2D
        {
        private:
            // the object transformation of the rectangular object
            basegfx::B2DHomMatrix       maTransform;

            // the bitmap shadow data
            DiscreteShadow              maDiscreteShadow;

            /// create local decomposition
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            DiscreteShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const DiscreteShadow& rDiscreteShadow);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const DiscreteShadow& getDiscreteShadow() const { return maDiscreteShadow; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
