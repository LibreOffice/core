/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salatsuifontutils.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <boost/assert.hpp>
#include <vector>
#include <set>

#include "salgdi.h"
#include "saldata.hxx"
#include "vcl/svapp.hxx"
#include "salatsuifontutils.hxx"

// we have to get the font attributes from the name table
// since neither head's macStyle nor OS/2's panose are easily available
// during font enumeration. macStyle bits would be not sufficient anyway
// and SFNT fonts on Mac usually do not contain an OS/2 table.
static void UpdateAttributesFromPSName( const String& aPSName, ImplDevFontAttributes& rDFA )
{
    if( (aPSName.SearchAscii("Regular") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Normal") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Roman") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Medium") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Plain") != STRING_NOTFOUND) )
    {
       rDFA.meWidthType = WIDTH_NORMAL;
       rDFA.meWeight    = WEIGHT_NORMAL;
       rDFA.meItalic    = ITALIC_NONE;
    }

    // heuristics for font weight
    if (aPSName.SearchAscii("ExtraBlack") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_BLACK;
    else if (aPSName.SearchAscii("Black") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_BLACK;
    //else if (aPSName.SearchAscii("Book") != STRING_NOTFOUND)
    //    rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.SearchAscii("SemiBold") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.SearchAscii("UltraBold") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    else if (aPSName.SearchAscii("ExtraBold") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_BLACK;
    else if (aPSName.SearchAscii("Bold") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_BOLD;
    else if (aPSName.SearchAscii("UltraLight") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    else if (aPSName.SearchAscii("Light") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.SearchAscii("Thin") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_THIN;
    else if (aPSName.SearchAscii("-W3") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_LIGHT;
    else if (aPSName.SearchAscii("-W4") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_SEMILIGHT;
    else if (aPSName.SearchAscii("-W5") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_NORMAL;
    else if (aPSName.SearchAscii("-W6") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    else if (aPSName.SearchAscii("-W8") != STRING_NOTFOUND)
        rDFA.meWeight = WEIGHT_BLACK;

    // heuristics for font slant
    if( (aPSName.SearchAscii("Italic") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Cursive") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("BookIt") != STRING_NOTFOUND) )
        rDFA.meItalic = ITALIC_NORMAL;
    if( (aPSName.SearchAscii("Oblique") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Inclined") != STRING_NOTFOUND) )
        rDFA.meItalic = ITALIC_OBLIQUE;

    // heuristics for font width
    if (aPSName.SearchAscii("Condensed") != STRING_NOTFOUND)
        rDFA.meWidthType = WIDTH_CONDENSED;
    else if (aPSName.SearchAscii("Narrow") != STRING_NOTFOUND)
        rDFA.meWidthType = WIDTH_SEMI_CONDENSED;
    else if (aPSName.SearchAscii("Expanded") != STRING_NOTFOUND)
        rDFA.meWidthType = WIDTH_EXPANDED;
    else if (aPSName.SearchAscii("Wide") != STRING_NOTFOUND)
        rDFA.meWidthType = WIDTH_EXPANDED;

    // heuristics for font pitch
    if( (aPSName.SearchAscii("Mono") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Courier") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Monaco") != STRING_NOTFOUND) )
        rDFA.mePitch = PITCH_FIXED;

    // heuristics for font semantic
    if( (aPSName.SearchAscii("Symbol") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("dings") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Dingbats") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Ornaments") != STRING_NOTFOUND)
    ||  (aPSName.SearchAscii("Embellishments") != STRING_NOTFOUND) )
        rDFA.mbSymbolFlag  = true;
}

// -----------------------------------------------------------------------

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

    // get the embeddable + subsettable status
    // TODO: remove test after PS-OpenType subsetting is implemented
    ATSFontRef rATSFontRef = FMGetATSFontRefFromFont( nFontID );
    ByteCount nGlyfLen  = 0;
    OSStatus rc = ATSFontGetTable( rATSFontRef, 0x676c7966/*glyf*/, 0, 0, NULL, &nGlyfLen);
    rDFA.mbSubsettable  = ((rc == noErr) && (nGlyfLen > 0));
    rDFA.mbEmbeddable   = false;
    // TODO: these members are needed only for our X11 platform targets
    rDFA.meAntiAlias    = ANTIALIAS_DONTKNOW;
    rDFA.meEmbeddedBitmap = EMBEDDEDBITMAP_DONTKNOW;

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
            case 0x100: nNameValue += 21;       // Mac Roman
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

        // get the encoded name
        aNameBuffer.reserve( nNameLength+1 ); // extra byte helps for debugging
        rc = ATSUGetIndFontName( nFontID, nNameIndex, nNameLength, &aNameBuffer[0],
           &nNameLength, &eFontNameCode, &eFontNamePlatform, &eFontNameScript, &eFontNameLanguage );
        if( rc != noErr )
            continue;

        // convert to unicode name
        UniString aUtf16Name;
        if( eEncoding == RTL_TEXTENCODING_UNICODE ) // we are just interested in UTF16 encoded names
            aUtf16Name = UniString( (const sal_Unicode*)&aNameBuffer[0], nNameLength/2 );
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

#if 0 // multiple-master fonts are mostly obsolete nowadays
      // if we still want to support them this should probably be done one frame higher
    ItemCount nMaxInstances = 0;
    rc = ATSUCountFontInstances ( nFontID, &nMaxInstances );
    for( ItemCount nInstanceIndex = 0; nInstanceIndex < nMaxInstances; ++nInstanceIndex )
    {
        ItemCount nMaxVariations = 0;
        rc = ATSUGetFontInstance( nFontID, nInstanceIndex, 0, NULL, NULL, &nMaxVariations );
        if( (rc == noErr) && (nMaxVariations > 0) )
        {
            fprintf(stderr,"\tnMaxVariations=%d\n",(int)nMaxVariations);
            typedef ::std::vector<ATSUFontVariationAxis> VariationAxisVector;
            typedef ::std::vector<ATSUFontVariationValue> VariationValueVector;
            VariationAxisVector aVariationAxes( nMaxVariations );
            VariationValueVector aVariationValues( nMaxVariations );
            ItemCount nVariationCount = 0;
            rc = ATSUGetFontInstance ( nFontID, nInstanceIndex, nMaxVariations,
                &aVariationAxes[0], &aVariationValues[0], &nVariationCount );
            fprintf(stderr,"\tnVariationCount=%d\n",(int)nVariationCount);
            for( ItemCount nVariationIndex = 0; nVariationIndex < nMaxVariations; ++nVariationIndex )
            {
                const char* pTag = (const char*)&aVariationAxes[nVariationIndex];
                fprintf(stderr,"\tvariation[%d] \'%c%c%c%c\' is %d\n", (int)nVariationIndex,
                    pTag[3],pTag[2],pTag[1],pTag[0], (int)aVariationValues[nVariationIndex]);
            }
       }
    }
#endif

#if 0 // selecting non-defaulted font features is not enabled yet
    ByteString aFName( rDFA.maName, RTL_TEXTENCODING_UTF8 );
    ByteString aSName( rDFA.maStyleName, RTL_TEXTENCODING_UTF8 );
    ItemCount nMaxFeatures = 0;
    rc = ATSUCountFontFeatureTypes( nFontID, &nMaxFeatures );
    fprintf(stderr,"Font \"%s\" \"%s\" has %d features\n",aFName.GetBuffer(),aSName.GetBuffer(),rc);
    if( (rc == noErr) && (nMaxFeatures > 0) )
    {
        typedef std::vector<ATSUFontFeatureType> FeatureVector;
        FeatureVector aFeatureVector( nMaxFeatures );
        ItemCount nFeatureCount = 0;
        rc = ATSUGetFontFeatureTypes( nFontID, nMaxFeatures, &aFeatureVector[0], &nFeatureCount );
        fprintf(stderr,"nFeatureCount=%d\n",(int)nFeatureCount);
        for( ItemCount nFeatureIndex = 0; nFeatureIndex < nFeatureCount; ++nFeatureIndex )
        {
            ItemCount nMaxSelectors = 0;
            rc = ATSUCountFontFeatureSelectors( nFontID, aFeatureVector[nFeatureIndex], &nMaxSelectors );
            fprintf(stderr,"\tFeature[%d] = %d has %d selectors\n",
               (int)nFeatureIndex, (int)aFeatureVector[nFeatureIndex], (int)nMaxSelectors );
            typedef std::vector<ATSUFontFeatureSelector> SelectorVector;
            SelectorVector aSelectorVector( nMaxSelectors );
            typedef std::vector<MacOSBoolean> BooleanVector;
            BooleanVector aEnabledVector( nMaxSelectors );
            BooleanVector aExclusiveVector( nMaxSelectors );
            ItemCount nSelectorCount = 0;
            rc = ATSUGetFontFeatureSelectors ( nFontID, aFeatureVector[nFeatureIndex], nMaxSelectors,
                &aSelectorVector[0], &aEnabledVector[0], &nSelectorCount, &aExclusiveVector[0]);
            for( ItemCount nSelectorIndex = 0; nSelectorIndex < nSelectorCount; ++nSelectorIndex )
            {
                FontNameCode eFontNameCode;
                rc = ATSUGetFontFeatureNameCode( nFontID, aFeatureVector[nFeatureIndex],
                    aSelectorVector[nSelectorIndex], &eFontNameCode );
                fprintf(stderr,"\t\tselector[%d] n=%d e=%d, x=%d\n",
                    (int)nSelectorIndex, (int)eFontNameCode,
                    aEnabledVector[nSelectorIndex], aExclusiveVector[nSelectorIndex] );
            }
        }
    }
#endif

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

