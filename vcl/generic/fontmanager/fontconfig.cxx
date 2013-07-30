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


#include "fontcache.hxx"
#include "impfont.hxx"
#include <vcl/fontmanager.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/wrkwin.hxx>
#include "outfont.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/unicode.hxx>
#include <rtl/strbuf.hxx>
#include <unicode/uchar.h>
#include <unicode/uscript.h>

using namespace psp;

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <fontconfig/fcfreetype.h>
// allow compile on baseline (currently with fontconfig 2.2.0)
#ifndef FC_WEIGHT_BOOK      // TODO: remove when baseline moves to fc>=2.2.1
    #define FC_WEIGHT_BOOK 75
#endif
#ifndef FC_EMBEDDED_BITMAP  // TODO: remove when baseline moves to fc>=2.3.92
    #define FC_EMBEDDED_BITMAP "embeddedbitmap"
#endif
#ifndef FC_FAMILYLANG       // TODO: remove when baseline moves to fc>=2.2.97
    #define FC_FAMILYLANG "familylang"
#endif
#ifndef FC_CAPABILITY       // TODO: remove when baseline moves to fc>=2.2.97
    #define FC_CAPABILITY "capability"
#endif
#ifndef FC_STYLELANG        // TODO: remove when baseline moves to fc>=2.2.97
    #define FC_STYLELANG "stylelang"
#endif
#ifndef FC_HINT_STYLE       // TODO: remove when baseline moves to fc>=2.2.91
    #define FC_HINT_STYLE  "hintstyle"
    #define FC_HINT_NONE   0
    #define FC_HINT_SLIGHT 1
    #define FC_HINT_MEDIUM 2
    #define FC_HINT_FULL   3
#endif
#ifndef FC_FT_FACE
    #define FC_FT_FACE "ftface"
#endif
#ifndef FC_EMBOLDEN
    #define FC_EMBOLDEN "embolden"
#endif
#ifndef FC_MATRIX
    #define FC_MATRIX "matrix"
#endif
#ifndef FC_FONTFORMAT
    #define FC_FONTFORMAT "fontformat"
#endif

#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
#include <dbus/dbus-glib.h>
#endif

#include <cstdio>
#include <cstdarg>

#include "unotools/atom.hxx"

#include "osl/module.h"
#include "osl/thread.h"
#include "osl/process.h"

#include "rtl/ustrbuf.hxx"

#include "sal/alloca.h"

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
    FcResult LocalizedElementFromPattern(FcPattern* pPattern, FcChar8 **family,
                                         const char *elementtype, const char *elementlangtype);
//to-do, make private and add some cleanish accessor methods
    boost::unordered_map< OString, OString, OStringHash > m_aFontNameToLocalized;
    boost::unordered_map< OString, OString, OStringHash > m_aLocalizedToCanonical;
private:
    void cacheLocalizedFontNames(const FcChar8 *origfontname, const FcChar8 *bestfontname, const std::vector< lang_and_element > &lang_and_elements);

    LanguageTag* m_pLanguageTag;
};

FontCfgWrapper::FontCfgWrapper()
    :
        m_pOutlineSet( NULL ),
        m_pLanguageTag( NULL )
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
        FcPatternReference( pPattern );
        FcFontSetAdd( m_pOutlineSet, pPattern );
    }

    // TODO?: FcFontSetDestroy( pOrig );
}

namespace
{
    int compareFontNames(const FcPattern *a, const FcPattern *b)
    {
        FcChar8 *pNameA=NULL, *pNameB=NULL;

        int nHaveA = FcPatternGetString(a, FC_FAMILY, 0, &pNameA) == FcResultMatch;
        int nHaveB = FcPatternGetString(b, FC_FAMILY, 0, &pNameB) == FcResultMatch;

        if (nHaveA && nHaveB)
            return strcmp((const char*)pNameA, (const char*)pNameB);

        return nHaveA - nHaveB;
    }

    //Sort fonts so that fonts with the same family name are side-by-side, with
    //those with higher version numbers first
    class SortFont : public ::std::binary_function< const FcPattern*, const FcPattern*, bool >
    {
    public:
        bool operator()(const FcPattern *a, const FcPattern *b)
        {
            int comp = compareFontNames(a, b);
            if (comp != 0)
                return comp < 0;

            int nVersionA=0, nVersionB=0;

            int nHaveA = FcPatternGetInteger(a, FC_FONTVERSION, 0, &nVersionA) == FcResultMatch;
            int nHaveB = FcPatternGetInteger(b, FC_FONTVERSION, 0, &nVersionB) == FcResultMatch;

            if (nHaveA && nHaveB)
                return nVersionA > nVersionB;

            return nHaveA > nHaveB;
        }
    };

