/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svtools/grfmgr.hxx>



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

            /// the GraphicObject with all its content possibilities
            GraphicObject                               maGraphicObject;

            /// The GraphicAttr with all its modification possibilities
            GraphicAttr                                 maGraphicAttr;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor(s)
            GraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const GraphicObject& rGraphicObject,
                const GraphicAttr& rGraphicAttr);
            GraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const GraphicObject& rGraphicObject);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const GraphicObject& getGraphicObject() const { return maGraphicObject; }
            const GraphicAttr& getGraphicAttr() const { return maGraphicAttr; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GRAPHICPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
