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

#include <gcach_ftyp.hxx>
#include <sallayout.hxx>
#include <salgdi.hxx>

#include <boost/static_assert.hpp>

#include <i18npool/mslangid.hxx>

#include <vcl/svapp.hxx>

#include <sal/alloca.h>
#include <rtl/instance.hxx>

#include <layout/LayoutEngine.h>
#include <layout/LEFontInstance.h>
#include <layout/LELanguages.h>
#include <layout/LEScripts.h>

#include <unicode/uscript.h>
#include <unicode/ubidi.h>

// =======================================================================
// layout implementation for ServerFont
// =======================================================================

ServerFontLayout::ServerFontLayout( ServerFont& rFont )
:   mrServerFont( rFont )
{}

void ServerFontLayout::DrawText( SalGraphics& rSalGraphics ) const
{
    rSalGraphics.DrawServerFontLayout( *this );
}

// -----------------------------------------------------------------------

bool ServerFontLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    ServerFontLayoutEngine* pLE = mrServerFont.GetLayoutEngine();
    assert(pLE);
    bool bRet = pLE ? pLE->layout(*this, rArgs) : false;
    return bRet;
}

// -----------------------------------------------------------------------

void ServerFontLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    GenericSalLayout::AdjustLayout( rArgs );

    // apply asian kerning if the glyphs are not already formatted
    if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN)
    && !(rArgs.mnFlags & SAL_LAYOUT_VERTICAL) )
        if( (rArgs.mpDXArray != NULL) || (rArgs.mnLayoutWidth != 0) )
            ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );

    // insert kashidas where requested by the formatting array
    if( (rArgs.mnFlags & SAL_LAYOUT_KASHIDA_JUSTIFICATON) && rArgs.mpDXArray )
    {
        int nKashidaIndex = mrServerFont.GetGlyphIndex( 0x0640 );
        if( nKashidaIndex != 0 )
        {
            const GlyphMetric& rGM = mrServerFont.GetGlyphMetric( nKashidaIndex );
            KashidaJustify( nKashidaIndex, rGM.GetCharWidth() );
            // TODO: kashida-GSUB/GPOS
        }
    }
}

// =======================================================================
// bridge to ICU LayoutEngine
// =======================================================================

using namespace U_ICU_NAMESPACE;

static const LEGlyphID ICU_DELETED_GLYPH = 0xFFFF;
static const LEGlyphID ICU_MARKED_GLYPH = 0xFFFE;

// -----------------------------------------------------------------------

class IcuFontFromServerFont
: public LEFontInstance
{
private:
    ServerFont&     mrServerFont;

public:
                            IcuFontFromServerFont( ServerFont& rFont )
                            : mrServerFont( rFont )
                            {}

    using LEFontInstance::getFontTable;
    virtual const void*     getFontTable(LETag tableTag) const;
    virtual le_int32        getUnitsPerEM() const;
    virtual float           getXPixelsPerEm() const;
    virtual float           getYPixelsPerEm() const;
    virtual float           getScaleFactorX() const;
    virtual float           getScaleFactorY() const;

    using LEFontInstance::mapCharToGlyph;
    virtual LEGlyphID       mapCharToGlyph( LEUnicode32 ch ) const;
    virtual LEGlyphID       mapCharToGlyph( LEUnicode32 ch, const LECharMapper *mapper, le_bool filterZeroWidth ) const;

    virtual le_int32        getAscent() const;
    virtual le_int32        getDescent() const;
    virtual le_int32        getLeading() const;

    virtual void            getGlyphAdvance( LEGlyphID glyph, LEPoint &advance ) const;
    virtual le_bool         getGlyphPoint( LEGlyphID glyph, le_int32 pointNumber, LEPoint& point ) const;
};

// -----------------------------------------------------------------------

