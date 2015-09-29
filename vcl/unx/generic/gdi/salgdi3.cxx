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

#include <sal/types.h>

#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <fontconfig/fontconfig.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/tencinfo.h>
#include <tools/stream.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/jobdata.hxx>
#include <vcl/printerinfomanager.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metric.hxx>

#include "fontmanager.hxx"
#include "impfont.hxx"
#include "gcach_xpeer.hxx"
#include "generic/genpspgraphics.h"
#include "generic/printergfx.hxx"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "salframe.hxx"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/salunx.h"
#include "unx/salvd.h"
#include "textrender.hxx"
#include "xrender_peer.hxx"

// X11SalGraphics

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

void X11SalGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    mxTextRenderImpl->DrawServerFontLayout(rLayout);
}

const FontCharMapPtr X11SalGraphics::GetFontCharMap() const
{
    return mxTextRenderImpl->GetFontCharMap();
}

bool X11SalGraphics::GetFontCapabilities(vcl::FontCapabilities &rGetImplFontCapabilities) const
{
    return mxTextRenderImpl->GetFontCapabilities(rGetImplFontCapabilities);
}

// SalGraphics

sal_uInt16 X11SalGraphics::SetFont( FontSelectPattern *pEntry, int nFallbackLevel )
{
    return mxTextRenderImpl->SetFont(pEntry, nFallbackLevel);
}

void
X11SalGraphics::SetTextColor( SalColor nSalColor )
{
    mxTextRenderImpl->SetTextColor(nSalColor);
    nTextPixel_     = GetPixel( nSalColor );
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
X11SalGraphics::GetFontMetric( ImplFontMetricData *pMetric, int nFallbackLevel )
{
    mxTextRenderImpl->GetFontMetric(pMetric, nFallbackLevel);
}

bool X11SalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    return mxTextRenderImpl->GetGlyphBoundRect(aGlyphId, rRect);
}

bool X11SalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId,
    ::basegfx::B2DPolyPolygon& rPolyPoly )
{
    return mxTextRenderImpl->GetGlyphOutline(aGlyphId, rPolyPoly);
}

SalLayout* X11SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    return mxTextRenderImpl->GetTextLayout(rArgs, nFallbackLevel);
}

SystemFontData X11SalGraphics::GetSysFontData( int nFallbackLevel ) const
{
    return mxTextRenderImpl->GetSysFontData(nFallbackLevel);
}

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

const void* X11SalGraphics::GetEmbedFontData( const PhysicalFontFace* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, size_t nLen, FontSubsetInfo& rInfo, long* pDataLen )
{
    return mxTextRenderImpl->GetEmbedFontData(pFont, pUnicodes, pWidths, nLen, rInfo, pDataLen);
}

void X11SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    mxTextRenderImpl->FreeEmbedFontData(pData, nLen);
}

const Ucs2SIntMap* X11SalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded, std::set<sal_Unicode> const** ppPriority)
{
    return mxTextRenderImpl->GetFontEncodingVector(pFont, pNonEncoded, ppPriority);
}

void X11SalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    mxTextRenderImpl->GetGlyphWidths(pFont, bVertical, rWidths, rUnicodeEnc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
