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
#include <vcl/implimagetree.hxx>
#include <image.h>

#include <vcl/BitmapProcessor.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <rtl/strbuf.hxx>
#endif

using namespace ::com::sun::star;

Image::Image() :
    mpImplData( nullptr )
{
}

Image::Image( const ResId& rResId ) :
    mpImplData( nullptr )
{

    rResId.SetRT( RSC_IMAGE );

    ResMgr* pResMgr = rResId.GetResMgr();
    if( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        BitmapEx    aBmpEx;
        sal_uLong       nObjMask = pResMgr->ReadLong();

        if( nObjMask & RSC_IMAGE_IMAGEBITMAP )
        {
            aBmpEx = BitmapEx( ResId( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()), *pResMgr ) );
            pResMgr->Increment( ResMgr::GetObjSize( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()) ) );
        }

        if( nObjMask & RSC_IMAGE_MASKBITMAP )
        {
            if( !aBmpEx.IsEmpty() && aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
            {
                const Bitmap aMaskBitmap( ResId( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()), *pResMgr ) );
                aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskBitmap );
            }

            pResMgr->Increment( ResMgr::GetObjSize( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()) ) );
        }

        if( nObjMask & RSC_IMAGE_MASKCOLOR )
        {
            if( !aBmpEx.IsEmpty() && aBmpEx.GetTransparentType() == TRANSPARENT_NONE )
            {
                const Color aMaskColor( ResId( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()), *pResMgr ) );
                aBmpEx = BitmapEx( aBmpEx.GetBitmap(), aMaskColor );
            }

            pResMgr->Increment( ResMgr::GetObjSize( static_cast<RSHEADER_TYPE*>(pResMgr->GetClass()) ) );
        }
        if( ! aBmpEx.IsEmpty() )
            ImplInit( aBmpEx );
    }
}

Image::Image( const Image& rImage ) :
    mpImplData( rImage.mpImplData )
{

    if( mpImplData )
        ++mpImplData->mnRefCount;
}

Image::Image( const BitmapEx& rBitmapEx ) :
    mpImplData( nullptr )
{

    ImplInit( rBitmapEx );
}

Image::Image( const Bitmap& rBitmap ) :
    mpImplData( nullptr )
{

    ImplInit( rBitmap );
}

Image::Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap ) :
    mpImplData( nullptr )
{

    const BitmapEx aBmpEx( rBitmap, rMaskBitmap );

    ImplInit( aBmpEx );
}

Image::Image( const Bitmap& rBitmap, const Color& rColor ) :
    mpImplData( nullptr )
{

    const BitmapEx aBmpEx( rBitmap, rColor );

    ImplInit( aBmpEx );
}

Image::Image( const uno::Reference< graphic::XGraphic >& rxGraphic ) :
    mpImplData( nullptr )
{

    const Graphic aGraphic( rxGraphic );
    ImplInit( aGraphic.GetBitmapEx() );
}

Image::Image( const OUString &rFileUrl ) :
    mpImplData( nullptr )
{
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );
    Graphic aGraphic;
    const OUString aFilterName( IMP_PNG );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic ) )
    {
        ImplInit( aGraphic.GetBitmapEx() );
    }
}

Image::~Image()
{
    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;
}

void Image::ImplInit(const BitmapEx& rBitmapEx)
{
    if (!rBitmapEx.IsEmpty())
    {
        mpImplData = new ImplImage;
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

uno::Reference< graphic::XGraphic > Image::GetXGraphic() const
{
    const Graphic aGraphic( GetBitmapEx() );

    return aGraphic.GetXGraphic();
}

Image& Image::operator=( const Image& rImage )
{

    if( rImage.mpImplData )
        ++rImage.mpImplData->mnRefCount;

    if( mpImplData && ( 0 == --mpImplData->mnRefCount ) )
        delete mpImplData;

    mpImplData = rImage.mpImplData;

    return *this;
}

bool Image::operator==(const Image& rImage) const
{
    bool bRet = false;

    if (rImage.mpImplData == mpImplData)
        bRet = true;
    else if (!rImage.mpImplData || !mpImplData)
        bRet = false;
    else if (rImage.mpImplData->mpBitmapEx == mpImplData->mpBitmapEx)
        bRet = (rImage.mpImplData->mpBitmapEx == mpImplData->mpBitmapEx);

    return bRet;
}

void Image::Draw(OutputDevice* pOutDev, const Point& rPos, DrawImageFlags nStyle, const Size* pSize)
{
    if (mpImplData == nullptr || !mpImplData->mpBitmapEx || !pOutDev->IsDeviceOutputNecessary())
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
