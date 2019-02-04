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

#include <memory>
#include <unx/fontmanager.hxx>
#include <impfont.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/wrkwin.hxx>
#include <fontinstance.hxx>
#include <sallayout.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/unicode.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <unicode/uchar.h>
#include <unicode/uscript.h>
#include <officecfg/Office/Common.hxx>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>

using namespace psp;

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <fontconfig/fcfreetype.h>

#include <cstdio>
#include <cstdarg>

#include <unotools/configmgr.hxx>

#include <osl/module.h>
#include <osl/thread.h>
#include <osl/process.h>

#include <rtl/ustrbuf.hxx>

#include <utility>
#include <algorithm>

using namespace osl;

namespace
{
    typedef std::pair<FcChar8*, FcChar8*> lang_and_element;
}

class FontCfgWrapper
{
    FcFontSet* m_pOutlineSet;

    void addFontSet( FcSetName );

    FontCfgWrapper();
    ~FontCfgWrapper();

public:
    static FontCfgWrapper& get();
    static void release();

    FcFontSet* getFontSet();

    void clear();

public:
    FcResult LocalizedElementFromPattern(FcPattern const * pPattern, FcChar8 **family,
                                         const char *elementtype, const char *elementlangtype);
//to-do, make private and add some cleaner accessor methods
    std::unordered_map< OString, OString > m_aFontNameToLocalized;
    std::unordered_map< OString, OString > m_aLocalizedToCanonical;
private:
    void cacheLocalizedFontNames(const FcChar8 *origfontname, const FcChar8 *bestfontname, const std::vector< lang_and_element > &lang_and_elements);

    std::unique_ptr<LanguageTag> m_pLanguageTag;
};

FontCfgWrapper::FontCfgWrapper()
    : m_pOutlineSet( nullptr )
{
    FcInit();
}

