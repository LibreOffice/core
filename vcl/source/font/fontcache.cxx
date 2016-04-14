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

#include "svdata.hxx"

#include "fontinstance.hxx"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

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
    if (
        (rA.mpFontData && rA.mpFontData->IsSymbolFont()) ||
        (rB.mpFontData && rB.mpFontData->IsSymbolFont())
       )
    {
        if (rA.maTargetName != rB.maTargetName)
            return false;
    }

#if ENABLE_GRAPHITE
    // check for features
    if ((rA.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
         != -1 ||
         rB.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
         != -1) && rA.maTargetName != rB.maTargetName)
        return false;
#endif

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

ImplFontCache::ImplFontCache()
:   mpFirstEntry( nullptr ),
    mnRef0Count( 0 )
{}

ImplFontCache::~ImplFontCache()
{
    FontInstanceList::iterator it = maFontInstanceList.begin();
    for(; it != maFontInstanceList.end(); ++it )
    {
        LogicalFontInstance* pFontInstance = (*it).second;
        delete pFontInstance;
    }
}

LogicalFontInstance* ImplFontCache::GetFontInstance( PhysicalFontCollection* pFontList,
    const vcl::Font& rFont, const Size& rSize, float fExactHeight )
{
    const OUString& aSearchName = rFont.GetFamilyName();

    // initialize internal font request object
    FontSelectPattern aFontSelData( rFont, aSearchName, rSize, fExactHeight );
    return GetFontInstance( pFontList, aFontSelData );
}

LogicalFontInstance* ImplFontCache::GetFontInstance( PhysicalFontCollection* pFontList,
    FontSelectPattern& aFontSelData )
{
    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    LogicalFontInstance *pFontInstance = nullptr;
    PhysicalFontFamily* pFontFamily = nullptr;
    IFSD_Equal aIFSD_Equal;
    if( mpFirstEntry && aIFSD_Equal( aFontSelData, mpFirstEntry->maFontSelData ) )
        pFontInstance = mpFirstEntry;
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
        DBG_ASSERT( (pFontFamily != nullptr), "ImplFontCache::Get() No logical font found!" );
        if( pFontFamily )
            aFontSelData.maSearchName = pFontFamily->GetSearchName();

        // check if an indirectly matching logical font instance is already cached
        FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
        if( it != maFontInstanceList.end() )
        {
            // we have an indirect cache hit
            pFontInstance = (*it).second;
        }
    }

    PhysicalFontFace* pFontData = nullptr;

    if (!pFontInstance && pFontFamily)// no cache hit => find the best matching physical font face
    {
        bool bOrigWasSymbol = aFontSelData.mpFontData && aFontSelData.mpFontData->IsSymbolFont();
        pFontData = pFontFamily->FindBestFontFace( aFontSelData );
        aFontSelData.mpFontData = pFontData;
        bool bNewIsSymbol = aFontSelData.mpFontData && aFontSelData.mpFontData->IsSymbolFont();

        if (bNewIsSymbol != bOrigWasSymbol)
        {
            // it is possible, though generally unlikely, that at this point we
            // will attempt to use a symbol font as a last-ditch fallback for a
            // non-symbol font request or vice versa, and by changing
            // aFontSelData.mpFontData to/from a symbol font we may now find
            // something in the cache that can be reused which previously
            // wasn't a candidate
            FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
            if( it != maFontInstanceList.end() )
                pFontInstance = (*it).second;
        }
    }

    if( pFontInstance ) // cache hit => use existing font instance
    {
        // increase the font instance's reference count
        Acquire(pFontInstance);
    }

    if (!pFontInstance && pFontData)// still no cache hit => create a new font instance
    {
        // create a new logical font instance from this physical font face
        pFontInstance = pFontData->CreateFontInstance( aFontSelData );
        pFontInstance->mpFontCache = this;

        // if we're subtituting from or to a symbol font we may need a symbol
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

        // add the new entry to the cache
        maFontInstanceList[ aFontSelData ] = pFontInstance;
    }

    mpFirstEntry = pFontInstance;
    return pFontInstance;
}

LogicalFontInstance* ImplFontCache::GetGlyphFallbackFont( PhysicalFontCollection* pFontCollection,
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
        //prioritise it here if it's available. Ideally we would remove from
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

    LogicalFontInstance* pFallbackFont = GetFontInstance( pFontCollection, rFontSelData );
    return pFallbackFont;
}

void ImplFontCache::Acquire(LogicalFontInstance* pFontInstance)
{
    assert(pFontInstance->mpFontCache == this);

    if (0 == pFontInstance->mnRefCount++)
        --mnRef0Count;
}

void ImplFontCache::Release(LogicalFontInstance* pFontInstance)
{
    static const int FONTCACHE_MAX = getenv("LO_TESTNAME") ? 1 : 50;

    assert(pFontInstance->mnRefCount > 0 && "ImplFontCache::Release() - font refcount underflow");
    if( --pFontInstance->mnRefCount > 0 )
        return;

    if (++mnRef0Count < FONTCACHE_MAX)
        return;

    assert(CountUnreferencedEntries() == mnRef0Count);

    // remove unused entries from font instance cache
    FontInstanceList::iterator it_next = maFontInstanceList.begin();
    while( it_next != maFontInstanceList.end() )
    {
        FontInstanceList::iterator it = it_next++;
        LogicalFontInstance* pFontEntry = (*it).second;
        if( pFontEntry->mnRefCount > 0 )
            continue;

        maFontInstanceList.erase( it );
        delete pFontEntry;
        --mnRef0Count;
        assert(mnRef0Count>=0 && "ImplFontCache::Release() - refcount0 underflow");

        if( mpFirstEntry == pFontEntry )
            mpFirstEntry = nullptr;
    }

    assert(mnRef0Count==0 && "ImplFontCache::Release() - refcount0 mismatch");
}

int ImplFontCache::CountUnreferencedEntries() const
{
    size_t nCount = 0;
    // count unreferenced entries
    for (FontInstanceList::const_iterator it = maFontInstanceList.begin();
         it != maFontInstanceList.end(); ++it)
    {
        const LogicalFontInstance* pFontEntry = it->second;
        if (pFontEntry->mnRefCount > 0)
            continue;
        ++nCount;
    }
    return nCount;
}

void ImplFontCache::Invalidate()
{
    assert(CountUnreferencedEntries() == mnRef0Count);

    // delete unreferenced entries
    FontInstanceList::iterator it = maFontInstanceList.begin();
    for(; it != maFontInstanceList.end(); ++it )
    {
        LogicalFontInstance* pFontEntry = (*it).second;
        if( pFontEntry->mnRefCount > 0 )
            continue;

        delete pFontEntry;
        --mnRef0Count;
    }

    // #112304# make sure the font cache is really clean
    mpFirstEntry = nullptr;
    maFontInstanceList.clear();

    assert(mnRef0Count==0 && "ImplFontCache::Invalidate() - mnRef0Count non-zero");
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
