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

#include <texture/texture.hxx>
#include <vcl/bitmapex.hxx>

namespace drawinglayer::primitive3d {
    class HatchTexturePrimitive3D;
}

namespace drawinglayer::texture
    {
        class GeoTexSvxMono final : public GeoTexSvx
        {
            basegfx::BColor                             maSingleColor;
            double                                      mfOpacity;

        public:
            GeoTexSvxMono(
                const basegfx::BColor& rSingleColor,
                double fOpacity);

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const override;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };

} // end of namespace drawinglayer::texture

namespace drawinglayer::texture
    {
        class GeoTexSvxBitmapEx : public GeoTexSvx
        {
        protected:
            BitmapEx                                    maBitmapEx;
            Bitmap                                      maBitmap;   // Bitmap held within maBitmapEx, to exist during mpReadBitmap scope
            Bitmap::ScopedReadAccess                    mpReadBitmap;
            Bitmap                                      maTransparence;
            Bitmap::ScopedReadAccess                    mpReadTransparence;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;
            double                                      mfMulX;
            double                                      mfMulY;

            bool                                        mbIsAlpha : 1;

            // helpers
            bool impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const;
            sal_uInt8 impGetTransparence(sal_Int32 rX, sal_Int32 rY) const;

        public:
            GeoTexSvxBitmapEx(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DRange& rRange);
            virtual ~GeoTexSvxBitmapEx() override;

            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };

} // end of namespace drawinglayer::texture

namespace drawinglayer::texture
    {
        class GeoTexSvxBitmapExTiled final : public GeoTexSvxBitmapEx
        {
            double                                      mfOffsetX;
            double                                      mfOffsetY;

            bool                                        mbUseOffsetX : 1;
            bool                                        mbUseOffsetY : 1;

            // helpers
            basegfx::B2DPoint impGetCorrected(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxBitmapExTiled(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DRange& rRange,
                double fOffsetX,
                double fOffsetY);

            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };

} // end of namespace drawinglayer::texture

namespace drawinglayer::texture
    {
        class GeoTexSvxMultiHatch final : public GeoTexSvx
        {
            basegfx::BColor                 maColor;
            double                          mfLogicPixelSize;
            std::unique_ptr<GeoTexSvxHatch> mp0;
            std::unique_ptr<GeoTexSvxHatch> mp1;
            std::unique_ptr<GeoTexSvxHatch> mp2;

            bool                            mbFillBackground : 1;

            // helpers
            bool impIsOnHatch(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize);
            virtual ~GeoTexSvxMultiHatch() override;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };

} // end of namespace drawinglayer::texture

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
