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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_RENDERGRAPHICPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_RENDERGRAPHICPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/rendergraphic.hxx>
#include <memory>

//////////////////////////////////////////////////////////////////////////////
// RenderGraphicPrimitive2D class

namespace vcl { class RenderGraphicRasterizer; }

namespace drawinglayer
{
    namespace primitive2d
    {
        /** RenderGraphicPrimitive2D class

            This class is the central primitive for RenderGraphic-based primitives.
         */
        class RenderGraphicPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the RenderGraphic data
            vcl::RenderGraphic                                      maRenderGraphic;
            mutable std::auto_ptr< vcl::RenderGraphicRasterizer >   mapCurrentRasterizer;

            /** the object transformation from unit coordinates, defining
                size, shear, rotate and position
             */
            basegfx::B2DHomMatrix   maTransform;

        public:
            /// constructor
            RenderGraphicPrimitive2D(
                const vcl::RenderGraphic& rRenderGraphic,
                const basegfx::B2DHomMatrix& rTransform);

            /// data read access
            inline const vcl::RenderGraphic&    getRenderGraphic() const { return maRenderGraphic; }
            inline const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

            // access to latest used vcl::GraphicRasterizer for possibly reusing
            // an already rendered vcl::RenderGraphic with the same transform
            // properties during the next rendering process
            inline const vcl::RenderGraphicRasterizer* getCurrentRasterizer() const { return( mapCurrentRasterizer.get() ); }

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

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_RENDERGRAPHICPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
