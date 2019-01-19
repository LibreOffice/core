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

#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/outdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/ImageTree.hxx>
#include <sal/types.h>
#include <image.h>

#include <BitmapDisabledImageFilter.hxx>
#include <BitmapColorizeFilter.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

using namespace css;

Image::Image()
{
}

Image::Image(const BitmapEx& rBitmapEx)
{
    ImplInit(rBitmapEx);
}

Image::Image(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    const Graphic aGraphic(rxGraphic);
    ImplInit(aGraphic.GetBitmapEx());
}

Image::Image(const OUString & rFileUrl)
{
    OUString sImageName;
    if (rFileUrl.startsWith("private:graphicrepository/", &sImageName))
    {
        mpImplData = std::make_shared<ImplImage>(sImageName);
    }
    else
    {
        Graphic aGraphic;
        if (ERRCODE_NONE == GraphicFilter::LoadGraphic(rFileUrl, IMP_PNG, aGraphic))
        {
            ImplInit(aGraphic.GetBitmapEx());
        }
    }
}

Image::Image(StockImage, const OUString & rFileUrl)
    : mpImplData(std::make_shared<ImplImage>(rFileUrl))
{
}

void Image::ImplInit(const BitmapEx& rBitmapEx)
{
    if (!rBitmapEx.IsEmpty())
        mpImplData = std::make_shared<ImplImage>(rBitmapEx);
}

Size Image::GetSizePixel() const
{
    if (mpImplData)
        return mpImplData->getSizePixel();
    else
        return Size();
}

BitmapEx Image::GetBitmapEx() const
{
    if (mpImplData)
        return mpImplData->getBitmapEx();
    else
        return BitmapEx();
}

bool Image::operator==(const Image& rImage) const
{
    bool bRet = false;

    if (rImage.mpImplData == mpImplData)
        bRet = true;
    else if (!rImage.mpImplData || !mpImplData)
        bRet = false;
    else
        bRet = rImage.mpImplData->isEqual(*mpImplData);

    return bRet;
}

void Image::Draw(OutputDevice* pOutDev, const Point& rPos, DrawImageFlags nStyle, const Size* pSize)
{
    if (!mpImplData || (!pOutDev->IsDeviceOutputNecessary() && pOutDev->GetConnectMetaFile() == nullptr))
        return;

    Size aOutSize = pSize ? *pSize : pOutDev->PixelToLogic(mpImplData->getSizePixel());

    BitmapEx aRenderBmp = mpImplData->getBitmapExForHiDPI(!!(nStyle & DrawImageFlags::Disable));

    if (!(nStyle & DrawImageFlags::Disable) &&
        (nStyle & (DrawImageFlags::ColorTransform | DrawImageFlags::Highlight |
                   DrawImageFlags::Deactive | DrawImageFlags::SemiTransparent)))
    {
        BitmapEx aTempBitmapEx(aRenderBmp);

        if (nStyle & (DrawImageFlags::Highlight | DrawImageFlags::Deactive))
        {
            const StyleSettings& rSettings = pOutDev->GetSettings().GetStyleSettings();
            Color aColor;
            if (nStyle & DrawImageFlags::Highlight)
                aColor = rSettings.GetHighlightColor();
            else
                aColor = rSettings.GetDeactiveColor();

            BitmapFilter::Filter(aTempBitmapEx, BitmapColorizeFilter(aColor));
        }

        if (nStyle & DrawImageFlags::SemiTransparent)
        {
            if (aTempBitmapEx.IsTransparent())
            {
                Bitmap aAlphaBmp(aTempBitmapEx.GetAlpha().GetBitmap());
                aAlphaBmp.Adjust(50);
                aTempBitmapEx = BitmapEx(aTempBitmapEx.GetBitmap(), AlphaMask(aAlphaBmp));
            }
            else
            {
                sal_uInt8 cErase = 128;
                aTempBitmapEx = BitmapEx(aTempBitmapEx.GetBitmap(), AlphaMask(aTempBitmapEx.GetSizePixel(), &cErase));
            }
        }
        aRenderBmp = aTempBitmapEx;
    }

    pOutDev->DrawBitmapEx(rPos, aOutSize, aRenderBmp);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
