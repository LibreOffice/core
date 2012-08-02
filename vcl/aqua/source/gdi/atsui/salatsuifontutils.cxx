/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <boost/assert.hpp>
#include <vector>
#include <set>

#include "vcl/svapp.hxx"

#include "aqua/atsui/salgdi.h"
#include "aqua/saldata.hxx"
#include "aqua/atsui/salatsuifontutils.hxx"

// ATSUI is deprecated in 10.6 (or already 10.5?)
#if defined LIBO_WERROR && defined __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40201
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#endif

// we have to get the font attributes from the name table
// since neither head's macStyle nor OS/2's panose are easily available
// during font enumeration. macStyle bits would be not sufficient anyway
// and SFNT fonts on Mac usually do not contain an OS/2 table.
static void UpdateAttributesFromPSName( const String& rPSName, ImplDevFontAttributes& rDFA )
{
    rtl::OString aPSName( rtl::OUStringToOString( rPSName, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() );

    // TODO: use a multi-string ignore-case matcher once it becomes available
    if( (aPSName.indexOf("regular") != -1)
    ||  (aPSName.indexOf("normal") != -1)
    ||  (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("medium") != -1)
    ||  (aPSName.indexOf("plain") != -1)
    ||  (aPSName.indexOf("standard") != -1)
    ||  (aPSName.indexOf("std") != -1) )
    {
       rDFA.meWidthType = WIDTH_NORMAL;
       rDFA.meWeight    = WEIGHT_NORMAL;
       rDFA.meItalic    = ITALIC_NONE;
    }

    // heuristics for font weight
    if (aPSName.indexOf("extrablack") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    else if (aPSName.indexOf("black") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    //else if (aPSName.indexOf("book") != -1)
    //    rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if( (aPSName.indexOf("semibold") != -1)
    ||  (aPSName.indexOf("smbd") != -1))
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.indexOf("ultrabold") != -1)
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    else if (aPSName.indexOf("extrabold") != -1)
        rDFA.meWeight = WEIGHT_BLACK;
    else if( (aPSName.indexOf("bold") != -1)
    ||  (aPSName.indexOf("-bd") != -1))
        rDFA.meWeight = WEIGHT_BOLD;
    else if (aPSName.indexOf("extralight") != -1)
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    else if (aPSName.indexOf("ultralight") != -1)
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    else if (aPSName.indexOf("light") != -1)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.indexOf("thin") != -1)
        rDFA.meWeight = WEIGHT_THIN;
    else if (aPSName.indexOf("-w3") != -1)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.indexOf("-w4") != -1)
        rDFA.meWeight = WEIGHT_SEMILIGHT;
    else if (aPSName.indexOf("-w5") != -1)
        rDFA.meWeight = WEIGHT_NORMAL;
    else if (aPSName.indexOf("-w6") != -1)
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.indexOf("-w7") != -1)
        rDFA.meWeight = WEIGHT_BOLD;
    else if (aPSName.indexOf("-w8") != -1)
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    else if (aPSName.indexOf("-w9") != -1)
        rDFA.meWeight = WEIGHT_BLACK;

    // heuristics for font slant
    if( (aPSName.indexOf("italic") != -1)
    ||  (aPSName.indexOf(" ital") != -1)
    ||  (aPSName.indexOf("cursive") != -1)
    ||  (aPSName.indexOf("-it") != -1)
    ||  (aPSName.indexOf("lightit") != -1)
    ||  (aPSName.indexOf("mediumit") != -1)
    ||  (aPSName.indexOf("boldit") != -1)
    ||  (aPSName.indexOf("cnit") != -1)
    ||  (aPSName.indexOf("bdcn") != -1)
    ||  (aPSName.indexOf("bdit") != -1)
    ||  (aPSName.indexOf("condit") != -1)
    ||  (aPSName.indexOf("bookit") != -1)
    ||  (aPSName.indexOf("blackit") != -1) )
        rDFA.meItalic = ITALIC_NORMAL;
    if( (aPSName.indexOf("oblique") != -1)
    ||  (aPSName.indexOf("inclined") != -1)
    ||  (aPSName.indexOf("slanted") != -1) )
        rDFA.meItalic = ITALIC_OBLIQUE;

    // heuristics for font width
    if( (aPSName.indexOf("condensed") != -1)
    ||  (aPSName.indexOf("-cond") != -1)
    ||  (aPSName.indexOf("boldcond") != -1)
    ||  (aPSName.indexOf("boldcn") != -1)
    ||  (aPSName.indexOf("cnit") != -1) )
        rDFA.meWidthType = WIDTH_CONDENSED;
    else if (aPSName.indexOf("narrow") != -1)
        rDFA.meWidthType = WIDTH_SEMI_CONDENSED;
    else if (aPSName.indexOf("expanded") != -1)
        rDFA.meWidthType = WIDTH_EXPANDED;
    else if (aPSName.indexOf("wide") != -1)
        rDFA.meWidthType = WIDTH_EXPANDED;

    // heuristics for font pitch
    if( (aPSName.indexOf("mono") != -1)
    ||  (aPSName.indexOf("courier") != -1)
    ||  (aPSName.indexOf("monaco") != -1)
    ||  (aPSName.indexOf("typewriter") != -1) )
        rDFA.mePitch = PITCH_FIXED;

    // heuristics for font family type
    if( (aPSName.indexOf("script") != -1)
    ||  (aPSName.indexOf("chancery") != -1)
    ||  (aPSName.indexOf("zapfino") != -1))
        rDFA.meFamily = FAMILY_SCRIPT;
    else if( (aPSName.indexOf("comic") != -1)
    ||  (aPSName.indexOf("outline") != -1)
    ||  (aPSName.indexOf("pinpoint") != -1) )
        rDFA.meFamily = FAMILY_DECORATIVE;
    else if( (aPSName.indexOf("sans") != -1)
    ||  (aPSName.indexOf("arial") != -1) )
        rDFA.meFamily = FAMILY_SWISS;
    else if( (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("times") != -1) )
        rDFA.meFamily = FAMILY_ROMAN;

    // heuristics for codepoint semantic
    if( (aPSName.indexOf("symbol") != -1)
    ||  (aPSName.indexOf("dings") != -1)
    ||  (aPSName.indexOf("dingbats") != -1)
    ||  (aPSName.indexOf("ornaments") != -1)
    ||  (aPSName.indexOf("embellishments") != -1) )
        rDFA.mbSymbolFlag  = true;

   // #i100020# special heuristic for names with single-char styles
   // NOTE: we are checking name that hasn't been lower-cased
   if( rPSName.Len() > 3 )
   {
        int i = rPSName.Len();
        sal_Unicode c = rPSName.GetChar( --i );
        if( c == 'C' ) { // "capitals"
            rDFA.meFamily = FAMILY_DECORATIVE;
            c = rPSName.GetChar( --i );
        }
        if( c == 'O' ) { // CFF-based OpenType
            c = rPSName.GetChar( --i );
        }
        if( c == 'I' ) { // "italic"
            rDFA.meItalic = ITALIC_NORMAL;
            c = rPSName.GetChar( --i );
        }
        if( c == 'B' )   // "bold"
            rDFA.meWeight = WEIGHT_BOLD;
        if( c == 'C' )   // "capitals"
            rDFA.meFamily = FAMILY_DECORATIVE;
        // TODO: check that all single-char styles have been resolved?
    }
}

