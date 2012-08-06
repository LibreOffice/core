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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** TransformPrimitive2D class

            This is one of the basic grouping primitives and it provides
            embedding a sequence of primitives (a geometry) into a
            transformation. All renderers have to handle this, usually by
            building a current transformation stack (linear combination)
            and applying this to all to-be-rendered geometry. If not handling
            this, the output will be mostly wrong since this primitive is
            widely used.

            It does transform by embedding an existing geometry into a
            transformation as Child-content. This allows re-usage of the
            refcounted Uno-Api primitives and their existung, buffered
            decompositions.

            It could e.g. be used to show a single object geometry in 1000
            different, transformed states without the need to create those
            thousand primitive contents.
         */
        class DRAWINGLAYER_DLLPUBLIC TransformPrimitive2D : public GroupPrimitive2D
        {
        private:
            // the transformation to apply to the child geometry
            basegfx::B2DHomMatrix                   maTransformation;

        public:
            /// constructor
            TransformPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                const Primitive2DSequence& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }

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

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TRANSFORMPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
