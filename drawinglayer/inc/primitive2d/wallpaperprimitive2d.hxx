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

#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/wall.hxx>


namespace drawinglayer::primitive2d
    {
        /** WallpaperBitmapPrimitive2D class

            This is a specialized primitive for the Wallpaper definitions included in
            VCL and Metafiles. The extraordinary about the bitmap definition part of
            the Wallpaper is that it uses PIXEL size of the given Bitmap and not
            the logic and/or discrete size derived by PrefMapMode/PrefSize methods.
            To emulate this, a ViewTransformation dependent primitive is needed which
            takes over the correct scaling(s).

            Since a specialized primitive is needed anyways, i opted to also add the
            layouting which is dependent from WallpaperStyle; thus it does not need
            to be handled anywhere else in the future.
         */
        class WallpaperBitmapPrimitive2D final : public ViewTransformationDependentPrimitive2D
        {
        private:
            basegfx::B2DRange                   maObjectRange;
            BitmapEx                            maBitmapEx;
            WallpaperStyle                      meWallpaperStyle;

            /// create local decomposition
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, VisitingParameters const & rParameters) const override;

        public:
            /// constructor
            WallpaperBitmapPrimitive2D(
                const basegfx::B2DRange& rObjectRange,
                const BitmapEx& rBitmapEx,
                WallpaperStyle eWallpaperStyle);

            /// data read access
            const basegfx::B2DRange& getLocalObjectRange() const { return maObjectRange; }
            const BitmapEx& getBitmapEx() const { return maBitmapEx ; }
            WallpaperStyle getWallpaperStyle() const { return meWallpaperStyle; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get B2Drange
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
