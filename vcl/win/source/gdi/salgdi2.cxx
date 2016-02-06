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

#include <string.h>
#include <stdlib.h>

#include <svsys.h>
#include <tools/debug.hxx>

#include <win/wincomp.hxx>
#include <win/salbmp.h>
#include <win/saldata.hxx>
#include <win/salids.hrc>
#include <win/salgdi.h>
#include <win/salframe.h>
#include <opengl/salbmp.hxx>

#include "vcl/salbtype.hxx"
#include "vcl/bitmapaccess.hxx"
#include "outdata.hxx"
#include "salgdiimpl.hxx"
#include "opengl/win/gdiimpl.hxx"

bool WinSalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    static bool bAllowForTest(true);
    bool bRet = false;

    switch( eType )
    {
    case OutDevSupport_TransparentRect:
        bRet = mbVirDev || mbWindow;
        break;
    case OutDevSupport_B2DClip:
        bRet = true;
        break;
    case OutDevSupport_B2DDraw:
        bRet = bAllowForTest;
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
                            sal_uInt16 nFlags )
{
    mpImpl->copyArea( nDestX, nDestY, nSrcX, nSrcY,
            nSrcWidth, nSrcHeight, nFlags );
}

namespace
{

void convertToWinSalBitmap(SalBitmap& rSalBitmap, WinSalBitmap& rWinSalBitmap)
{
        BitmapPalette aBitmapPalette;
        OpenGLSalBitmap* pGLSalBitmap = dynamic_cast<OpenGLSalBitmap*>(&rSalBitmap);
        if (pGLSalBitmap != nullptr)
        {
            aBitmapPalette = pGLSalBitmap->GetBitmapPalette();
        }

        BitmapBuffer* pRead = rSalBitmap.AcquireBuffer(BITMAP_READ_ACCESS);
        rWinSalBitmap.Create(rSalBitmap.GetSize(), rSalBitmap.GetBitCount(), aBitmapPalette);
        BitmapBuffer* pWrite = rWinSalBitmap.AcquireBuffer(BITMAP_WRITE_ACCESS);

        sal_uInt8* pSource(pRead->mpBits);
        sal_uInt8* pDestination(pWrite->mpBits);

        for (long y = 0; y < pRead->mnHeight; y++)
        {
            memcpy(pDestination, pSource, pRead->mnScanlineSize);
            pSource += pRead->mnScanlineSize;
            pDestination += pWrite->mnScanlineSize;
        }
        rWinSalBitmap.ReleaseBuffer(pWrite, BITMAP_WRITE_ACCESS);

        rSalBitmap.ReleaseBuffer(pRead, BITMAP_READ_ACCESS);
}

} // end anonymous namespace

void WinSalGraphics::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (dynamic_cast<WinOpenGLSalGraphicsImpl*>(mpImpl.get()) == nullptr &&
        dynamic_cast<const WinSalBitmap*>(&rSalBitmap) == nullptr)
    {
        std::unique_ptr<WinSalBitmap> pWinSalBitmap(new WinSalBitmap());
        SalBitmap& rConstBitmap = const_cast<SalBitmap&>(rSalBitmap);
        convertToWinSalBitmap(rConstBitmap, *pWinSalBitmap.get());
        mpImpl->drawBitmap(rPosAry, *pWinSalBitmap.get());
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
        convertToWinSalBitmap(rConstBitmap, *pWinSalBitmap.get());


        std::unique_ptr<WinSalBitmap> pWinTransparentSalBitmap(new WinSalBitmap());
        SalBitmap& rConstTransparentBitmap = const_cast<SalBitmap&>(rSTransparentBitmap);
        convertToWinSalBitmap(rConstTransparentBitmap, *pWinTransparentSalBitmap.get());

        mpImpl->drawBitmap(rPosAry, *pWinSalBitmap.get(), *pWinTransparentSalBitmap.get());
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
                            SalColor nMaskColor )
{
    mpImpl->drawMask( rPosAry, rSSalBitmap, nMaskColor );
}

SalBitmap* WinSalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    return mpImpl->getBitmap( nX, nY, nDX, nDY );
}

SalColor WinSalGraphics::getPixel( long nX, long nY )
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
