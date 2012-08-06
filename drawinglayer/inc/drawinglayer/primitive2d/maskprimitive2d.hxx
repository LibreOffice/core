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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MaskPrimitive2D class

            This is the central masking primitive. It's a grouping
            primitive and contains a PolyPolygon which defines the visible
            area. Only visualisation parts of the Child primitive sequence
            inside of the mask PolyPolygon is defined to be visible.

            This primitive should be handled by a renderer. If it is not handled,
            it decomposes to it's Child content, and thus the visualisation would
            contaiun no clips.

            The geometrc range of this primitive is completely defined by the Mask
            PolyPolygon since by definition nothing outside of the mask is visible.
         */
        class DRAWINGLAYER_DLLPUBLIC MaskPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the mask PolyPolygon
            basegfx::B2DPolyPolygon                 maMask;

        public:
            /// constructor
            MaskPrimitive2D(
                const basegfx::B2DPolyPolygon& rMask,
                const Primitive2DSequence& rChildren);

            /// data read access
            const basegfx::B2DPolyPolygon& getMask() const { return maMask; }

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

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