void FontCfgWrapper::addFontSet( FcSetName eSetName )
{
    /*
      add only acceptable outlined fonts to our config,
      for future fontconfig use
    */
    FcFontSet* pOrig = FcConfigGetFonts( FcConfigGetCurrent(), eSetName );
    if( !pOrig )
        return;

    // filter the font sets to remove obsolete faces
    for( int i = 0; i < pOrig->nfont; ++i )
    {
        FcPattern* pPattern = pOrig->fonts[i];
        // #i115131# ignore non-outline fonts
        FcBool bOutline = FcFalse;
        FcResult eOutRes = FcPatternGetBool( pPattern, FC_OUTLINE, 0, &bOutline );
        if( (eOutRes != FcResultMatch) || (bOutline == FcFalse) )
            continue;

        // Ignore Type 1 fonts, too.
        FcChar8* pFormat = nullptr;
        FcResult eFormatRes = FcPatternGetString(pPattern, FC_FONTFORMAT, 0, &pFormat);
        if ((eFormatRes == FcResultMatch) && (strcmp(reinterpret_cast<char*>(pFormat), "Type 1") == 0))
            continue;

        FcPatternReference( pPattern );
        FcFontSetAdd( m_pOutlineSet, pPattern );
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
    if( !m_pOutlineSet )
    {
        m_pOutlineSet = FcFontSetCreate();
        addFontSet( FcSetSystem );
        addFontSet( FcSetApplication );

        ::std::sort(m_pOutlineSet->fonts,m_pOutlineSet->fonts+m_pOutlineSet->nfont,SortFont());
    }

    return m_pOutlineSet;
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
        OString sFullMatch = sLangMatch;
        sFullMatch += OString('-');
        sFullMatch += OUStringToOString(rLangTag.getCountry().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8);

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

            //possible to-do, sort by UILocale instead of process locale
            if (!m_pLanguageTag)
            {
                rtl_Locale* pLoc = nullptr;
                osl_getProcessLocale(&pLoc);
                m_pLanguageTag.reset( new LanguageTag(*pLoc) );
            }
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
    if( m_pOutlineSet )
    {
        FcFontSetDestroy( m_pOutlineSet );
        m_pOutlineSet = nullptr;
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

//FontConfig doesn't come with a way to remove an element from a FontSet as far
//as I can see
static void lcl_FcFontSetRemove(FcFontSet* pFSet, int i)
{
    FcPatternDestroy(pFSet->fonts[i]);

    int nTail = pFSet->nfont - (i + 1);
    --pFSet->nfont;
    if (!nTail)
        return;
    memmove(pFSet->fonts + i, pFSet->fonts + i + 1, nTail*sizeof(FcPattern*));
}

void PrintFontManager::countFontconfigFonts( std::unordered_map<OString, int>& o_rVisitedPaths )
{
    int nFonts = 0;
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcFontSet* pFSet = rWrapper.getFontSet();
    const bool bMinimalFontset = utl::ConfigManager::IsFuzzing();
    if( pFSet )
    {
        SAL_INFO("vcl.fonts", "found " << pFSet->nfont << " entries in fontconfig fontset");
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
            int nCollectionEntry = -1;
            FcBool outline = false;

            FcResult eFileRes         = FcPatternGetString(pFSet->fonts[i], FC_FILE, 0, &file);
            FcResult eFamilyRes       = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &family, FC_FAMILY, FC_FAMILYLANG );
            if (bMinimalFontset && strncmp(reinterpret_cast<char*>(family), "Liberation", strlen("Liberation")))
                continue;
            FcResult eStyleRes        = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &style, FC_STYLE, FC_STYLELANG );
            FcResult eSlantRes        = FcPatternGetInteger(pFSet->fonts[i], FC_SLANT, 0, &slant);
            FcResult eWeightRes       = FcPatternGetInteger(pFSet->fonts[i], FC_WEIGHT, 0, &weight);
            FcResult eWidthRes        = FcPatternGetInteger(pFSet->fonts[i], FC_WIDTH, 0, &width);
            FcResult eSpacRes         = FcPatternGetInteger(pFSet->fonts[i], FC_SPACING, 0, &spacing);
            FcResult eOutRes          = FcPatternGetBool(pFSet->fonts[i], FC_OUTLINE, 0, &outline);
            FcResult eIndexRes        = FcPatternGetInteger(pFSet->fonts[i], FC_INDEX, 0, &nCollectionEntry);
            FcResult eFormatRes       = FcPatternGetString(pFSet->fonts[i], FC_FONTFORMAT, 0, &format);

            if( eFileRes != FcResultMatch || eFamilyRes != FcResultMatch || eOutRes != FcResultMatch )
                continue;

            SAL_INFO(
                "vcl.fonts.detail",
                "found font \"" << family << "\" in file " << file << ", weight = "
                << (eWeightRes == FcResultMatch ? weight : -1) << ", slant = "
                << (eSpacRes == FcResultMatch ? slant : -1) << ", style = \""
                << (eStyleRes == FcResultMatch ? reinterpret_cast<const char*>(style) : "<nil>")
                << "\",  width = " << (eWeightRes == FcResultMatch ? width : -1) << ", spacing = "
                << (eSpacRes == FcResultMatch ? spacing : -1) << ", outline = "
                << (eOutRes == FcResultMatch ? outline : -1) << ", format "
                << (eFormatRes == FcResultMatch
                    ? reinterpret_cast<const char*>(format) : "<unknown>"));

//            OSL_ASSERT(eOutRes != FcResultMatch || outline);

            // only outline fonts are usable to psprint anyway
            if( eOutRes == FcResultMatch && ! outline )
                continue;

            if (isPreviouslyDuplicateOrObsoleted(pFSet, i))
            {
                SAL_INFO("vcl.fonts.detail", "Ditching " << file << " as duplicate/obsolete");
                continue;
            }

            // see if this font is already cached
            // update attributes
            OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
            splitPath( aOrgPath, aDir, aBase );

            o_rVisitedPaths[aDir] = 1;

            int nDirID = getDirectoryAtom( aDir );
            SAL_INFO("vcl.fonts.detail", "file " << aBase << " not cached");
            // not known, analyze font file to get attributes
            // not described by fontconfig (e.g. alias names, PSName)
            if (eFormatRes != FcResultMatch)
                format = nullptr;
            std::vector<std::unique_ptr<PrintFont>> aFonts = analyzeFontFile( nDirID, aBase, reinterpret_cast<char*>(format) );
            if(aFonts.empty())
            {
                SAL_INFO(
                    "vcl.fonts", "Warning: file \"" << aOrgPath << "\" is unusable to psprint");
                //remove font, reuse index
                //we want to remove unusable fonts here, in case there is a usable font
                //which duplicates the properties of the unusable one

                //not removing the unusable font will risk the usable font being rejected
                //as a duplicate by isPreviouslyDuplicateOrObsoleted
                lcl_FcFontSetRemove(pFSet, i--);
                continue;
            }

            std::unique_ptr<PrintFont> xUpdate;

            if (aFonts.size() == 1) // one font
                xUpdate = std::move(aFonts.front());
            else // more than one font
            {
                // a collection entry, get the correct index
                if( eIndexRes == FcResultMatch && nCollectionEntry != -1 )
                {
                    for (auto & font : aFonts)
                    {
                        if( font->m_nCollectionEntry == nCollectionEntry )
                        {
                            xUpdate = std::move(font);
                            break;
                        }
                    }
                }

                if (xUpdate)
                {
                    // update collection entry
                    // additional entries will be created in the cache
                    // if this is a new index (that is if the loop above
                    // ran to the end of the list)
                    xUpdate->m_nCollectionEntry = nCollectionEntry;
                }
                else
                {
                    SAL_INFO(
                        "vcl.fonts",
                        "multiple fonts for file, but no index in fontconfig pattern ! (index res ="
                        << eIndexRes << " collection entry = " << nCollectionEntry
                        << "; file will not be used");
                    // we have found more than one font in this file
                    // but fontconfig will not tell us which index is meant
                    // -> something is in disorder, do not use this font
                }
            }

            if (xUpdate)
            {
                // set family name
                if( eWeightRes == FcResultMatch )
                    xUpdate->m_eWeight = convertWeight(weight);
                if( eWidthRes == FcResultMatch )
                    xUpdate->m_eWidth = convertWidth(width);
                if( eSpacRes == FcResultMatch )
                    xUpdate->m_ePitch = convertSpacing(spacing);
                if( eSlantRes == FcResultMatch )
                    xUpdate->m_eItalic = convertSlant(slant);
                if( eStyleRes == FcResultMatch )
                {
                    xUpdate->m_aStyleName = OStringToOUString( OString( reinterpret_cast<char*>(style) ), RTL_TEXTENCODING_UTF8 );
                }

                // sort into known fonts
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = std::move(xUpdate);
                m_aFontFileToFontID[ aBase ].insert( aFont );
                nFonts++;
                SAL_INFO("vcl.fonts.detail", "inserted font " << family << " as fontID " << aFont);
            }
        }
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
        if( !bCfgOk )
            fprintf( stderr, "FcConfigParseAndLoad( \"%s\") => %d\n", aConfFileName.getStr(), bCfgOk );
    } else {
        SAL_INFO("vcl.fonts", "cannot open " << aConfFileName);
    }
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
    if( ePitch != PITCH_DONTKNOW )
    {
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
}

namespace
{
    //Someday fontconfig will hopefully use bcp47, see fdo#19869
    //In the meantime try something that will fit to workaround fdo#35118
    OString mapToFontConfigLangTag(const LanguageTag &rLangTag)
    {
#if defined(FC_VERSION) && (FC_VERSION >= 20492)
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
            sLangAttrib = sLang + OString('-') + sRegion;
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
#else
        OString sLangAttrib = OUStringToOString(rLangTag.getLanguageAndScript(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        if (sLangAttrib.equalsIgnoreAsciiCase("pa-in"))
            sLangAttrib = "pa";
        return sLangAttrib;
#endif
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
        int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
        UScriptCode eScript = static_cast<UScriptCode>(script);
        OStringBuffer aBuf(unicode::getExemplarLanguageForUScriptCode(eScript));
        if (const char* pScriptCode = uscript_getShortName(eScript))
            aBuf.append('-').append(pScriptCode);
        return OStringToOUString(aBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
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
    catch (const css::uno::Exception& e)
    {
        SAL_INFO("vcl.fonts", "InstallFontconfigResources problem, caught " << e);
        // Disable this method from now on. It's simply not available on some systems
        // and leads to an error dialog being shown each time this is called tdf#104883
        std::shared_ptr<comphelper::ConfigurationChanges> batch( comphelper::ConfigurationChanges::create() );
        officecfg::Office::Common::PackageKit::EnableFontInstallation::set(false, batch);
        batch->commit();
    }

    m_aCurrentRequests.clear();
}

void PrintFontManager::Substitute(FontSelectPattern &rPattern, OUString& rMissingCodes)
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

    // Add required Unicode characters, if any
    if ( !rMissingCodes.isEmpty() )
    {
        FcCharSet *codePoints = FcCharSetCreate();
        for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
        {
            // also handle unicode surrogates
            const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
            FcCharSetAddChar( codePoints, nCode );
            //if the codepoint is impossible for this lang tag, then clear it
            //and autodetect something useful
            if (!aLangAttrib.isEmpty() && isImpossibleCodePointForLang(aLangTag, nCode))
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
            int nCollectionEntry = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nCollectionEntry);
            if (eIndexRes != FcResultMatch)
                nCollectionEntry = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir );
                fontID aFont = findFontFileID( nDirID, aBase, nCollectionEntry );
                if( aFont > 0 )
                {
                    FastPrintFontInfo aInfo;
                    bRet = getFontFastInfo( aFont, aInfo );
                    rPattern.maSearchName = aInfo.m_aFamilyName;
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
                            if (m_aPreviousLangSupportRequests.find(sTag) != m_aPreviousLangSupportRequests.end())
                                continue;
                            m_aPreviousLangSupportRequests.insert(sTag);
                            sTag = mapToFontConfigLangTag(aOurTag);
                            if (!sTag.isEmpty() && m_aPreviousLangSupportRequests.find(sTag) == m_aPreviousLangSupportRequests.end())
                            {
                                OString sReq = OString(":lang=") + sTag;
                                m_aCurrentRequests.push_back(OUString::fromUtf8(sReq));
                                m_aPreviousLangSupportRequests.insert(sTag);
                            }
                        }
                    }
                    if (!m_aCurrentRequests.empty())
                    {
                        m_aFontInstallerTimer.Stop();
                        m_aFontInstallerTimer.Start();
                    }
                }
                rMissingCodes = sStillMissing;
            }
        }

        FcFontSetDestroy( pSet );
    }

    SAL_INFO("vcl.fonts", "PrintFontManager::Substitute: replacing missing font: '"
                              << rPattern.maTargetName << "' with '" << rPattern.maSearchName
                              << "'");
}

