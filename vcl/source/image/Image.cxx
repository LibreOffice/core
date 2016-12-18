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
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/rc.h>
#include <tools/rc.hxx>
#include <tools/resmgr.hxx>
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

#include <vcl/BitmapProcessor.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

Image::Image()
{
}

Image::Image( const ResId& rResId )
{
    rResId.SetRT( RSC_IMAGE );

    ResMgr* pResMgr = rResId.GetResMgr();
    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        BitmapEx      aBmpEx;
        RscImageFlags nObjMask = (RscImageFlags)pResMgr->ReadLong();

        if( nObjMask & RscImageFlags::ImageBitmap )
        {
            aBmpEx = BitmapEx( ResId( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()), *pResMgr ) );
            pResMgr->Increment( ResMgr::GetObjSize( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()) ) );
        }

        if( ! aBmpEx.IsEmpty() )
            ImplInit( aBmpEx );
    }
}

Image::Image(const BitmapEx& rBitmapEx)
{
    ImplInit(rBitmapEx);
}

Image::Image(const Bitmap& rBitmap)
{
    ImplInit(rBitmap);
}

Image::Image(const Bitmap& rBitmap, const Bitmap& rMaskBitmap)
{
    const BitmapEx aBitmapEx(rBitmap, rMaskBitmap);
    ImplInit(aBitmapEx);
}

Image::Image(const Bitmap& rBitmap, const Color& rColor)
{
    const BitmapEx aBitmapEx(rBitmap, rColor);
    ImplInit(aBitmapEx);
}

Image::Image(const css::uno::Reference< css::graphic::XGraphic >& rxGraphic)
{
    const Graphic aGraphic(rxGraphic);
    ImplInit(aGraphic.GetBitmapEx());
}

Image::Image(const OUString & rFileUrl)
{
    OUString aPath;
    osl::FileBase::getSystemPathFromFileURL(rFileUrl, aPath);
    Graphic aGraphic;
    const OUString aFilterName(IMP_PNG);
    if (GRFILTER_OK == GraphicFilter::LoadGraphic(aPath, aFilterName, aGraphic))
    {
        ImplInit(aGraphic.GetBitmapEx());
    }
}

void Image::ImplInit(const BitmapEx& rBitmapEx)
{
    if (!rBitmapEx.IsEmpty())
    {
        mpImplData.reset(new ImplImage);
        mpImplData->mpBitmapEx.reset(new BitmapEx(rBitmapEx));
    }
}

Size Image::GetSizePixel() const
{
    Size aRet;

    if (mpImplData && mpImplData->mpBitmapEx)
    {
        aRet = mpImplData->mpBitmapEx->GetSizePixel();
    }

    return aRet;
}

BitmapEx Image::GetBitmapEx() const
{
    BitmapEx aRet;

    if (mpImplData && mpImplData->mpBitmapEx)
    {
        aRet = BitmapEx(*mpImplData->mpBitmapEx);
    }

    return aRet;
}

css::uno::Reference< css::graphic::XGraphic > Image::GetXGraphic() const
{
    const Graphic aGraphic( GetBitmapEx() );

    return aGraphic.GetXGraphic();
}

bool Image::operator==(const Image& rImage) const
{
    bool bRet = false;

    if (rImage.mpImplData == mpImplData)
        bRet = true;
    else if (!rImage.mpImplData || !mpImplData)
        bRet = false;
    else
        bRet = *rImage.mpImplData->mpBitmapEx == *mpImplData->mpBitmapEx;

    return bRet;
}

void Image::Draw(OutputDevice* pOutDev, const Point& rPos, DrawImageFlags nStyle, const Size* pSize)
{
    if (!mpImplData || !mpImplData->mpBitmapEx ||
        (!pOutDev->IsDeviceOutputNecessary() && pOutDev->GetConnectMetaFile() == nullptr))
        return;

    const Point aSrcPos(0, 0);
    Size aBitmapSizePixel = mpImplData->mpBitmapEx->GetSizePixel();

    Size aOutSize = pSize ? *pSize : pOutDev->PixelToLogic(aBitmapSizePixel);

    if (nStyle & DrawImageFlags::Disable)
    {
        BitmapChecksum aChecksum = mpImplData->mpBitmapEx->GetChecksum();
        if (mpImplData->maBitmapChecksum != aChecksum)
        {
            mpImplData->maBitmapChecksum = aChecksum;
            mpImplData->maDisabledBitmapEx = BitmapProcessor::createDisabledImage(*mpImplData->mpBitmapEx);
        }
        pOutDev->DrawBitmapEx(rPos, aOutSize, aSrcPos, aBitmapSizePixel, mpImplData->maDisabledBitmapEx);
    }
    else
    {
        if (nStyle & (DrawImageFlags::ColorTransform | DrawImageFlags::Highlight |
                      DrawImageFlags::Deactive | DrawImageFlags::SemiTransparent))
        {
            BitmapEx aTempBitmapEx(*mpImplData->mpBitmapEx);

            if (nStyle & (DrawImageFlags::Highlight | DrawImageFlags::Deactive))
            {
                const StyleSettings& rSettings = pOutDev->GetSettings().GetStyleSettings();
                Color aColor;
                if (nStyle & DrawImageFlags::Highlight)
                    aColor = rSettings.GetHighlightColor();
                else
                    aColor = rSettings.GetDeactiveColor();

                BitmapProcessor::colorizeImage(aTempBitmapEx, aColor);
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
            pOutDev->DrawBitmapEx(rPos, aOutSize, aSrcPos, aTempBitmapEx.GetSizePixel(), aTempBitmapEx);
        }
        else
        {
            pOutDev->DrawBitmapEx(rPos, aOutSize, aSrcPos, mpImplData->mpBitmapEx->GetSizePixel(), *mpImplData->mpBitmapEx);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