const void* IcuFontFromServerFont::getFontTable( LETag nICUTableTag ) const
{
    char pTagName[5];
    pTagName[0] = (char)(nICUTableTag >> 24);
    pTagName[1] = (char)(nICUTableTag >> 16);
    pTagName[2] = (char)(nICUTableTag >>  8);
    pTagName[3] = (char)(nICUTableTag);
    pTagName[4] = 0;

    sal_uLong nLength;
    const unsigned char* pBuffer = mrServerFont.GetTable( pTagName, &nLength );
    SAL_INFO("vcl", "IcuGetTable(\"" << pTagName << "\") => " << pBuffer);
    SAL_INFO(
        "vcl",
        "font( h=" << mrServerFont.GetFontSelData().mnHeight << ", \""
        << mrServerFont.GetFontFileName()->getStr() << "\" )");
    return pBuffer;
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getUnitsPerEM() const
{
    return mrServerFont.GetEmUnits();
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getXPixelsPerEm() const
{
    const FontSelectPattern& r = mrServerFont.GetFontSelData();
    float fX = r.mnWidth ? r.mnWidth : r.mnHeight;
    return fX;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getYPixelsPerEm() const
{
    float fY = mrServerFont.GetFontSelData().mnHeight;
    return fY;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getScaleFactorX() const
{
    return 1.0;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getScaleFactorY() const
{
    return 1.0;
}

// -----------------------------------------------------------------------

LEGlyphID IcuFontFromServerFont::mapCharToGlyph( LEUnicode32 ch ) const
{
    LEGlyphID nGlyphIndex = mrServerFont.GetRawGlyphIndex( ch );
    return nGlyphIndex;
}

LEGlyphID IcuFontFromServerFont::mapCharToGlyph( LEUnicode32 ch, const LECharMapper *mapper, le_bool /*filterZeroWidth*/ ) const
{
    /*
     fdo#31821, icu has...
      >│93          if (filterZeroWidth && (mappedChar == 0x200C || mappedChar == 0x200D)) {                                            │
       │94              return canDisplay(mappedChar) ? 0x0001 : 0xFFFF;                                                                 │
       │95          }
     so only the Indic layouts allow the joiners to get mapped to glyphs
    */
    return LEFontInstance::mapCharToGlyph( ch, mapper, false );
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getAscent() const
{
    const FT_Size_Metrics& rMetrics = mrServerFont.GetMetricsFT();
    le_int32 nAscent = (+rMetrics.ascender + 32) >> 6;
    return nAscent;
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getDescent() const
{
    const FT_Size_Metrics& rMetrics = mrServerFont.GetMetricsFT();
    le_int32 nDescent = (-rMetrics.descender + 32) >> 6;
    return nDescent;
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getLeading() const
{
    const FT_Size_Metrics& rMetrics = mrServerFont.GetMetricsFT();
    le_int32 nLeading = ((rMetrics.height - rMetrics.ascender + rMetrics.descender) + 32) >> 6;
    return nLeading;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::getGlyphAdvance( LEGlyphID nGlyphIndex,
    LEPoint &advance ) const
{
    if( (nGlyphIndex == ICU_MARKED_GLYPH)
    ||  (nGlyphIndex == ICU_DELETED_GLYPH) )
    {
        // deleted glyph or mark glyph has not advance
        advance.fX = 0;
    }
    else
    {
        const GlyphMetric& rGM = mrServerFont.GetGlyphMetric( nGlyphIndex );
        advance.fX = rGM.GetCharWidth();
    }

    advance.fY = 0;
}

// -----------------------------------------------------------------------

le_bool IcuFontFromServerFont::getGlyphPoint( LEGlyphID,
    le_int32 pointNumber, LEPoint& ) const
{
    //TODO: replace dummy implementation
    SAL_INFO("vcl", "getGlyphPoint(" << pointNumber << ")");
    return false;
}

// =======================================================================

class IcuLayoutEngine : public ServerFontLayoutEngine
{
private:
    IcuFontFromServerFont   maIcuFont;

    LanguageCodes           meLanguageCode;
    le_int32                meScriptCode;
    le_int32                mnLayoutFlags;
    LayoutEngine*           mpIcuLE;

public:
                            IcuLayoutEngine( ServerFont& );
    virtual                 ~IcuLayoutEngine();

    virtual bool            layout( ServerFontLayout&, ImplLayoutArgs& );
};

// -----------------------------------------------------------------------

IcuLayoutEngine::IcuLayoutEngine( ServerFont& rServerFont )
:   maIcuFont( rServerFont ),
    meLanguageCode( nullLanguageCode ),
    meScriptCode( USCRIPT_INVALID_CODE ),
    mnLayoutFlags( 0 ),
    mpIcuLE( NULL )
{}

// -----------------------------------------------------------------------

IcuLayoutEngine::~IcuLayoutEngine()
{
    delete mpIcuLE;
}

// -----------------------------------------------------------------------

static bool lcl_CharIsJoiner(sal_Unicode cChar)
{
    return ((cChar == 0x200C) || (cChar == 0x200D));
}

static bool needPreviousCode(sal_Unicode cChar)
{
    return lcl_CharIsJoiner(cChar) || U16_IS_LEAD(cChar);
}

static bool needNextCode(sal_Unicode cChar)
{
    return lcl_CharIsJoiner(cChar) || U16_IS_TRAIL(cChar);
}

namespace
{
    LanguageCodes mapLanguageTypetoICU(LanguageType eLangCode)
    {
        LanguageTag aLangTag(eLangCode);
        OUString sLanguage = aLangTag.getLanguage();

        if (sLanguage == "af") // Afrikaans
            return afkLanguageCode;
        else if (sLanguage == "ar") // Arabic
            return araLanguageCode;
        else if (sLanguage == "as") // Assamese
            return asmLanguageCode;
        else if (sLanguage == "be") // Belarussian
            return belLanguageCode;
        else if (sLanguage == "bn") // Bengali
            return benLanguageCode;
        else if (sLanguage == "bo") // Tibetan
            return tibLanguageCode;
        else if (sLanguage == "bu") // Bulgarian
            return bgrLanguageCode;
        else if (sLanguage == "ca") // Catalan
            return catLanguageCode;
        else if (sLanguage == "cs") // Czech
            return csyLanguageCode;
        else if (sLanguage == "ch") // Chechen
            return cheLanguageCode;
        else if (sLanguage == "co") // Coptic
            return copLanguageCode;
        else if (sLanguage == "cy") // Welsh
            return welLanguageCode;
        else if (sLanguage == "da") // Danish
            return danLanguageCode;
        else if (sLanguage == "de") // German
            return deuLanguageCode;
        else if (sLanguage == "dz") // Dzongkha
            return dznLanguageCode;
        else if (sLanguage == "el") // Greek
            return ellLanguageCode;
        else if (sLanguage == "en") // English
            return engLanguageCode;
        else if (sLanguage == "et") // Estonian
            return etiLanguageCode;
        else if (sLanguage == "eu") // Basque
            return euqLanguageCode;
        else if (sLanguage == "fa") // Farsi
            return farLanguageCode;
        else if (sLanguage == "fi") // Finnish
            return finLanguageCode;
        else if (sLanguage == "fr") // French
            return fraLanguageCode;
        else if (sLanguage == "ga") // Irish Gaelic
            return gaeLanguageCode;
        else if (sLanguage == "gu") // Gujarati
            return gujLanguageCode;
        else if (sLanguage == "ha") // Hausa
            return hauLanguageCode;
        else if (sLanguage == "he") // Hebrew
            return iwrLanguageCode;
        else if (sLanguage == "hi") // Hindi
            return hinLanguageCode;
        else if (sLanguage == "hr") // Croatian
            return hrvLanguageCode;
        else if (sLanguage == "hu") // Hungarian
            return hunLanguageCode;
        else if (sLanguage == "hy") // Armenian
            return hyeLanguageCode;
        else if (sLanguage == "id") // Indonesian
            return indLanguageCode;
        else if (sLanguage == "it") // Italian
            return itaLanguageCode;
        else if (sLanguage == "ja") // Japanese
            return janLanguageCode;
        else if (sLanguage == "kn") // Kannada
            return kanLanguageCode;
        else if (sLanguage == "ks") // Kashmiri
            return kshLanguageCode;
        else if (sLanguage == "kh") // Khmer
            return khmLanguageCode;
        else if (sLanguage == "kok") // Konkani
            return kokLanguageCode;
        else if (sLanguage == "ko") // Korean
            return korLanguageCode;
        else if (sLanguage == "ml") // Malayalam - Reformed (should there be some bcp47 tag for Traditional Malayalam)
            return mlrLanguageCode;
        else if (sLanguage == "mr") // Marathi
            return marLanguageCode;
        else if (sLanguage == "mt") // Maltese
            return mtsLanguageCode;
        else if (sLanguage == "mni") // Manipuri
            return mniLanguageCode;
        else if (sLanguage == "mn") // Mongolian
            return mngLanguageCode;
        else if (sLanguage == "ne") // Nepali
            return nepLanguageCode;
        else if (sLanguage == "or") // Oriya
            return oriLanguageCode;
        else if (sLanguage == "pl") // Polish
            return plkLanguageCode;
        else if (sLanguage == "po") // Portuguese
            return ptgLanguageCode;
        else if (sLanguage == "ps") // Pashto
            return pasLanguageCode;
        else if (sLanguage == "ro") // Romanian
            return romLanguageCode;
        else if (sLanguage == "ru") // Russian
            return rusLanguageCode;
        else if (sLanguage == "sa") // Sanskrit
            return sanLanguageCode;
        else if (sLanguage == "si") // Sinhalese
            return snhLanguageCode;
        else if (sLanguage == "sk") // Slovak
            return skyLanguageCode;
        else if (sLanguage == "sd") // Sindhi
            return sndLanguageCode;
        else if (sLanguage == "sl") // Slovenian
            return slvLanguageCode;
        else if (sLanguage == "es") // Spanish
            return espLanguageCode;
        else if (sLanguage == "sq") // Albanian
            return sqiLanguageCode;
        else if (sLanguage == "sr") // Serbian
            return srbLanguageCode;
        else if (sLanguage == "sv") // Swedish
            return sveLanguageCode;
        else if (sLanguage == "syr") // Syriac
            return syrLanguageCode;
        else if (sLanguage == "ta") // Tamil
            return tamLanguageCode;
        else if (sLanguage == "te") // Telugu
            return telLanguageCode;
        else if (sLanguage == "th") // Thai
            return thaLanguageCode;
        else if (sLanguage == "tu") // Turkish
            return trkLanguageCode;
        else if (sLanguage == "ur") // Urdu
            return urdLanguageCode;
        else if (sLanguage == "yi") // Yiddish
            return jiiLanguageCode;
        else if (sLanguage == "zh") // Chinese
        {
            OUString sScript = aLangTag.getScript();
            if (sScript.isEmpty())
            {
                if (MsLangId::isTraditionalChinese(eLangCode))
                    sScript = "Hant";
                else
                    sScript = "Hans";
            }
            if (sScript == "Latn")
                return zhpLanguageCode;
            else if (sScript == "Hans")
                return zhsLanguageCode;
            else if (sScript == "Hant")
                return zhtLanguageCode;
        }

        //if there are new ones, please reexamine the mapping list for the new ones
        BOOST_STATIC_ASSERT(languageCodeCount == 72);
        return nullLanguageCode;
    }
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=31016
#define ARABIC_BANDAID

bool IcuLayoutEngine::layout(ServerFontLayout& rLayout, ImplLayoutArgs& rArgs)
{
    le_int32 nLayoutFlags = 0;
#if (U_ICU_VERSION_MAJOR_NUM > 4)
    if (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS)
        nLayoutFlags |= LayoutEngine::kTypoFlagKern;
    if (rArgs.mnFlags & SAL_LAYOUT_ENABLE_LIGATURES)
        nLayoutFlags |= LayoutEngine::kTypoFlagLiga;
#else
    if (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS)
        nLayoutFlags |= 0x01;
    if (rArgs.mnFlags & SAL_LAYOUT_ENABLE_LIGATURES)
        nLayoutFlags |= 0x10;
#endif

    LEUnicode* pIcuChars;
    if( sizeof(LEUnicode) == sizeof(*rArgs.mpStr) )
        pIcuChars = (LEUnicode*)rArgs.mpStr;
    else
    {
        // this conversion will only be needed when either
        // ICU's or OOo's unicodes stop being unsigned shorts
        // TODO: watch out for surrogates!
        pIcuChars = (LEUnicode*)alloca( rArgs.mnLength * sizeof(LEUnicode) );
        for( xub_StrLen ic = 0; ic < rArgs.mnLength; ++ic )
            pIcuChars[ic] = static_cast<LEUnicode>( rArgs.mpStr[ic] );
    }

    // allocate temporary arrays, note: round to even
    int nGlyphCapacity = (3 * (rArgs.mnEndCharPos - rArgs.mnMinCharPos ) | 15) + 1;

    rLayout.Reserve(nGlyphCapacity);

    struct IcuPosition{ float fX, fY; };
    const int nAllocSize = sizeof(LEGlyphID) + sizeof(le_int32) + sizeof(IcuPosition);
    LEGlyphID* pIcuGlyphs = (LEGlyphID*)alloca( (nGlyphCapacity * nAllocSize) + sizeof(IcuPosition) );
    le_int32* pCharIndices = (le_int32*)((char*)pIcuGlyphs + nGlyphCapacity * sizeof(LEGlyphID) );
    IcuPosition* pGlyphPositions = (IcuPosition*)((char*)pCharIndices + nGlyphCapacity * sizeof(le_int32) );

    ServerFont& rFont = rLayout.GetServerFont();

    UErrorCode rcI18n = U_ZERO_ERROR;
    LEErrorCode rcIcu = LE_NO_ERROR;
    Point aNewPos( 0, 0 );
    for( int nGlyphCount = 0;; )
    {
        int nMinRunPos, nEndRunPos;
        bool bRightToLeft;
        if( !rArgs.GetNextRun( &nMinRunPos, &nEndRunPos, &bRightToLeft ) )
            break;

        // find matching script
        // TODO: split up bidi run into script runs
        le_int32 eScriptCode = -1;
        for( int i = nMinRunPos; i < nEndRunPos; ++i )
        {
            le_int32 eNextScriptCode = uscript_getScript( pIcuChars[i], &rcI18n );
            if( (eNextScriptCode > USCRIPT_INHERITED) )
            {
                eScriptCode = eNextScriptCode;
                if (eNextScriptCode != latnScriptCode)
                    break;
            }
        }
        if( eScriptCode < 0 )   // TODO: handle errors better
            eScriptCode = latnScriptCode;

        LanguageCodes eLanguageCode = mapLanguageTypetoICU(rArgs.meLanguage);

        // get layout engine matching to this script and ligature/kerning combination
        // no engine change necessary if script is latin
        if ( !mpIcuLE ||
             ((eScriptCode != meScriptCode) && (eScriptCode > USCRIPT_INHERITED)) ||
             (mnLayoutFlags != nLayoutFlags) || (meLanguageCode != eLanguageCode) )
        {
            // TODO: cache multiple layout engines when multiple scripts are used
            delete mpIcuLE;
            meLanguageCode = eLanguageCode;
            meScriptCode = eScriptCode;
            mnLayoutFlags = nLayoutFlags;
            mpIcuLE = LayoutEngine::layoutEngineFactory( &maIcuFont, eScriptCode, eLanguageCode, nLayoutFlags, rcIcu );
            if( LE_FAILURE(rcIcu) )
            {
                delete mpIcuLE;
                mpIcuLE = NULL;
            }
        }

        // fall back to default layout if needed
        if( !mpIcuLE )
            break;

        // run ICU layout engine
        // TODO: get enough context, remove extra glyps below
        int nRawRunGlyphCount = mpIcuLE->layoutChars( pIcuChars,
            nMinRunPos, nEndRunPos - nMinRunPos, rArgs.mnLength,
            bRightToLeft, aNewPos.X(), aNewPos.Y(), rcIcu );
        if( LE_FAILURE(rcIcu) )
            return false;

        // import layout info from icu
        mpIcuLE->getGlyphs( pIcuGlyphs, rcIcu );
        mpIcuLE->getCharIndices( pCharIndices, rcIcu );
        mpIcuLE->getGlyphPositions( &pGlyphPositions->fX, rcIcu );
        mpIcuLE->reset(); // TODO: get rid of this, PROBLEM: crash at exit when removed
        if( LE_FAILURE(rcIcu) )
            return false;

        // layout bidi/script runs and export them to a ServerFontLayout
        // convert results to GlyphItems
        int nLastCharPos = -1;
        int nClusterMinPos = -1;
        int nClusterMaxPos = -1;
        bool bClusterStart = true;
        int nFilteredRunGlyphCount = 0;
        const IcuPosition* pPos = pGlyphPositions;
        for( int i = 0; i < nRawRunGlyphCount; ++i, ++pPos )
        {
            LEGlyphID nGlyphIndex = pIcuGlyphs[i];
            // ignore glyphs which were marked or deleted by ICU
            if( (nGlyphIndex == ICU_MARKED_GLYPH)
            ||  (nGlyphIndex == ICU_DELETED_GLYPH) )
                continue;

            // adjust the relative char pos
            int nCharPos = pCharIndices[i];
            if( nCharPos >= 0 ) {
                nCharPos += nMinRunPos;
                // ICU seems to return bad pCharIndices
                // for some combinations of ICU+font+text
                // => better give up now than crash later
                if( nCharPos >= nEndRunPos )
                    continue;
            }

            // if needed request glyph fallback by updating LayoutArgs
            if( !nGlyphIndex )
            {
                if( nCharPos >= 0 )
                {
                    rArgs.NeedFallback( nCharPos, bRightToLeft );
                    if ( (nCharPos > 0) && needPreviousCode(rArgs.mpStr[nCharPos-1]) )
                        rArgs.NeedFallback( nCharPos-1, bRightToLeft );
                    else if ( (nCharPos + 1 < nEndRunPos) && needNextCode(rArgs.mpStr[nCharPos+1]) )
                        rArgs.NeedFallback( nCharPos+1, bRightToLeft );
                }

                if( SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags )
                    continue;
            }


            // apply vertical flags, etc.
            bool bDiacritic = false;
            if( nCharPos >= 0 )
            {
                sal_UCS4 aChar = rArgs.mpStr[ nCharPos ];
                nGlyphIndex = rFont.FixupGlyphIndex( nGlyphIndex, aChar );

                // #i99367# HACK: try to detect all diacritics
                if( aChar>=0x0300 && aChar<0x2100 )
                    bDiacritic = IsDiacritic( aChar );
            }

            // get glyph position and its metrics
            aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
            const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyphIndex );
            int nGlyphWidth = rGM.GetCharWidth();
            int nNewWidth = nGlyphWidth;
            if( nGlyphWidth <= 0 )
                bDiacritic |= true;
            // #i99367# force all diacritics to zero width
            // TODO: we need mnOrigWidth/mnLogicWidth/mnNewWidth
            else if( bDiacritic )
                nGlyphWidth = nNewWidth = 0;
            else
            {
                // Hack, find next +ve width glyph and calculate current
                // glyph width by substracting the two posituons
                const IcuPosition* pNextPos = pPos+1;
                for ( int j = i + 1; j <= nRawRunGlyphCount; ++j, ++pNextPos )
                {
                    if ( j == nRawRunGlyphCount )
                    {
                        nNewWidth = static_cast<int>(pNextPos->fX - pPos->fX);
                        break;
                    }

                    LEGlyphID nNextGlyphIndex = pIcuGlyphs[j];
                    if( (nNextGlyphIndex == ICU_MARKED_GLYPH)
                    ||  (nNextGlyphIndex == ICU_DELETED_GLYPH) )
                        continue;

                    const GlyphMetric& rNextGM = rFont.GetGlyphMetric( nNextGlyphIndex );
                    int nNextGlyphWidth = rNextGM.GetCharWidth();
                    if ( nNextGlyphWidth > 0 )
                    {
                        nNewWidth = static_cast<int>(pNextPos->fX - pPos->fX);
                        break;
                    }
                }
            }

            // heuristic to detect glyph clusters
            bool bInCluster = true;
            if( nLastCharPos == -1 )
            {
                nClusterMinPos = nClusterMaxPos = nCharPos;
                bInCluster = false;
            }
            else if( !bRightToLeft )
            {
                // left-to-right case
                if( nClusterMinPos > nCharPos )
                    nClusterMinPos = nCharPos;      // extend cluster
                else if( nCharPos <= nClusterMaxPos )
                    /*NOTHING*/;                    // inside cluster
                else if( bDiacritic )
                    nClusterMaxPos = nCharPos;      // add diacritic to cluster
                else {
                    nClusterMinPos = nClusterMaxPos = nCharPos; // new cluster
                    bInCluster = false;
                }
            }
            else
            {
                // right-to-left case
                if( nClusterMaxPos < nCharPos )
                    nClusterMaxPos = nCharPos;      // extend cluster
                else if( nCharPos >= nClusterMinPos )
                    /*NOTHING*/;                    // inside cluster
                else if( bDiacritic )
                {
                    nClusterMinPos = nCharPos;      // ICU often has [diacritic* baseglyph*]
                    if( bClusterStart ) {
                        nClusterMaxPos = nCharPos;
                        bInCluster = false;
                    }
                }
                else
                {
                    nClusterMinPos = nClusterMaxPos = nCharPos; // new cluster
                    bInCluster = !bClusterStart;
                }
            }

            long nGlyphFlags = 0;
            if( bInCluster )
                nGlyphFlags |= GlyphItem::IS_IN_CLUSTER;
            if( bRightToLeft )
                nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
            if( bDiacritic )
                nGlyphFlags |= GlyphItem::IS_DIACRITIC;

            // add resulting glyph item to layout
            GlyphItem aGI( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
#ifdef ARABIC_BANDAID
            aGI.mnNewWidth = nNewWidth;
#endif
            rLayout.AppendGlyph( aGI );
            ++nFilteredRunGlyphCount;
            nLastCharPos = nCharPos;
            bClusterStart = !aGI.IsDiacritic(); // TODO: only needed in RTL-codepath
        }
        aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
        nGlyphCount += nFilteredRunGlyphCount;
    }

    // sort glyphs in visual order
    // and then in logical order (e.g. diacritics after cluster start)
    rLayout.SortGlyphItems();

    // determine need for kashida justification
    if( (rArgs.mpDXArray || rArgs.mnLayoutWidth)
    &&  ((meScriptCode == arabScriptCode) || (meScriptCode == syrcScriptCode)) )
        rArgs.mnFlags |= SAL_LAYOUT_KASHIDA_JUSTIFICATON;

    return true;
}

// =======================================================================

ServerFontLayoutEngine* ServerFont::GetLayoutEngine()
{
    // find best layout engine for font, platform, script and language
    if (!mpLayoutEngine)
        mpLayoutEngine = new IcuLayoutEngine(*this);
    return mpLayoutEngine;
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
