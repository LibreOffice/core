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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// BackgroundColorPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BackgroundColorPrimitive2D class

            This primitive is defined to fill the whole visible Viewport with
            the given color (and thus decomposes to a filled polygon). This
            makes it a view-depnendent primitive by definition. It only has
            a valid decomposition if a valid Viewport is given in the
            ViewInformation2D at decomposition time.

            It will try to buffer it's last decomposition using maLastViewport
            to detect changes in the get2DDecomposition call.
         */
        class DRAWINGLAYER_DLLPUBLIC BackgroundColorPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the fill color to use
            basegfx::BColor                             maBColor;

            /// the last used viewInformation, used from getDecomposition for buffering
            basegfx::B2DRange                           maLastViewport;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            explicit BackgroundColorPrimitive2D(
                const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get B2Drange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
