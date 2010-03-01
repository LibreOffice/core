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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HITTESTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HITTESTPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** HitTestPrimitive2D class

            This primitive is used to represent geometry for non-visible objects,
            e.g. a PresObj's outline. To still be able to use primitives for HitTest
            functionality, the 2d decompositions will produce an as much as possible
            simplified line geometry encapsulated in this primtive when there is no
            line geometry. In a further enchanced version this may change to 'if neither
            filled nor lines' creation criteria. The whole primitive decomposes to nothing,
            so no one not knowing it will be influenced. Only helper processors for hit test
            (and maybe BoundRect extractors) will use it and it's children subcontent.
         */
        class HitTestPrimitive2D : public GroupPrimitive2D
        {
        public:
            /// constructor
            HitTestPrimitive2D(const Primitive2DSequence& rChildren);

            /** despite returning an empty decomposition since it's no visualisation data,
                range calculation is intended to use invisible replacement geometry, so
                the local implementation will return the children's range
             */
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /** local decomposition. Implementation will return empty Primitive2DSequence
                since this is no visualisation data
             */
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HITTESTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
