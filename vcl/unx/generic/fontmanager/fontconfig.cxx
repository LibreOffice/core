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

#include <iostream>
#include <memory>
#include <string_view>

#include <o3tl/lru_map.hxx>
#include <unx/fontmanager.hxx>
#include <unx/helper.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <font/FontSelectPattern.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/unicode.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unicode/uchar.h>
#include <unicode/uscript.h>
#include <officecfg/Office/Common.hxx>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <config_fonts.h>

#include <fontconfig/fontconfig.h>

#include <cstdio>

#include <unotools/configmgr.hxx>
#include <unotools/syslocaleoptions.hxx>

#include <osl/process.h>

#include <o3tl/hash_combine.hxx>
#include <utility>
#include <algorithm>

using namespace psp;

namespace
{

struct FontOptionsKey
{
    OUString m_sFamilyName;
    int m_nFontSize;
    FontItalic m_eItalic;
    FontWeight m_eWeight;
    FontWidth m_eWidth;
    FontPitch m_ePitch;

    bool operator==(const FontOptionsKey& rOther) const
    {
        return m_sFamilyName == rOther.m_sFamilyName &&
               m_nFontSize == rOther.m_nFontSize &&
               m_eItalic == rOther.m_eItalic &&
               m_eWeight == rOther.m_eWeight &&
               m_eWidth == rOther.m_eWidth &&
               m_ePitch == rOther.m_ePitch;
    }
};

}

namespace std
{

template <> struct hash<FontOptionsKey>
{
    std::size_t operator()(const FontOptionsKey& k) const noexcept
    {
        std::size_t seed = k.m_sFamilyName.hashCode();
        o3tl::hash_combine(seed, k.m_nFontSize);
        o3tl::hash_combine(seed, k.m_eItalic);
        o3tl::hash_combine(seed, k.m_eWeight);
        o3tl::hash_combine(seed, k.m_eWidth);
        o3tl::hash_combine(seed, k.m_ePitch);
        return seed;
    }
};

} // end std namespace

namespace
{

struct FcPatternDeleter
{
    void operator()(FcPattern* pPattern) const
    {
        FcPatternDestroy(pPattern);
    }
};

typedef std::unique_ptr<FcPattern, FcPatternDeleter> FcPatternUniquePtr;

class CachedFontConfigFontOptions
{
private:
    o3tl::lru_map<FontOptionsKey, FcPatternUniquePtr> lru_options_cache;

public:
    CachedFontConfigFontOptions()
        : lru_options_cache(10) // arbitrary cache size of 10
    {
    }

    std::unique_ptr<FontConfigFontOptions> lookup(const FontOptionsKey& rKey)
    {
        auto it = lru_options_cache.find(rKey);
        if (it != lru_options_cache.end())
            return std::make_unique<FontConfigFontOptions>(FcPatternDuplicate(it->second.get()));
        return nullptr;
    }

    void cache(const FontOptionsKey& rKey, const FcPattern* pPattern)
    {
        lru_options_cache.insert(std::make_pair(rKey, FcPatternUniquePtr(FcPatternDuplicate(pPattern))));
    }

};

typedef std::pair<FcChar8*, FcChar8*> lang_and_element;

class FontCfgWrapper
{
    FcFontSet* m_pFontSet;
    bool m_bRestrictFontSetToApplicationFonts;

    FontCfgWrapper();
    ~FontCfgWrapper();

public:
    static FontCfgWrapper& get();
    static void release();

    void addFontSet( FcSetName );

    FcFontSet* getFontSet();
    void replaceFontSet(FcFontSet* pFilteredFontSet);

    void clear();

    bool isRestrictingFontSetForTesting() const
    {
        return m_bRestrictFontSetToApplicationFonts;
    }

public:
    FcResult LocalizedElementFromPattern(FcPattern const * pPattern, FcChar8 **family,
                                         const char *elementtype, const char *elementlangtype);
//to-do, make private and add some cleaner accessor methods
    std::unordered_map< OString, OString > m_aFontNameToLocalized;
    std::unordered_map< OString, OString > m_aLocalizedToCanonical;
    CachedFontConfigFontOptions m_aCachedFontOptions;
private:
    void cacheLocalizedFontNames(const FcChar8 *origfontname, const FcChar8 *bestfontname, const std::vector< lang_and_element > &lang_and_elements);

    std::unique_ptr<LanguageTag> m_pLanguageTag;
};

}

FontCfgWrapper::FontCfgWrapper()
    : m_pFontSet(nullptr)
    , m_bRestrictFontSetToApplicationFonts(false)
{
    FcInit();
}

#ifndef FC_FONT_WRAPPER
#define FC_FONT_WRAPPER "fontwrapper"
#endif

void FontCfgWrapper::addFontSet( FcSetName eSetName )
{
    // Add only acceptable fonts to our config, for future fontconfig use.
    FcFontSet* pOrig = FcConfigGetFonts( FcConfigGetCurrent(), eSetName );
    if( !pOrig )
        return;

    // filter the font sets to remove obsolete faces
    for( int i = 0; i < pOrig->nfont; ++i )
    {
        FcPattern* pPattern = pOrig->fonts[i];
        // #i115131# ignore non-scalable fonts
        // Scalable fonts are usually outline fonts, but some bitmaps fonts
        // (like Noto Color Emoji) are also scalable.
        FcBool bScalable = FcFalse;
        FcResult eScalableRes = FcPatternGetBool(pPattern, FC_SCALABLE, 0, &bScalable);
        if ((eScalableRes != FcResultMatch) || (bScalable == FcFalse))
            continue;

        // Ignore Type 1 fonts, too.
        FcChar8* pFormat = nullptr;
        FcResult eFormatRes = FcPatternGetString(pPattern, FC_FONTFORMAT, 0, &pFormat);
        if ((eFormatRes == FcResultMatch) && (strcmp(reinterpret_cast<char*>(pFormat), "Type 1") == 0))
            continue;

        // Ignore any other non-SFNT wrapper format, including WOFF and WOFF2, too.
        FcChar8* pWrapper = nullptr;
        FcResult eWrapperRes = FcPatternGetString(pPattern, FC_FONT_WRAPPER, 0, &pWrapper);
        if ((eWrapperRes == FcResultMatch) && (strcmp(reinterpret_cast<char*>(pWrapper), "SFNT") != 0))
            continue;

        FcPatternReference( pPattern );
        FcFontSetAdd( m_pFontSet, pPattern );
    }

    // TODO?: FcFontSetDestroy( pOrig );
}

