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
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>


// DiscreteShadowPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** DiscreteShadow data class

         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadow
        {
        private:
            /// the original shadow BitmapEx in a special form
            BitmapEx                    maBitmapEx;

            /// buffered extracted parts of CombinedShadow for easier usage
            BitmapEx                    maTopLeft;
            BitmapEx                    maTop;
            BitmapEx                    maTopRight;
            BitmapEx                    maRight;
            BitmapEx                    maBottomRight;
            BitmapEx                    maBottom;
            BitmapEx                    maBottomLeft;
            BitmapEx                    maLeft;

        public:
            /// constructor
            explicit DiscreteShadow(const BitmapEx& rBitmapEx);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }

            /// compare operator
            bool operator==(const DiscreteShadow& rCompare) const
            {
                return getBitmapEx() == rCompare.getBitmapEx();
            }

            /// helper accesses which create on-demand needed segments
            const BitmapEx& getTopLeft() const;
            const BitmapEx& getTop() const;
            const BitmapEx& getTopRight() const;
            const BitmapEx& getRight() const;
            const BitmapEx& getBottomRight() const;
            const BitmapEx& getBottom() const;
            const BitmapEx& getBottomLeft() const;
            const BitmapEx& getLeft() const;
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
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, VisitingParameters const & rParameters) const override;

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
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
