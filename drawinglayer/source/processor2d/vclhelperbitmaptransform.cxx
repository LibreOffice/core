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

#include <vclhelperbitmaptransform.hxx>
#include <vcl/bmpacc.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/color/bcolormodifier.hxx>

//////////////////////////////////////////////////////////////////////////////
// support for rendering Bitmap and BitmapEx contents

namespace drawinglayer
{
    namespace
    {
        void impSmoothPoint(BitmapColor& rValue, const basegfx::B2DPoint& rSource, sal_Int32 nIntX, sal_Int32 nIntY, BitmapReadAccess& rRead)
        {
            double fDeltaX(rSource.getX() - nIntX);
            double fDeltaY(rSource.getY() - nIntY);
            sal_Int32 nIndX(0L);
            sal_Int32 nIndY(0L);

            if(fDeltaX > 0.0 && nIntX + 1L < rRead.Width())
            {
                nIndX++;
            }
            else if(fDeltaX < 0.0 && nIntX >= 1L)
            {
                fDeltaX = -fDeltaX;
                nIndX--;
            }

            if(fDeltaY > 0.0 && nIntY + 1L < rRead.Height())
            {
                nIndY++;
            }
            else if(fDeltaY < 0.0 && nIntY >= 1L)
            {
                fDeltaY = -fDeltaY;
                nIndY--;
            }

            if(nIndX || nIndY)
            {
                const double fColorToReal(1.0 / 255.0);
                double fR(rValue.GetRed() * fColorToReal);
                double fG(rValue.GetGreen() * fColorToReal);
                double fB(rValue.GetBlue() * fColorToReal);
                double fRBottom(0.0), fGBottom(0.0), fBBottom(0.0);

                if(nIndX)
                {
                    const double fMulA(fDeltaX * fColorToReal);
                    double fMulB(1.0 - fDeltaX);
                    const BitmapColor aTopPartner(rRead.GetColor(nIntY, nIntX + nIndX));

                    fR = (fR * fMulB) + (aTopPartner.GetRed() * fMulA);
                    fG = (fG * fMulB) + (aTopPartner.GetGreen() * fMulA);
                    fB = (fB * fMulB) + (aTopPartner.GetBlue() * fMulA);

                    if(nIndY)
                    {
                        fMulB *= fColorToReal;
                        const BitmapColor aBottom(rRead.GetColor(nIntY + nIndY, nIntX));
                        const BitmapColor aBottomPartner(rRead.GetColor(nIntY + nIndY, nIntX + nIndX));

                        fRBottom = (aBottom.GetRed() * fMulB) + (aBottomPartner.GetRed() * fMulA);
                        fGBottom = (aBottom.GetGreen() * fMulB) + (aBottomPartner.GetGreen() * fMulA);
                        fBBottom = (aBottom.GetBlue() * fMulB) + (aBottomPartner.GetBlue() * fMulA);
                    }
                }

                if(nIndY)
                {
                    if(!nIndX)
                    {
                        const BitmapColor aBottom(rRead.GetColor(nIntY + nIndY, nIntX));

                        fRBottom = aBottom.GetRed() * fColorToReal;
                        fGBottom = aBottom.GetGreen() * fColorToReal;
                        fBBottom = aBottom.GetBlue() * fColorToReal;
                    }

                    const double fMulB(1.0 - fDeltaY);

                    fR = (fR * fMulB) + (fRBottom * fDeltaY);
                    fG = (fG * fMulB) + (fGBottom * fDeltaY);
                    fB = (fB * fMulB) + (fBBottom * fDeltaY);
                }

                rValue.SetRed((sal_uInt8)(fR * 255.0));
                rValue.SetGreen((sal_uInt8)(fG * 255.0));
                rValue.SetBlue((sal_uInt8)(fB * 255.0));
            }
        }

        Bitmap impTransformBitmap(
            const Bitmap& rSource,
            const Size aDestinationSize,
            const basegfx::B2DHomMatrix& rTransform,
            bool bSmooth)
        {
            Bitmap aDestination(aDestinationSize, 24);
            BitmapWriteAccess* pWrite = aDestination.AcquireWriteAccess();

            if(pWrite)
            {
                const Size aContentSizePixel(rSource.GetSizePixel());
                BitmapReadAccess* pRead = (const_cast< Bitmap& >(rSource)).AcquireReadAccess();

                if(pRead)
                {
                    const Size aDestinationSizePixel(aDestination.GetSizePixel());
                    bool bWorkWithIndex(rSource.GetBitCount() <= 8);
                    BitmapColor aOutside(BitmapColor(0xff, 0xff, 0xff));

                    for(sal_Int32 y(0L); y < aDestinationSizePixel.getHeight(); y++)
                    {
                        for(sal_Int32 x(0L); x < aDestinationSizePixel.getWidth(); x++)
                        {
                            const basegfx::B2DPoint aSourceCoor(rTransform * basegfx::B2DPoint(x, y));
                            const sal_Int32 nIntX(basegfx::fround(aSourceCoor.getX()));

                            if(nIntX >= 0L && nIntX < aContentSizePixel.getWidth())
                            {
                                const sal_Int32 nIntY(basegfx::fround(aSourceCoor.getY()));

                                if(nIntY >= 0L && nIntY < aContentSizePixel.getHeight())
                                {
                                    // inside pixel
                                    BitmapColor aValue;

                                    if(bWorkWithIndex)
                                    {
                                        aValue = pRead->GetPaletteColor(pRead->GetPixelIndex(nIntY, nIntX));
                                    }
                                    else
                                    {
                                        aValue = pRead->GetPixel(nIntY, nIntX);
                                    }

                                    if(bSmooth)
                                    {
                                        impSmoothPoint(aValue, aSourceCoor, nIntX, nIntY, *pRead);
                                    }

                                    pWrite->SetPixel(y, x, aValue);
                                    continue;
                                }
                            }

                            // here are outside pixels. Complete mask
                            if(bWorkWithIndex)
                            {
                                pWrite->SetPixel(y, x, aOutside);
                            }
                        }
                    }

                    delete pRead;
                }

                delete pWrite;
            }

            rSource.AdaptBitCount(aDestination);

            return aDestination;
        }
    } // end of anonymous namespace
} // end of namespace drawinglayer

