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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** ShadowPrimitive3D class

            This 3D grouping primitive is used to define a shadow for
            3d geometry by embedding it. The shadow of 3D objects are
            2D polygons, so the shadow transformation is a 2D transformation.

            If the Shadow3D flag is set, the shadow definition has to be
            combined with the scene and camera definition to create the correct
            projected shadow 2D-Polygons.
         */
        class DRAWINGLAYER_DLLPUBLIC ShadowPrimitive3D : public GroupPrimitive3D
        {
        protected:
            /// 2D shadow definition
            basegfx::B2DHomMatrix                   maShadowTransform;
            basegfx::BColor                         maShadowColor;
            double                                  mfShadowTransparence;

            /// bitfield
            unsigned                                mbShadow3D : 1;

        public:
            /// constructor
            ShadowPrimitive3D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                double fShadowTransparence,
                bool bShadow3D,
                const Primitive3DSequence& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }
            double getShadowTransparence() const { return mfShadowTransparence; }
            bool getShadow3D() const { return mbShadow3D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_SHADOWPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
