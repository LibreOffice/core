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

#include <stdlib.h>
#include <math.h>
#include <unx/freetype_glyphcache.hxx>
#include <unx/gendata.hxx>

#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <fontinstance.hxx>
#include <fontattributes.hxx>

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>

GlyphCache::GlyphCache()
:   mnBytesUsed(sizeof(GlyphCache)),
    mpCurrentGCFont(nullptr)
    , m_nMaxFontId(0)
{
    InitFreetype();
}

GlyphCache::~GlyphCache()
{
    ClearFontCache();
}

void GlyphCache::ClearFontCache()
{
    for (auto &aFontPair : maFontList)
        static_cast<FreetypeFontInstance*>(aFontPair.first.get())->SetFreetypeFont(nullptr);
    maFontList.clear();
    mpCurrentGCFont = nullptr;
    m_aFontInfoList.clear();
}

void GlyphCache::ClearFontOptions()
{
    for (auto const& font : maFontList)
    {
        FreetypeFont* pFreetypeFont = font.second.get();
        // free demand-loaded FontConfig related data
        pFreetypeFont->ClearFontOptions();
    }
}

static sal_IntPtr GetFontId(const LogicalFontInstance& rFontInstance)
{
    if (rFontInstance.GetFontFace())
        return rFontInstance.GetFontFace()->GetFontId();
    return 0;
}

