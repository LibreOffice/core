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

}

void WinSalGraphics::convertToWinSalBitmap(SalBitmap& rSalBitmap, WinSalBitmap& rWinSalBitmap)
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
    if(pRead->mnFormat & ScanlineFormat::TopDown)
    {
        pSource += pRead->mnScanlineSize * (pRead->mnHeight - 1);
        readRowChange = -readRowChange;
    }

    std::unique_ptr<ColorScanlineConverter> pConverter;

    if (RemoveScanline(pRead->mnFormat) == ScanlineFormat::N24BitTcRgb)
    {
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N24BitTcRgb,
                                                    3, pRead->mnScanlineSize));
    }
    else if (RemoveScanline(pRead->mnFormat) == ScanlineFormat::N32BitTcRgba)
    {
        pConverter.reset(new ColorScanlineConverter(ScanlineFormat::N32BitTcRgba,
                                                    4, pRead->mnScanlineSize));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