namespace
{
    int compareFontNames(const FcPattern *a, const FcPattern *b)
    {
        FcChar8 *pNameA=nullptr, *pNameB=nullptr;

        bool bHaveA = FcPatternGetString(a, FC_FAMILY, 0, &pNameA) == FcResultMatch;
        bool bHaveB = FcPatternGetString(b, FC_FAMILY, 0, &pNameB) == FcResultMatch;

        if (bHaveA && bHaveB)
            return strcmp(reinterpret_cast<const char*>(pNameA), reinterpret_cast<const char*>(pNameB));

        return int(bHaveA) - int(bHaveB);
    }

    //Sort fonts so that fonts with the same family name are side-by-side, with
    //those with higher version numbers first
    class SortFont
    {
    public:
        bool operator()(const FcPattern *a, const FcPattern *b)
        {
            int comp = compareFontNames(a, b);
            if (comp != 0)
                return comp < 0;

            int nVersionA=0, nVersionB=0;

            bool bHaveA = FcPatternGetInteger(a, FC_FONTVERSION, 0, &nVersionA) == FcResultMatch;
            bool bHaveB = FcPatternGetInteger(b, FC_FONTVERSION, 0, &nVersionB) == FcResultMatch;

            if (bHaveA && bHaveB)
                return nVersionA > nVersionB;

            return bHaveA > bHaveB;
        }
    };

    //See fdo#30729 for where an old opensymbol installed system-wide can
    //clobber the new opensymbol installed locally

    //See if this font is a duplicate with equal attributes which has already been
    //inserted, or if it an older version of an inserted fonts. Depends on FcFontSet
    //on being sorted with SortFont
    bool isPreviouslyDuplicateOrObsoleted(FcFontSet const *pFSet, int i)
    {
        const FcPattern *a = pFSet->fonts[i];

        FcPattern* pTestPatternA = FcPatternDuplicate(a);
        FcPatternDel(pTestPatternA, FC_FILE);
        FcPatternDel(pTestPatternA, FC_CHARSET);
        FcPatternDel(pTestPatternA, FC_CAPABILITY);
        FcPatternDel(pTestPatternA, FC_FONTVERSION);
        FcPatternDel(pTestPatternA, FC_LANG);

        bool bIsDup(false);

        // fdo#66715: loop for case of several font files for same font
        for (int j = i - 1; 0 <= j && !bIsDup; --j)
        {
            const FcPattern *b = pFSet->fonts[j];

            if (compareFontNames(a, b) != 0)
                break;

            FcPattern* pTestPatternB = FcPatternDuplicate(b);
            FcPatternDel(pTestPatternB, FC_FILE);
            FcPatternDel(pTestPatternB, FC_CHARSET);
            FcPatternDel(pTestPatternB, FC_CAPABILITY);
            FcPatternDel(pTestPatternB, FC_FONTVERSION);
            FcPatternDel(pTestPatternB, FC_LANG);

            bIsDup = FcPatternEqual(pTestPatternA, pTestPatternB);

            FcPatternDestroy(pTestPatternB);
        }

        FcPatternDestroy(pTestPatternA);

        return bIsDup;
    }
}

FcFontSet* FontCfgWrapper::getFontSet()
{
    if( !m_pFontSet )
    {
        m_pFontSet = FcFontSetCreate();
#if HAVE_MORE_FONTS
        m_bRestrictFontSetToApplicationFonts = [] {
            return getenv("SAL_NON_APPLICATION_FONT_USE") != nullptr;
        }();
#endif
        // Add the application fonts before the system fonts.
        // tdf#157939 We will remove duplicate fonts, where the duplicate is
        // the one with a smaller version number. If the same version font is
        // available system-wide or bundled with our application, then we
        // prefer via stable-sort the first one we see. Load application fonts
        // first to prefer the one we bundle in the application in that case.
        addFontSet( FcSetApplication );
        if (!m_bRestrictFontSetToApplicationFonts)
            addFontSet( FcSetSystem );

        std::stable_sort(m_pFontSet->fonts,m_pFontSet->fonts+m_pFontSet->nfont,SortFont());
    }

    return m_pFontSet;
}

void FontCfgWrapper::replaceFontSet(FcFontSet* pFilteredFontSet)
{
    if (m_pFontSet)
        FcFontSetDestroy(m_pFontSet);
    m_pFontSet = pFilteredFontSet;
}

FontCfgWrapper::~FontCfgWrapper()
{
    clear();
    //To-Do: get gtk vclplug smoketest to pass
    //FcFini();
}

static FontCfgWrapper* pOneInstance = nullptr;

FontCfgWrapper& FontCfgWrapper::get()
{
    if( ! pOneInstance )
        pOneInstance = new FontCfgWrapper();
    return *pOneInstance;
}

void FontCfgWrapper::release()
{
    if( pOneInstance )
    {
        delete pOneInstance;
        pOneInstance = nullptr;
    }
}

namespace
{
    FcChar8* bestname(const std::vector<lang_and_element> &elements, const LanguageTag & rLangTag);

    FcChar8* bestname(const std::vector<lang_and_element> &elements, const LanguageTag & rLangTag)
    {
        FcChar8* candidate = elements.begin()->second;
        /* FIXME-BCP47: once fontconfig supports language tags this
         * language-territory stuff needs to be changed! */
        SAL_INFO_IF( !rLangTag.isIsoLocale(), "vcl.fonts", "localizedsorter::bestname - not an ISO locale");
        OString sLangMatch(OUStringToOString(rLangTag.getLanguage().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8));
        OString sFullMatch = sLangMatch +
            "-" +
            OUStringToOString(rLangTag.getCountry().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8);

        bool alreadyclosematch = false;
        bool found_fallback_englishname = false;
        for (auto const& element : elements)
        {
            const char *pLang = reinterpret_cast<const char*>(element.first);
            if( sFullMatch == pLang)
            {
                // both language and country match
                candidate = element.second;
                break;
            }
            else if( alreadyclosematch )
            {
                // current candidate matches lang of lang-TERRITORY
                // override candidate only if there is a full match
                continue;
            }
            else if( sLangMatch == pLang)
            {
                // just the language matches
                candidate = element.second;
                alreadyclosematch = true;
            }
            else if( found_fallback_englishname )
            {
                // already found an english fallback, don't override candidate
                // unless there is a better language match
                continue;
            }
            else if( rtl_str_compare( pLang, "en") == 0)
            {
                // select a fallback candidate of the first english element
                // name
                candidate = element.second;
                found_fallback_englishname = true;
            }
        }
        return candidate;
    }
}

