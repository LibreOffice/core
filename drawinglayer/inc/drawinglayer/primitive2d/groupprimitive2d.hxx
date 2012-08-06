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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// GroupPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** GroupPrimitive2D class

            Baseclass for all grouping 2D primitives

            The grouping primitive in it's basic form is capable of holding
            a child primitive content and returns it on decomposition on default.
            It is used for two main purposes, but more may apply:

            - to transport extended information, e.g. for text classification,
              see e.g. TextHierarchy*Primitive2D implementations. Since they
              decompose to their child content, renderers not aware/interested
              in that extra information will just ignore these primitives

            - to encapsulate common geometry, e.g. the ShadowPrimitive2D implements
              applying a generic shadow to a child sequence by adding the needed
              offset and color stuff in the decomposition

            In most cases the decomposition is straightforward, so by default
            this primitive will not buffer the result and is not derived from
            BufferedDecompositionPrimitive2D, but from BasePrimitive2D.

            A renderer has to take GroupPrimitive2D derivations into account which
            are used to hold a state.

            Current Basic 2D StatePrimitives are:

            - TransparencePrimitive2D (objects with freely defined transparence)
            - InvertPrimitive2D (for XOR)
            - MaskPrimitive2D (for masking)
            - ModifiedColorPrimitive2D (for a stack of color modifications)
            - TransformPrimitive2D (for a transformation stack)
         */
        class DRAWINGLAYER_DLLPUBLIC GroupPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive2DSequence                             maChildren;

        public:
            /// constructor
            explicit GroupPrimitive2D(const Primitive2DSequence& rChildren);

            /// data read access
            const Primitive2DSequence& getChildren() const { return maChildren; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// local decomposition. Implementation will just return children
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_GROUPPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
