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
#include <PhysicalFontCollection.hxx>
#include <PhysicalFontFace.hxx>
#include <PhysicalFontFamily.hxx>

size_t ImplFontCache::IFSD_Hash::operator()( const FontSelectPattern& rFSD ) const
{
    return rFSD.hashCode();
}

bool ImplFontCache::IFSD_Equal::operator()(const FontSelectPattern& rA, const FontSelectPattern& rB) const
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
    if( (rA.GetWeight()       != rB.GetWeight())
    ||  (rA.GetItalic()    != rB.GetItalic())
//  ||  (rA.meFamily       != rB.meFamily) // TODO: remove this mostly obsolete member
    ||  (rA.GetPitch()     != rB.GetPitch()) )
        return false;

    // check style name
    if( rA.GetStyleName() != rB.GetStyleName() )
        return false;

    // Symbol fonts may recode from one type to another So they are only
    // safely equivalent for equal targets
    if (rA.IsSymbolFont() || rB.IsSymbolFont())
    {
        if (rA.maTargetName != rB.maTargetName)
            return false;
    }

    // check for features
    if ((rA.maTargetName.indexOf(FontSelectPatternAttributes::FEAT_PREFIX)
         != -1 ||
         rB.maTargetName.indexOf(FontSelectPatternAttributes::FEAT_PREFIX)
         != -1) && rA.maTargetName != rB.maTargetName)
        return false;

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

ImplFontCache::ImplFontCache()
:   mpLastHitCacheEntry( nullptr )
{}

ImplFontCache::~ImplFontCache()
{
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetFontInstance( PhysicalFontCollection const * pFontList,
    const vcl::Font& rFont, const Size& rSize, float fExactHeight )
{
    // initialize internal font request object
    FontSelectPattern aFontSelData(rFont, rFont.GetFamilyName(), rSize, fExactHeight);
    return GetFontInstance( pFontList, aFontSelData );
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetFontInstance( PhysicalFontCollection const * pFontList,
    FontSelectPattern& aFontSelData )
{
    rtl::Reference<LogicalFontInstance> pFontInstance;
    PhysicalFontFamily* pFontFamily = nullptr;

    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    if (mpLastHitCacheEntry && IFSD_Equal()(aFontSelData, mpLastHitCacheEntry->GetFontSelectPattern()))
        pFontInstance = mpLastHitCacheEntry;
    else
    {
        FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
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
            FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
            if( it != maFontInstanceList.end() )
                pFontInstance = (*it).second;
        }
    }

    if( !pFontInstance && pFontFamily) // still no cache hit => create a new font instance
    {
        PhysicalFontFace* pFontData = pFontFamily->FindBestFontFace(aFontSelData);

        // create a new logical font instance from this physical font face
        pFontInstance = pFontData->CreateFontInstance( aFontSelData );
        pFontInstance->mpFontCache = this;

        // if we're substituting from or to a symbol font we may need a symbol
        // conversion table
        if( pFontData->IsSymbolFont() || aFontSelData.IsSymbolFont() )
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
            pFontInstance->mpConversion = ConvertChar::GetRecodeData( "Symbol", "AppleSymbol" );
        }
#endif

        static const size_t FONTCACHE_MAX = getenv("LO_TESTNAME") ? 1 : 50;

        if (maFontInstanceList.size() >= FONTCACHE_MAX)
        {
            // remove entries from font instance cache that are only referenced by the cache
            FontInstanceList::iterator it_next = maFontInstanceList.begin();
            while( it_next != maFontInstanceList.end() )
            {
                LogicalFontInstance* pFontEntry = (*it_next).second.get();
                if( pFontEntry->m_nCount > 1 )
                {
                    ++it_next;
                    continue;
                }
                maFontInstanceList.erase(it_next);
                if (mpLastHitCacheEntry == pFontEntry)
                    mpLastHitCacheEntry = nullptr;
                // just remove one entry, which will bring us back under FONTCACHE_MAX size again
                break;
            }
        }

        assert(pFontInstance);
        // add the new entry to the cache
        maFontInstanceList.insert({aFontSelData, pFontInstance.get()});
    }

    mpLastHitCacheEntry = pFontInstance.get();
    return pFontInstance;
}

rtl::Reference<LogicalFontInstance> ImplFontCache::GetGlyphFallbackFont( PhysicalFontCollection const * pFontCollection,
    FontSelectPattern& rFontSelData, int nFallbackLevel, OUString& rMissingCodes )
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
            pFallbackData = pFontCollection->FindFontFamily("EUDC");
        if (!pFallbackData)
            pFallbackData = pFontCollection->GetGlyphFallbackFont(rFontSelData, rMissingCodes, nFallbackLevel-1);
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
    // #112304# make sure the font cache is really clean
    mpLastHitCacheEntry = nullptr;
    for (auto const & pair : maFontInstanceList)
        pair.second->mpFontCache = nullptr;
    maFontInstanceList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