//Set up maps to quickly map between a fonts best UI name and all the rest of its names, and vice versa
void FontCfgWrapper::cacheLocalizedFontNames(const FcChar8 *origfontname, const FcChar8 *bestfontname,
    const std::vector< lang_and_element > &lang_and_elements)
{
    for (auto const& element : lang_and_elements)
    {
        const char *candidate = reinterpret_cast<const char*>(element.second);
        if (rtl_str_compare(candidate, reinterpret_cast<const char*>(bestfontname)) != 0)
            m_aFontNameToLocalized[OString(candidate)] = OString(reinterpret_cast<const char*>(bestfontname));
    }
    if (rtl_str_compare(reinterpret_cast<const char*>(origfontname), reinterpret_cast<const char*>(bestfontname)) != 0)
        m_aLocalizedToCanonical[OString(reinterpret_cast<const char*>(bestfontname))] = OString(reinterpret_cast<const char*>(origfontname));
}

FcResult FontCfgWrapper::LocalizedElementFromPattern(FcPattern const * pPattern, FcChar8 **element,
                                                     const char *elementtype, const char *elementlangtype)
{                                                /* e. g.:      ^ FC_FAMILY              ^ FC_FAMILYLANG */
    FcChar8 *origelement;
    FcResult eElementRes = FcPatternGetString( pPattern, elementtype, 0, &origelement );
    *element = origelement;

    if( eElementRes == FcResultMatch)
    {
        FcChar8* elementlang = nullptr;
        if (FcPatternGetString( pPattern, elementlangtype, 0, &elementlang ) == FcResultMatch)
        {
            std::vector< lang_and_element > lang_and_elements;
            lang_and_elements.emplace_back(elementlang, *element);
            int k = 1;
            while (true)
            {
                if (FcPatternGetString( pPattern, elementlangtype, k, &elementlang ) != FcResultMatch)
                    break;
                if (FcPatternGetString( pPattern, elementtype, k, element ) != FcResultMatch)
                    break;
                lang_and_elements.emplace_back(elementlang, *element);
                ++k;
            }

            if (!m_pLanguageTag)
                m_pLanguageTag.reset(new LanguageTag(SvtSysLocaleOptions().GetRealUILanguageTag()));

            // FontConfig orders Typographic Family/Subfamily before old
            // R/B/I/BI-compatible ones, but we want the later, so reverse the
            // names to match them first.
            std::reverse(lang_and_elements.begin(), lang_and_elements.end());

            *element = bestname(lang_and_elements, *m_pLanguageTag);

            //if this element is a fontname, map the other names to this best-name
            if (rtl_str_compare(elementtype, FC_FAMILY) == 0)
                cacheLocalizedFontNames(origelement, *element, lang_and_elements);
        }
    }

    return eElementRes;
}

void FontCfgWrapper::clear()
{
    m_aFontNameToLocalized.clear();
    m_aLocalizedToCanonical.clear();
    if( m_pFontSet )
    {
        FcFontSetDestroy( m_pFontSet );
        m_pFontSet = nullptr;
    }
    m_pLanguageTag.reset();
}

/*
 * PrintFontManager::initFontconfig
 */
void PrintFontManager::initFontconfig()
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    rWrapper.clear();
}

namespace
{
    FontWeight convertWeight(int weight)
    {
        // set weight
        if( weight <= FC_WEIGHT_THIN )
            return WEIGHT_THIN;
        else if( weight <= FC_WEIGHT_ULTRALIGHT )
            return WEIGHT_ULTRALIGHT;
        else if( weight <= FC_WEIGHT_LIGHT )
            return WEIGHT_LIGHT;
        else if( weight <= FC_WEIGHT_BOOK )
            return WEIGHT_SEMILIGHT;
        else if( weight <= FC_WEIGHT_NORMAL )
            return WEIGHT_NORMAL;
        else if( weight <= FC_WEIGHT_MEDIUM )
            return WEIGHT_MEDIUM;
        else if( weight <= FC_WEIGHT_SEMIBOLD )
            return WEIGHT_SEMIBOLD;
        else if( weight <= FC_WEIGHT_BOLD )
            return WEIGHT_BOLD;
        else if( weight <= FC_WEIGHT_ULTRABOLD )
            return WEIGHT_ULTRABOLD;
        return WEIGHT_BLACK;
    }

    FontItalic convertSlant(int slant)
    {
        // set italic
        if( slant == FC_SLANT_ITALIC )
            return ITALIC_NORMAL;
        else if( slant == FC_SLANT_OBLIQUE )
            return ITALIC_OBLIQUE;
        return ITALIC_NONE;
    }

    FontPitch convertSpacing(int spacing)
    {
        // set pitch
        if( spacing == FC_MONO || spacing == FC_CHARCELL )
            return PITCH_FIXED;
        return PITCH_VARIABLE;
    }

    // translation: fontconfig enum -> vcl enum
    FontWidth convertWidth(int width)
    {
        if (width == FC_WIDTH_ULTRACONDENSED)
            return WIDTH_ULTRA_CONDENSED;
        else if (width == FC_WIDTH_EXTRACONDENSED)
            return WIDTH_EXTRA_CONDENSED;
        else if (width == FC_WIDTH_CONDENSED)
            return WIDTH_CONDENSED;
        else if (width == FC_WIDTH_SEMICONDENSED)
            return WIDTH_SEMI_CONDENSED;
        else if (width == FC_WIDTH_SEMIEXPANDED)
            return WIDTH_SEMI_EXPANDED;
        else if (width == FC_WIDTH_EXPANDED)
            return WIDTH_EXPANDED;
        else if (width == FC_WIDTH_EXTRAEXPANDED)
            return WIDTH_EXTRA_EXPANDED;
        else if (width == FC_WIDTH_ULTRAEXPANDED)
            return WIDTH_ULTRA_EXPANDED;
        return WIDTH_NORMAL;
    }
}

namespace
{
    // for variable fonts, FC_INDEX has been changed such that the lower half is now the
    // index of the font within the collection, and the upper half has been repurposed
    // as the index within the variations
    unsigned int GetCollectionIndex(unsigned int nEntryId)
    {
        return nEntryId & 0xFFFF;
    }