// -----------------------------------------------------------------------

#if MACOSX_SDK_VERSION >= 1070
extern "C" {
extern ATSFontRef FMGetATSFontRefFromFont(FMFont iFont);
}
#endif

static bool GetDevFontAttributes( ATSUFontID nFontID, ImplDevFontAttributes& rDFA )
{
    // all ATSU fonts are device fonts that can be directly rotated
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

    // reset the attributes
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.mePitch      = PITCH_VARIABLE;
    rDFA.meWidthType  = WIDTH_NORMAL;
    rDFA.meWeight     = WEIGHT_NORMAL;
    rDFA.meItalic     = ITALIC_NONE;
    rDFA.mbSymbolFlag = false;

    // ignore bitmap fonts
    ATSFontRef rATSFontRef = FMGetATSFontRefFromFont( nFontID );
    ByteCount nHeadLen = 0;
    OSStatus rc = ATSFontGetTable( rATSFontRef, 0x68656164/*head*/, 0, 0, NULL, &nHeadLen );
    if( (rc != noErr) || (nHeadLen <= 0) )
        return false;

    // all scalable fonts on this platform are subsettable
    rDFA.mbSubsettable  = true;
    rDFA.mbEmbeddable   = false;

    // prepare iterating over all name strings of the font
    ItemCount nFontNameCount = 0;
    rc = ATSUCountFontNames( nFontID, &nFontNameCount );
    if( rc != noErr )
        return false;
    int nBestNameValue = 0;
    int nBestStyleValue = 0;
    FontLanguageCode eBestLangCode = 0;
    const FontLanguageCode eUILangCode = Application::GetSettings().GetUILanguage();
    typedef std::vector<char> NameBuffer;
    NameBuffer aNameBuffer( 256 );

    // iterate over all available name strings of the font
    for( ItemCount nNameIndex = 0; nNameIndex < nFontNameCount; ++nNameIndex )
    {
        ByteCount nNameLength = 0;

        FontNameCode     eFontNameCode;
        FontPlatformCode eFontNamePlatform;
        FontScriptCode   eFontNameScript;
        FontLanguageCode eFontNameLanguage;
        rc = ATSUGetIndFontName( nFontID, nNameIndex, 0, NULL,
            &nNameLength, &eFontNameCode, &eFontNamePlatform, &eFontNameScript, &eFontNameLanguage );
        if( rc != noErr )
            continue;

        // ignore non-interesting name entries
        if( (eFontNameCode != kFontFamilyName)
        &&  (eFontNameCode != kFontStyleName)
        &&  (eFontNameCode != kFontPostscriptName) )
            continue;

        // heuristic to find the most common font name
        // prefering default language names or even better the names matching to the UI language
        int nNameValue = (eFontNameLanguage==eUILangCode) ? 0 : ((eFontNameLanguage==0) ? -10 : -20);
        rtl_TextEncoding eEncoding = RTL_TEXTENCODING_UNICODE;
        const int nPlatformEncoding = ((int)eFontNamePlatform << 8) + (int)eFontNameScript;
        switch( nPlatformEncoding )
        {
            case 0x000: nNameValue += 23; break;    // Unicode 1.0
            case 0x001: nNameValue += 24; break;    // Unicode 1.1
            case 0x002: nNameValue += 25; break;    // iso10646_1993
            case 0x003: nNameValue += 26; break;    // UCS-2
            case 0x301: nNameValue += 27; break;    // Win UCS-2
            case 0x004:                             // UCS-4
            case 0x30A: nNameValue += 0;            // Win-UCS-4
                        eEncoding = RTL_TEXTENCODING_UCS4;
                        break;
            case 0x100: nNameValue += 21;           // Mac Roman
                        eEncoding = RTL_TEXTENCODING_APPLE_ROMAN;
                        break;
            case 0x300: nNameValue =  0;            // Win Symbol encoded name!
                        rDFA.mbSymbolFlag = true;   // (often seen for symbol fonts)
                        break;
            default:    nNameValue = 0;             // ignore other encodings
            break;
        }

        // ignore name entries with no useful encoding
        if( nNameValue <= 0 )
            continue;
        if( nNameLength >= aNameBuffer.size() )
            continue;

        // get the encoded name
        aNameBuffer.reserve( nNameLength+1 ); // extra byte helps for debugging
        rc = ATSUGetIndFontName( nFontID, nNameIndex, nNameLength, &aNameBuffer[0],
           &nNameLength, &eFontNameCode, &eFontNamePlatform, &eFontNameScript, &eFontNameLanguage );
        if( rc != noErr )
            continue;

        // convert to unicode name
        UniString aUtf16Name;
        if( eEncoding == RTL_TEXTENCODING_UNICODE ) // we are just interested in UTF16 encoded names
            aUtf16Name = rtl::OUString( (const sal_Unicode*)&aNameBuffer[0], nNameLength/2 );
        else if( eEncoding == RTL_TEXTENCODING_UCS4 )
            aUtf16Name = UniString(); // TODO
        else // assume the non-unicode encoded names are byte encoded
            aUtf16Name = UniString( &aNameBuffer[0], nNameLength, eEncoding );

        // ignore empty strings
        if( aUtf16Name.Len() <= 0 )
            continue;

        // handle the name depending on its namecode
        switch( eFontNameCode )
        {
            case kFontFamilyName:
                // ignore font names starting with '.'
                if( aUtf16Name.GetChar(0) == '.' )
                    nNameValue = 0;
                else if( rDFA.maName.Len() )
                {
                    // even if a family name is not the one we are looking for
                    // it is still useful as a font name alternative
                    if( rDFA.maMapNames.Len() )
                        rDFA.maMapNames += ';';
                    rDFA.maMapNames += (nBestNameValue < nNameValue) ? rDFA.maName : aUtf16Name;
                }
                if( nBestNameValue < nNameValue )
                {
                    // get the best family name
                    nBestNameValue = nNameValue;
                    eBestLangCode = eFontNameLanguage;
                    rDFA.maName = aUtf16Name;
                }
                break;
            case kFontStyleName:
                // get a style name matching to the family name
                if( nBestStyleValue < nNameValue )
                {
                    nBestStyleValue = nNameValue;
                    rDFA.maStyleName = aUtf16Name;
                }
                break;
            case kFontPostscriptName:
                // use the postscript name to get some useful info
                UpdateAttributesFromPSName( aUtf16Name, rDFA );
                break;
            default:
                // TODO: use other name entries too?
                break;
        }
    }

    bool bRet = (rDFA.maName.Len() > 0);
    return bRet;
}

