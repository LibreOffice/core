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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/graph.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MediaPrimitive2D class

            This is a unified primitive for Media visualisation, e.g. animations
            or sounds. It's geometry is defined by Transform. For conveinience,
            it also contains a discrete border size (aka Pixels) which will be added
            if used. This makes it a view-dependent primitive. It also gets a filled
            background and the decomposition will try to create a graphic representation
            if te content (defined by the URL), e.g. a still frome for animated stuff.
         */
        class DRAWINGLAYER_DLLPUBLIC MediaPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometry definition
            basegfx::B2DHomMatrix                       maTransform;

            /// the content definition
            rtl::OUString                               maURL;

            /// style: background color
            basegfx::BColor                             maBackgroundColor;

            /// discrete border (in 'pixels')
            sal_uInt32                                  mnDiscreteBorder;

            const Graphic                               maSnapshot;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            MediaPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const rtl::OUString& rURL,
                const basegfx::BColor& rBackgroundColor,
                sal_uInt32 nDiscreteBorder,
                const Graphic &rSnapshot);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const rtl::OUString& getURL() const { return maURL; }
            const basegfx::BColor& getBackgroundColor() const { return maBackgroundColor; }
            sal_uInt32 getDiscreteBorder() const { return mnDiscreteBorder; }

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

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
