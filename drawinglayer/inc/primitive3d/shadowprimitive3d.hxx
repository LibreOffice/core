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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/groupprimitive3d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>


namespace drawinglayer::primitive3d
    {
        /** ShadowPrimitive3D class

            This 3D grouping primitive is used to define a shadow for
            3d geometry by embedding it. The shadow of 3D objects are
            2D polygons, so the shadow transformation is a 2D transformation.

            If the Shadow3D flag is set, the shadow definition has to be
            combined with the scene and camera definition to create the correct
            projected shadow 2D-Polygons.
         */
        class ShadowPrimitive3D final : public GroupPrimitive3D
        {
            /// 2D shadow definition
            basegfx::B2DHomMatrix                   maShadowTransform;
            basegfx::BColor                         maShadowColor;
            double                                  mfShadowTransparence;

            bool                                    mbShadow3D : 1;

        public:
            /// constructor
            ShadowPrimitive3D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                double fShadowTransparence,
                bool bShadow3D,
                const Primitive3DContainer& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }
            double getShadowTransparence() const { return mfShadowTransparence; }
            bool getShadow3D() const { return mbShadow3D; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };

} // end of namespace drawinglayer::primitive3d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
