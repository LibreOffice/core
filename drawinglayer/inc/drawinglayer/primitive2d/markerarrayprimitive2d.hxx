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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// MarkerArrayPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MarkerArrayPrimitive2D class

            This primtive defines an array of markers. Their size is defined
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
        class DRAWINGLAYER_DLLPUBLIC MarkerArrayPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the positions for the marker
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the marker definintion to visualize
            BitmapEx                                        maMarker;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            MarkerArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                const BitmapEx& rMarker);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const BitmapEx& getMarker() const { return maMarker; }

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

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