    unsigned int GetVariationIndex(unsigned int nEntryId)
    {
        return nEntryId >> 16;
    }
}

void PrintFontManager::countFontconfigFonts()
{
    int nFonts = 0;
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcFontSet* pFSet = rWrapper.getFontSet();
    const bool bMinimalFontset = comphelper::IsFuzzing();
    if( pFSet )
    {
        SAL_INFO("vcl.fonts", "found " << pFSet->nfont << " entries in fontconfig fontset");

        FcFontSet* pFilteredSet = FcFontSetCreate();

        for( int i = 0; i < pFSet->nfont; i++ )
        {
            FcChar8* file = nullptr;
            FcChar8* family = nullptr;
            FcChar8* style = nullptr;
            FcChar8* format = nullptr;
            int slant = 0;
            int weight = 0;
            int width = 0;
            int spacing = 0;
            int symbol = 0;
            int nEntryId = -1;
            FcBool scalable = false;

            FcResult eFileRes         = FcPatternGetString(pFSet->fonts[i], FC_FILE, 0, &file);
            FcResult eFamilyRes       = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &family, FC_FAMILY, FC_FAMILYLANG );
            if (bMinimalFontset && strncmp(reinterpret_cast<char*>(family), "Liberation", strlen("Liberation")))
                continue;
            FcResult eStyleRes        = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &style, FC_STYLE, FC_STYLELANG );
            FcResult eSlantRes        = FcPatternGetInteger(pFSet->fonts[i], FC_SLANT, 0, &slant);
            FcResult eWeightRes       = FcPatternGetInteger(pFSet->fonts[i], FC_WEIGHT, 0, &weight);
            FcResult eWidthRes        = FcPatternGetInteger(pFSet->fonts[i], FC_WIDTH, 0, &width);
            FcResult eSpacRes         = FcPatternGetInteger(pFSet->fonts[i], FC_SPACING, 0, &spacing);
            FcResult eScalableRes     = FcPatternGetBool(pFSet->fonts[i], FC_SCALABLE, 0, &scalable);
            FcResult eSymbolRes       = FcPatternGetBool(pFSet->fonts[i], FC_SYMBOL, 0, &symbol);
            FcResult eIndexRes        = FcPatternGetInteger(pFSet->fonts[i], FC_INDEX, 0, &nEntryId);
            FcResult eFormatRes       = FcPatternGetString(pFSet->fonts[i], FC_FONTFORMAT, 0, &format);

            if( eFileRes != FcResultMatch || eFamilyRes != FcResultMatch || eScalableRes != FcResultMatch || eStyleRes != FcResultMatch )
                continue;

            SAL_INFO(
                "vcl.fonts.detail",
                "found font \"" << family << "\" in file " << file << ", weight = "
                << (eWeightRes == FcResultMatch ? weight : -1) << ", slant = "
                << (eSpacRes == FcResultMatch ? slant : -1) << ", style = \""
                << (eStyleRes == FcResultMatch ? reinterpret_cast<const char*>(style) : "<nil>")
                << "\",  width = " << (eWeightRes == FcResultMatch ? width : -1) << ", spacing = "
                << (eSpacRes == FcResultMatch ? spacing : -1) << ", scalable = "
                << (eScalableRes == FcResultMatch ? scalable : -1) << ", format "
                << (eFormatRes == FcResultMatch
                    ? reinterpret_cast<const char*>(format) : "<unknown>")
                << " symbol = " << (eSymbolRes == FcResultMatch ? symbol : -1));

//            OSL_ASSERT(eScalableRes != FcResultMatch || scalable);

            // We support only scalable fonts
            if( eScalableRes == FcResultMatch && ! scalable )
                continue;

            if (isPreviouslyDuplicateOrObsoleted(pFSet, i))
            {
                SAL_INFO("vcl.fonts.detail", "Ditching " << file << " as duplicate/obsolete");
                continue;
            }

            OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
            splitPath( aOrgPath, aDir, aBase );
            int nDirID = getDirectoryAtom( aDir );

            PrintFont aFont;
            aFont.m_nDirectory = nDirID;
            aFont.m_aFontFile = aBase;
            if (eIndexRes == FcResultMatch)
            {
                aFont.m_nCollectionEntry = GetCollectionIndex(nEntryId);
                aFont.m_nVariationEntry = GetVariationIndex(nEntryId);
            }

            auto& rFA = aFont.m_aFontAttributes;
            rFA.SetWeight(WEIGHT_NORMAL);
            rFA.SetWidthType(WIDTH_NORMAL);
            rFA.SetPitch(PITCH_VARIABLE);
            rFA.SetQuality(512);

            rFA.SetFamilyName(OStringToOUString(std::string_view(reinterpret_cast<char*>(family)), RTL_TEXTENCODING_UTF8));
            if (eStyleRes == FcResultMatch)
                rFA.SetStyleName(OStringToOUString(std::string_view(reinterpret_cast<char*>(style)), RTL_TEXTENCODING_UTF8));
            if (eWeightRes == FcResultMatch)
                rFA.SetWeight(convertWeight(weight));
            if (eWidthRes == FcResultMatch)
                rFA.SetWidthType(convertWidth(width));
            if (eSpacRes == FcResultMatch)
                rFA.SetPitch(convertSpacing(spacing));
            if (eSlantRes == FcResultMatch)
                rFA.SetItalic(convertSlant(slant));
            if (eSymbolRes == FcResultMatch)
                rFA.SetMicrosoftSymbolEncoded(bool(symbol));

            // sort into known fonts
            fontID nFontID = m_nNextFontID++;
            m_aFonts.emplace(nFontID, aFont);
            m_aFontFileToFontID[aBase].insert(nFontID);
            nFonts++;

            FcPattern* pPattern = pFSet->fonts[i];
            FcPatternReference(pPattern);
            FcFontSetAdd(pFilteredSet, pPattern);

            SAL_INFO("vcl.fonts.detail", "inserted font " << family << " as fontID " << nFontID);
        }

        // tdf#157939 if we drop fonts, drop them from the FcConfig set too so they are not
        // candidates for suggestions by fontconfig
        if (pFSet->nfont != pFilteredSet->nfont)
            rWrapper.replaceFontSet(pFilteredSet);
        else
            FcFontSetDestroy(pFilteredSet);

    }

    // how does one get rid of the config ?
    SAL_INFO("vcl.fonts", "inserted " << nFonts << " fonts from fontconfig");
}

