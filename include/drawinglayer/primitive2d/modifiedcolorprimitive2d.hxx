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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        /** ModifiedColorPrimitive2D class

            This primitive is a grouping primitive and allows to define
            how the colors of its child content shall be modified for
            visualisation. This can be (and is) used e.g. for generic shadow
            visualisation by forcing all color usages of the contained
            sub-content to the shadow color.

            For the possibilities of color modifications, please refer
            to the basegfx::BColorModifier definitions in basegfx. For
            processing there is tooling in basegfx to build a stack of
            BColorModifierSharedPtrs to always be able to process the correct
            colors.

            If a renderer does not handle this primitive, the content will
            be visualized unchanged.
         */
        class DRAWINGLAYER_DLLPUBLIC ModifiedColorPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// The ColorModifier to use
            basegfx::BColorModifierSharedPtr    maColorModifier;

        public:
            /// constructor
            ModifiedColorPrimitive2D(
                const Primitive2DSequence& rChildren,
                const basegfx::BColorModifierSharedPtr& rColorModifier);

            /// data read access
            const basegfx::BColorModifierSharedPtr& getColorModifier() const { return maColorModifier; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MODIFIEDCOLORPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
