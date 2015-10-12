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

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX
#define INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/texture/texture.hxx>
#include <vcl/bitmapex.hxx>

namespace drawinglayer { namespace primitive3d {
    class HatchTexturePrimitive3D;
}}

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxMono : public GeoTexSvx
        {
        protected:
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
    } // end of namespace texture
} // end of namespace drawinglayer

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxBitmapEx : public GeoTexSvx
        {
        protected:
            BitmapEx                                    maBitmapEx;
            BitmapReadAccess*                           mpReadBitmap;
            Bitmap                                      maTransparence;
            BitmapReadAccess*                           mpReadTransparence;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;
            double                                      mfMulX;
            double                                      mfMulY;

            /// bitfield
            bool                                        mbIsAlpha : 1;
            bool                                        mbIsTransparent : 1;

            // helpers
            bool impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const;
            sal_uInt8 impGetTransparence(sal_Int32& rX, sal_Int32& rY) const;

        public:
            GeoTexSvxBitmapEx(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DRange& rRange);
            virtual ~GeoTexSvxBitmapEx();

            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxBitmapExTiled : public GeoTexSvxBitmapEx
        {
        protected:
            double                                      mfOffsetX;
            double                                      mfOffsetY;

            /// bitfield
            bool                                        mbUseOffsetX : 1;
            bool                                        mbUseOffsetY : 1;

            // helpers
            basegfx::B2DPoint impGetCorrected(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxBitmapExTiled(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DRange& rRange,
                double fOffsetX = 0.0,
                double fOffsetY = 0.0);

            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxMultiHatch : public GeoTexSvx
        {
        protected:
            basegfx::BColor                 maColor;
            double                          mfLogicPixelSize;
            GeoTexSvxHatch*                 mp0;
            GeoTexSvxHatch*                 mp1;
            GeoTexSvxHatch*                 mp2;

            // bitfield
            bool                            mbFillBackground : 1;

            // helpers
            bool impIsOnHatch(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize);
            virtual ~GeoTexSvxMultiHatch();
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const override;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const override;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

#endif // INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