void PrintFontManager::deinitFontconfig()
{
    FontCfgWrapper::release();
}

void PrintFontManager::addFontconfigDir( const OString& rDirName )
{
    const char* pDirName = rDirName.getStr();
    bool bDirOk = (FcConfigAppFontAddDir(FcConfigGetCurrent(), reinterpret_cast<FcChar8 const *>(pDirName) ) == FcTrue);

    SAL_INFO("vcl.fonts", "FcConfigAppFontAddDir( \"" << pDirName << "\") => " << bDirOk);

    if( !bDirOk )
        return;

    // load dir-specific fc-config file too if available
    const OString aConfFileName = rDirName + "/fc_local.conf";
    FILE* pCfgFile = fopen( aConfFileName.getStr(), "rb" );
    if( pCfgFile )
    {
        fclose( pCfgFile);
        bool bCfgOk = FcConfigParseAndLoad(FcConfigGetCurrent(),
                        reinterpret_cast<FcChar8 const *>(aConfFileName.getStr()), FcTrue);

        SAL_INFO_IF(!bCfgOk,
                "vcl.fonts", "FcConfigParseAndLoad( \""
                << aConfFileName << "\") => " << bCfgOk);
    } else {
        SAL_INFO("vcl.fonts", "cannot open " << aConfFileName);
    }
}

void PrintFontManager::addFontconfigFile( const OString& rFileName )
{
    const char* pFileName = rFileName.getStr();
    bool bFileOk = (FcConfigAppFontAddFile(FcConfigGetCurrent(), reinterpret_cast<FcChar8 const *>(pFileName) ) == FcTrue);

    SAL_INFO("vcl.fonts", "FcConfigAppFontAddFile(\"" << pFileName << "\") => " << std::boolalpha << bFileOk);

    if( !bFileOk )
        return;

    // FIXME: we want to add only the newly added font not re-add the whole
    // application font set.
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();
    rWrapper.addFontSet( FcSetApplication );
}

static void addtopattern(FcPattern *pPattern,
    FontItalic eItalic, FontWeight eWeight, FontWidth eWidth, FontPitch ePitch)
{
    if( eItalic != ITALIC_DONTKNOW )
    {
        int nSlant = FC_SLANT_ROMAN;
        switch( eItalic )
        {
            case ITALIC_NORMAL:
                nSlant = FC_SLANT_ITALIC;
                break;
            case ITALIC_OBLIQUE:
                nSlant = FC_SLANT_OBLIQUE;
                break;
            default:
                break;
        }
        FcPatternAddInteger(pPattern, FC_SLANT, nSlant);
    }
    if( eWeight != WEIGHT_DONTKNOW )
    {
        int nWeight = FC_WEIGHT_NORMAL;
        switch( eWeight )
        {
            case WEIGHT_THIN:           nWeight = FC_WEIGHT_THIN;break;
            case WEIGHT_ULTRALIGHT:     nWeight = FC_WEIGHT_ULTRALIGHT;break;
            case WEIGHT_LIGHT:          nWeight = FC_WEIGHT_LIGHT;break;
            case WEIGHT_SEMILIGHT:      nWeight = FC_WEIGHT_BOOK;break;
            case WEIGHT_NORMAL:         nWeight = FC_WEIGHT_NORMAL;break;
            case WEIGHT_MEDIUM:         nWeight = FC_WEIGHT_MEDIUM;break;
            case WEIGHT_SEMIBOLD:       nWeight = FC_WEIGHT_SEMIBOLD;break;
            case WEIGHT_BOLD:           nWeight = FC_WEIGHT_BOLD;break;
            case WEIGHT_ULTRABOLD:      nWeight = FC_WEIGHT_ULTRABOLD;break;
            case WEIGHT_BLACK:          nWeight = FC_WEIGHT_BLACK;break;
            default:
                break;
        }
        FcPatternAddInteger(pPattern, FC_WEIGHT, nWeight);
    }
    if( eWidth != WIDTH_DONTKNOW )
    {
        int nWidth = FC_WIDTH_NORMAL;
        switch( eWidth )
        {
            case WIDTH_ULTRA_CONDENSED: nWidth = FC_WIDTH_ULTRACONDENSED;break;
            case WIDTH_EXTRA_CONDENSED: nWidth = FC_WIDTH_EXTRACONDENSED;break;
            case WIDTH_CONDENSED:       nWidth = FC_WIDTH_CONDENSED;break;
            case WIDTH_SEMI_CONDENSED:  nWidth = FC_WIDTH_SEMICONDENSED;break;
            case WIDTH_NORMAL:          nWidth = FC_WIDTH_NORMAL;break;
            case WIDTH_SEMI_EXPANDED:   nWidth = FC_WIDTH_SEMIEXPANDED;break;
            case WIDTH_EXPANDED:        nWidth = FC_WIDTH_EXPANDED;break;
            case WIDTH_EXTRA_EXPANDED:  nWidth = FC_WIDTH_EXTRAEXPANDED;break;
            case WIDTH_ULTRA_EXPANDED:  nWidth = FC_WIDTH_ULTRAEXPANDED;break;
            default:
                break;
        }
        FcPatternAddInteger(pPattern, FC_WIDTH, nWidth);
    }
    if( ePitch == PITCH_DONTKNOW )
        return;

    int nSpacing = FC_PROPORTIONAL;
    switch( ePitch )
    {
        case PITCH_FIXED:           nSpacing = FC_MONO;break;
        case PITCH_VARIABLE:        nSpacing = FC_PROPORTIONAL;break;
        default:
            break;
    }
    FcPatternAddInteger(pPattern, FC_SPACING, nSpacing);
    if (nSpacing == FC_MONO)
        FcPatternAddString(pPattern, FC_FAMILY, reinterpret_cast<FcChar8 const *>("monospace"));
}

