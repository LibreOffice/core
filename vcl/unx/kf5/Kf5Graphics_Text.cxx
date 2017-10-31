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

#include "Kf5Graphics.hxx"
#include "Kf5FontFace.hxx"

#include <vcl/fontcharmap.hxx>

#include <PhysicalFontCollection.hxx>

#include <QtGui/QFontDatabase>
#include <QtCore/QStringList>

void Kf5Graphics::SetTextColor( SalColor nSalColor )
{
}

void Kf5Graphics::SetFont( const FontSelectPattern*, int nFallbackLevel )
{
}

void Kf5Graphics::GetFontMetric( ImplFontMetricDataRef &rFMD, int nFallbackLevel )
{
}

const FontCharMapRef Kf5Graphics::GetFontCharMap() const
{
    return nullptr;
}

bool Kf5Graphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    return false;
}

void Kf5Graphics::GetDevFontList( PhysicalFontCollection* pPFC )
{
    m_pFontCollection = pPFC;
    if ( pPFC->Count() )
        return;

    QFontDatabase aFDB;
    for ( auto& family : aFDB.families() )
        for ( auto& style : aFDB.styles( family ) )
        {
            // Just get any size - we don't care
            QList<int> sizes = aFDB.smoothSizes(family, style);
            pPFC->Add( Kf5FontFace::fromQFont( aFDB.font( family, style, *sizes.begin() ) ) );
        }
}

void Kf5Graphics::ClearDevFontCache()
{
}

bool Kf5Graphics::AddTempDevFont( PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName )
{
    return false;
}

bool Kf5Graphics::CreateFontSubset( const OUString& rToFile, const PhysicalFontFace* pFont,
                                    const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                    sal_Int32* pWidths, int nGlyphs, FontSubsetInfo& rInfo )
{
    return false;
}

const void* Kf5Graphics::GetEmbedFontData( const PhysicalFontFace*, long* pDataLen )
{
    return nullptr;
}

void Kf5Graphics::FreeEmbedFontData( const void* pData, long nDataLen )
{
}

void Kf5Graphics::GetGlyphWidths( const PhysicalFontFace*, bool bVertical,
                                  std::vector< sal_Int32 >& rWidths,
                                  Ucs2UIntMap& rUnicodeEnc )
{
}

bool Kf5Graphics::GetGlyphBoundRect( const GlyphItem&, tools::Rectangle& )
{
    return false;
}

bool Kf5Graphics::GetGlyphOutline( const GlyphItem&, basegfx::B2DPolyPolygon& )
{
    return false;
}

SalLayout* Kf5Graphics::GetTextLayout( ImplLayoutArgs&, int nFallbackLevel )
{
    return nullptr;
}

void Kf5Graphics::DrawTextLayout( const CommonSalLayout& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
