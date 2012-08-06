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
