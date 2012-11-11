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
#include <vcl/bitmap.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace drawinglayer { namespace primitive3d {
    class HatchTexturePrimitive3D;
}}

//////////////////////////////////////////////////////////////////////////////

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
            GeoTexSvxMono(const basegfx::BColor& rSingleColor, double fOpacity);

            // compare operator
            virtual bool operator==(const GeoTexSvx& rGeoTexSvx) const;
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxBitmap : public GeoTexSvx
        {
        protected:
            Bitmap                                      maBitmap;
            BitmapReadAccess*                           mpRead;
            basegfx::B2DPoint                           maTopLeft;
            basegfx::B2DVector                          maSize;
            double                                      mfMulX;
            double                                      mfMulY;

            // helpers
            bool impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const;

        public:
            GeoTexSvxBitmap(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual ~GeoTexSvxBitmap();
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        class DRAWINGLAYER_DLLPUBLIC GeoTexSvxBitmapTiled : public GeoTexSvxBitmap
        {
        protected:
            // helpers
            basegfx::B2DPoint impGetCorrected(const basegfx::B2DPoint& rUV) const
            {
                double fX(fmod(rUV.getX() - maTopLeft.getX(), maSize.getX()));
                double fY(fmod(rUV.getY() - maTopLeft.getY(), maSize.getY()));

                if(fX < 0.0)
                {
                    fX += maSize.getX();
                }

                if(fY < 0.0)
                {
                    fY += maSize.getY();
                }

                return basegfx::B2DPoint(fX + maTopLeft.getX(), fY + maTopLeft.getY());
            }

        public:
            GeoTexSvxBitmapTiled(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize);
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

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
            unsigned                        mbFillBackground : 1;

            // helpers
            bool impIsOnHatch(const basegfx::B2DPoint& rUV) const;

        public:
            GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize);
            virtual ~GeoTexSvxMultiHatch();
            virtual void modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const;
            virtual void modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const;

            // dada access
            bool getFillBackground() const { return mbFillBackground; }
        };
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
