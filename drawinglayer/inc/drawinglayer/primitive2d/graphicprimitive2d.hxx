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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svtools/grfmgr.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** GraphicPrimitive2D class

            Primitive to hold graphics defined by GraphicObject and GraphicAttr
            combination. This includes MetaFiles and diverse pixel-oriented graphic
            formats. It even includes animated GIFs, Croppings and other changes
            defined in GraphicAttr.

            This makes the decomposition contain a wide variety of possibilites,
            too. From a simple BitmapPrimitive over AnimatedSwitchPrimitive2D,
            MetafilePrimitive2D (with and without embedding in a masking when e.g.
            the Metafile is bigger than the geometry) and embeddings in
            TransformPrimitive2D and MaskPrimitive2D for croppings.

            The primitive geometry area is defined by Transform.
         */
        class DRAWINGLAYER_DLLPUBLIC GraphicPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DHomMatrix                       maTransform;

            /// the GraphicObject with all it's content possibilities
            GraphicObject                               maGraphicObject;

            /// The GraphicAttr with all it's modification possibilities
            GraphicAttr                                 maGraphicAttr;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor(s)
            GraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const GraphicObject& rGraphicObject,
                const GraphicAttr& rGraphicAttr);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const GraphicObject& getGraphicObject() const { return maGraphicObject; }
            const GraphicAttr& getGraphicAttr() const { return maGraphicAttr; }
            bool isTransparent() const;

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

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
