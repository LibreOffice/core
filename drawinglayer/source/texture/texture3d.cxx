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
            mpReadBitmap(nullptr),
            maTransparence(),
            mpReadTransparence(nullptr),
            maTopLeft(rRange.getMinimum()),
            maSize(rRange.getRange()),
            mfMulX(0.0),
            mfMulY(0.0),
            mbIsAlpha(false),
            mbIsTransparent(maBitmapEx.IsTransparent())
        {
            // #121194# Todo: use alpha channel, too (for 3d)
            mpReadBitmap = maBitmapEx.GetBitmap().AcquireReadAccess();
            OSL_ENSURE(mpReadBitmap, "GeoTexSvxBitmapEx: Got no read access to Bitmap (!)");

            if(mbIsTransparent)
            {
                if(maBitmapEx.IsAlpha())
                {
                    mbIsAlpha = true;
                    maTransparence = rBitmapEx.GetAlpha().GetBitmap();
                }
                else
                {
                    maTransparence = rBitmapEx.GetMask();
                }

                mpReadTransparence = maTransparence.AcquireReadAccess();
            }

            mfMulX = (double)mpReadBitmap->Width() / maSize.getX();
            mfMulY = (double)mpReadBitmap->Height() / maSize.getY();

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
            delete mpReadTransparence;
            delete mpReadBitmap;
        }

        sal_uInt8 GeoTexSvxBitmapEx::impGetTransparence(sal_Int32& rX, sal_Int32& rY) const
        {
            switch(maBitmapEx.GetTransparentType())
            {
                case TRANSPARENT_NONE:
                {
                    break;
                }
                case TRANSPARENT_COLOR:
                {
                    const BitmapColor aBitmapColor(mpReadBitmap->GetColor(rY, rX));

                    if(maBitmapEx.GetTransparentColor() == aBitmapColor.operator Color())
                    {
                        return 255;
                    }

                    break;
                }
                case TRANSPARENT_BITMAP:
                {
                    OSL_ENSURE(mpReadTransparence, "OOps, transparence type Bitmap, but no read access created in the constructor (?)");
                    const BitmapColor aBitmapColor(mpReadTransparence->GetPixel(rY, rX));

                    if(mbIsAlpha)
                    {
                        return aBitmapColor.GetIndex();
                    }
                    else
                    {
                        if(0x00 != aBitmapColor.GetIndex())
                        {
                            return 255;
                        }
                    }
                    break;
                }
            }

            return 0;
        }

        bool GeoTexSvxBitmapEx::impIsValid(const basegfx::B2DPoint& rUV, sal_Int32& rX, sal_Int32& rY) const
        {
            if(mpReadBitmap)
            {
                rX = (sal_Int32)((rUV.getX() - maTopLeft.getX()) * mfMulX);

                if(rX >= 0L && rX < mpReadBitmap->Width())
                {
                    rY = (sal_Int32)((rUV.getY() - maTopLeft.getY()) * mfMulY);

                    return (rY >= 0L && rY < mpReadBitmap->Height());
                }
            }

            return false;
        }

        void GeoTexSvxBitmapEx::modifyBColor(const basegfx::B2DPoint& rUV, basegfx::BColor& rBColor, double& rfOpacity) const
        {
            sal_Int32 nX, nY;

            if(impIsValid(rUV, nX, nY))
            {
                const double fConvertColor(1.0 / 255.0);
                const BitmapColor aBMCol(mpReadBitmap->GetColor(nY, nX));
                const basegfx::BColor aBSource(
                    (double)aBMCol.GetRed() * fConvertColor,
                    (double)aBMCol.GetGreen() * fConvertColor,
                    (double)aBMCol.GetBlue() * fConvertColor);

                rBColor = aBSource;

                if(mbIsTransparent)
                {
                    // when we have a transparence, make use of it
                    const sal_uInt8 aLuminance(impGetTransparence(nX, nY));

                    rfOpacity = ((double)(0xff - aLuminance) * (1.0 / 255.0));
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
                if(mbIsTransparent)
                {
                    // this texture has an alpha part, use it
                    const sal_uInt8 aLuminance(impGetTransparence(nX, nY));
                    const double fNewOpacity((double)(0xff - aLuminance) * (1.0 / 255.0));

                    rfOpacity = 1.0 - ((1.0 - fNewOpacity) * (1.0 - rfOpacity));
                }
                else
                {
                    // this texture is a color bitmap used as transparence map
                    const BitmapColor aBMCol(mpReadBitmap->GetColor(nY, nX));
                    const Color aColor(aBMCol.GetRed(), aBMCol.GetGreen(), aBMCol.GetBlue());

                    rfOpacity = ((double)(0xff - aColor.GetLuminance()) * (1.0 / 255.0));
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
            if(mpReadBitmap)
            {
                GeoTexSvxBitmapEx::modifyBColor(impGetCorrected(rUV), rBColor, rfOpacity);
            }
        }

        void GeoTexSvxBitmapExTiled::modifyOpacity(const basegfx::B2DPoint& rUV, double& rfOpacity) const
        {
            if(mpReadBitmap)
            {
                GeoTexSvxBitmapEx::modifyOpacity(impGetCorrected(rUV), rfOpacity);
            }
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
            mp0 = new GeoTexSvxHatch(
                aOutlineRange,
                aOutlineRange,
                rHatch.getDistance(),
                fAngleA);

            if(attribute::HATCHSTYLE_DOUBLE == rHatch.getStyle() || attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp1 = new GeoTexSvxHatch(
                    aOutlineRange,
                    aOutlineRange,
                    rHatch.getDistance(),
                    fAngleA + F_PI2);
            }

            if(attribute::HATCHSTYLE_TRIPLE == rHatch.getStyle())
            {
                mp2 = new GeoTexSvxHatch(
                    aOutlineRange,
                    aOutlineRange,
                    rHatch.getDistance(),
                    fAngleA + F_PI4);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
