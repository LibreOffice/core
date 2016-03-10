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

#include <vcl/event.hxx>
#include <vcl/imgctrl.hxx>
#include <tools/rcid.h>

#include <com/sun/star/awt/ImageScaleMode.hpp>

namespace ImageScaleMode = css::awt::ImageScaleMode;

ImageControl::ImageControl( vcl::Window* pParent, WinBits nStyle )
    :FixedImage( pParent, nStyle )
    ,mnScaleMode( ImageScaleMode::ANISOTROPIC )
{
}

void ImageControl::SetScaleMode( const ::sal_Int16 _nMode )
{
    if ( _nMode != mnScaleMode )
    {
        mnScaleMode = _nMode;
        Invalidate();
    }
}

void ImageControl::Resize()
{
    Invalidate();
}

namespace
{
    Size lcl_calcPaintSize( const Rectangle& _rPaintRect, const Size& _rBitmapSize )
    {
        const Size aPaintSize = _rPaintRect.GetSize();

        const double nRatioX = 1.0 * aPaintSize.Width() / _rBitmapSize.Width();
        const double nRatioY = 1.0 * aPaintSize.Height() / _rBitmapSize.Height();
        const double nRatioMin = ::std::min( nRatioX, nRatioY );

        return Size( long( _rBitmapSize.Width() * nRatioMin ), long( _rBitmapSize.Height() * nRatioMin ) );
    }

    Point lcl_centerWithin( const Rectangle& _rArea, const Size& _rObjectSize )
    {
        Point aPos( _rArea.TopLeft() );
        aPos.X() += ( _rArea.GetWidth() - _rObjectSize.Width() ) / 2;
        aPos.Y() += ( _rArea.GetHeight() - _rObjectSize.Height() ) / 2;
        return aPos;
    }
}

void ImageControl::ImplDraw(OutputDevice& rDev, DrawFlags nDrawFlags, const Point& rPos, const Size& rSize) const
{
    DrawImageFlags nStyle = DrawImageFlags::NONE;
    if ( !(nDrawFlags & DrawFlags::NoDisable) )
    {
        if ( !IsEnabled() )
            nStyle |= DrawImageFlags::Disable;
    }

    const Image& rImage( GetModeImage() );
    const Image* pImage = &rImage;
    const Rectangle aDrawRect( rPos, rSize );
    if ( !*pImage )
    {
        OUString  sText( GetText() );
        if ( sText.isEmpty() )
            return;

        WinBits nWinStyle = GetStyle();
        DrawTextFlags nTextStyle = FixedText::ImplGetTextStyle( nWinStyle );
        if ( !(nDrawFlags & DrawFlags::NoDisable) )
            if ( !IsEnabled() )
                nTextStyle |= DrawTextFlags::Disable;

        rDev.DrawText( aDrawRect, sText, nTextStyle );
        return;
    }

    const Size&      rBitmapSize = pImage->GetSizePixel();

    switch ( mnScaleMode )
    {
    case ImageScaleMode::NONE:
    {
        rDev.DrawImage( lcl_centerWithin( aDrawRect, rBitmapSize ), *pImage, nStyle );
    }
    break;

    case ImageScaleMode::ISOTROPIC:
    {
        const Size aPaintSize = lcl_calcPaintSize( aDrawRect, rBitmapSize );
        rDev.DrawImage(
            lcl_centerWithin( aDrawRect, aPaintSize ),
            aPaintSize,
            *pImage, nStyle );
    }
    break;

    case ImageScaleMode::ANISOTROPIC:
    {
        rDev.DrawImage(
            aDrawRect.TopLeft(),
            aDrawRect.GetSize(),
            *pImage, nStyle );
    }
    break;

    default:
        OSL_ENSURE( false, "ImageControl::ImplDraw: unhandled scale mode!" );
        break;

    }   // switch ( mnScaleMode )
}

void ImageControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    ImplDraw(rRenderContext, DrawFlags::NONE, Point(), GetOutputSizePixel());

    if (HasFocus())
    {
        vcl::Window* pBorderWindow = GetWindow(GetWindowType::Border);

        bool bFlat = (GetBorderStyle() == WindowBorderStyle::MONO);
        Rectangle aRect(Point(0,0), pBorderWindow->GetOutputSizePixel());
        Color oldLineCol = pBorderWindow->GetLineColor();
        Color oldFillCol = pBorderWindow->GetFillColor();
        pBorderWindow->SetFillColor();
        pBorderWindow->SetLineColor(bFlat ? COL_WHITE : COL_BLACK);
        pBorderWindow->DrawRect(aRect);
        ++aRect.Left();
        --aRect.Right();
        ++aRect.Top();
        --aRect.Bottom();
        pBorderWindow->SetLineColor(bFlat ? COL_BLACK : COL_WHITE);
        pBorderWindow->DrawRect(aRect);
        pBorderWindow->SetLineColor(oldLineCol);
        pBorderWindow->SetFillColor(oldFillCol);
    }
}

void ImageControl::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags )
{
    const Point     aPos  = pDev->LogicToPixel( rPos );
    const Size      aSize = pDev->LogicToPixel( rSize );
          Rectangle aRect( aPos, aSize );

    pDev->Push();
    pDev->SetMapMode();

    // Border
    if ( !(nFlags & DrawFlags::NoBorder) && (GetStyle() & WB_BORDER) )
    {
        ImplDrawFrame( pDev, aRect );
    }
    pDev->IntersectClipRegion( aRect );
    ImplDraw( *pDev, nFlags, aRect.TopLeft(), aRect.GetSize() );

    pDev->Pop();
}

void ImageControl::GetFocus()
{
    FixedImage::GetFocus();
    GetWindow( GetWindowType::Border )->Invalidate();
}

void ImageControl::LoseFocus()
{
    FixedImage::GetFocus();
    GetWindow( GetWindowType::Border )->Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
