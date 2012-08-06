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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// DiscreteBitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** DiscreteBitmapPrimitive2D class

            This class defines a view-dependent BitmapPrimitive which has a
            logic position for the top-left position and is always to be
            painted in 1:1 pixel resolution. It will never be sheared, rotated
            or scaled with the view.
         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteBitmapPrimitive2D : public ObjectAndViewTransformationDependentPrimitive2D
        {
        private:
            /// the RGBA Bitmap-data
            BitmapEx                                    maBitmapEx;

            /** the top-left object position */
            basegfx::B2DPoint                           maTopLeft;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            DiscreteBitmapPrimitive2D(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rTopLeft);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
