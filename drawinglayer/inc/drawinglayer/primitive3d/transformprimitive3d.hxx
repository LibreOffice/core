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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** TransformPrimitive3D class

            This is one of the basic grouping 3D primitives and it provides
            embedding a sequence of primitives (a geometry) into a
            3D transformation.

            Please see the description for TransformPrimitive2D since these
            primitives are pretty similar.
         */
        class DRAWINGLAYER_DLLPUBLIC TransformPrimitive3D : public GroupPrimitive3D
        {
        private:
            // the 3D transformation to apply
            basegfx::B3DHomMatrix                   maTransformation;

        public:
            /// constructor
            TransformPrimitive3D(
                const basegfx::B3DHomMatrix& rTransformation,
                const Primitive3DSequence& rChildren);

            /// data read access
            const basegfx::B3DHomMatrix& getTransformation() const { return maTransformation; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_TRANSFORMPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