    //See fdo#30729 for where an old opensymbol installed system-wide can
    //clobber the new opensymbol installed locally
    //
    //See if this font is a duplicate with equal attributes which has already been
    //inserted, or if it an older version of an inserted fonts. Depends on FcFontSet
    //on being sorted with SortFont
    bool isPreviouslyDuplicateOrObsoleted(FcFontSet *pFSet, int i)
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
        if( FcGetVersion() > 20400 ) // #i85462# prevent crashes
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

static FontCfgWrapper* pOneInstance = NULL;

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
        pOneInstance = NULL;
    }
}

namespace
{
    class localizedsorter
    {
        public:
            localizedsorter() {};
            FcChar8* bestname(const std::vector<lang_and_element> &elements, const LanguageTag & rLangTag);
    };

    FcChar8* localizedsorter::bestname(const std::vector<lang_and_element> &elements, const LanguageTag & rLangTag)
    {
        FcChar8* candidate = elements.begin()->second;
        /* FIXME-BCP47: once fontconfig supports language tags this
         * language-territory stuff needs to be changed! */
        SAL_INFO_IF( !rLangTag.isIsoLocale(), "i18n", "localizedsorter::bestname - not an ISO locale");
        OString sLangMatch(OUStringToOString(rLangTag.getLanguage().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8));
        OString sFullMatch = sLangMatch;
        sFullMatch += OString('-');
        sFullMatch += OUStringToOString(rLangTag.getCountry().toAsciiLowerCase(), RTL_TEXTENCODING_UTF8);

        std::vector<lang_and_element>::const_iterator aEnd = elements.end();
        bool alreadyclosematch = false;
        bool found_fallback_englishname = false;
        for( std::vector<lang_and_element>::const_iterator aIter = elements.begin(); aIter != aEnd; ++aIter )
        {
            const char *pLang = (const char*)aIter->first;
            if( rtl_str_compare( pLang, sFullMatch.getStr() ) == 0)
            {
                // both language and country match
                candidate = aIter->second;
                break;
            }
            else if( alreadyclosematch )
            {
                // current candidate matches lang of lang-TERRITORY
                // override candidate only if there is a full match
                continue;
            }
            else if( rtl_str_compare( pLang, sLangMatch.getStr()) == 0)
            {
                // just the language matches
                candidate = aIter->second;
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
                candidate = aIter->second;
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
    std::vector<lang_and_element>::const_iterator aEnd = lang_and_elements.end();
    for (std::vector<lang_and_element>::const_iterator aIter = lang_and_elements.begin(); aIter != aEnd; ++aIter)
    {
        const char *candidate = (const char*)(aIter->second);
        if (rtl_str_compare(candidate, (const char*)bestfontname) != 0)
            m_aFontNameToLocalized[OString(candidate)] = OString((const char*)bestfontname);
    }
    if (rtl_str_compare((const char*)origfontname, (const char*)bestfontname) != 0)
        m_aLocalizedToCanonical[OString((const char*)bestfontname)] = OString((const char*)origfontname);
}

FcResult FontCfgWrapper::LocalizedElementFromPattern(FcPattern* pPattern, FcChar8 **element,
                                                     const char *elementtype, const char *elementlangtype)
{                                                /* e. g.:      ^ FC_FAMILY              ^ FC_FAMILYLANG */
    FcChar8 *origelement;
    FcResult eElementRes = FcPatternGetString( pPattern, elementtype, 0, &origelement );
    *element = origelement;

    if( eElementRes == FcResultMatch)
    {
        FcChar8* elementlang = NULL;
        if (FcPatternGetString( pPattern, elementlangtype, 0, &elementlang ) == FcResultMatch)
        {
            std::vector< lang_and_element > lang_and_elements;
            lang_and_elements.push_back(lang_and_element(elementlang, *element));
            int k = 1;
            while (1)
            {
                if (FcPatternGetString( pPattern, elementlangtype, k, &elementlang ) != FcResultMatch)
                    break;
                if (FcPatternGetString( pPattern, elementtype, k, element ) != FcResultMatch)
                    break;
                lang_and_elements.push_back(lang_and_element(elementlang, *element));
                ++k;
            }

            //possible to-do, sort by UILocale instead of process locale
            if (!m_pLanguageTag)
            {
                rtl_Locale* pLoc = NULL;
                osl_getProcessLocale(&pLoc);
                m_pLanguageTag = new LanguageTag(*pLoc);
            }
            *element = localizedsorter().bestname(lang_and_elements, *m_pLanguageTag);

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
        m_pOutlineSet = NULL;
    }
    delete m_pLanguageTag;
    m_pLanguageTag = NULL;
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

void PrintFontManager::countFontconfigFonts( boost::unordered_map<OString, int, OStringHash>& o_rVisitedPaths )
{
#if OSL_DEBUG_LEVEL > 1
    int nFonts = 0;
#endif
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcFontSet* pFSet = rWrapper.getFontSet();
    if( pFSet )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "found %d entries in fontconfig fontset\n", pFSet->nfont );
#endif
        for( int i = 0; i < pFSet->nfont; i++ )
        {
            FcChar8* file = NULL;
            FcChar8* family = NULL;
            FcChar8* style = NULL;
            FcChar8* format = NULL;
            int slant = 0;
            int weight = 0;
            int spacing = 0;
            int nCollectionEntry = -1;
            FcBool outline = false;

            FcResult eFileRes         = FcPatternGetString(pFSet->fonts[i], FC_FILE, 0, &file);
            FcResult eFamilyRes       = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &family, FC_FAMILY, FC_FAMILYLANG );
            FcResult eStyleRes        = rWrapper.LocalizedElementFromPattern( pFSet->fonts[i], &style, FC_STYLE, FC_STYLELANG );
            FcResult eSlantRes        = FcPatternGetInteger(pFSet->fonts[i], FC_SLANT, 0, &slant);
            FcResult eWeightRes       = FcPatternGetInteger(pFSet->fonts[i], FC_WEIGHT, 0, &weight);
            FcResult eSpacRes         = FcPatternGetInteger(pFSet->fonts[i], FC_SPACING, 0, &spacing);
            FcResult eOutRes          = FcPatternGetBool(pFSet->fonts[i], FC_OUTLINE, 0, &outline);
            FcResult eIndexRes        = FcPatternGetInteger(pFSet->fonts[i], FC_INDEX, 0, &nCollectionEntry);
            FcResult eFormatRes       = FcPatternGetString(pFSet->fonts[i], FC_FONTFORMAT, 0, &format);

            if( eFileRes != FcResultMatch || eFamilyRes != FcResultMatch || eOutRes != FcResultMatch )
                continue;

#if (OSL_DEBUG_LEVEL > 2)
            fprintf( stderr, "found font \"%s\" in file %s\n"
                     "   weight = %d, slant = %d, style = \"%s\"\n"
                     "   spacing = %d, outline = %d, format %s\n"
                     , family, file
                     , eWeightRes == FcResultMatch ? weight : -1
                     , eSpacRes == FcResultMatch ? slant : -1
                     , eStyleRes == FcResultMatch ? (const char*) style : "<nil>"
                     , eSpacRes == FcResultMatch ? spacing : -1
                     , eOutRes == FcResultMatch ? outline : -1
                     , eFormatRes == FcResultMatch ? (const char*)format : "<unknown>"
                     );
#endif

//            OSL_ASSERT(eOutRes != FcResultMatch || outline);

            // only outline fonts are usable to psprint anyway
            if( eOutRes == FcResultMatch && ! outline )
                continue;

            if (isPreviouslyDuplicateOrObsoleted(pFSet, i))
            {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "Ditching %s as duplicate/obsolete\n", file);
#endif
                continue;
            }

            // see if this font is already cached
            // update attributes
            std::list< PrintFont* > aFonts;
            OString aDir, aBase, aOrgPath( (sal_Char*)file );
            splitPath( aOrgPath, aDir, aBase );

            o_rVisitedPaths[aDir] = 1;

            int nDirID = getDirectoryAtom( aDir, true );
            if( ! m_pFontCache->getFontCacheFile( nDirID, aBase, aFonts ) )
            {
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "file %s not cached\n", aBase.getStr() );
#endif
                // not known, analyze font file to get attributes
                // not described by fontconfig (e.g. alias names, PSName)
                if (eFormatRes != FcResultMatch)
                    format = NULL;
                analyzeFontFile( nDirID, aBase, aFonts, (const char*)format );
#if OSL_DEBUG_LEVEL > 1
                if( aFonts.empty() )
                    fprintf( stderr, "Warning: file \"%s\" is unusable to psprint\n", aOrgPath.getStr() );
#endif
            }
            if( aFonts.empty() )
            {
                //remove font, reuse index
                //we want to remove unusable fonts here, in case there is a usable font
                //which duplicates the properties of the unusable one
                //
                //not removing the unusable font will risk the usable font being rejected
                //as a duplicate by isPreviouslyDuplicateOrObsoleted
                lcl_FcFontSetRemove(pFSet, i--);
                continue;
            }

            int nFamilyName = m_pAtoms->getAtom( ATOM_FAMILYNAME, OStringToOUString( OString( (sal_Char*)family ), RTL_TEXTENCODING_UTF8 ), sal_True );
            PrintFont* pUpdate = aFonts.front();
            std::list<PrintFont*>::const_iterator second_font = aFonts.begin();
            ++second_font;
            if( second_font != aFonts.end() ) // more than one font
            {
                // a collection entry, get the correct index
                if( eIndexRes == FcResultMatch && nCollectionEntry != -1 )
                {
                    for( std::list< PrintFont* >::iterator it = aFonts.begin(); it != aFonts.end(); ++it )
                    {
                        if( (*it)->m_eType == fonttype::TrueType &&
                            static_cast<TrueTypeFontFile*>(*it)->m_nCollectionEntry == nCollectionEntry )
                        {
                            pUpdate = *it;
                            break;
                        }
                    }
                    // update collection entry
                    // additional entries will be created in the cache
                    // if this is a new index (that is if the loop above
                    // ran to the end of the list)
                    if( pUpdate->m_eType == fonttype::TrueType ) // sanity check, this should always be the case here
                        static_cast<TrueTypeFontFile*>(pUpdate)->m_nCollectionEntry = nCollectionEntry;
                }
                else
                {
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "multiple fonts for file, but no index in fontconfig pattern ! (index res = %d collection entry = %d\nfile will not be used\n", eIndexRes, nCollectionEntry );
#endif
                    // we have found more than one font in this file
                    // but fontconfig will not tell us which index is meant
                    // -> something is in disorder, do not use this font
                    pUpdate = NULL;
                }
            }

            if( pUpdate )
            {
                // set family name
                if( pUpdate->m_nFamilyName != nFamilyName )
                {
                }
                if( eWeightRes == FcResultMatch )
                    pUpdate->m_eWeight = convertWeight(weight);
                if( eSpacRes == FcResultMatch )
                    pUpdate->m_ePitch = convertSpacing(spacing);
                if( eSlantRes == FcResultMatch )
                    pUpdate->m_eItalic = convertSlant(slant);
                if( eStyleRes == FcResultMatch )
                {
                    pUpdate->m_aStyleName = OStringToOUString( OString( (sal_Char*)style ), RTL_TEXTENCODING_UTF8 );
                }

                // update font cache
                m_pFontCache->updateFontCacheEntry( pUpdate, false );
                // sort into known fonts
                fontID aFont = m_nNextFontID++;
                m_aFonts[ aFont ] = pUpdate;
                m_aFontFileToFontID[ aBase ].insert( aFont );
#if OSL_DEBUG_LEVEL > 1
                nFonts++;
#endif
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "inserted font %s as fontID %d\n", family, aFont );
#endif
            }
            // clean up the fonts we did not put into the list
            for( std::list< PrintFont* >::iterator it = aFonts.begin(); it != aFonts.end(); ++it )
            {
                if( *it != pUpdate )
                {
                    m_pFontCache->updateFontCacheEntry( *it, false ); // prepare a cache entry for a collection item
                    delete *it;
                }
            }
        }
    }

    // how does one get rid of the config ?
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "inserted %d fonts from fontconfig\n", nFonts );
#endif
}