// =======================================================================

SystemFontList::SystemFontList()
{
    // count available system fonts
    ItemCount nATSUICompatibleFontsAvailable = 0;
    if( ATSUFontCount(&nATSUICompatibleFontsAvailable) != noErr )
        return;
    if( nATSUICompatibleFontsAvailable <= 0 )
       return;

    // enumerate available system fonts
    typedef std::vector<ATSUFontID> AtsFontIDVector;
    AtsFontIDVector aFontIDVector( nATSUICompatibleFontsAvailable );
    ItemCount nFontItemsCount = 0;
    if( ATSUGetFontIDs( &aFontIDVector[0], aFontIDVector.capacity(), &nFontItemsCount ) != noErr )
       return;

    BOOST_ASSERT(nATSUICompatibleFontsAvailable == nFontItemsCount && "Strange I would expect them to be equal");

    // prepare use of the available fonts
    AtsFontIDVector::const_iterator it = aFontIDVector.begin();
    for(; it != aFontIDVector.end(); ++it )
    {
    const ATSUFontID nFontID = *it;
        ImplDevFontAttributes aDevFontAttr;
        if( !GetDevFontAttributes( nFontID, aDevFontAttr ) )
            continue;
        ImplMacFontData* pFontData = new ImplMacFontData(  aDevFontAttr, nFontID );
        maFontContainer[ nFontID ] = pFontData;
    }

    InitGlyphFallbacks();
}

