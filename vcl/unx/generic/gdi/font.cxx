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

#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>

#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <textrender.hxx>
#include <sallayout.hxx>

GC
X11SalGraphics::GetFontGC()
{
    Display *pDisplay = GetXDisplay();

    if( !pFontGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;
        values.foreground           = nTextPixel_;
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCForeground,
                              &values );
    }
    if( !bFontGC_ )
    {
        XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
        SetClipRegion( pFontGC_ );
        bFontGC_ = true;
    }

    return pFontGC_;
}

void X11SalGraphics::DrawTextLayout(const GenericSalLayout& rLayout)
{
    mxTextRenderImpl->DrawTextLayout(rLayout, *this);
}

const FontCharMapRef X11SalGraphics::GetFontCharMap() const
{
    return mxTextRenderImpl->GetFontCharMap();
}

bool X11SalGraphics::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    return mxTextRenderImpl->GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics
void X11SalGraphics::SetFont(LogicalFontInstance* pEntry, int nFallbackLevel)
{
    mxTextRenderImpl->SetFont(pEntry, nFallbackLevel);
}

void
X11SalGraphics::SetTextColor( Color nColor )
{
    mxTextRenderImpl->SetTextColor(nColor);
    nTextPixel_     = GetPixel( nColor );
    bFontGC_        = false;
}

bool X11SalGraphics::AddTempDevFont( PhysicalFontCollection* pFontCollection,
                                     const OUString& rFileURL,
                                     const OUString& rFontName )
{
    return mxTextRenderImpl->AddTempDevFont(pFontCollection, rFileURL, rFontName);
}

void X11SalGraphics::ClearDevFontCache()
{
    mxTextRenderImpl->ClearDevFontCache();
}

void X11SalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    mxTextRenderImpl->GetDevFontList(pFontCollection);
}

void
X11SalGraphics::GetFontMetric( ImplFontMetricDataRef &rxFontMetric, int nFallbackLevel )
{
    mxTextRenderImpl->GetFontMetric(rxFontMetric, nFallbackLevel);
}

std::unique_ptr<GenericSalLayout> X11SalGraphics::GetTextLayout(int nFallbackLevel)
{
    return mxTextRenderImpl->GetTextLayout(nFallbackLevel);
}

#if ENABLE_CAIRO_CANVAS

SystemFontData X11SalGraphics::GetSysFontData( int nFallbackLevel ) const
{
    return mxTextRenderImpl->GetSysFontData(nFallbackLevel);
}

#endif

bool X11SalGraphics::CreateFontSubset(
                                   const OUString& rToFile,
                                   const PhysicalFontFace* pFont,
                                   const sal_GlyphId* pGlyphIds,
                                   const sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphCount,
                                   FontSubsetInfo& rInfo
                                   )
{
    return mxTextRenderImpl->CreateFontSubset(rToFile, pFont,
            pGlyphIds, pEncoding, pWidths, nGlyphCount, rInfo);
}

const void* X11SalGraphics::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    return mxTextRenderImpl->GetEmbedFontData(pFont, pDataLen);
}

void X11SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    mxTextRenderImpl->FreeEmbedFontData(pData, nLen);
}

void X11SalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   std::vector< sal_Int32 >& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    mxTextRenderImpl->GetGlyphWidths(pFont, bVertical, rWidths, rUnicodeEnc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