namespace
{
    //Someday fontconfig will hopefully use bcp47, see:
    //https://gitlab.freedesktop.org/fontconfig/fontconfig/-/issues/50
    //In the meantime try something that will fit to workaround, see:
    //https://gitlab.freedesktop.org/fontconfig/fontconfig/-/issues/30
    OString mapToFontConfigLangTag(const LanguageTag &rLangTag)
    {
        std::shared_ptr<FcStrSet> xLangSet(FcGetLangs(), FcStrSetDestroy);
        OString sLangAttrib;

        sLangAttrib = OUStringToOString(rLangTag.getBcp47(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        if (FcStrSetMember(xLangSet.get(), reinterpret_cast<const FcChar8*>(sLangAttrib.getStr())))
        {
            return sLangAttrib;
        }

        sLangAttrib = OUStringToOString(rLangTag.getLanguageAndScript(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        if (FcStrSetMember(xLangSet.get(), reinterpret_cast<const FcChar8*>(sLangAttrib.getStr())))
        {
            return sLangAttrib;
        }

        OString sLang = OUStringToOString(rLangTag.getLanguage(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        OString sRegion = OUStringToOString(rLangTag.getCountry(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();

        if (!sRegion.isEmpty())
        {
            sLangAttrib = sLang + "-" + sRegion;
            if (FcStrSetMember(xLangSet.get(), reinterpret_cast<const FcChar8*>(sLangAttrib.getStr())))
            {
                return sLangAttrib;
            }
        }

        if (FcStrSetMember(xLangSet.get(), reinterpret_cast<const FcChar8*>(sLang.getStr())))
        {
            return sLang;
        }

        return OString();
    }

    bool isEmoji(sal_uInt32 nCurrentChar)
    {
        return u_hasBinaryProperty(nCurrentChar, UCHAR_EMOJI);
    }

    //returns true if the given code-point couldn't possibly be in rLangTag.
    bool isImpossibleCodePointForLang(const LanguageTag &rLangTag, sal_uInt32 currentChar)
    {
        //a non-default script is set, lets believe it
        if (rLangTag.hasScript())
            return false;

        int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
        UScriptCode eScript = static_cast<UScriptCode>(script);
        bool bIsImpossible = false;
        OUString sLang = rLangTag.getLanguage();
        switch (eScript)
        {
            //http://en.wiktionary.org/wiki/Category:Oriya_script_languages
            case USCRIPT_ORIYA:
                bIsImpossible =
                    sLang != "or" &&
                    sLang != "kxv";
                break;
            //http://en.wiktionary.org/wiki/Category:Telugu_script_languages
            case USCRIPT_TELUGU:
                bIsImpossible =
                    sLang != "te" &&
                    sLang != "gon" &&
                    sLang != "kfc";
                break;
            //http://en.wiktionary.org/wiki/Category:Bengali_script_languages
            case USCRIPT_BENGALI:
                bIsImpossible =
                    sLang != "bn" &&
                    sLang != "as" &&
                    sLang != "bpy" &&
                    sLang != "ctg" &&
                    sLang != "sa";
                break;
            default:
                break;
        }
        SAL_WARN_IF(bIsImpossible, "vcl.fonts", "In glyph fallback throwing away the language property of "
            << sLang << " because the detected script for '0x"
            << OUString::number(currentChar, 16)
            << "' is " << uscript_getName(eScript)
            << " and that language doesn't make sense. Autodetecting instead.");
        return bIsImpossible;
    }

    OUString getExemplarLangTagForCodePoint(sal_uInt32 currentChar)
    {
        if (isEmoji(currentChar))
            return "und-zsye";
        int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
        UScriptCode eScript = static_cast<UScriptCode>(script);
        OStringBuffer aBuf(unicode::getExemplarLanguageForUScriptCode(eScript));
        if (const char* pScriptCode = uscript_getShortName(eScript))
            aBuf.append(OStringChar('-') + pScriptCode);
        return OStringToOUString(aBuf, RTL_TEXTENCODING_UTF8);
    }
}

IMPL_LINK_NOARG(PrintFontManager, autoInstallFontLangSupport, Timer *, void)
{
    try
    {
        using namespace org::freedesktop::PackageKit;
        css::uno::Reference<XSyncDbusSessionHelper> xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()));
        xSyncDbusSessionHelper->InstallFontconfigResources(comphelper::containerToSequence(m_aCurrentRequests), "hide-finished");
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION("vcl.fonts", "InstallFontconfigResources problem");
        // Disable this method from now on. It's simply not available on some systems
        // and leads to an error dialog being shown each time this is called tdf#104883
        std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::Common::PackageKit::EnableFontInstallation::set(false, batch);
        batch->commit();
    }

    m_aCurrentRequests.clear();
}

void PrintFontManager::Substitute(vcl::font::FontSelectPattern &rPattern, OUString& rMissingCodes)
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    // build pattern argument for fontconfig query
    FcPattern* pPattern = FcPatternCreate();

    // Prefer scalable fonts
    FcPatternAddBool(pPattern, FC_SCALABLE, FcTrue);

    const OString aTargetName = OUStringToOString( rPattern.maTargetName, RTL_TEXTENCODING_UTF8 );
    const FcChar8* pTargetNameUtf8 = reinterpret_cast<FcChar8 const *>(aTargetName.getStr());
    FcPatternAddString(pPattern, FC_FAMILY, pTargetNameUtf8);

    LanguageTag aLangTag(rPattern.meLanguage);
    OString aLangAttrib = mapToFontConfigLangTag(aLangTag);

    bool bMissingJustBullet = false;

    // Add required Unicode characters, if any
    if ( !rMissingCodes.isEmpty() )
    {
        FcCharSet *codePoints = FcCharSetCreate();
        bMissingJustBullet = rMissingCodes.getLength() == 1 && rMissingCodes[0] == 0xb7;
        for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
        {
            // also handle unicode surrogates
            const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
            FcCharSetAddChar( codePoints, nCode );
            //if the codepoint is impossible for this lang tag, then clear it
            //and autodetect something useful
            if (!aLangAttrib.isEmpty() && (isImpossibleCodePointForLang(aLangTag, nCode) || isEmoji(nCode)))
                aLangAttrib.clear();
            //#i105784#/rhbz#527719  improve selection of fallback font
            if (aLangAttrib.isEmpty())
            {
                aLangTag.reset(getExemplarLangTagForCodePoint(nCode));
                aLangAttrib = mapToFontConfigLangTag(aLangTag);
            }
        }
        FcPatternAddCharSet(pPattern, FC_CHARSET, codePoints);
        FcCharSetDestroy(codePoints);
    }

    if (!aLangAttrib.isEmpty())
        FcPatternAddString(pPattern, FC_LANG, reinterpret_cast<FcChar8 const *>(aLangAttrib.getStr()));

    // bodge: testTdf153440 wants a fallback to an emoji font it adds as a temp
    // testing font which has the required glyphs, but that emoji font is not
    // seen as a "color" font, while it is possible that OpenDyslexic can be
    // bundled, which *is* a "color" font. The default rules (See in Fedora 38
    // at least) then prefer a color font *without* the glyphs over a non-color
    // font *with* the glyphs, which seems like a bug to me.
    // Maybe this is an attempt to prefer color emoji fonts over non-color emoji
    // containing fonts like Symbola which has gone awry?
    // For testing purposes (isRestrictingFontSetForTesting is true) force a
    // preference for non-color fonts.
    if (rWrapper.isRestrictingFontSetForTesting())
        FcPatternAddBool(pPattern, FC_COLOR, FcFalse);

    addtopattern(pPattern, rPattern.GetItalic(), rPattern.GetWeight(),
        rPattern.GetWidthType(), rPattern.GetPitch());

    // query fontconfig for a substitute
    FcConfigSubstitute(FcConfigGetCurrent(), pPattern, FcMatchPattern);
    FcDefaultSubstitute(pPattern);

    // process the result of the fontconfig query
    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch(FcConfigGetCurrent(), &pFontSet, 1, pPattern, &eResult);
    FcPatternDestroy( pPattern );

    FcFontSet*  pSet = nullptr;
    if( pResult )
    {
        pSet = FcFontSetCreate();
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        FcFontSetAdd( pSet, pResult );
    }

    if( pSet )
    {
        if( pSet->nfont > 0 )
        {
            bool bRet = false;

            //extract the closest match
            FcChar8* file = nullptr;
            FcResult eFileRes = FcPatternGetString(pSet->fonts[0], FC_FILE, 0, &file);
            int nEntryId = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nEntryId);
            if (eIndexRes != FcResultMatch)
                nEntryId = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir );
                fontID nFontID = findFontFileID(nDirID, aBase, GetCollectionIndex(nEntryId), GetVariationIndex(nEntryId));
                auto const* pFont = getFont(nFontID);
                if (pFont)
                {
                    rPattern.maSearchName = pFont->m_aFontAttributes.GetFamilyName();
                    bRet = true;
                }
            }

            SAL_WARN_IF(!bRet, "vcl.fonts", "no FC_FILE found, falling back to name search");

            if (!bRet)
            {
                FcChar8* family = nullptr;
                FcResult eFamilyRes = FcPatternGetString( pSet->fonts[0], FC_FAMILY, 0, &family );

                // get the family name
                if( eFamilyRes == FcResultMatch )
                {
                    OString sFamily(reinterpret_cast<char*>(family));
                    std::unordered_map< OString, OString >::const_iterator aI =
                        rWrapper.m_aFontNameToLocalized.find(sFamily);
                    if (aI != rWrapper.m_aFontNameToLocalized.end())
                        sFamily = aI->second;
                    rPattern.maSearchName = OStringToOUString( sFamily, RTL_TEXTENCODING_UTF8 );
                    bRet = true;
                }
            }

            if (bRet)
            {
                int val = 0;
                if (FcResultMatch == FcPatternGetInteger(pSet->fonts[0], FC_WEIGHT, 0, &val))
                    rPattern.SetWeight( convertWeight(val) );
                if (FcResultMatch == FcPatternGetInteger(pSet->fonts[0], FC_SLANT, 0, &val))
                    rPattern.SetItalic( convertSlant(val) );
                if (FcResultMatch == FcPatternGetInteger(pSet->fonts[0], FC_SPACING, 0, &val))
                    rPattern.SetPitch ( convertSpacing(val) );
                if (FcResultMatch == FcPatternGetInteger(pSet->fonts[0], FC_WIDTH, 0, &val))
                    rPattern.SetWidthType ( convertWidth(val) );
                FcBool bEmbolden;
                if (FcResultMatch == FcPatternGetBool(pSet->fonts[0], FC_EMBOLDEN, 0, &bEmbolden))
                    rPattern.mbEmbolden = bEmbolden;
                FcMatrix *pMatrix = nullptr;
                if (FcResultMatch == FcPatternGetMatrix(pSet->fonts[0], FC_MATRIX, 0, &pMatrix))
                {
                    rPattern.maItalicMatrix.xx = pMatrix->xx;
                    rPattern.maItalicMatrix.xy = pMatrix->xy;
                    rPattern.maItalicMatrix.yx = pMatrix->yx;
                    rPattern.maItalicMatrix.yy = pMatrix->yy;
                }
            }

            // update rMissingCodes by removing resolved code points
            if( !rMissingCodes.isEmpty() )
            {
                std::unique_ptr<sal_uInt32[]> const pRemainingCodes(new sal_uInt32[rMissingCodes.getLength()]);
                int nRemainingLen = 0;
                FcCharSet* codePoints;
                if (!FcPatternGetCharSet(pSet->fonts[0], FC_CHARSET, 0, &codePoints))
                {
                    for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
                    {
                        // also handle surrogates
                        const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
                        if (FcCharSetHasChar(codePoints, nCode) != FcTrue)
                            pRemainingCodes[ nRemainingLen++ ] = nCode;
                    }
                }
                OUString sStillMissing(pRemainingCodes.get(), nRemainingLen);
                if (!Application::IsHeadlessModeEnabled() && officecfg::Office::Common::PackageKit::EnableFontInstallation::get())
                {
                    if (sStillMissing == rMissingCodes) //replaced nothing
                    {
                        //It'd be better if we could ask packagekit using the
                        //missing codepoints or some such rather than using
                        //"language" as a proxy to how fontconfig considers
                        //scripts to default to a given language.
                        for (sal_Int32 i = 0; i < nRemainingLen; ++i)
                        {
                            LanguageTag aOurTag(getExemplarLangTagForCodePoint(pRemainingCodes[i]));
                            OString sTag = OUStringToOString(aOurTag.getBcp47(), RTL_TEXTENCODING_UTF8);
                            if (!m_aPreviousLangSupportRequests.insert(sTag).second)
                                continue;
                            sTag = mapToFontConfigLangTag(aOurTag);
                            if (!sTag.isEmpty() && m_aPreviousLangSupportRequests.find(sTag) == m_aPreviousLangSupportRequests.end())
                            {
                                OString sReq = OString::Concat(":lang=") + sTag;
                                m_aCurrentRequests.push_back(OUString::fromUtf8(sReq));
                                m_aPreviousLangSupportRequests.insert(sTag);
                            }
                        }
                    }
                    if (!m_aCurrentRequests.empty())
                        m_aFontInstallerTimer.Start();
                }
                rMissingCodes = sStillMissing;
            }
        }

        FcFontSetDestroy( pSet );
    }

    SAL_INFO("vcl.fonts", "PrintFontManager::Substitute: replacing missing font: '"
                              << rPattern.maTargetName << "' with '" << rPattern.maSearchName
                              << "'");

    static const bool bAbortOnFontSubstitute = [] {
        const char* pEnv = getenv("SAL_NON_APPLICATION_FONT_USE");
        return pEnv && strcmp(pEnv, "abort") == 0;
    }();
    if (bAbortOnFontSubstitute && rPattern.maTargetName != rPattern.maSearchName)
    {
        if (bMissingJustBullet)
        {
            // Some fonts exist in "more_fonts", but have no U+00B7 MIDDLE DOT
            // so will always glyph fallback on measuring mnBulletOffset in
            // FontMetricData::ImplInitTextLineSize
            return;
        }
        if (rPattern.maTargetName == "Linux Libertine G" && rPattern.maSearchName == "Linux Libertine O")
            return;
        SAL_WARN("vcl.fonts", "PrintFontManager::Substitute: missing font: '" << rPattern.maTargetName <<
                              "' try: " << rPattern.maSearchName << " instead");
        std::cerr << "terminating test due to missing font: " << rPattern.maTargetName << std::endl;
        std::abort();
    }
}

FontConfigFontOptions::~FontConfigFontOptions()
{
    FcPatternDestroy(mpPattern);
}

FcPattern *FontConfigFontOptions::GetPattern() const
{
    return mpPattern;
}

void FontConfigFontOptions::SyncPattern(const OString& rFileName, sal_uInt32 nIndex, sal_uInt32 nVariation, bool bEmbolden)
{
    FcPatternDel(mpPattern, FC_FILE);
    FcPatternAddString(mpPattern, FC_FILE, reinterpret_cast<FcChar8 const *>(rFileName.getStr()));
    FcPatternDel(mpPattern, FC_INDEX);
    sal_uInt32 nFcIndex = (nVariation << 16) | nIndex;
    FcPatternAddInteger(mpPattern, FC_INDEX, nFcIndex);
    FcPatternDel(mpPattern, FC_EMBOLDEN);
    FcPatternAddBool(mpPattern, FC_EMBOLDEN, bEmbolden ? FcTrue : FcFalse);
}

std::unique_ptr<FontConfigFontOptions> PrintFontManager::getFontOptions(const FontAttributes& rInfo, int nSize)
{
    FontOptionsKey aKey{ rInfo.GetFamilyName(), nSize, rInfo.GetItalic(),
                         rInfo.GetWeight(), rInfo.GetWidthType(), rInfo.GetPitch() };

    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    std::unique_ptr<FontConfigFontOptions> pOptions = rWrapper.m_aCachedFontOptions.lookup(aKey);
    if (pOptions)
        return pOptions;

    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    OString sFamily = OUStringToOString(aKey.m_sFamilyName, RTL_TEXTENCODING_UTF8);

    std::unordered_map< OString, OString >::const_iterator aI = rWrapper.m_aLocalizedToCanonical.find(sFamily);
    if (aI != rWrapper.m_aLocalizedToCanonical.end())
        sFamily = aI->second;
    if( !sFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, reinterpret_cast<FcChar8 const *>(sFamily.getStr()));

    addtopattern(pPattern, aKey.m_eItalic, aKey.m_eWeight, aKey.m_eWidth, aKey.m_ePitch);
    FcPatternAddDouble(pPattern, FC_PIXEL_SIZE, nSize);

    FcConfigSubstitute(pConfig, pPattern, FcMatchPattern);
    FontConfigFontOptions::cairo_font_options_substitute(pPattern);
    FcDefaultSubstitute(pPattern);

    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    if (FcPattern* pResult = FcFontSetMatch(pConfig, &pFontSet, 1, pPattern, &eResult))
    {
        rWrapper.m_aCachedFontOptions.cache(aKey, pResult);
        pOptions.reset(new FontConfigFontOptions(pResult));
    }

    // cleanup
    FcPatternDestroy( pPattern );

    return pOptions;
}


bool PrintFontManager::matchFont(FontAttributes& rDFA, const css::lang::Locale& rLocale)
{
    bool bFound = false;
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    // populate pattern with font characteristics
    const LanguageTag aLangTag(rLocale);
    const OString aLangAttrib = mapToFontConfigLangTag(aLangTag);
    if (!aLangAttrib.isEmpty())
        FcPatternAddString(pPattern, FC_LANG, reinterpret_cast<FcChar8 const *>(aLangAttrib.getStr()));

    OString aFamily = OUStringToOString(rDFA.GetFamilyName(), RTL_TEXTENCODING_UTF8);
    if( !aFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, reinterpret_cast<FcChar8 const *>(aFamily.getStr()));

    addtopattern(pPattern, rDFA.GetItalic(), rDFA.GetWeight(), rDFA.GetWidthType(), rDFA.GetPitch());

    FcConfigSubstitute(pConfig, pPattern, FcMatchPattern);
    FcDefaultSubstitute(pPattern);
    FcResult eResult = FcResultNoMatch;
    FcFontSet *pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch(pConfig, &pFontSet, 1, pPattern, &eResult);
    if( pResult )
    {
        FcFontSet* pSet = FcFontSetCreate();
        FcFontSetAdd( pSet, pResult );
        if( pSet->nfont > 0 )
        {
            //extract the closest match
            FcChar8* file = nullptr;
            FcResult eFileRes = FcPatternGetString(pSet->fonts[0], FC_FILE, 0, &file);
            int nEntryId = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nEntryId);
            if (eIndexRes != FcResultMatch)
                nEntryId = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir );
                fontID nFontID = findFontFileID(nDirID, aBase,
                                              GetCollectionIndex(nEntryId),
                                              GetVariationIndex(nEntryId));
                auto const* pFont = getFont(nFontID);
                if (pFont)
                {
                    rDFA = pFont->m_aFontAttributes;
                    bFound = true;
                }
            }
        }
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        FcFontSetDestroy( pSet );
    }

    // cleanup
    FcPatternDestroy( pPattern );

    return bFound;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
