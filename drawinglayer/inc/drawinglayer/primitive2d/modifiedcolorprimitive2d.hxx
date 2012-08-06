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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ModifiedColorPrimitive2D class

            This primitive is a grouping primitive and allows to define
            how the colors of it's child content shall be modified for
            visualisation. This can be (and is) used e.g. for generic shadow
            visualisation by forcing all color usages of the contained
            sub-content to the shadow color.

            For the possibilities of color modifications, please refer
            to the basegfx::BColorModifier definitions in basegfx. For
            processing there is tooling in basegfx to build a stack of
            BColorModifiers to always be able to proccess the correct
            colors.

            If a renderer does not handle this primitive, the content will
            be visualized unchanged.
         */
        class DRAWINGLAYER_DLLPUBLIC ModifiedColorPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// The ColorModifier to use
            basegfx::BColorModifier                 maColorModifier;

        public:
            /// constructor
            ModifiedColorPrimitive2D(
                const Primitive2DSequence& rChildren,
                const basegfx::BColorModifier& rColorModifier);

            /// data read access
            const basegfx::BColorModifier& getColorModifier() const { return maColorModifier; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
