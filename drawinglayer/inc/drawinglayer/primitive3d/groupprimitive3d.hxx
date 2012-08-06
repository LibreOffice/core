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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** GroupPrimitive3D class

            Baseclass for all grouping 3D primitives

            The description/functionality is identical with the 2D case in groupprimitive2d.hxx,
            please see there for detailed information.

            Current Basic 3D StatePrimitives are:

            - ModifiedColorPrimitive3D (for a stack of color modifications)
            - ShadowPrimitive3D (for 3D objects with shadow; this is a special case
              since the shadow of a 3D primitive is a 2D primitive set)
            - TexturePrimitive3D (with the following variations)
                - GradientTexturePrimitive3D (for 3D gradient fill)
                - BitmapTexturePrimitive3D (for 3D Bitmap fill)
                - TransparenceTexturePrimitive3D (for 3D transparence)
                - HatchTexturePrimitive3D (for 3D hatch fill)
            - TransformPrimitive3D (for a transformation stack)
         */
        class DRAWINGLAYER_DLLPUBLIC GroupPrimitive3D : public BasePrimitive3D
        {
        private:
            /// the children. Declared private since this shall never be changed at all after construction
            Primitive3DSequence                             maChildren;

        public:
            /// constructor
            explicit GroupPrimitive3D(const Primitive3DSequence& rChildren);

            /// data read access
            Primitive3DSequence getChildren() const { return maChildren; }

            /// compare operator
            virtual bool operator==( const BasePrimitive3D& rPrimitive ) const;

            /// local decomposition. Implementation will just return children
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_GROUPPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
