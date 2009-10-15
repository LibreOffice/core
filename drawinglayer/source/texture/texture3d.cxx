/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texture3d.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:34 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/texture/texture3d.hxx>
#include <vcl/bmpacc.hxx>
#include <drawinglayer/attribute/fillattribute.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxMono::GeoTexSvxMono(const basegfx::BColor& rSingleColor, double fOpacity)
        :   maSingleColor(rSingleColor),
            mfOpacity(fOpacity)
        {
        }

        bool GeoTexSvxMono::operator==(const GeoTexSvx& rGeoTexSvx) const
        {
            const GeoTexSvxMono* pCompare = dynamic_cast< const GeoTexSvxMono* >(&rGeoTexSvx);
            return (pCompare
                && maSingleColor == pCompare->maSingleColor
                && mfOpacity == pCompare->mfOpacity);
        }

        void GeoTexSvxMono::modifyBColor(const basegfx::B2DPoint& /*rUV*/, basegfx::BColor& rBColor, double& /*rfOpacity*/) const
        {
            rBColor = maSingleColor;
        }

        void GeoTexSvxMono::modifyOpacity(const basegfx::B2DPoint& /*rUV*/, double& rfOpacity) const
        {
            rfOpacity = mfOpacity;
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxBitmap::GeoTexSvxBitmap(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   maBitmap(rBitmap),
            mpRead(0L),
            maTopLeft(rTopLeft),
            maSize(rSize),
            mfMulX(0.0),
            mfMulY(0.0)
        {
            mpRead = maBitmap.AcquireReadAccess();
            OSL_ENSURE(mpRead, "GeoTexSvxBitmap: Got no read access to Bitmap (!)");
            mfMulX = (double)mpRead->Width() / maSize.getX();
            mfMulY = (double)mpRead->Height() / maSize.getY();
        }

        GeoTexSvxBitmap::~GeoTexSvxBitmap()
        {
            delete mpRead;
        }

        bool GeoTexSvxBitmap::impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const
        {
            if(mpRead)
            {
                rX = (sal_Int32)((rUV.getX() - maTopLeft.getX()) * mfMulX);

                if(rX >= 0L && rX < mpRead->Width())
                {
                    rY = (sal_Int32)((rUV.getY() - maTopLeft.getY()) * mfMulY);

                    return (rY >= 0L && rY < mpRead->Height());
                }
            }

            return false;
        }

        void GeoTexSvxBitmap::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const double fConvertColor(1.0 / 255.0);
                const BitmapColor aBMCol(mpRead->GetColor(nY, nX));
                const basegfx::BColor aBSource(
                    (double)aBMCol.GetRed() * fConvertColor,
                    (double)aBMCol.GetGreen() * fConvertColor,
                    (double)aBMCol.GetBlue() * fConvertColor);

                rBColor = aBSource;
            }
            else
            {
                rfOpacity = 0.0;
            }
        }

        void GeoTexSvxBitmap::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const BitmapColor aBMCol(mpRead->GetColor(nY, nX));
                const Color aColor(aBMCol.GetRed(), aBMCol.GetGreen(), aBMCol.GetBlue());

                rfOpacity = ((double)(0xff - aColor.GetLuminance()) * (1.0 / 255.0));
            }
            else
            {
                rfOpacity = 0.0;
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxBitmapTiled::GeoTexSvxBitmapTiled(const Bitmap& rBitmap, const basegfx::B2DPoint& rTopLeft, const basegfx::B2DVector& rSize)
        :   GeoTexSvxBitmap(rBitmap, rTopLeft, rSize)
        {
        }

        void GeoTexSvxBitmapTiled::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            if(mpRead)
            {
                GeoTexSvxBitmap::modifyBColor(impGetCorrected(rUV), rBColor, rfOpacity);
            }
        }

        void GeoTexSvxBitmapTiled::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            if(mpRead)
            {
                GeoTexSvxBitmap::modifyOpacity(impGetCorrected(rUV), rfOpacity);
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxMultiHatch::GeoTexSvxMultiHatch(const primitive3d::HatchTexturePrimitive3D& rPrimitive, double fLogicPixelSize)
        :   mfLogicPixelSize(fLogicPixelSize),
            mp0(0L),
            mp1(0L),
            mp2(0L)
        {
            const attribute::FillHatchAttribute& rHatch(rPrimitive.getHatch());
            const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
            const double fAngleA(rHatch.getAngle());
            maColor = rHatch.getColor();
            mbFillBackground = rHatch.isFillBackground();
            mp0 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA);

            if(attribute::HATCHSTYLE_DOUBLE == rHatch.getStyle() || attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp1 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA + F_PI2);
            }

            if(attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp2 = new GeoTexSvxHatch(aOutlineRange, rHatch.getDistance(), fAngleA + F_PI4);
            }
        }

        GeoTexSvxMultiHatch::~GeoTexSvxMultiHatch()
        {
            delete mp0;
            delete mp1;
            delete mp2;
        }

        bool GeoTexSvxMultiHatch::impIsOnHatch(const basegfx::B2DPoint& rUV) const
        {
            if(mp0->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            if(mp1 && mp1->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            if(mp2 && mp2->getDistanceToHatch(rUV) < mfLogicPixelSize)
            {
                return true;
            }

            return false;
        }

        void GeoTexSvxMultiHatch::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            if(impIsOnHatch(rUV))
            {
                rBColor = maColor;
            }
            else if(!mbFillBackground)
            {
                rfOpacity = 0.0;
            }
        }

        void GeoTexSvxMultiHatch::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            if(mbFillBackground || impIsOnHatch(rUV))
            {
                rfOpacity = 1.0;
            }
            else
            {
                rfOpacity = 0.0;
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
