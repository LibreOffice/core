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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/attribute/fillbitmapattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// FillbitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** FillBitmapPrimitive2D class

            This class defines a bitmap filling for a rectangular area. The
            Range is defined by the Transformation, the fill by the FillBitmapAttribute.
            There, the fill consists of a Bitmap (not transparent) defining the fill data
            and a Point/Vector pair defining the relative position/size [0.0 .. 1.0]
            inside the area where the bitmap is positioned. A flag defines then if this
            is tiled or not.

            Renderers should handle this primitive; it has a geometrically correct
            decomposition, but on pixel oututs the areas where the tiled pieces are
            aligned tend to show up (one overlapping or empty pixel)
         */
        class DRAWINGLAYER_DLLPUBLIC FillBitmapPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DHomMatrix                       maTransformation;

            /// the fill attributes
            attribute::FillBitmapAttribute              maFillBitmap;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            FillBitmapPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                const attribute::FillBitmapAttribute& rFillBitmap);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            const attribute::FillBitmapAttribute& getFillBitmap() const { return maFillBitmap; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