// -----------------------------------------------------------------------

SystemFontList::~SystemFontList()
{
    MacFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        delete (*it).second;
    maFontContainer.clear();

    ATSUDisposeFontFallbacks( maFontFallbacks );
}

// -----------------------------------------------------------------------

void SystemFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
    MacFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
        rFontList.Add( (*it).second->Clone() );
}

// -----------------------------------------------------------------------

// not all fonts are suitable for glyph fallback => sort them
struct GfbCompare{ bool operator()(const ImplMacFontData*, const ImplMacFontData*); };

inline bool GfbCompare::operator()( const ImplMacFontData* pA, const ImplMacFontData* pB )
{
    // use symbol fonts only as last resort
    bool bPreferA = !pA->IsSymbolFont();
    bool bPreferB = !pB->IsSymbolFont();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer scalable fonts
    bPreferA = pA->IsScalable();
    bPreferB = pB->IsScalable();
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer non-slanted fonts
    bPreferA = (pA->GetSlant() == ITALIC_NONE);
    bPreferB = (pB->GetSlant() == ITALIC_NONE);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal weight fonts
    bPreferA = (pA->GetWeight() == WEIGHT_NORMAL);
    bPreferB = (pB->GetWeight() == WEIGHT_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    // prefer normal width fonts
    bPreferA = (pA->GetWidthType() == WIDTH_NORMAL);
    bPreferB = (pB->GetWidthType() == WIDTH_NORMAL);
    if( bPreferA != bPreferB )
        return bPreferA;
    return false;
}

void SystemFontList::InitGlyphFallbacks()
{
    // sort fonts for "glyph fallback"
    typedef std::multiset<const ImplMacFontData*,GfbCompare> FallbackSet;
    FallbackSet aFallbackSet;
    MacFontContainer::const_iterator it = maFontContainer.begin();
    for(; it != maFontContainer.end(); ++it )
    {
    const ImplMacFontData* pIFD = (*it).second;
    // TODO: subsettable/embeddable glyph fallback only for PDF export?
        if( pIFD->IsSubsettable() || pIFD->IsEmbeddable() )
        aFallbackSet.insert( pIFD );
    }

    // tell ATSU about font preferences for "glyph fallback"
    typedef std::vector<ATSUFontID> AtsFontIDVector;
    AtsFontIDVector aFallbackVector;
    aFallbackVector.reserve( maFontContainer.size() );
    FallbackSet::const_iterator itFData = aFallbackSet.begin();
    for(; itFData != aFallbackSet.end(); ++itFData )
    {
        const ImplMacFontData* pFontData = (*itFData);
        ATSUFontID nFontID = (ATSUFontID)pFontData->GetFontId();
        aFallbackVector.push_back( nFontID );
    }

    ATSUCreateFontFallbacks( &maFontFallbacks );
    ATSUSetObjFontFallbacks( maFontFallbacks,
        aFallbackVector.size(), &aFallbackVector[0], kATSUSequentialFallbacksPreferred );
}

// -----------------------------------------------------------------------

ImplMacFontData* SystemFontList::GetFontDataFromId( ATSUFontID nFontId ) const
{
    MacFontContainer::const_iterator it = maFontContainer.find( nFontId );
    if( it == maFontContainer.end() )
    return NULL;
    return (*it).second;
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
