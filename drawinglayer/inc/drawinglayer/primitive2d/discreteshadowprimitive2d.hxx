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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// DiscreteShadowPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
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
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadowPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            // the object transformation of the rectangular object
            basegfx::B2DHomMatrix       maTransform;

            // the bitmap shadow data
            DiscreteShadow              maDiscreteShadow;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            DiscreteShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const DiscreteShadow& rDiscreteShadow);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const DiscreteShadow& getDiscreteShadow() const { return maDiscreteShadow; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
