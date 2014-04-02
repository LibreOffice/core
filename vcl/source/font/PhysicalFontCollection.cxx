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

#include <vector>

#include <i18nlangtag/mslangid.hxx>
#include <tools/debug.hxx>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#include "magic.h"
#include "outdev.h"
#include "outfont.hxx"
#include "PhysicalFontFace.hxx"

#include "PhysicalFontCollection.hxx"

static OUString lcl_stripCharSetFromName(const OUString& _aName)
{
    // I worry that someone will have a font which *does* have
    // e.g. "Greek" legitimately at the end of its name :-(
    const char*suffixes[] = { " baltic",
                              " ce",
                              " cyr",
                              " greek",
                              " tur",
                              " (arabic)",
                              " (hebrew)",
                              " (thai)",
                              " (vietnamese)"
                            };

    OUString aName = _aName;
    // These can be crazily piled up, e.g. Times New Roman CYR Greek
    bool bFinished = false;
    while (!bFinished)
    {
        bFinished = true;
        for (size_t i = 0; i < SAL_N_ELEMENTS(suffixes); ++i)
        {
            size_t nLen = strlen(suffixes[i]);
            if (aName.endsWithIgnoreAsciiCaseAsciiL(suffixes[i], nLen))
            {
                bFinished = false;
                aName = aName.copy(0, aName.getLength() - nLen);
            }
        }
    }
    return aName;
}

