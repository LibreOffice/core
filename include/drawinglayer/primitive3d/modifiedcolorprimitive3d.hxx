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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/color/bcolormodifier.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** ModifiedColorPrimitive3D class

            This primitive is a grouping primitive and allows to define
            how the colors of its child content shall be modified for
            visualisation. Please see the ModifiedColorPrimitive2D
            description for more info.
         */
        class DRAWINGLAYER_DLLPUBLIC ModifiedColorPrimitive3D : public GroupPrimitive3D
        {
        private:
            /// The ColorModifier to use
            basegfx::BColorModifierSharedPtr    maColorModifier;

        public:
            /// constructor
            ModifiedColorPrimitive3D(
                const Primitive3DSequence& rChildren,
                const basegfx::BColorModifierSharedPtr& rColorModifier);

            /// data read access
            const basegfx::BColorModifierSharedPtr& getColorModifier() const { return maColorModifier; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_MODIFIEDCOLORPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