void PrintFontManager::deinitFontconfig()
{
    FontCfgWrapper::release();
}

bool PrintFontManager::addFontconfigDir( const OString& rDirName )
{
    // workaround for a stability problems in older FC versions
    // when handling application specifc fonts
    const int nVersion = FcGetVersion();
    if( nVersion <= 20400 )
        return false;
    const char* pDirName = (const char*)rDirName.getStr();
    bool bDirOk = (FcConfigAppFontAddDir(FcConfigGetCurrent(), (FcChar8*)pDirName ) == FcTrue);

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "FcConfigAppFontAddDir( \"%s\") => %d\n", pDirName, bDirOk );
#endif

    if( !bDirOk )
        return false;

    // load dir-specific fc-config file too if available
    const OString aConfFileName = rDirName + "/fc_local.conf";
    FILE* pCfgFile = fopen( aConfFileName.getStr(), "rb" );
    if( pCfgFile )
    {
        fclose( pCfgFile);
        bool bCfgOk = FcConfigParseAndLoad(FcConfigGetCurrent(),
                        (FcChar8*)aConfFileName.getStr(), FcTrue);
        if( !bCfgOk )
            fprintf( stderr, "FcConfigParseAndLoad( \"%s\") => %d\n", aConfFileName.getStr(), bCfgOk );
    }

    return true;
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
            FcPatternAddString(pPattern, FC_FAMILY, (FcChar8*)"monospace");
    }
}