inline
size_t GlyphCache::IFSD_Hash::operator()(const rtl::Reference<LogicalFontInstance>& rFontInstance) const
{
    // TODO: is it worth to improve this hash function?
    sal_uIntPtr nFontId = GetFontId(*rFontInstance);

    const FontSelectPattern& rFontSelData = rFontInstance->GetFontSelectPattern();

    if (rFontSelData.maTargetName.indexOf(FontSelectPattern::FEAT_PREFIX)
        != -1)
    {
        OString aFeatName = OUStringToOString( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
        nFontId ^= aFeatName.hashCode();
    }

    size_t nHash = nFontId << 8;
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    nHash   += size_t(rFontSelData.mbVertical);
    nHash   += rFontSelData.GetItalic();
    nHash   += rFontSelData.GetWeight();
    nHash   += static_cast<sal_uInt16>(rFontSelData.meLanguage);

    return nHash;
}

bool GlyphCache::IFSD_Equal::operator()(const rtl::Reference<LogicalFontInstance>& rAFontInstance,
                                        const rtl::Reference<LogicalFontInstance>& rBFontInstance) const
{
    if (!rAFontInstance->GetFontCache() || !rBFontInstance->GetFontCache())
        return false;

    // check font ids
    if (GetFontId(*rAFontInstance) != GetFontId(*rBFontInstance))
        return false;

    const FontSelectPattern& rA = rAFontInstance->GetFontSelectPattern();
    const FontSelectPattern& rB = rBFontInstance->GetFontSelectPattern();

    // compare with the requested metrics
    if( (rA.mnHeight         != rB.mnHeight)
    ||  (rA.mnOrientation    != rB.mnOrientation)
    ||  (rA.mbVertical       != rB.mbVertical)
    ||  (rA.mbNonAntialiased != rB.mbNonAntialiased) )
        return false;

    if( (rA.GetItalic() != rB.GetItalic())
    ||  (rA.GetWeight() != rB.GetWeight()) )
        return false;

    // NOTE: ignoring meFamily deliberately

    // compare with the requested width, allow default width
    int nAWidth = rA.mnWidth != 0 ? rA.mnWidth : rA.mnHeight;
    int nBWidth = rB.mnWidth != 0 ? rB.mnWidth : rB.mnHeight;
    if( nAWidth != nBWidth )
        return false;

    if (rA.meLanguage != rB.meLanguage)
        return false;
   // check for features
    if ((rA.maTargetName.indexOf(FontSelectPattern::FEAT_PREFIX)
        != -1 ||
        rB.maTargetName.indexOf(FontSelectPattern::FEAT_PREFIX)
        != -1) && rA.maTargetName != rB.maTargetName)
        return false;

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

GlyphCache& GlyphCache::GetInstance()
{
    GenericUnixSalData* const pSalData(GetGenericUnixSalData());
    assert(pSalData);
    return *pSalData->GetGlyphCache();
}

FreetypeFont* GlyphCache::CacheFont(LogicalFontInstance* pFontInstance)
{
    // a serverfont request has a fontid > 0
    if (GetFontId(*pFontInstance) <= 0)
        return nullptr;

    FontList::iterator it = maFontList.find(pFontInstance);
    if( it != maFontList.end() )
    {
        FreetypeFont* pFound = it->second.get();
        assert(pFound);
        pFound->AddRef();
        return pFound;
    }

    // font not cached yet => create new font item
    FreetypeFont* pNew = CreateFont(pFontInstance);

    if( pNew )
    {
        maFontList[pFontInstance].reset(pNew);
        mnBytesUsed += pNew->GetByteCount();

        // enable garbage collection for new font
        if( !mpCurrentGCFont )
        {
            mpCurrentGCFont = pNew;
            pNew->mpNextGCFont = pNew;
            pNew->mpPrevGCFont = pNew;
        }
        else
        {
            pNew->mpNextGCFont = mpCurrentGCFont;
            pNew->mpPrevGCFont = mpCurrentGCFont->mpPrevGCFont;
            pNew->mpPrevGCFont->mpNextGCFont = pNew;
            mpCurrentGCFont->mpPrevGCFont = pNew;
        }
    }

    return pNew;
}

void GlyphCache::UncacheFont( FreetypeFont& rFreetypeFont )
{
    if( (rFreetypeFont.Release() <= 0) && (gnMaxSize <= mnBytesUsed) )
    {
        mpCurrentGCFont = &rFreetypeFont;
        GarbageCollect();
    }
}

void GlyphCache::GarbageCollect()
{
    // when current GC font has been destroyed get another one
    if( !mpCurrentGCFont )
    {
        FontList::iterator it = maFontList.begin();
        if( it != maFontList.end() )
            mpCurrentGCFont = it->second.get();
    }

    // unless there is no other font to collect
    if( !mpCurrentGCFont )
        return;

    // prepare advance to next font for garbage collection
    FreetypeFont* const pFreetypeFont = mpCurrentGCFont;
    mpCurrentGCFont = pFreetypeFont->mpNextGCFont;

    if( (pFreetypeFont != mpCurrentGCFont)    // no other fonts
    &&  (pFreetypeFont->GetRefCount() <= 0) )  // font still used
    {
        SAL_WARN_IF( (pFreetypeFont->GetRefCount() != 0), "vcl",
            "GlyphCache::GC detected RefCount underflow" );

        // free all pFreetypeFont related data
        if( pFreetypeFont == mpCurrentGCFont )
            mpCurrentGCFont = nullptr;
        mnBytesUsed -= pFreetypeFont->GetByteCount();

        // remove font from list of garbage collected fonts
        if( pFreetypeFont->mpPrevGCFont )
            pFreetypeFont->mpPrevGCFont->mpNextGCFont = pFreetypeFont->mpNextGCFont;
        if( pFreetypeFont->mpNextGCFont )
            pFreetypeFont->mpNextGCFont->mpPrevGCFont = pFreetypeFont->mpPrevGCFont;
        if( pFreetypeFont == mpCurrentGCFont )
            mpCurrentGCFont = nullptr;

        maFontList.erase(pFreetypeFont->GetFontInstance());
    }
}

void FreetypeFont::ReleaseFromGarbageCollect()
{
    // remove from GC list
    FreetypeFont* pPrev = mpPrevGCFont;
    FreetypeFont* pNext = mpNextGCFont;
    if( pPrev ) pPrev->mpNextGCFont = pNext;
    if( pNext ) pNext->mpPrevGCFont = pPrev;
    mpPrevGCFont = nullptr;
    mpNextGCFont = nullptr;
}

long FreetypeFont::Release() const
{
    SAL_WARN_IF( mnRefCount <= 0, "vcl", "FreetypeFont: RefCount underflow" );
    return --mnRefCount;
}

FreetypeFontInstance::FreetypeFontInstance(const PhysicalFontFace& rPFF, const FontSelectPattern& rFSP)
    : LogicalFontInstance(rPFF, rFSP)
    , mpFreetypeFont(nullptr)
{}

void FreetypeFontInstance::SetFreetypeFont(FreetypeFont* p)
{
    if (p == mpFreetypeFont)
        return;
    mpFreetypeFont = p;
}

FreetypeFontInstance::~FreetypeFontInstance()
{
}

static hb_blob_t* getFontTable(hb_face_t* /*face*/, hb_tag_t nTableTag, void* pUserData)
{
    char pTagName[5];
    LogicalFontInstance::DecodeOpenTypeTag( nTableTag, pTagName );

    sal_uLong nLength = 0;
    FreetypeFontInstance* pFontInstance = static_cast<FreetypeFontInstance*>( pUserData );
    FreetypeFont* pFont = pFontInstance->GetFreetypeFont();
    const char* pBuffer = reinterpret_cast<const char*>(
        pFont->GetTable(pTagName, &nLength) );

    hb_blob_t* pBlob = nullptr;
    if (pBuffer != nullptr)
        pBlob = hb_blob_create(pBuffer, nLength, HB_MEMORY_MODE_READONLY, nullptr, nullptr);

    return pBlob;
}

hb_font_t* FreetypeFontInstance::ImplInitHbFont()
{
    return InitHbFont(hb_face_create_for_tables(getFontTable, this, nullptr));
}

bool FreetypeFontInstance::ImplGetGlyphBoundRect(sal_GlyphId nId, tools::Rectangle& rRect, bool bVertical) const
{
    assert(mpFreetypeFont);
    if (!mpFreetypeFont)
        return false;
    return mpFreetypeFont->GetGlyphBoundRect(nId, rRect, bVertical);
}

bool FreetypeFontInstance::GetGlyphOutline(sal_GlyphId nId, basegfx::B2DPolyPolygon& rPoly, bool bVertical) const
{
    assert(mpFreetypeFont);
    if (!mpFreetypeFont)
        return false;
    return mpFreetypeFont->GetGlyphOutline(nId, rPoly, bVertical);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
