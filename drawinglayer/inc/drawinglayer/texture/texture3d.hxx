/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texture3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2008-04-16 04:59:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX
#define INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE3D_HXX

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

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
        class GeoTexSvxMono : public GeoTexSvx
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
        class GeoTexSvxBitmap : public GeoTexSvx
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
        class GeoTexSvxBitmapTiled : public GeoTexSvxBitmap
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
        class GeoTexSvxMultiHatch : public GeoTexSvx
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

// eof