namespace
{
    //Someday fontconfig will hopefully use bcp47, see fdo#19869
    //In the meantime try something that will fit to workaround fdo#35118
    OString mapToFontConfigLangTag(const LanguageTag &rLangTag)
    {
#if defined(FC_VERSION) && (FC_VERSION >= 20492)
        boost::shared_ptr<FcStrSet> xLangSet(FcGetLangs(), FcStrSetDestroy);
        OString sLangAttrib;

        sLangAttrib = OUStringToOString(rLangTag.getBcp47(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        if (FcStrSetMember(xLangSet.get(), (const FcChar8*)sLangAttrib.getStr()))
        {
            return sLangAttrib;
        }

        sLangAttrib = OUStringToOString(rLangTag.getLanguageAndScript(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        if (FcStrSetMember(xLangSet.get(), (const FcChar8*)sLangAttrib.getStr()))
        {
            return sLangAttrib;
        }

        OString sLang = OUStringToOString(rLangTag.getLanguage(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();
        OString sRegion = OUStringToOString(rLangTag.getCountry(), RTL_TEXTENCODING_UTF8).toAsciiLowerCase();

        if (!sRegion.isEmpty())
        {
            sLangAttrib = sLang + OString('-') + sRegion;
            if (FcStrSetMember(xLangSet.get(), (const FcChar8*)sLangAttrib.getStr()))
            {
                return sLangAttrib;
            }
        }

        if (FcStrSetMember(xLangSet.get(), (const FcChar8*)sLang.getStr()))
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
        SAL_WARN_IF(bIsImpossible, "vcl", "In glyph fallback throwing away the language property of "
            << sLang << " because the detected script for '0x"
            << OUString::number(currentChar, 16)
            << "' is " << uscript_getName(eScript)
            << " and that language doesn't make sense. Autodetecting instead.");
        return bIsImpossible;
    }

    LanguageTag getExemplerLangTagForCodePoint(sal_uInt32 currentChar)
    {
        int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
        UScriptCode eScript = static_cast<UScriptCode>(script);
        OStringBuffer aBuf(unicode::getExemplerLanguageForUScriptCode(eScript));
        const char* pScriptCode = uscript_getShortName(eScript);
        if (pScriptCode)
            aBuf.append('-').append(pScriptCode);
        return LanguageTag(OStringToOUString(aBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
    }

#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
    guint get_xid_for_dbus()
    {
        const Window *pTopWindow = Application::IsHeadlessModeEnabled() ? NULL : Application::GetActiveTopWindow();
        const SystemEnvData* pEnvData = pTopWindow ? pTopWindow->GetSystemData() : NULL;
        return pEnvData ? pEnvData->aWindow : 0;
    }
#endif
}

IMPL_LINK_NOARG(PrintFontManager, autoInstallFontLangSupport)
{
#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
    guint xid = get_xid_for_dbus();

    if (!xid)
        return -1;

    GError *error = NULL;
    /* get the DBUS session connection */
    DBusGConnection *session_connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (error != NULL)
    {
        g_debug ("DBUS cannot connect : %s", error->message);
        g_error_free (error);
        return -1;
    }

    /* get the proxy with gnome-session-manager */
    DBusGProxy *proxy = dbus_g_proxy_new_for_name(session_connection,
                                       "org.freedesktop.PackageKit",
                                       "/org/freedesktop/PackageKit",
                                       "org.freedesktop.PackageKit.Modify");
    if (proxy == NULL)
    {
        g_debug("Could not get DBUS proxy: org.freedesktop.PackageKit");
        return -1;
    }

    gchar **fonts = (gchar**)g_malloc((m_aCurrentRequests.size() + 1) * sizeof(gchar*));
    gchar **font = fonts;
    for (std::vector<OString>::const_iterator aI = m_aCurrentRequests.begin(); aI != m_aCurrentRequests.end(); ++aI)
        *font++ = (gchar*)aI->getStr();
    *font = NULL;
    gboolean res = dbus_g_proxy_call(proxy, "InstallFontconfigResources", &error,
                 G_TYPE_UINT, xid, /* xid */
                 G_TYPE_STRV, fonts, /* data */
                 G_TYPE_STRING, "hide-finished", /* interaction */
                 G_TYPE_INVALID,
                 G_TYPE_INVALID);
    /* check the return value */
    if (!res)
       g_debug("InstallFontconfigResources method failed");

    /* check the error value */
    if (error != NULL)
    {
        g_debug("InstallFontconfigResources problem : %s", error->message);
        g_error_free(error);
    }

    g_free(fonts);
    g_object_unref(G_OBJECT (proxy));
    m_aCurrentRequests.clear();
#endif
    return 0;
}

bool PrintFontManager::Substitute( FontSelectPattern &rPattern, OUString& rMissingCodes )
{
    bool bRet = false;

    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    // build pattern argument for fontconfig query
    FcPattern* pPattern = FcPatternCreate();

    // Prefer scalable fonts
    FcPatternAddBool(pPattern, FC_SCALABLE, FcTrue);

    const OString aTargetName = OUStringToOString( rPattern.maTargetName, RTL_TEXTENCODING_UTF8 );
    const FcChar8* pTargetNameUtf8 = (FcChar8*)aTargetName.getStr();
    FcPatternAddString(pPattern, FC_FAMILY, pTargetNameUtf8);

    LanguageTag aLangTag(rPattern.meLanguage);
    OString aLangAttrib = mapToFontConfigLangTag(aLangTag);

    // Add required Unicode characters, if any
    if ( !rMissingCodes.isEmpty() )
    {
        FcCharSet *unicodes = FcCharSetCreate();
        for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
        {
            // also handle unicode surrogates
            const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
            FcCharSetAddChar( unicodes, nCode );
            //if the codepoint is impossible for this lang tag, then clear it
            //and autodetect something useful
            if (!aLangAttrib.isEmpty() && isImpossibleCodePointForLang(aLangTag, nCode))
                aLangAttrib = OString();
            //#i105784#/rhbz#527719  improve selection of fallback font
            if (aLangAttrib.isEmpty())
            {
                aLangTag = getExemplerLangTagForCodePoint(nCode);
                aLangAttrib = mapToFontConfigLangTag(aLangTag);
            }
        }
        FcPatternAddCharSet(pPattern, FC_CHARSET, unicodes);
        FcCharSetDestroy(unicodes);
    }

    if (!aLangAttrib.isEmpty())
        FcPatternAddString(pPattern, FC_LANG, (FcChar8*)aLangAttrib.getStr());

    addtopattern(pPattern, rPattern.GetSlant(), rPattern.GetWeight(),
        rPattern.GetWidthType(), rPattern.GetPitch());

    // query fontconfig for a substitute
    FcConfigSubstitute(FcConfigGetCurrent(), pPattern, FcMatchPattern);
    FcDefaultSubstitute(pPattern);

    // process the result of the fontconfig query
    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch(FcConfigGetCurrent(), &pFontSet, 1, pPattern, &eResult);
    FcPatternDestroy( pPattern );

    FcFontSet*  pSet = NULL;
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
            //extract the closest match
            FcChar8* file = NULL;
            FcResult eFileRes = FcPatternGetString(pSet->fonts[0], FC_FILE, 0, &file);
            int nCollectionEntry = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nCollectionEntry);
            if (eIndexRes != FcResultMatch)
                nCollectionEntry = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( (sal_Char*)file );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir, true );
                fontID aFont = findFontFileID( nDirID, aBase, nCollectionEntry );
                if( aFont > 0 )
                {
                    FastPrintFontInfo aInfo;
                    bRet = getFontFastInfo( aFont, aInfo );
                    rPattern.maSearchName = aInfo.m_aFamilyName;
                }
            }

            SAL_WARN_IF(!bRet, "vcl", "no FC_FILE found, falling back to name search");

            if (!bRet)
            {
                FcChar8* family = NULL;
                FcResult eFamilyRes = FcPatternGetString( pSet->fonts[0], FC_FAMILY, 0, &family );

                // get the family name
                if( eFamilyRes == FcResultMatch )
                {
                    OString sFamily((sal_Char*)family);
                    boost::unordered_map< OString, OString, OStringHash >::const_iterator aI =
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
                FcMatrix *pMatrix = 0;
                if (FcResultMatch == FcPatternGetMatrix(pSet->fonts[0], FC_MATRIX, 0, &pMatrix))
                {
                    rPattern.maItalicMatrix.xx = pMatrix->xx;
                    rPattern.maItalicMatrix.xy = pMatrix->xy;
                    rPattern.maItalicMatrix.yx = pMatrix->yx;
                    rPattern.maItalicMatrix.yy = pMatrix->yy;
                }
            }

            // update rMissingCodes by removing resolved unicodes
            if( !rMissingCodes.isEmpty() )
            {
                sal_uInt32* pRemainingCodes = (sal_uInt32*)alloca( rMissingCodes.getLength() * sizeof(sal_uInt32) );
                int nRemainingLen = 0;
                FcCharSet* unicodes;
                if (!FcPatternGetCharSet(pSet->fonts[0], FC_CHARSET, 0, &unicodes))
                {
                    for( sal_Int32 nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
                    {
                        // also handle unicode surrogates
                        const sal_uInt32 nCode = rMissingCodes.iterateCodePoints( &nStrIndex );
                        if (FcCharSetHasChar(unicodes, nCode) != FcTrue)
                            pRemainingCodes[ nRemainingLen++ ] = nCode;
                    }
                }
                OUString sStillMissing(pRemainingCodes, nRemainingLen);
#if defined(ENABLE_DBUS) && defined(ENABLE_PACKAGEKIT)
                if (get_xid_for_dbus())
                {
                    if (sStillMissing == rMissingCodes) //replaced nothing
                    {
                        //It'd be better if we could ask packagekit using the
                        //missing codepoints or some such rather than using
                        //"language" as a proxy to how fontconfig considers
                        //scripts to default to a given language.
                        for (sal_Int32 i = 0; i < nRemainingLen; ++i)
                        {
                            LanguageTag aOurTag = getExemplerLangTagForCodePoint(pRemainingCodes[i]);
                            OString sTag = OUStringToOString(aOurTag.getBcp47(), RTL_TEXTENCODING_UTF8);
                            if (m_aPreviousLangSupportRequests.find(sTag) != m_aPreviousLangSupportRequests.end())
                                continue;
                            m_aPreviousLangSupportRequests.insert(sTag);
                            sTag = mapToFontConfigLangTag(aOurTag);
                            if (!sTag.isEmpty() && m_aPreviousLangSupportRequests.find(sTag) == m_aPreviousLangSupportRequests.end())
                            {
                                OString sReq = OString(":lang=") + sTag;
                                m_aCurrentRequests.push_back(sReq);
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
#endif
                rMissingCodes = sStillMissing;
            }
        }

        FcFontSetDestroy( pSet );
    }

    return bRet;
}

class FontConfigFontOptions : public ImplFontOptions
{
public:
    FontConfigFontOptions() : mpPattern(0) {}
    ~FontConfigFontOptions()
    {
        FcPatternDestroy(mpPattern);
    }
    virtual void *GetPattern(void * face, bool bEmbolden, bool /*bVerticalLayout*/) const
    {
        FcValue value;
        value.type = FcTypeFTFace;
        value.u.f = face;
        FcPatternDel(mpPattern, FC_FT_FACE);
        FcPatternAdd (mpPattern, FC_FT_FACE, value, FcTrue);
        FcPatternDel(mpPattern, FC_EMBOLDEN);
        FcPatternAddBool(mpPattern, FC_EMBOLDEN, bEmbolden ? FcTrue : FcFalse);
#if 0
        FcPatternDel(mpPattern, FC_VERTICAL_LAYOUT);
        FcPatternAddBool(mpPattern, FC_VERTICAL_LAYOUT, bVerticalLayout ? FcTrue : FcFalse);
#endif
        return mpPattern;
    }
    FcPattern* mpPattern;
};

ImplFontOptions* PrintFontManager::getFontOptions(
    const FastPrintFontInfo& rInfo, int nSize, void (*subcallback)(void*)) const
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FontConfigFontOptions* pOptions = NULL;
    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    OString sFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );

    boost::unordered_map< OString, OString, OStringHash >::const_iterator aI = rWrapper.m_aLocalizedToCanonical.find(sFamily);
    if (aI != rWrapper.m_aLocalizedToCanonical.end())
        sFamily = aI->second;
    if( !sFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, (FcChar8*)sFamily.getStr());

    addtopattern(pPattern, rInfo.m_eItalic, rInfo.m_eWeight, rInfo.m_eWidth, rInfo.m_ePitch);
    FcPatternAddDouble(pPattern, FC_PIXEL_SIZE, nSize);

    FcBool embitmap = true, antialias = true, autohint = true, hinting = true;
    int hintstyle = FC_HINT_FULL;

    FcConfigSubstitute(pConfig, pPattern, FcMatchPattern);
    if (subcallback)
        subcallback(pPattern);
    FcDefaultSubstitute(pPattern);

    FcResult eResult = FcResultNoMatch;
    FcFontSet* pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch( pConfig, &pFontSet, 1, pPattern, &eResult );
    if( pResult )
    {
        FcResult eEmbeddedBitmap = FcPatternGetBool(pResult,
            FC_EMBEDDED_BITMAP, 0, &embitmap);
        FcResult eAntialias = FcPatternGetBool(pResult,
            FC_ANTIALIAS, 0, &antialias);
        FcResult eAutoHint = FcPatternGetBool(pResult,
            FC_AUTOHINT, 0, &autohint);
        FcResult eHinting = FcPatternGetBool(pResult,
            FC_HINTING, 0, &hinting);
        /*FcResult eHintStyle =*/ FcPatternGetInteger(pResult,
            FC_HINT_STYLE, 0, &hintstyle);

        pOptions = new FontConfigFontOptions;

        pOptions->mpPattern = pResult;

        if( eEmbeddedBitmap == FcResultMatch )
            pOptions->meEmbeddedBitmap = embitmap ? EMBEDDEDBITMAP_TRUE : EMBEDDEDBITMAP_FALSE;
        if( eAntialias == FcResultMatch )
            pOptions->meAntiAlias = antialias ? ANTIALIAS_TRUE : ANTIALIAS_FALSE;
        if( eAutoHint == FcResultMatch )
            pOptions->meAutoHint = autohint ? AUTOHINT_TRUE : AUTOHINT_FALSE;
        if( eHinting == FcResultMatch )
            pOptions->meHinting = hinting ? HINTING_TRUE : HINTING_FALSE;
        switch (hintstyle)
        {
            case FC_HINT_NONE:   pOptions->meHintStyle = HINT_NONE; break;
            case FC_HINT_SLIGHT: pOptions->meHintStyle = HINT_SLIGHT; break;
            case FC_HINT_MEDIUM: pOptions->meHintStyle = HINT_MEDIUM; break;
            default: // fall through
            case FC_HINT_FULL:   pOptions->meHintStyle = HINT_FULL; break;
        }
    }

    // cleanup
    FcPatternDestroy( pPattern );

    return pOptions;
}

bool PrintFontManager::matchFont( FastPrintFontInfo& rInfo, const com::sun::star::lang::Locale& rLocale )
{
    FontCfgWrapper& rWrapper = FontCfgWrapper::get();

    FcConfig* pConfig = FcConfigGetCurrent();
    FcPattern* pPattern = FcPatternCreate();

    // populate pattern with font characteristics
    const LanguageTag aLangTag(rLocale);
    const OString aLangAttrib = mapToFontConfigLangTag(aLangTag);
    if (!aLangAttrib.isEmpty())
        FcPatternAddString(pPattern, FC_LANG, (FcChar8*)aLangAttrib.getStr());

    OString aFamily = OUStringToOString( rInfo.m_aFamilyName, RTL_TEXTENCODING_UTF8 );
    if( !aFamily.isEmpty() )
        FcPatternAddString(pPattern, FC_FAMILY, (FcChar8*)aFamily.getStr());

    addtopattern(pPattern, rInfo.m_eItalic, rInfo.m_eWeight, rInfo.m_eWidth, rInfo.m_ePitch);

    FcConfigSubstitute(pConfig, pPattern, FcMatchPattern);
    FcDefaultSubstitute(pPattern);
    FcResult eResult = FcResultNoMatch;
    FcFontSet *pFontSet = rWrapper.getFontSet();
    FcPattern* pResult = FcFontSetMatch(pConfig, &pFontSet, 1, pPattern, &eResult);
    bool bSuccess = false;
    if( pResult )
    {
        FcFontSet* pSet = FcFontSetCreate();
        FcFontSetAdd( pSet, pResult );
        if( pSet->nfont > 0 )
        {
            //extract the closest match
            FcChar8* file = NULL;
            FcResult eFileRes = FcPatternGetString(pSet->fonts[0], FC_FILE, 0, &file);
            int nCollectionEntry = 0;
            FcResult eIndexRes = FcPatternGetInteger(pSet->fonts[0], FC_INDEX, 0, &nCollectionEntry);
            if (eIndexRes != FcResultMatch)
                nCollectionEntry = 0;
            if( eFileRes == FcResultMatch )
            {
                OString aDir, aBase, aOrgPath( (sal_Char*)file );
                splitPath( aOrgPath, aDir, aBase );
                int nDirID = getDirectoryAtom( aDir, true );
                fontID aFont = findFontFileID( nDirID, aBase, nCollectionEntry );
                if( aFont > 0 )
                    bSuccess = getFontFastInfo( aFont, rInfo );
            }
        }
        // info: destroying the pSet destroys pResult implicitly
        // since pResult was "added" to pSet
        FcFontSetDestroy( pSet );
    }

    // cleanup
    FcPatternDestroy( pPattern );

    return bSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
