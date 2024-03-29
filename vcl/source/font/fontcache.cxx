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

#include <sal/config.h>

#include <sal/log.hxx>

#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/PhysicalFontFamily.hxx>
#include <font/LogicalFontInstance.hxx>
#include <tools/debug.hxx>
#include <impfontcache.hxx>

using namespace vcl::font;

size_t ImplFontCache::IFSD_Hash::operator()( const vcl::font::FontSelectPattern& rFSD ) const
{
    return rFSD.hashCode();
}

bool ImplFontCache::IFSD_Equal::operator()(const vcl::font::FontSelectPattern& rA, const vcl::font::FontSelectPattern& rB) const
{
    // check normalized font family name
    if( rA.maSearchName != rB.maSearchName )
        return false;

    // check font transformation
    if( (rA.mnHeight       != rB.mnHeight)
    ||  (rA.mnWidth        != rB.mnWidth)
    ||  (rA.mnOrientation  != rB.mnOrientation) )
        return false;

    // check mapping relevant attributes
    if( (rA.mbVertical     != rB.mbVertical)
    ||  (rA.meLanguage     != rB.meLanguage) )
        return false;

    // check font face attributes
    if( (rA.GetWeight()    != rB.GetWeight())
    ||  (rA.GetItalic()    != rB.GetItalic())
//  ||  (rA.meFamily       != rB.meFamily) // TODO: remove this mostly obsolete member
    ||  (rA.GetPitch()     != rB.GetPitch()) )
        return false;

    // check style name
    if( rA.GetStyleName() != rB.GetStyleName() )
        return false;

    // Symbol fonts may recode from one type to another So they are only
    // safely equivalent for equal targets
    if (rA.IsMicrosoftSymbolEncoded() || rB.IsMicrosoftSymbolEncoded())
    {
        if (rA.maTargetName != rB.maTargetName)
            return false;
    }

    // check for features
    if ((rA.maTargetName.indexOf(vcl::font::FontSelectPattern::FEAT_PREFIX)
         != -1 ||
         rB.maTargetName.indexOf(vcl::font::FontSelectPattern::FEAT_PREFIX)
         != -1) && rA.maTargetName != rB.maTargetName)
        return false;

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

ImplFontCache::ImplFontCache()
    : mpLastHitCacheEntry( nullptr )
    , maFontInstanceList(std::numeric_limits<size_t>::max()) // "unlimited", i.e. no cleanup
    // The cache limit is set by the rough number of characters needed to read your average Asian newspaper.
    , m_aBoundRectCache(3000)
{}

ImplFontCache::~ImplFontCache()
{
    DBG_TESTSOLARMUTEX();
    for (const auto & rLFI : maFontInstanceList)
    {
        rLFI.second->mpFontCache = nullptr;
    }
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetFontInstance( PhysicalFontCollection const * pFontList,
    const vcl::Font& rFont, const Size& rSize, float fExactHeight, bool bNonAntialias )
{
    // initialize internal font request object
    vcl::font::FontSelectPattern aFontSelData(rFont, rFont.GetFamilyName(), rSize, fExactHeight, bNonAntialias);
    return GetFontInstance( pFontList, aFontSelData );
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetFontInstance( PhysicalFontCollection const * pFontList,
    vcl::font::FontSelectPattern& aFontSelData )
{
    DBG_TESTSOLARMUTEX();
    rtl::Reference<LogicalFontInstance> pFontInstance;
    PhysicalFontFamily* pFontFamily = nullptr;

    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    if (mpLastHitCacheEntry && IFSD_Equal()(aFontSelData, mpLastHitCacheEntry->GetFontSelectPattern()))
        pFontInstance = mpLastHitCacheEntry;
    else
    {
        FontInstanceList::const_iterator it = maFontInstanceList.find( aFontSelData );
        if( it != maFontInstanceList.end() )
            pFontInstance = (*it).second;
    }

    if( !pFontInstance ) // no direct cache hit
    {
        // find the best matching logical font family and update font selector accordingly
        pFontFamily = pFontList->FindFontFamily( aFontSelData );
        SAL_WARN_IF( (pFontFamily == nullptr), "vcl", "ImplFontCache::Get() No logical font found!" );
        if( pFontFamily )
        {
            aFontSelData.maSearchName = pFontFamily->GetSearchName();

            // check if an indirectly matching logical font instance is already cached
            FontInstanceList::const_iterator it = maFontInstanceList.find( aFontSelData );
            if( it != maFontInstanceList.end() )
                pFontInstance = (*it).second;
        }
    }

    if( !pFontInstance && pFontFamily) // still no cache hit => create a new font instance
    {
        vcl::font::PhysicalFontFace* pFontData = pFontFamily->FindBestFontFace(aFontSelData);

        // create a new logical font instance from this physical font face
        pFontInstance = pFontData->CreateFontInstance( aFontSelData );
        pFontInstance->mpFontCache = this;

        // if we're substituting from or to a symbol font we may need a symbol
        // conversion table
        if( pFontData->IsMicrosoftSymbolEncoded() || aFontSelData.IsMicrosoftSymbolEncoded() || IsOpenSymbol(aFontSelData.maSearchName) )
        {
            if( aFontSelData.maTargetName != aFontSelData.maSearchName )
                pFontInstance->mpConversion = ConvertChar::GetRecodeData( aFontSelData.maTargetName, aFontSelData.maSearchName );
        }

#ifdef MACOSX
        //It might be better to dig out the font version of the target font
        //to see if it's a modern re-coded apple symbol font in case that
        //font shows up on a different platform
        if (!pFontInstance->mpConversion &&
            aFontSelData.maTargetName.equalsIgnoreAsciiCase("symbol") &&
            aFontSelData.maSearchName.equalsIgnoreAsciiCase("symbol"))
        {
            pFontInstance->mpConversion = ConvertChar::GetRecodeData( u"Symbol", u"AppleSymbol" );
        }
#endif

        static const size_t FONTCACHE_MAX = getenv("LO_TESTNAME") ? 1 : 50;

        if (maFontInstanceList.size() >= FONTCACHE_MAX)
        {
            struct limit_exception : public std::exception {};
            try
            {
                maFontInstanceList.remove_if([this] (FontInstanceList::key_value_pair_t const& rFontPair)
                    {
                        if (maFontInstanceList.size() < FONTCACHE_MAX)
                            throw limit_exception();
                        LogicalFontInstance* pFontEntry = rFontPair.second.get();
                        if (pFontEntry->m_nCount > 1)
                            return false;
                        m_aBoundRectCache.remove_if([&pFontEntry] (GlyphBoundRectCache::key_value_pair_t const& rGlyphPair)
                            { return rGlyphPair.first.m_pFont == pFontEntry; });
                        if (mpLastHitCacheEntry == pFontEntry)
                            mpLastHitCacheEntry = nullptr;
                        return true;
                    });
            }
            catch (limit_exception&) {}
        }

        assert(pFontInstance);
        // add the new entry to the cache
        maFontInstanceList.insert({aFontSelData, pFontInstance.get()});
    }

    mpLastHitCacheEntry = pFontInstance.get();
    return pFontInstance;
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetGlyphFallbackFont( PhysicalFontCollection const * pFontCollection,
    vcl::font::FontSelectPattern& rFontSelData, LogicalFontInstance* pFontInstance, int nFallbackLevel, OUString& rMissingCodes )
{
    // get a candidate font for glyph fallback
    // unless the previously selected font got a device specific substitution
    // e.g. PsPrint Arial->Helvetica for udiaeresis when Helvetica doesn't support it
    if( nFallbackLevel >= 1)
    {
        PhysicalFontFamily* pFallbackData = nullptr;

        //fdo#33898 If someone has EUDC installed then they really want that to
        //be used as the first-choice glyph fallback seeing as it's filled with
        //private area codes with don't make any sense in any other font so
        //prioritize it here if it's available. Ideally we would remove from
        //rMissingCodes all the glyphs which it is able to resolve as an
        //optimization, but that's tricky to achieve cross-platform without
        //sufficient heavy-weight code that's likely to undo the value of the
        //optimization
        if (nFallbackLevel == 1)
            pFallbackData = pFontCollection->FindFontFamily(u"EUDC");
        if (!pFallbackData)
            pFallbackData = pFontCollection->GetGlyphFallbackFont(rFontSelData, pFontInstance, rMissingCodes, nFallbackLevel-1);
        // escape when there are no font candidates
        if( !pFallbackData  )
            return nullptr;
        // override the font name
        rFontSelData.SetFamilyName( pFallbackData->GetFamilyName() );
        // clear the cached normalized name
        rFontSelData.maSearchName.clear();
    }

    rtl::Reference<LogicalFontInstance> pFallbackFont = GetFontInstance( pFontCollection, rFontSelData );
    return pFallbackFont;
}

void ImplFontCache::Invalidate()
{
    DBG_TESTSOLARMUTEX();
    // #112304# make sure the font cache is really clean
    mpLastHitCacheEntry = nullptr;
    for (auto const & pair : maFontInstanceList)
        pair.second->mpFontCache = nullptr;
    maFontInstanceList.clear();
    m_aBoundRectCache.clear();
}

bool ImplFontCache::GetCachedGlyphBoundRect(const LogicalFontInstance *pFont, sal_GlyphId nID, basegfx::B2DRectangle &rRect)
{
    if (!pFont->GetFontCache())
        return false;
    assert(pFont->GetFontCache() == this);
    if (pFont->GetFontCache() != this)
        return false;

    auto it = m_aBoundRectCache.find({pFont, nID});
    if (it != m_aBoundRectCache.end())
    {
        rRect = it->second;
        return true;
    }
    return false;
}

void ImplFontCache::CacheGlyphBoundRect(const LogicalFontInstance *pFont, sal_GlyphId nID, basegfx::B2DRectangle &rRect)
{
    if (!pFont->GetFontCache())
        return;
    assert(pFont->GetFontCache() == this);
    if (pFont->GetFontCache() != this)
        return;

    m_aBoundRectCache.insert({{pFont, nID}, rRect});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