static unsigned lcl_IsCJKFont( const OUString& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    for(int i = 0; i < rFontName.getLength(); i++)
    {
        const sal_Unicode ch = rFontName[i];
        // japanese
        if ( ((ch >= 0x3040) && (ch <= 0x30FF)) ||
             ((ch >= 0x3190) && (ch <= 0x319F)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_JP;

        // korean
        if ( ((ch >= 0xAC00) && (ch <= 0xD7AF)) ||
             ((ch >= 0x3130) && (ch <= 0x318F)) ||
             ((ch >= 0x1100) && (ch <= 0x11FF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_KR;

        // chinese
        if ( ((ch >= 0x3400) && (ch <= 0x9FFF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_TC|IMPL_FONT_ATTR_CJK_SC;

        // cjk
        if ( ((ch >= 0x3000) && (ch <= 0xD7AF)) ||
             ((ch >= 0xFF00) && (ch <= 0xFFEE)) )
            return IMPL_FONT_ATTR_CJK;

    }

    return 0;
}

PhysicalFontCollection::PhysicalFontCollection()
    : mbMatchData( false )
    , mbMapNames( false )
    , mpPreMatchHook( NULL )
    , mpFallbackHook( NULL )
    , mpFallbackList( NULL )
    , mnFallbackCount( -1 )
{}

PhysicalFontCollection::~PhysicalFontCollection()
{
    Clear();
}

void PhysicalFontCollection::SetPreMatchHook( ImplPreMatchFontSubstitution* pHook )
{
    mpPreMatchHook = pHook;
}

void PhysicalFontCollection::SetFallbackHook( ImplGlyphFallbackFontSubstitution* pHook )
{
    mpFallbackHook = pHook;
}

void PhysicalFontCollection::Clear()
{
    // remove fallback lists
    delete[] mpFallbackList;
    mpFallbackList = NULL;
    mnFallbackCount = -1;

    // clear all entries in the device font list
    PhysicalFontFamilies::iterator it = maPhysicalFontFamilies.begin();
    for(; it != maPhysicalFontFamilies.end(); ++it )
    {
        PhysicalFontFamily* pEntry = (*it).second;
        delete pEntry;
    }

    maPhysicalFontFamilies.clear();

    // match data must be recalculated too
    mbMatchData = false;
}

void PhysicalFontCollection::InitGenericGlyphFallback( void ) const
{
    // normalized family names of fonts suited for glyph fallback
    // if a font is available related fonts can be ignored
    // TODO: implement dynamic lists
    static const char* aGlyphFallbackList[] = {
        // empty strings separate the names of unrelated fonts
        "eudc", "",
        "arialunicodems", "cyberbit", "code2000", "",
        "andalesansui", "",
        "starsymbol", "opensymbol", "",
        "msmincho", "fzmingti", "fzheiti", "ipamincho", "sazanamimincho", "kochimincho", "",
        "sunbatang", "sundotum", "baekmukdotum", "gulim", "batang", "dotum", "",
        "hgmincholightj", "msunglightsc", "msunglighttc", "hymyeongjolightk", "",
        "tahoma", "dejavusans", "timesnewroman", "liberationsans", "",
        "shree", "mangal", "",
        "raavi", "shruti", "tunga", "",
        "latha", "gautami", "kartika", "vrinda", "",
        "shayyalmt", "naskmt", "scheherazade", "",
        "david", "nachlieli", "lucidagrande", "",
        "norasi", "angsanaupc", "",
        "khmerossystem", "",
        "muktinarrow", "",
        "phetsarathot", "",
        "padauk", "pinlonmyanmar", "",
        "iskoolapota", "lklug", "",
        0
    };

    bool bHasEudc = false;
    int nMaxLevel = 0;
    int nBestQuality = 0;
    PhysicalFontFamily** pFallbackList = NULL;

    for( const char** ppNames = &aGlyphFallbackList[0];; ++ppNames )
    {
        // advance to next sub-list when end-of-sublist marker
        if( !**ppNames ) // #i46456# check for empty string, i.e., deref string itself not only ptr to it
        {
            if( nBestQuality > 0 )
                if( ++nMaxLevel >= MAX_FALLBACK )
                    break;

            if( !ppNames[1] )
                break;

            nBestQuality = 0;
            continue;
        }

        // test if the glyph fallback candidate font is available and scalable
        OUString aTokenName( *ppNames, strlen(*ppNames), RTL_TEXTENCODING_UTF8 );
        PhysicalFontFamily* pFallbackFont = FindFontFamily( aTokenName );

        if( !pFallbackFont )
            continue;

        if( !pFallbackFont->IsScalable() )
            continue;

        // keep the best font of the glyph fallback sub-list
        if( nBestQuality < pFallbackFont->GetMinQuality() )
        {
            nBestQuality = pFallbackFont->GetMinQuality();
            // store available glyph fallback fonts
            if( !pFallbackList )
                pFallbackList = new PhysicalFontFamily*[ MAX_FALLBACK ];

            pFallbackList[ nMaxLevel ] = pFallbackFont;
            if( !bHasEudc && !nMaxLevel )
                bHasEudc = !strncmp( *ppNames, "eudc", 5 );
        }
    }

#ifdef SAL_FONTENUM_STABLE_ON_PLATFORM // #i113472#
    // sort the list of fonts for glyph fallback by quality (highest first)
    // #i33947# keep the EUDC font at the front of the list
    // an insertion sort is good enough for this short list
    const int nSortStart = bHasEudc ? 1 : 0;
    for( int i = nSortStart+1, j; i < nMaxLevel; ++i )
    {
        PhysicalFontFamily* pTestFont = pFallbackList[ i ];
        int nTestQuality = pTestFont->GetMinQuality();

        for( j = i; --j >= nSortStart; )
        {
            if( nTestQuality > pFallbackList[j]->GetMinQuality() )
                pFallbackList[ j+1 ] = pFallbackList[ j ];
            else
                break;
        }
        pFallbackList[ j+1 ] = pTestFont;
    }
#endif

    mnFallbackCount = nMaxLevel;
    mpFallbackList  = pFallbackList;
}

PhysicalFontFamily* PhysicalFontCollection::GetGlyphFallbackFont( FontSelectPattern& rFontSelData,
                                                                  OUString& rMissingCodes,
                                                                  int nFallbackLevel ) const
{
    PhysicalFontFamily* pFallbackData = NULL;

    // find a matching font candidate for platform specific glyph fallback
    if( mpFallbackHook )
    {
        // check cache for the first matching entry
        // to avoid calling the expensive fallback hook (#i83491#)
        sal_UCS4 cChar = 0;
        bool bCached = true;
        sal_Int32 nStrIndex = 0;
        while( nStrIndex < rMissingCodes.getLength() )
        {
            cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
            bCached = rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &rFontSelData.maSearchName );

            // ignore entries which don't have a fallback
            if( !bCached || !rFontSelData.maSearchName.isEmpty() )
                break;
        }

        if( bCached )
        {
            // there is a matching fallback in the cache
            // so update rMissingCodes with codepoints not yet resolved by this fallback
            int nRemainingLength = 0;
            sal_UCS4* pRemainingCodes = (sal_UCS4*)alloca( rMissingCodes.getLength() * sizeof(sal_UCS4) );
            OUString aFontName;

            while( nStrIndex < rMissingCodes.getLength() )
            {
                cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
                bCached = rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &aFontName );
                if( !bCached || (rFontSelData.maSearchName != aFontName) )
                    pRemainingCodes[ nRemainingLength++ ] = cChar;
            }
            rMissingCodes = OUString( pRemainingCodes, nRemainingLength );
        }
        else
        {
            OUString aOldMissingCodes = rMissingCodes;

            // call the hook to query the best matching glyph fallback font
            if( mpFallbackHook->FindFontSubstitute( rFontSelData, rMissingCodes ) )
                // apply outdev3.cxx specific fontname normalization
                GetEnglishSearchFontName( rFontSelData.maSearchName );
            else
                rFontSelData.maSearchName = "";

            // See fdo#32665 for an example. FreeSerif that has glyphs in normal
            // font, but not in the italic or bold version
            bool bSubSetOfFontRequiresPropertyFaking = rFontSelData.mbEmbolden || rFontSelData.maItalicMatrix != ItalicMatrix();

            // Cache the result even if there was no match, unless its from part of a font for which the properties need
            // to be faked. We need to rework this cache to take into account that fontconfig can return different fonts
            // for different input sizes, weights, etc. Basically the cache is way to naive
            if (!bSubSetOfFontRequiresPropertyFaking)
            {
                for(;;)
                {
                     if( !rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &rFontSelData.maSearchName ) )
                         rFontSelData.mpFontEntry->AddFallbackForUnicode( cChar, rFontSelData.GetWeight(), rFontSelData.maSearchName );
                     if( nStrIndex >= aOldMissingCodes.getLength() )
                         break;
                     cChar = aOldMissingCodes.iterateCodePoints( &nStrIndex );
                }
                if( !rFontSelData.maSearchName.isEmpty() )
                {
                    // remove cache entries that were still not resolved
                    for( nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
                    {
                        cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
                        rFontSelData.mpFontEntry->IgnoreFallbackForUnicode( cChar, rFontSelData.GetWeight(), rFontSelData.maSearchName );
                    }
                }
            }
        }

        // find the matching device font
        if( !rFontSelData.maSearchName.isEmpty() )
            pFallbackData = FindFontFamily( rFontSelData.maSearchName );
    }

    // else find a matching font candidate for generic glyph fallback
    if( !pFallbackData )
    {
        // initialize font candidates for generic glyph fallback if needed
        if( mnFallbackCount < 0 )
            InitGenericGlyphFallback();

        // TODO: adjust nFallbackLevel by number of levels resolved by the fallback hook
        if( nFallbackLevel < mnFallbackCount )
            pFallbackData = mpFallbackList[ nFallbackLevel ];
    }

    return pFallbackData;
}

void PhysicalFontCollection::Add( PhysicalFontFace* pNewData )
{
    OUString aSearchName = pNewData->GetFamilyName();
    GetEnglishSearchFontName( aSearchName );

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.find( aSearchName );
    PhysicalFontFamily* pFoundData = NULL;

    if( it != maPhysicalFontFamilies.end() )
        pFoundData = (*it).second;

    if( !pFoundData )
    {
        pFoundData = new PhysicalFontFamily( aSearchName );
        maPhysicalFontFamilies[ aSearchName ] = pFoundData;
    }

    bool bKeepNewData = pFoundData->AddFontFace( pNewData );

    if( !bKeepNewData )
        delete pNewData;
}

// find the font from the normalized font family name
PhysicalFontFamily* PhysicalFontCollection::ImplFindBySearchName( const OUString& rSearchName ) const
{
#ifdef DEBUG
    OUString aTempName = rSearchName;
    GetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rSearchName, "PhysicalFontCollection::ImplFindBySearchName() called with non-normalized name" );
#endif

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.find( rSearchName );
    if( it == maPhysicalFontFamilies.end() )
        return NULL;

    PhysicalFontFamily* pFoundData = (*it).second;
    return pFoundData;
}

PhysicalFontFamily* PhysicalFontCollection::ImplFindByAliasName(const OUString& rSearchName,
    const OUString& rShortName) const
{
    // short circuit for impossible font name alias
    if (rSearchName.isEmpty())
        return NULL;

    // short circuit if no alias names are available
    if (!mbMapNames)
        return NULL;

    // use the font's alias names to find the font
    // TODO: get rid of linear search
    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    while( it != maPhysicalFontFamilies.end() )
    {
        PhysicalFontFamily* pData = (*it).second;
        if( pData->GetAliasNames().isEmpty() )
            continue;

        // if one alias name matches we found a matching font
        OUString aTempName;
        sal_Int32 nIndex = 0;

        do
        {
            aTempName = GetNextFontToken( pData->GetAliasNames(), nIndex );
           // Test, if the Font name match with one of the mapping names
           if ( (aTempName == rSearchName) || (aTempName == rShortName) )
              return pData;
        }
        while ( nIndex != -1 );
     }

     return NULL;
}

PhysicalFontFamily* PhysicalFontCollection::FindFontFamily( const OUString& rFontName ) const
{
    // normalize the font family name and
    OUString aName = rFontName;
    GetEnglishSearchFontName( aName );

    PhysicalFontFamily* pFound = ImplFindBySearchName( aName );
    return pFound;
}

PhysicalFontFamily* PhysicalFontCollection::ImplFindByTokenNames(const OUString& rTokenStr) const
{
    PhysicalFontFamily* pFoundData = NULL;

    // use normalized font name tokens to find the font
    for( sal_Int32 nTokenPos = 0; nTokenPos != -1; )
    {
        OUString aSearchName = GetNextFontToken( rTokenStr, nTokenPos );
        if( aSearchName.isEmpty() )
            continue;

        GetEnglishSearchFontName( aSearchName );
        pFoundData = ImplFindBySearchName( aSearchName );

        if( pFoundData )
            break;
    }

    return pFoundData;
}

PhysicalFontFamily* PhysicalFontCollection::ImplFindBySubstFontAttr( const utl::FontNameAttr& rFontAttr ) const
{
    PhysicalFontFamily* pFoundData = NULL;

    // use the font substitutions suggested by the FontNameAttr to find the font
    ::std::vector< OUString >::const_iterator it = rFontAttr.Substitutions.begin();
    for(; it != rFontAttr.Substitutions.end(); ++it )
    {
        OUString aSearchName( *it );
        GetEnglishSearchFontName( aSearchName );

        pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // use known attributes from the configuration to find a matching substitute
    const sal_uLong nSearchType = rFontAttr.Type;
    if( nSearchType != 0 )
    {
        const FontWeight eSearchWeight = rFontAttr.Weight;
        const FontWidth  eSearchWidth  = rFontAttr.Width;
        const FontItalic eSearchSlant  = ITALIC_DONTKNOW;
        const OUString aSearchName;

        pFoundData = ImplFindByAttributes( nSearchType,
            eSearchWeight, eSearchWidth, eSearchSlant, aSearchName );

        if( pFoundData )
            return pFoundData;
    }

    return NULL;
}

void PhysicalFontCollection::InitMatchData() const
{
    // short circuit if already done
    if( mbMatchData )
        return;
    mbMatchData = true;

    // calculate MatchData for all entries
    const utl::FontSubstConfiguration& rFontSubst = utl::FontSubstConfiguration::get();

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    for(; it != maPhysicalFontFamilies.end(); ++it )
    {
        const OUString& rSearchName = (*it).first;
        PhysicalFontFamily* pEntry = (*it).second;

        pEntry->InitMatchData( rFontSubst, rSearchName );
    }
}

PhysicalFontFamily* PhysicalFontCollection::ImplFindByAttributes( sal_uLong nSearchType,
                                                                  FontWeight eSearchWeight,
                                                                  FontWidth eSearchWidth,
                                                                  FontItalic eSearchItalic,
                                                                  const OUString& rSearchFamilyName ) const
{
    if( (eSearchItalic != ITALIC_NONE) && (eSearchItalic != ITALIC_DONTKNOW) )
        nSearchType |= IMPL_FONT_ATTR_ITALIC;

    // don't bother to match attributes if the attributes aren't worth matching
    if( !nSearchType
    && ((eSearchWeight == WEIGHT_DONTKNOW) || (eSearchWeight == WEIGHT_NORMAL))
    && ((eSearchWidth == WIDTH_DONTKNOW) || (eSearchWidth == WIDTH_NORMAL)) )
        return NULL;

    InitMatchData();
    PhysicalFontFamily* pFoundData = NULL;

    long    nTestMatch;
    long    nBestMatch = 40000;
    sal_uLong   nBestType = 0;

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    for(; it != maPhysicalFontFamilies.end(); ++it )
    {
        PhysicalFontFamily* pData = (*it).second;

        // Get all information about the matching font
        sal_uLong   nMatchType  = pData->GetMatchType();
        FontWeight  eMatchWeight= pData->GetMatchWeight();
        FontWidth   eMatchWidth = pData->GetMatchWidth();

        // Calculate Match Value
        // 1000000000
        //  100000000
        //   10000000   CJK, CTL, None-Latin, Symbol
        //    1000000   FamilyName, Script, Fixed, -Special, -Decorative,
        //              Titling, Capitals, Outline, Shadow
        //     100000   Match FamilyName, Serif, SansSerif, Italic,
        //              Width, Weight
        //      10000   Scalable, Standard, Default,
        //              full, Normal, Knownfont,
        //              Otherstyle, +Special, +Decorative,
        //       1000   Typewriter, Rounded, Gothic, Schollbook
        //        100
        nTestMatch = 0;

        // test CJK script attributes
        if ( nSearchType & IMPL_FONT_ATTR_CJK )
        {
            // Matching language
            if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_CJK_ALLLANG) )
                nTestMatch += 10000000*3;
            if( nMatchType & IMPL_FONT_ATTR_CJK )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }
        else if ( nMatchType & IMPL_FONT_ATTR_CJK )
        {
            nTestMatch -= 10000000;
        }

        // test CTL script attributes
        if( nSearchType & IMPL_FONT_ATTR_CTL )
        {
            if( nMatchType & IMPL_FONT_ATTR_CTL )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }
        else if ( nMatchType & IMPL_FONT_ATTR_CTL )
        {
            nTestMatch -= 10000000;
        }

        // test LATIN script attributes
        if( nSearchType & IMPL_FONT_ATTR_NONELATIN )
        {
            if( nMatchType & IMPL_FONT_ATTR_NONELATIN )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }

        // test SYMBOL attributes
        if ( nSearchType & IMPL_FONT_ATTR_SYMBOL )
        {
            const OUString& rSearchName = it->first;
            // prefer some special known symbol fonts
            if ( rSearchName == "starsymbol" )
            {
                nTestMatch += 10000000*6+(10000*3);
            }
            else if ( rSearchName == "opensymbol" )
            {
                nTestMatch += 10000000*6;
            }
            else if ( rSearchName == "starbats" ||
                      rSearchName == "wingdings" ||
                      rSearchName == "monotypesorts" ||
                      rSearchName == "dingbats" ||
                      rSearchName == "zapfdingbats" )
            {
                nTestMatch += 10000000*5;
            }
            else if ( pData->GetTypeFaces() & FONT_FAMILY_SYMBOL )
            {
                nTestMatch += 10000000*4;
            }
            else
            {
                if( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                    nTestMatch += 10000000*2;
                if( nMatchType & IMPL_FONT_ATTR_FULL )
                    nTestMatch += 10000000;
            }
        }
        else if ( (pData->GetTypeFaces() & (FONT_FAMILY_SYMBOL | FONT_FAMILY_NONESYMBOL)) == FONT_FAMILY_SYMBOL )
        {
            nTestMatch -= 10000000;
        }
        else if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
        {
            nTestMatch -= 10000;
        }

        // match stripped family name
        if( !rSearchFamilyName.isEmpty() && (rSearchFamilyName == pData->GetMatchFamilyName()) )
        {
            nTestMatch += 1000000*3;
        }

        // match ALLSCRIPT? attribute
        if( nSearchType & IMPL_FONT_ATTR_ALLSCRIPT )
        {
            if( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
            {
                nTestMatch += 1000000*2;
            }
            if( nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT )
            {
                if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_ALLSUBSCRIPT) )
                    nTestMatch += 1000000*2;
                if( 0 != ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_BRUSHSCRIPT) )
                    nTestMatch -= 1000000;
            }
        }
        else if( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
        {
            nTestMatch -= 1000000;
        }

        // test MONOSPACE+TYPEWRITER attributes
        if( nSearchType & IMPL_FONT_ATTR_FIXED )
        {
            if( nMatchType & IMPL_FONT_ATTR_FIXED )
                nTestMatch += 1000000*2;
            // a typewriter attribute is even better
            if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_TYPEWRITER) )
                nTestMatch += 10000*2;
        }
        else if( nMatchType & IMPL_FONT_ATTR_FIXED )
        {
            nTestMatch -= 1000000;
        }

        // test SPECIAL attribute
        if( nSearchType & IMPL_FONT_ATTR_SPECIAL )
        {
            if( nMatchType & IMPL_FONT_ATTR_SPECIAL )
            {
                nTestMatch += 10000;
            }
            else if( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
            {
                 if( nMatchType & IMPL_FONT_ATTR_SERIF )
                 {
                     nTestMatch += 1000*2;
                 }
                 else if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                 {
                     nTestMatch += 1000;
                 }
             }
        }
        else if( (nMatchType & IMPL_FONT_ATTR_SPECIAL) && !(nSearchType & IMPL_FONT_ATTR_SYMBOL) )
        {
            nTestMatch -= 1000000;
        }

        // test DECORATIVE attribute
        if( nSearchType & IMPL_FONT_ATTR_DECORATIVE )
        {
            if( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
            {
                nTestMatch += 10000;
            }
            else if( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
            {
                if( nMatchType & IMPL_FONT_ATTR_SERIF )
                    nTestMatch += 1000*2;
                else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                    nTestMatch += 1000;
            }
        }
        else if( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
        {
            nTestMatch -= 1000000;
        }

        // test TITLE+CAPITALS attributes
        if( nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
        {
            if( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
            {
                nTestMatch += 1000000*2;
            }
            if( 0 == ((nSearchType^nMatchType) & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)))
            {
                nTestMatch += 1000000;
            }
            else if( (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)) &&
                     (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
            {
                nTestMatch += 1000000;
            }
        }
        else if( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
        {
            nTestMatch -= 1000000;
        }

        // test OUTLINE+SHADOW attributes
        if( nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
        {
            if( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
            {
                nTestMatch += 1000000*2;
            }
            if( 0 == ((nSearchType ^ nMatchType) & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) )
            {
                nTestMatch += 1000000;
            }
            else if( (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) &&
                     (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
            {
                nTestMatch += 1000000;
            }
        }
        else if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
        {
            nTestMatch -= 1000000;
        }

        // test font name substrings
        // TODO: calculate name matching score using e.g. Levenstein distance
        if( (rSearchFamilyName.getLength() >= 4) &&
            (pData->GetMatchFamilyName().getLength() >= 4) &&
            ((rSearchFamilyName.indexOf( pData->GetMatchFamilyName() ) != -1) ||
             (pData->GetMatchFamilyName().indexOf( rSearchFamilyName ) != -1)) )
        {
            nTestMatch += 5000;
        }
        // test SERIF attribute
        if( nSearchType & IMPL_FONT_ATTR_SERIF )
        {
            if( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch += 1000000*2;
            else if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch -= 1000000;
        }

        // test SANSERIF attribute
        if( nSearchType & IMPL_FONT_ATTR_SANSSERIF )
        {
            if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch += 1000000;
            else if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch -= 1000000;
        }

        // test ITALIC attribute
        if( nSearchType & IMPL_FONT_ATTR_ITALIC )
        {
            if( pData->GetTypeFaces() & FONT_FAMILY_ITALIC )
                nTestMatch += 1000000*3;
            if( nMatchType & IMPL_FONT_ATTR_ITALIC )
                nTestMatch += 1000000;
        }
        else if( !(nSearchType & IMPL_FONT_ATTR_ALLSCRIPT) &&
                 ((nMatchType & IMPL_FONT_ATTR_ITALIC) ||
                  !(pData->GetTypeFaces() & FONT_FAMILY_NONEITALIC)) )
        {
            nTestMatch -= 1000000*2;
        }

        // test WIDTH attribute
        if( (eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL) )
        {
            if( eSearchWidth < WIDTH_NORMAL )
            {
                if( eSearchWidth == eMatchWidth )
                    nTestMatch += 1000000*3;
                else if( (eMatchWidth < WIDTH_NORMAL) && (eMatchWidth != WIDTH_DONTKNOW) )
                    nTestMatch += 1000000;
            }
            else
            {
                if( eSearchWidth == eMatchWidth )
                    nTestMatch += 1000000*3;
                else if( eMatchWidth > WIDTH_NORMAL )
                    nTestMatch += 1000000;
            }
        }
        else if( (eMatchWidth != WIDTH_DONTKNOW) && (eMatchWidth != WIDTH_NORMAL) )
        {
            nTestMatch -= 1000000;
        }

        // test WEIGHT attribute
        if( (eSearchWeight != WEIGHT_DONTKNOW) &&
            (eSearchWeight != WEIGHT_NORMAL) &&
            (eSearchWeight != WEIGHT_MEDIUM) )
        {
            if( eSearchWeight < WEIGHT_NORMAL )
            {
                if( pData->GetTypeFaces() & FONT_FAMILY_LIGHT )
                    nTestMatch += 1000000;
                if( (eMatchWeight < WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_DONTKNOW) )
                    nTestMatch += 1000000;
            }
            else
            {
                if( pData->GetTypeFaces() & FONT_FAMILY_BOLD )
                    nTestMatch += 1000000;
                if( eMatchWeight > WEIGHT_BOLD )
                    nTestMatch += 1000000;
            }
        }
        else if( ((eMatchWeight != WEIGHT_DONTKNOW) &&
                  (eMatchWeight != WEIGHT_NORMAL) &&
                  (eMatchWeight != WEIGHT_MEDIUM)) ||
                 !(pData->GetTypeFaces() & FONT_FAMILY_NORMAL) )
        {
            nTestMatch -= 1000000;
        }

        // prefer scalable fonts
        if( pData->GetTypeFaces() & FONT_FAMILY_SCALABLE )
            nTestMatch += 10000*4;
        else
            nTestMatch -= 10000*4;

        // test STANDARD+DEFAULT+FULL+NORMAL attributes
        if( nMatchType & IMPL_FONT_ATTR_STANDARD )
            nTestMatch += 10000*2;
        if( nMatchType & IMPL_FONT_ATTR_DEFAULT )
            nTestMatch += 10000;
        if( nMatchType & IMPL_FONT_ATTR_FULL )
            nTestMatch += 10000;
        if( nMatchType & IMPL_FONT_ATTR_NORMAL )
            nTestMatch += 10000;

        // test OTHERSTYLE attribute
        if( ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_OTHERSTYLE) != 0 )
        {
            nTestMatch -= 10000;
        }

        // test ROUNDED attribute
        if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_ROUNDED) )
            nTestMatch += 1000;

        // test TYPEWRITER attribute
        if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_TYPEWRITER) )
            nTestMatch += 1000;

        // test GOTHIC attribute
        if( nSearchType & IMPL_FONT_ATTR_GOTHIC )
        {
            if( nMatchType & IMPL_FONT_ATTR_GOTHIC )
                nTestMatch += 1000*3;
            if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch += 1000*2;
        }

        // test SCHOOLBOOK attribute
        if( nSearchType & IMPL_FONT_ATTR_SCHOOLBOOK )
        {
            if( nMatchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                nTestMatch += 1000*3;
            if( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch += 1000*2;
        }

        // compare with best matching font yet
        if ( nTestMatch > nBestMatch )
        {
            pFoundData  = pData;
            nBestMatch  = nTestMatch;
            nBestType   = nMatchType;
        }
        else if( nTestMatch == nBestMatch )
        {
            // some fonts are more suitable defaults
            if( nMatchType & IMPL_FONT_ATTR_DEFAULT )
            {
                pFoundData  = pData;
                nBestType   = nMatchType;
            }
            else if( (nMatchType & IMPL_FONT_ATTR_STANDARD) &&
                    !(nBestType & IMPL_FONT_ATTR_DEFAULT) )
            {
                 pFoundData  = pData;
                 nBestType   = nMatchType;
            }
        }
    }

    return pFoundData;
}

PhysicalFontFamily* PhysicalFontCollection::FindDefaultFont() const
{
    // try to find one of the default fonts of the
    // UNICODE, SANSSERIF, SERIF or FIXED default font lists
    const utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
    LanguageTag aLanguageTag( OUString( "en"));
    OUString aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SANS_UNICODE );
    PhysicalFontFamily* pFoundData = ImplFindByTokenNames( aFontname );

    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SANS );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SERIF );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_FIXED );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    // now try to find a reasonable non-symbol font

    InitMatchData();

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    for(; it !=  maPhysicalFontFamilies.end(); ++it )
    {
        PhysicalFontFamily* pData = (*it).second;
        if( pData->GetMatchType() & IMPL_FONT_ATTR_SYMBOL )
            continue;

        pFoundData = pData;
        if( pData->GetMatchType() & (IMPL_FONT_ATTR_DEFAULT|IMPL_FONT_ATTR_STANDARD) )
            break;
    }
    if( pFoundData )
        return pFoundData;

    // finding any font is better than finding no font at all
    it = maPhysicalFontFamilies.begin();
    if( it !=  maPhysicalFontFamilies.end() )
        pFoundData = (*it).second;

    return pFoundData;
}

PhysicalFontCollection* PhysicalFontCollection::Clone( bool bScalable, bool bEmbeddable ) const
{
    PhysicalFontCollection* pClonedCollection = new PhysicalFontCollection;
    pClonedCollection->mbMapNames     = mbMapNames;
    pClonedCollection->mpPreMatchHook = mpPreMatchHook;
    pClonedCollection->mpFallbackHook = mpFallbackHook;

    // TODO: clone the config-font attributes too?
    pClonedCollection->mbMatchData    = false;

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    for(; it != maPhysicalFontFamilies.end(); ++it )
    {
        const PhysicalFontFamily* pFontFace = (*it).second;
        pFontFace->UpdateCloneFontList( *pClonedCollection, bScalable, bEmbeddable );
    }

    return pClonedCollection;
}

ImplGetDevFontList* PhysicalFontCollection::GetDevFontList() const
{
    ImplGetDevFontList* pGetDevFontList = new ImplGetDevFontList;

    PhysicalFontFamilies::const_iterator it = maPhysicalFontFamilies.begin();
    for(; it != maPhysicalFontFamilies.end(); ++it )
    {
        const PhysicalFontFamily* pFontFamily = (*it).second;
        pFontFamily->UpdateDevFontList( *pGetDevFontList );
    }

    return pGetDevFontList;
}

ImplGetDevSizeList* PhysicalFontCollection::GetDevSizeList( const OUString& rFontName ) const
{
    ImplGetDevSizeList* pGetDevSizeList = new ImplGetDevSizeList( rFontName );

    PhysicalFontFamily* pFontFamily = FindFontFamily( rFontName );
    if( pFontFamily != NULL )
    {
        std::set<int> rHeights;
        pFontFamily->GetFontHeights( rHeights );

        std::set<int>::const_iterator it = rHeights.begin();
        for(; it != rHeights.begin(); ++it )
            pGetDevSizeList->Add( *it );
    }

    return pGetDevSizeList;
}

PhysicalFontFamily* PhysicalFontCollection::ImplFindByFont( FontSelectPattern& rFSD ) const
{
    // give up if no fonts are available
    if( !Count() )
        return NULL;

    bool bMultiToken = false;
    sal_Int32 nTokenPos = 0;
    OUString& aSearchName = rFSD.maSearchName; // TODO: get rid of reference
    for(;;)
    {
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        aSearchName = rFSD.maTargetName;

#if ENABLE_GRAPHITE
        // Until features are properly supported, they are appended to the
        // font name, so we need to strip them off so the font is found.
        sal_Int32 nFeat = aSearchName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX);
        OUString aOrigName = rFSD.maTargetName;
        OUString aBaseFontName = aSearchName.copy( 0, (nFeat != -1) ? nFeat : aSearchName.getLength() );

        if (nFeat != -1 &&
            -1 != aSearchName.indexOf(grutils::GrFeatureParser::FEAT_ID_VALUE_SEPARATOR, nFeat))
        {
            aSearchName = aBaseFontName;
            rFSD.maTargetName = aBaseFontName;
        }

#endif

        GetEnglishSearchFontName( aSearchName );
        ImplFontSubstitute( aSearchName );
        // #114999# special emboldening for Ricoh fonts
        // TODO: smarter check for special cases by using PreMatch infrastructure?
        if( (rFSD.GetWeight() > WEIGHT_MEDIUM) &&
            aSearchName.startsWithIgnoreAsciiCase( "hg" ) )
        {
            OUString aBoldName;
            if( aSearchName.startsWithIgnoreAsciiCase( "hggothicb" ) )
                aBoldName = "hggothice";
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgpgothicb" ) )
                aBoldName = "hgpgothice";
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgminchol" ) )
                aBoldName = "hgminchob";
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgpminchol" ) )
                aBoldName = "hgpminchob";
            else if( aSearchName.equalsIgnoreAsciiCase( "hgminchob" ) )
                aBoldName = "hgminchoe";
            else if( aSearchName.equalsIgnoreAsciiCase( "hgpminchob" ) )
                aBoldName = "hgpminchoe";

            if( !aBoldName.isEmpty() && ImplFindBySearchName( aBoldName ) )
            {
                // the other font is available => use it
                aSearchName = aBoldName;
                // prevent synthetic emboldening of bold version
                rFSD.SetWeight(WEIGHT_DONTKNOW);
            }
        }

#if ENABLE_GRAPHITE
        // restore the features to make the font selection data unique
        rFSD.maTargetName = aOrigName;
#endif
        // check if the current font name token or its substitute is valid
        PhysicalFontFamily* pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;

        // some systems provide special customization
        // e.g. they suggest "serif" as UI-font, but this name cannot be used directly
        //      because the system wants to map it to another font first, e.g. "Helvetica"
#if ENABLE_GRAPHITE
        // use the target name to search in the prematch hook
        rFSD.maTargetName = aBaseFontName;
#endif

        // Related: fdo#49271 RTF files often contain weird-ass
        // Win 3.1/Win95 style fontnames which attempt to put the
        // charset encoding into the filename
        // http://www.webcenter.ru/~kazarn/eng/fonts_ttf.htm
        OUString sStrippedName = lcl_stripCharSetFromName(rFSD.maTargetName);
        if (sStrippedName != rFSD.maTargetName)
        {
            rFSD.maTargetName = sStrippedName;
            aSearchName = rFSD.maTargetName;
            GetEnglishSearchFontName(aSearchName);
            pFoundData = ImplFindBySearchName(aSearchName);
            if( pFoundData )
                return pFoundData;
        }

        if( mpPreMatchHook )
        {
            if( mpPreMatchHook->FindFontSubstitute( rFSD ) )
                GetEnglishSearchFontName( aSearchName );
        }
#if ENABLE_GRAPHITE
        // the prematch hook uses the target name to search, but we now need
        // to restore the features to make the font selection data unique
        rFSD.maTargetName = aOrigName;
#endif
        pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;

        // break after last font name token was checked unsuccessfully
        if( nTokenPos == -1)
            break;
        bMultiToken = true;
    }

    // if the first font was not available find the next available font in
    // the semicolon separated list of font names. A font is also considered
    // available when there is a matching entry in the Tools->Options->Fonts
    // dialog witho neither ALWAYS nor SCREENONLY flags set and the substitution
    // font is available
    for( nTokenPos = 0; nTokenPos != -1; )
    {
        if( bMultiToken )
        {
            rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
            aSearchName = rFSD.maTargetName;
            GetEnglishSearchFontName( aSearchName );
        }
        else
            nTokenPos = -1;
        if( mpPreMatchHook )
            if( mpPreMatchHook->FindFontSubstitute( rFSD ) )
                GetEnglishSearchFontName( aSearchName );
        ImplFontSubstitute( aSearchName );
        PhysicalFontFamily* pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // if no font with a directly matching name is available use the
    // first font name token and get its attributes to find a replacement
    if ( bMultiToken )
    {
        nTokenPos = 0;
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        aSearchName = rFSD.maTargetName;
        GetEnglishSearchFontName( aSearchName );
    }

    OUString      aSearchShortName;
    OUString      aSearchFamilyName;
    FontWeight  eSearchWeight   = rFSD.GetWeight();
    FontWidth   eSearchWidth    = rFSD.GetWidthType();
    sal_uLong   nSearchType     = 0;
    utl::FontSubstConfiguration::getMapName( aSearchName, aSearchShortName, aSearchFamilyName,
                                             eSearchWeight, eSearchWidth, nSearchType );

    // note: the search name was already translated to english (if possible)
    // use the font's shortened name if needed
    if ( aSearchShortName != aSearchName )
    {
       PhysicalFontFamily* pFoundData = ImplFindBySearchName( aSearchShortName );
       if( pFoundData )
       {
#ifdef UNX
            /* #96738# don't use mincho as an replacement for "MS Mincho" on X11: Mincho is
            a korean bitmap font that is not suitable here. Use the font replacement table,
            that automatically leads to the desired "HG Mincho Light J". Same story for
            MS Gothic, there are thai and korean "Gothic" fonts, so we even prefer Andale */
            static OUString aMS_Mincho( "msmincho" );
            static OUString aMS_Gothic( "msgothic" );
            if ((aSearchName != aMS_Mincho) && (aSearchName != aMS_Gothic))
                // TODO: add heuristic to only throw out the fake ms* fonts
#endif
            {
                return pFoundData;
            }
        }
    }

    // use font fallback
    const utl::FontNameAttr* pFontAttr = NULL;
    if( !aSearchName.isEmpty() )
    {
        // get fallback info using FontSubstConfiguration and
        // the target name, it's shortened name and family name in that order
        const utl::FontSubstConfiguration& rFontSubst = utl::FontSubstConfiguration::get();
        pFontAttr = rFontSubst.getSubstInfo( aSearchName );
        if ( !pFontAttr && (aSearchShortName != aSearchName) )
            pFontAttr = rFontSubst.getSubstInfo( aSearchShortName );
        if ( !pFontAttr && (aSearchFamilyName != aSearchShortName) )
            pFontAttr = rFontSubst.getSubstInfo( aSearchFamilyName );

        // try the font substitutions suggested by the fallback info
        if( pFontAttr )
        {
            PhysicalFontFamily* pFoundData = ImplFindBySubstFontAttr( *pFontAttr );
            if( pFoundData )
                return pFoundData;
        }
    }

    // if a target symbol font is not available use a default symbol font
    if( rFSD.IsSymbolFont() )
    {
        LanguageTag aDefaultLanguageTag( OUString( "en"));
        aSearchName = utl::DefaultFontConfiguration::get().getDefaultFont( aDefaultLanguageTag, DEFAULTFONT_SYMBOL );
        PhysicalFontFamily* pFoundData = ImplFindByTokenNames( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // now try the other font name tokens
    while( nTokenPos != -1 )
    {
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        if( rFSD.maTargetName.isEmpty() )
            continue;

        aSearchName = rFSD.maTargetName;
        GetEnglishSearchFontName( aSearchName );

        OUString      aTempShortName;
        OUString      aTempFamilyName;
        sal_uLong   nTempType   = 0;
        FontWeight  eTempWeight = rFSD.GetWeight();
        FontWidth   eTempWidth  = WIDTH_DONTKNOW;
        utl::FontSubstConfiguration::getMapName( aSearchName, aTempShortName, aTempFamilyName,
                                                 eTempWeight, eTempWidth, nTempType );

        // use a shortend token name if available
        if( aTempShortName != aSearchName )
        {
            PhysicalFontFamily* pFoundData = ImplFindBySearchName( aTempShortName );
            if( pFoundData )
                return pFoundData;
        }

        // use a font name from font fallback list to determine font attributes
        // get fallback info using FontSubstConfiguration and
        // the target name, it's shortened name and family name in that order
        const utl::FontSubstConfiguration& rFontSubst = utl::FontSubstConfiguration::get();
        const utl::FontNameAttr* pTempFontAttr = rFontSubst.getSubstInfo( aSearchName );

        if ( !pTempFontAttr && (aTempShortName != aSearchName) )
            pTempFontAttr = rFontSubst.getSubstInfo( aTempShortName );

        if ( !pTempFontAttr && (aTempFamilyName != aTempShortName) )
            pTempFontAttr = rFontSubst.getSubstInfo( aTempFamilyName );

        // try the font substitutions suggested by the fallback info
        if( pTempFontAttr )
        {
            PhysicalFontFamily* pFoundData = ImplFindBySubstFontAttr( *pTempFontAttr );
            if( pFoundData )
                return pFoundData;
            if( !pFontAttr )
                pFontAttr = pTempFontAttr;
        }
    }

    // if still needed use the alias names of the installed fonts
    if( mbMapNames )
    {
        PhysicalFontFamily* pFoundData = ImplFindByAliasName( rFSD.maTargetName, aSearchShortName );
        if( pFoundData )
            return pFoundData;
    }

    // if still needed use the font request's attributes to find a good match
    if (MsLangId::isSimplifiedChinese(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC;
    else if (MsLangId::isTraditionalChinese(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC;
    else if (MsLangId::isKorean(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR;
    else if (rFSD.meLanguage == LANGUAGE_JAPANESE)
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP;
    else
    {
        nSearchType |= lcl_IsCJKFont( rFSD.GetFamilyName() );
        if( rFSD.IsSymbolFont() )
            nSearchType |= IMPL_FONT_ATTR_SYMBOL;
    }

    PhysicalFontFamily::CalcType( nSearchType, eSearchWeight, eSearchWidth, rFSD.GetFamilyType(), pFontAttr );
    PhysicalFontFamily* pFoundData = ImplFindByAttributes( nSearchType,
        eSearchWeight, eSearchWidth, rFSD.GetSlant(), aSearchFamilyName );

    if( pFoundData )
    {
        // overwrite font selection attributes using info from the typeface flags
        if( (eSearchWeight >= WEIGHT_BOLD) &&
            (eSearchWeight > rFSD.GetWeight()) &&
            (pFoundData->GetTypeFaces() & FONT_FAMILY_BOLD) )
        {
            rFSD.SetWeight( eSearchWeight );
        }
        else if( (eSearchWeight < WEIGHT_NORMAL) &&
                 (eSearchWeight < rFSD.GetWeight()) &&
                 (eSearchWeight != WEIGHT_DONTKNOW) &&
                 (pFoundData->GetTypeFaces() & FONT_FAMILY_LIGHT) )
        {
            rFSD.SetWeight( eSearchWeight );
        }

        if( (nSearchType & IMPL_FONT_ATTR_ITALIC) &&
            ((rFSD.GetSlant() == ITALIC_DONTKNOW) ||
             (rFSD.GetSlant() == ITALIC_NONE)) &&
            (pFoundData->GetTypeFaces() & FONT_FAMILY_ITALIC) )
        {
            rFSD.SetItalic( ITALIC_NORMAL );
        }
    }
    else
    {
        // if still needed fall back to default fonts
        pFoundData = FindDefaultFont();
    }

    return pFoundData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

