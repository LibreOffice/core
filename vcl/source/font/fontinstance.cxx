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


#include <svdata.hxx>
#include <fontinstance.hxx>
#include <impfontcache.hxx>

#include <PhysicalFontFace.hxx>

// extend std namespace to add custom hash needed for LogicalFontInstance

namespace std
{
    template <> struct hash< pair< sal_UCS4, FontWeight > >
    {
        size_t operator()(const pair< sal_UCS4, FontWeight >& rData) const
        {
            size_t h1 = hash<sal_UCS4>()(rData.first);
            size_t h2 = hash<int>()(rData.second);
            return h1 ^ h2;
        }
    };
}


LogicalFontInstance::LogicalFontInstance(const PhysicalFontFace& rFontFace, const FontSelectPattern& rFontSelData )
    : mxFontMetric( new ImplFontMetricData( rFontSelData ))
    , mpConversion( nullptr )
    , mnLineHeight( 0 )
    , mnOwnOrientation( 0 )
    , mnOrientation( 0 )
    , mbInit( false )
    , mpFontCache( nullptr )
    , mnRefCount( 1 )
    , m_aFontSelData(rFontSelData)
    , m_pHbFont(nullptr)
    , m_nAveWidthFactor(1.0f)
    , m_pFontFace(&const_cast<PhysicalFontFace&>(rFontFace))
{
    const_cast<FontSelectPattern*>(&m_aFontSelData)->mpFontInstance = this;
}

LogicalFontInstance::~LogicalFontInstance()
{
    mpUnicodeFallbackList.reset();
    mpFontCache = nullptr;
    mxFontMetric = nullptr;

    if (m_pHbFont)
        hb_font_destroy(m_pHbFont);
}

hb_font_t* LogicalFontInstance::InitHbFont(hb_face_t* pHbFace) const
{
    assert(pHbFace);
    hb_font_t* pHbFont = hb_font_create(pHbFace);
    unsigned int nUPEM = hb_face_get_upem(pHbFace);
    hb_font_set_scale(pHbFont, nUPEM, nUPEM);
    hb_ot_font_set_funcs(pHbFont);
    // hb_font_t keeps a reference to hb_face_t, so destroy this one.
    hb_face_destroy(pHbFace);
    return pHbFont;
}

int LogicalFontInstance::GetKashidaWidth()
{
    hb_font_t* pHbFont = GetHbFont();
    hb_position_t nWidth = 0;
    hb_codepoint_t nIndex = 0;

    if (hb_font_get_glyph(pHbFont, 0x0640, 0, &nIndex))
    {
        double nXScale = 0;
        GetScale(&nXScale, nullptr);
        nWidth = hb_font_get_glyph_h_advance(pHbFont, nIndex) * nXScale;
    }

    return nWidth;
}

void LogicalFontInstance::GetScale(double* nXScale, double* nYScale)
{
    hb_face_t* pHbFace = hb_font_get_face(GetHbFont());
    unsigned int nUPEM = hb_face_get_upem(pHbFace);

    double nHeight(m_aFontSelData.mnHeight);

    // On Windows, mnWidth is relative to average char width not font height,
    // and we need to keep it that way for GDI to correctly scale the glyphs.
    // Here we compensate for this so that HarfBuzz gives us the correct glyph
    // positions.
    double nWidth(m_aFontSelData.mnWidth ? m_aFontSelData.mnWidth * m_nAveWidthFactor : nHeight);

    if (nYScale)
        *nYScale = nHeight / nUPEM;

    if (nXScale)
        *nXScale = nWidth / nUPEM;
}

void LogicalFontInstance::Acquire()
{
    assert(mnRefCount < std::numeric_limits<decltype(mnRefCount)>::max()
        && "LogicalFontInstance::Release() - refcount overflow");
    if (mpFontCache)
        mpFontCache->Acquire(this);
    else
        ++mnRefCount;
}

void LogicalFontInstance::Release()
{
    assert(mnRefCount > 0 && "LogicalFontInstance::Release() - refcount underflow");

    if (mpFontCache)
        mpFontCache->Release(this);
    else
        if (--mnRefCount == 0)
            delete this;
}

void LogicalFontInstance::AddFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
    if( !mpUnicodeFallbackList )
        mpUnicodeFallbackList.reset(new UnicodeFallbackList);
    (*mpUnicodeFallbackList)[ std::pair< sal_UCS4, FontWeight >(cChar,eWeight) ] = rFontName;
}

bool LogicalFontInstance::GetFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, OUString* pFontName ) const
{
    if( !mpUnicodeFallbackList )
        return false;

    UnicodeFallbackList::const_iterator it = mpUnicodeFallbackList->find( std::pair< sal_UCS4, FontWeight >(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return false;

    *pFontName = (*it).second;
    return true;
}

void LogicalFontInstance::IgnoreFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
    UnicodeFallbackList::iterator it = mpUnicodeFallbackList->find( std::pair< sal_UCS4,FontWeight >(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return;
    if( (*it).second == rFontName )
        mpUnicodeFallbackList->erase( it );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
