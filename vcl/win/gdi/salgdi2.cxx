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

#include <memory>
#include <string.h>
#include <stdlib.h>

#include <svsys.h>

#include <win/wincomp.hxx>
#include <win/salbmp.h>
#include <win/saldata.hxx>
#include <win/salids.hrc>
#include <win/salgdi.h>
#include <win/salframe.h>

#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapPalette.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/Scanline.hxx>
#include <salgdiimpl.hxx>

#include <config_features.h>
#if HAVE_FEATURE_SKIA
#include <skia/win/gdiimpl.hxx>
#include <skia/salbmp.hxx>
#endif


bool WinSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    return mpImpl->supportsOperation(eType);
}

void WinSalGraphics::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    mpImpl->copyBits( rPosAry, pSrcGraphics );
}

void WinSalGraphics::copyArea( tools::Long nDestX, tools::Long nDestY,
                            tools::Long nSrcX, tools::Long nSrcY,
                            tools::Long nSrcWidth, tools::Long nSrcHeight,
                            bool bWindowInvalidate )
{
    mpImpl->copyArea( nDestX, nDestY, nSrcX, nSrcY,
            nSrcWidth, nSrcHeight, bWindowInvalidate );
}

namespace
{

class ColorScanlineConverter
{
public:
    ScanlineFormat meSourceFormat;

    int mnComponentSize;
    int mnComponentExchangeIndex;

    tools::Long mnScanlineSize;

    ColorScanlineConverter(ScanlineFormat eSourceFormat, int nComponentSize, tools::Long nScanlineSize)
        : meSourceFormat(eSourceFormat)
        , mnComponentSize(nComponentSize)
        , mnComponentExchangeIndex(0)
        , mnScanlineSize(nScanlineSize)
    {
        if (meSourceFormat == ScanlineFormat::N32BitTcAbgr ||
            meSourceFormat == ScanlineFormat::N32BitTcArgb)
        {
            mnComponentExchangeIndex = 1;
        }
    }

    void convertScanline(sal_uInt8* pSource, sal_uInt8* pDestination)
    {
        for (tools::Long x = 0; x < mnScanlineSize; x += mnComponentSize)
        {
            for (int i = 0; i < mnComponentSize; ++i)
            {
                pDestination[x + i] = pSource[x + i];
            }
            pDestination[x + mnComponentExchangeIndex + 0] = pSource[x + mnComponentExchangeIndex + 2];
            pDestination[x + mnComponentExchangeIndex + 2] = pSource[x + mnComponentExchangeIndex + 0];
        }
    }
};

void convertToWinSalBitmap(SalBitmap& rSalBitmap, WinSalBitmap& rWinSalBitmap)
{
    BitmapPalette aBitmapPalette;
#if HAVE_FEATURE_SKIA
    if(SkiaSalBitmap* pSkiaSalBitmap = dynamic_cast<SkiaSalBitmap*>(&rSalBitmap))
        aBitmapPalette = pSkiaSalBitmap->Palette();
#endif

    BitmapBuffer* pRead = rSalBitmap.AcquireBuffer(BitmapAccessMode::Read);

    rWinSalBitmap.Create(rSalBitmap.GetSize(), vcl::bitDepthToPixelFormat(rSalBitmap.GetBitCount()), aBitmapPalette);
    BitmapBuffer* pWrite = rWinSalBitmap.AcquireBuffer(BitmapAccessMode::Write);

    sal_uInt8* pSource(pRead->mpBits);
    sal_uInt8* pDestination(pWrite->mpBits);
    tools::Long readRowChange = pRead->mnScanlineSize;
    if (pRead->meDirection == ScanlineDirection::TopDown)
    {
        pSource += pRead->mnScanlineSize * (pRead->mnHeight - 1);
        readRowChange = -readRowChange;
    }

    std::unique_ptr<ColorScanlineConverter> pConverter;

    if (pRead->meFormat == ScanlineFormat::N24BitTcRgb)
    {
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N24BitTcRgb, 3, pRead->mnScanlineSize));
    }
    else if (pRead->meFormat == ScanlineFormat::N32BitTcRgba)
    {
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N32BitTcRgba, 4, pRead->mnScanlineSize));
    }
    if (pConverter)
    {
        for (tools::Long y = 0; y < pRead->mnHeight; y++)
        {
            pConverter->convertScanline(pSource, pDestination);
            pSource += readRowChange;
            pDestination += pWrite->mnScanlineSize;
        }
    }
    else
    {
        for (tools::Long y = 0; y < pRead->mnHeight; y++)
        {
            memcpy(pDestination, pSource, pRead->mnScanlineSize);
            pSource += readRowChange;
            pDestination += pWrite->mnScanlineSize;
        }
    }
    rWinSalBitmap.ReleaseBuffer(pWrite, BitmapAccessMode::Write);

    rSalBitmap.ReleaseBuffer(pRead, BitmapAccessMode::Read);
}

} // end anonymous namespace

void WinSalGraphics::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (dynamic_cast<const WinSalBitmap*>(&rSalBitmap) == nullptr
#if HAVE_FEATURE_SKIA
        && dynamic_cast<WinSkiaSalGraphicsImpl*>(mpImpl.get()) == nullptr
#endif
        )
    {
        WinSalBitmap aWinSalBitmap;
        SalBitmap& rConstBitmap = const_cast<SalBitmap&>(rSalBitmap);
        convertToWinSalBitmap(rConstBitmap, aWinSalBitmap);
        mpImpl->drawBitmap(rPosAry, aWinSalBitmap);
    }
    else
    {
        mpImpl->drawBitmap(rPosAry, rSalBitmap);
    }
}

void WinSalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSSalBitmap,
                              const SalBitmap& rSTransparentBitmap )
{
    if (dynamic_cast<const WinSalBitmap*>(&rSSalBitmap) == nullptr
#if HAVE_FEATURE_SKIA
        && dynamic_cast<WinSkiaSalGraphicsImpl*>(mpImpl.get()) == nullptr
#endif
        )
    {
        WinSalBitmap aWinSalBitmap;
        SalBitmap& rConstBitmap = const_cast<SalBitmap&>(rSSalBitmap);
        convertToWinSalBitmap(rConstBitmap, aWinSalBitmap);

        WinSalBitmap aWinTransparentSalBitmap;
        SalBitmap& rConstTransparentBitmap = const_cast<SalBitmap&>(rSTransparentBitmap);
        convertToWinSalBitmap(rConstTransparentBitmap, aWinTransparentSalBitmap);

        mpImpl->drawBitmap(rPosAry, aWinSalBitmap, aWinTransparentSalBitmap);
    }
    else
    {
        mpImpl->drawBitmap(rPosAry, rSSalBitmap, rSTransparentBitmap);
    }
}

bool WinSalGraphics::drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth,
                                    tools::Long nHeight, sal_uInt8 nTransparency )
{
    return mpImpl->drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

void WinSalGraphics::drawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSSalBitmap,
                            Color nMaskColor )
{
    mpImpl->drawMask( rPosAry, rSSalBitmap, nMaskColor );
}

std::shared_ptr<SalBitmap> WinSalGraphics::getBitmap( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY )
{
    return mpImpl->getBitmap( nX, nY, nDX, nDY );
}

Color WinSalGraphics::getPixel( tools::Long nX, tools::Long nY )
{
    return mpImpl->getPixel( nX, nY );
}

void WinSalGraphics::invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags )
{
    mpImpl->invert( nX, nY, nWidth, nHeight, nFlags );
}

void WinSalGraphics::invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nSalFlags )
{
    mpImpl->invert( nPoints, pPtAry, nSalFlags );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