namespace drawinglayer
{
    BitmapEx impTransformBitmapEx(
        const BitmapEx& rSource,
        const Rectangle& rCroppedRectPixel,
        const basegfx::B2DHomMatrix& rTransform)
    {
        // force destination to 24 bit, we want to smooth output
        const Size aDestinationSize(rCroppedRectPixel.GetSize());
        static bool bDoSmoothAtAll(true);
        const Bitmap aDestination(impTransformBitmap(rSource.GetBitmap(), aDestinationSize, rTransform, bDoSmoothAtAll));

        // create mask
        if(rSource.IsTransparent())
        {
            if(rSource.IsAlpha())
            {
                const Bitmap aAlpha(impTransformBitmap(rSource.GetAlpha().GetBitmap(), aDestinationSize, rTransform, bDoSmoothAtAll));
                return BitmapEx(aDestination, AlphaMask(aAlpha));
            }
            else
            {
                const Bitmap aMask(impTransformBitmap(rSource.GetMask(), aDestinationSize, rTransform, false));
                return BitmapEx(aDestination, aMask);
            }
        }

        return BitmapEx(aDestination);
    }

    BitmapEx impModifyBitmapEx(
        const basegfx::BColorModifierStack& rBColorModifierStack,
        const BitmapEx& rSource)
    {
        Bitmap aChangedBitmap(rSource.GetBitmap());
        bool bDone(false);

        for(sal_uInt32 a(rBColorModifierStack.count()); a && !bDone; )
        {
            const basegfx::BColorModifier& rModifier = rBColorModifierStack.getBColorModifier(--a);

            switch(rModifier.getMode())
            {
                case basegfx::BCOLORMODIFYMODE_REPLACE :
                {
                    // complete replace
                    if(rSource.IsTransparent())
                    {
                        // clear bitmap with dest color
                        if(aChangedBitmap.GetBitCount() <= 8)
                        {
                            // do NOT use erase; for e.g. 8bit Bitmaps, the nearest color to the given
                            // erase color is determined and used -> this may be different from what is
                            // wanted here. Better create a new bitmap with the needed color explicitly
                            BitmapReadAccess* pReadAccess = aChangedBitmap.AcquireReadAccess();
                            OSL_ENSURE(pReadAccess, "Got no Bitmap ReadAccess ?!?");

                            if(pReadAccess)
                            {
                                BitmapPalette aNewPalette(pReadAccess->GetPalette());
                                aNewPalette[0] = BitmapColor(Color(rModifier.getBColor()));
                                aChangedBitmap = Bitmap(
                                    aChangedBitmap.GetSizePixel(),
                                    aChangedBitmap.GetBitCount(),
                                    &aNewPalette);
                                delete pReadAccess;
                            }
                        }
                        else
                        {
                            aChangedBitmap.Erase(Color(rModifier.getBColor()));
                        }
                    }
                    else
                    {
                        // erase bitmap, caller will know to paint direct
                        aChangedBitmap.SetEmpty();
                    }

                    bDone = true;
                    break;
                }

                default : // BCOLORMODIFYMODE_INTERPOLATE, BCOLORMODIFYMODE_GRAY, BCOLORMODIFYMODE_BLACKANDWHITE
                {
                    BitmapWriteAccess* pContent = aChangedBitmap.AcquireWriteAccess();

                    if(pContent)
                    {
                        const double fConvertColor(1.0 / 255.0);

                        for(sal_uInt32 y(0L); y < (sal_uInt32)pContent->Height(); y++)
                        {
                            for(sal_uInt32 x(0L); x < (sal_uInt32)pContent->Width(); x++)
                            {
                                const BitmapColor aBMCol(pContent->GetColor(y, x));
                                const basegfx::BColor aBSource(
                                    (double)aBMCol.GetRed() * fConvertColor,
                                    (double)aBMCol.GetGreen() * fConvertColor,
                                    (double)aBMCol.GetBlue() * fConvertColor);
                                const basegfx::BColor aBDest(rModifier.getModifiedColor(aBSource));

                                pContent->SetPixel(y, x, BitmapColor(Color(aBDest)));
                            }
                        }

                        delete pContent;
                    }

                    break;
                }
            }
        }

        if(aChangedBitmap.IsEmpty())
        {
            return BitmapEx();
        }
        else
        {
            if(rSource.IsTransparent())
            {
                if(rSource.IsAlpha())
                {
                    return BitmapEx(aChangedBitmap, rSource.GetAlpha());
                }
                else
                {
                    return BitmapEx(aChangedBitmap, rSource.GetMask());
                }
            }
            else
            {
                return BitmapEx(aChangedBitmap);
            }
        }
    }
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