FontConfigFontOptions::~FontConfigFontOptions()
{
    FcPatternDestroy(mpPattern);
}

FcPattern *FontConfigFontOptions::GetPattern() const
{
    return mpPattern;
}

void FontConfigFontOptions::SyncPattern(const OString& rFileName, int nIndex, bool bEmbolden)
{
    FcPatternDel(mpPattern, FC_FILE);
    FcPatternAddString(mpPattern, FC_FILE, reinterpret_cast<FcChar8 const *>(rFileName.getStr()));
    FcPatternDel(mpPattern, FC_INDEX);
    FcPatternAddInteger(mpPattern, FC_INDEX, nIndex);
    FcPatternDel(mpPattern, FC_EMBOLDEN);
    FcPatternAddBool(mpPattern, FC_EMBOLDEN, bEmbolden ? FcTrue : FcFalse);
}

FontConfigFontOptions* PrintFontManager::getFontOptions(const FastPrintFontInfo& rInfo, int nSize)
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FontConfigFontOptions* pOptions = nullptr;
    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    OString sFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );

    std::unordered_map< OString, OString >::const_iterator aI = rWrapper.m_aLocalizedToCanonical.find(sFamily);
    if (aI != rWrapper.m_aLocalizedToCanonical.end())
        sFamily = aI->second;
    if( !sFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, reinterpret_cast<FcChar8 const *>(sFamily.getStr()));

    addtopattern(pPattern, rInfo.m_eItalic, rInfo.m_eWeight, rInfo.m_eWidth, rInfo.m_ePitch);
    FcPatternAddDouble(pPattern, FC_PIXEL_SIZE, nSize);

    int hintstyle = FC_HINT_FULL;

    FcConfigSubstitute(pConfig, pPattern, FcMatchPattern);
    FontConfigFontOptions::cairo_font_options_substitute(pPattern);
    FcDefaultSubstitute(pPattern);

    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch( pConfig, &pFontSet, 1, pPattern, &eResult );
    if( pResult )
    {
        (void) FcPatternGetInteger(pResult,
            FC_HINT_STYLE, 0, &hintstyle);

        pOptions = new FontConfigFontOptions(pResult);
    }

    // cleanup
    FcPatternDestroy( pPattern );

    return pOptions;
}

void PrintFontManager::matchFont( FastPrintFontInfo& rInfo, const css::lang::Locale& rLocale )
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    // populate pattern with font characteristics
    const LanguageTag aLangTag(rLocale);
    const OString aLangAttrib = mapToFontConfigLangTag(aLangTag);
    if (!aLangAttrib.isEmpty())
        FcPatternAddString(pPattern, FC_LANG, reinterpret_cast<FcChar8 const *>(aLangAttrib.getStr()));

    OString aFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );
    if( !aFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, reinterpret_cast<FcChar8 const *>(aFamily.getStr()));

    addtopattern(pPattern, rInfo.m_eItalic, rInfo.m_eWeight, rInfo.m_eWidth, rInfo.m_ePitch);

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
            int nCollectionEntry = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nCollectionEntry);
            if (eIndexRes != FcResultMatch)
                nCollectionEntry = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( reinterpret_cast<char*>(file) );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir );
                fontID aFont = findFontFileID( nDirID, aBase, nCollectionEntry );
                if( aFont > 0 )
                    getFontFastInfo( aFont, rInfo );
            }
        }
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        FcFontSetDestroy( pSet );
    }

    // cleanup
    FcPatternDestroy( pPattern );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
