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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// BitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BitmapPrimitive2D class

            This class is the central primitive for Bitmap-based primitives.
            It provides RGBA-based bitmaps, currently using a BitmapEx from VCL.
            This may change in the future to any other, maybe more general base
            class providing 24bit RGBA.
         */
        class DRAWINGLAYER_DLLPUBLIC BitmapPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the RGBA Bitmap-data
            BitmapEx                                    maBitmapEx;

            /** the object transformation from unit coordinates, defining
                size, shear, rotate and position
             */
            basegfx::B2DHomMatrix                       maTransform;

        public:
            /// constructor
            BitmapPrimitive2D(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DHomMatrix& rTransform);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

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

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
