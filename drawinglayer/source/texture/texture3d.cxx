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

#include <drawinglayer/texture/texture3d.hxx>
#include <vcl/bitmapaccess.hxx>
#include <drawinglayer/primitive3d/hatchtextureprimitive3d.hxx>


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxMono::GeoTexSvxMono(
            const basegfx::BColor& rSingleColor,
            double fOpacity)
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


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxBitmapEx::GeoTexSvxBitmapEx(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DRange& rRange)
        :   maBitmapEx(rBitmapEx),
            maTopLeft(rRange.getMinimum()),
            maSize(rRange.getRange()),
            mfMulX(0.0),
            mfMulY(0.0),
            mbIsAlpha(false)
        {
            if(maBitmapEx.IsTransparent())
            {
                if(maBitmapEx.IsAlpha())
                {
                    mbIsAlpha = true;
                }
            }

            mfMulX = static_cast<double>(maBitmapEx.GetSizePixel().Width()) / maSize.getX();
            mfMulY = static_cast<double>(maBitmapEx.GetSizePixel().Height()) / maSize.getY();

            if(maSize.getX() <= 1.0)
            {
                maSize.setX(1.0);
            }

            if(maSize.getY() <= 1.0)
            {
                maSize.setY(1.0);
            }
        }

        GeoTexSvxBitmapEx::~GeoTexSvxBitmapEx()
        {
        }

        sal_uInt8 GeoTexSvxBitmapEx::impGetTransparence(sal_Int32 nX, sal_Int32 nY) const
        {
            return maBitmapEx.GetTransparency(nX, nY);
        }

        bool GeoTexSvxBitmapEx::impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const
        {
            rX = static_cast<sal_Int32>((rUV.getX() - maTopLeft.getX()) * mfMulX);

            if(rX >= 0 && rX < maBitmapEx.GetSizePixel().Width())
            {
                rY = static_cast<sal_Int32>((rUV.getY() - maTopLeft.getY()) * mfMulY);

                return (rY >= 0 && rY < maBitmapEx.GetSizePixel().Height());
            }

            return false;
        }

        void GeoTexSvxBitmapEx::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const double fConvertColor(1.0 / 255.0);
                const ::Color aBMCol(maBitmapEx.GetPixelColor(nX, nY));
                const basegfx::BColor aBSource(
                    static_cast<double>(aBMCol.GetRed()) * fConvertColor,
                    static_cast<double>(aBMCol.GetGreen()) * fConvertColor,
                    static_cast<double>(aBMCol.GetBlue()) * fConvertColor);

                rBColor = aBSource;

                if(maBitmapEx.IsTransparent())
                {
                    // when we have a transparence, make use of it
                    const sal_uInt8 aLuminance = aBMCol.GetTransparency();

                    rfOpacity = (static_cast<double>(0xff - aLuminance) * (1.0 / 255.0));
                }
                else
                {
                    rfOpacity = 1.0;
                }
            }
            else
            {
                rfOpacity = 0.0;
            }
        }

        void GeoTexSvxBitmapEx::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                if(maBitmapEx.IsTransparent())
                {
                    // this texture has an alpha part, use it
                    const sal_uInt8 aLuminance(impGetTransparence(nX, nY));
                    const double fNewOpacity(static_cast<double>(0xff - aLuminance) * (1.0 / 255.0));

                    rfOpacity = 1.0 - ((1.0 - fNewOpacity) * (1.0 - rfOpacity));
                }
                else
                {
                    // this texture is a color bitmap used as transparence map
                    const ::Color aColor(maBitmapEx.GetPixelColor(nX, nY));

                    rfOpacity = (static_cast<double>(0xff - aColor.GetLuminance()) * (1.0 / 255.0));
                }
            }
            else
            {
                rfOpacity = 0.0;
            }
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        basegfx::B2DPoint GeoTexSvxBitmapExTiled::impGetCorrected(const basegfx::B2DPoint& rUV) const
        {
            double fX(rUV.getX() - maTopLeft.getX());
            double fY(rUV.getY() - maTopLeft.getY());

            if(mbUseOffsetX)
            {
                const sal_Int32 nCol(static_cast< sal_Int32 >((fY < 0.0 ? maSize.getY() -fY : fY) / maSize.getY()));

                if(nCol % 2)
                {
                    fX += mfOffsetX * maSize.getX();
                }
            }
            else if(mbUseOffsetY)
            {
                const sal_Int32 nRow(static_cast< sal_Int32 >((fX < 0.0 ? maSize.getX() -fX : fX) / maSize.getX()));

                if(nRow % 2)
                {
                    fY += mfOffsetY * maSize.getY();
                }
            }

            fX = fmod(fX, maSize.getX());
            fY = fmod(fY, maSize.getY());

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

        GeoTexSvxBitmapExTiled::GeoTexSvxBitmapExTiled(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DRange& rRange,
            double fOffsetX,
            double fOffsetY)
        :   GeoTexSvxBitmapEx(rBitmapEx, rRange),
            mfOffsetX(basegfx::clamp(fOffsetX, 0.0, 1.0)),
            mfOffsetY(basegfx::clamp(fOffsetY, 0.0, 1.0)),
            mbUseOffsetX(!basegfx::fTools::equalZero(mfOffsetX)),
            mbUseOffsetY(!mbUseOffsetX && !basegfx::fTools::equalZero(mfOffsetY))
        {
        }

        void GeoTexSvxBitmapExTiled::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            GeoTexSvxBitmapEx::modifyBColor(impGetCorrected(rUV), rBColor, rfOpacity);
        }

        void GeoTexSvxBitmapExTiled::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            GeoTexSvxBitmapEx::modifyOpacity(impGetCorrected(rUV), rfOpacity);
        }
    } // end of namespace texture
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace texture
    {
        GeoTexSvxMultiHatch::GeoTexSvxMultiHatch(
            const primitive3d::HatchTexturePrimitive3D& rPrimitive,
            double fLogicPixelSize)
        :   mfLogicPixelSize(fLogicPixelSize),
            mp0(nullptr),
            mp1(nullptr),
            mp2(nullptr)
        {
            const attribute::FillHatchAttribute& rHatch(rPrimitive.getHatch());
            const basegfx::B2DRange aOutlineRange(0.0, 0.0, rPrimitive.getTextureSize().getX(), rPrimitive.getTextureSize().getY());
            const double fAngleA(rHatch.getAngle());
            maColor = rHatch.getColor();
            mbFillBackground = rHatch.isFillBackground();
            mp0.reset( new GeoTexSvxHatch(
                aOutlineRange,
                aOutlineRange,
                rHatch.getDistance(),
                fAngleA) );

            if(attribute::HatchStyle::Double == rHatch.getStyle() || attribute::HatchStyle::Triple == rHatch.getStyle())
            {
                mp1.reset( new GeoTexSvxHatch(
                    aOutlineRange,
                    aOutlineRange,
                    rHatch.getDistance(),
                    fAngleA + F_PI2) );
            }

            if(attribute::HatchStyle::Triple == rHatch.getStyle())
            {
                mp2.reset( new GeoTexSvxHatch(
                    aOutlineRange,
                    aOutlineRange,
                    rHatch.getDistance(),
                    fAngleA + F_PI4) );
            }
        }

        GeoTexSvxMultiHatch::~GeoTexSvxMultiHatch()
        {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
