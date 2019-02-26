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
#include <opengl/salbmp.hxx>

#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#include <outdata.hxx>
#include <salgdiimpl.hxx>
#include <opengl/win/gdiimpl.hxx>


bool WinSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    static bool bAllowForTest(true);
    bool bRet = false;

    switch( eType )
    {
    case OutDevSupportType::TransparentRect:
        bRet = mbVirDev || mbWindow;
        break;
    case OutDevSupportType::B2DDraw:
        bRet = bAllowForTest;
        break;
    default: break;
    }
    return bRet;
}

void WinSalGraphics::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    mpImpl->copyBits( rPosAry, pSrcGraphics );
}

void WinSalGraphics::copyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
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
    ScanlineFormat meDestinationFormat;

    int mnComponentSize;
    int mnComponentExchangeIndex;

    long mnScanlineSize;

    ColorScanlineConverter(ScanlineFormat eSourceFormat, ScanlineFormat eDestinationFormat, int nComponentSize, long nScanlineSize)
        : meSourceFormat(eSourceFormat)
        , meDestinationFormat(eDestinationFormat)
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
        for (long x = 0; x < mnScanlineSize; x += mnComponentSize)
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
    OpenGLSalBitmap* pGLSalBitmap = dynamic_cast<OpenGLSalBitmap*>(&rSalBitmap);
    if (pGLSalBitmap != nullptr)
    {
        aBitmapPalette = pGLSalBitmap->GetBitmapPalette();
    }

    BitmapBuffer* pRead = rSalBitmap.AcquireBuffer(BitmapAccessMode::Read);

    rWinSalBitmap.Create(rSalBitmap.GetSize(), rSalBitmap.GetBitCount(), aBitmapPalette);
    BitmapBuffer* pWrite = rWinSalBitmap.AcquireBuffer(BitmapAccessMode::Write);

    sal_uInt8* pSource(pRead->mpBits);
    sal_uInt8* pDestination(pWrite->mpBits);

    std::unique_ptr<ColorScanlineConverter> pConverter;

    if (pRead->mnFormat == ScanlineFormat::N24BitTcRgb)
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N24BitTcRgb, ScanlineFormat::N24BitTcBgr,
                                                    3, pRead->mnScanlineSize));
    else if (pRead->mnFormat == ScanlineFormat::N32BitTcRgba)
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N32BitTcRgba, ScanlineFormat::N32BitTcBgra,
                                                    4, pRead->mnScanlineSize));
    if (pConverter)
    {
        for (long y = 0; y < pRead->mnHeight; y++)
        {
            pConverter->convertScanline(pSource, pDestination);
            pSource += pRead->mnScanlineSize;
            pDestination += pWrite->mnScanlineSize;
        }
    }
    else
    {
        for (long y = 0; y < pRead->mnHeight; y++)
        {
            memcpy(pDestination, pSource, pRead->mnScanlineSize);
            pSource += pRead->mnScanlineSize;
            pDestination += pWrite->mnScanlineSize;
        }
    }
    rWinSalBitmap.ReleaseBuffer(pWrite, BitmapAccessMode::Write);

    rSalBitmap.ReleaseBuffer(pRead, BitmapAccessMode::Read);
}

} // end anonymous namespace

void WinSalGraphics::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get()) == nullptr &&
        dynamic_cast<const WinSalBitmap*>(&rSalBitmap) == nullptr)
    {
        std::unique_ptr<WinSalBitmap> pWinSalBitmap(new WinSalBitmap());
        SalBitmap& rConstBitmap = const_cast<SalBitmap&>(rSalBitmap);
        convertToWinSalBitmap(rConstBitmap, *pWinSalBitmap);
        mpImpl->drawBitmap(rPosAry, *pWinSalBitmap);
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
    if (dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get()) == nullptr &&
        dynamic_cast<const WinSalBitmap*>(&rSSalBitmap) == nullptr)
    {
        std::unique_ptr<WinSalBitmap> pWinSalBitmap(new WinSalBitmap());
        SalBitmap& rConstBitmap = const_cast<SalBitmap&>(rSSalBitmap);
        convertToWinSalBitmap(rConstBitmap, *pWinSalBitmap);


        std::unique_ptr<WinSalBitmap> pWinTransparentSalBitmap(new WinSalBitmap());
        SalBitmap& rConstTransparentBitmap = const_cast<SalBitmap&>(rSTransparentBitmap);
        convertToWinSalBitmap(rConstTransparentBitmap, *pWinTransparentSalBitmap);

        mpImpl->drawBitmap(rPosAry, *pWinSalBitmap, *pWinTransparentSalBitmap);
    }
    else
    {
        mpImpl->drawBitmap(rPosAry, rSSalBitmap, rSTransparentBitmap);
    }
}

bool WinSalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                    long nHeight, sal_uInt8 nTransparency )
{
    return mpImpl->drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

void WinSalGraphics::drawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSSalBitmap,
                            Color nMaskColor )
{
    mpImpl->drawMask( rPosAry, rSSalBitmap, nMaskColor );
}

std::shared_ptr<SalBitmap> WinSalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    return mpImpl->getBitmap( nX, nY, nDX, nDY );
}

Color WinSalGraphics::getPixel( long nX, long nY )
{
    return mpImpl->getPixel( nX, nY );
}

void WinSalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    mpImpl->invert( nX, nY, nWidth, nHeight, nFlags );
}

void WinSalGraphics::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nSalFlags )
{
    mpImpl->invert( nPoints, pPtAry, nSalFlags );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
