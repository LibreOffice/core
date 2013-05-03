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


#define ENABLE_ICU_LAYOUT
#include <gcach_ftyp.hxx>
#include <sallayout.hxx>
#include <salgdi.hxx>

#include <vcl/svapp.hxx>

#include <sal/alloca.h>

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif
#include <rtl/instance.hxx>

namespace { struct SimpleLayoutEngine : public rtl::Static< ServerFontLayoutEngine, SimpleLayoutEngine > {}; }

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
    ServerFontLayoutEngine* pLE = NULL;
    if( !(rArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED) )
        pLE = mrServerFont.GetLayoutEngine();
    if( !pLE )
        pLE = &SimpleLayoutEngine::get();

    bool bRet = (*pLE)( *this, rArgs );
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

bool ServerFontLayoutEngine::operator()( ServerFontLayout& rLayout, ImplLayoutArgs& rArgs )
{
    ServerFont& rFont = rLayout.GetServerFont();

    Point aNewPos( 0, 0 );
    int nOldGlyphId = -1;
    int nGlyphWidth = 0;
    GlyphItem aPrevItem;
    bool bRightToLeft;

    rLayout.Reserve(rArgs.mnLength);
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        sal_UCS4 cChar = rArgs.mpStr[ nCharPos ];
        if( (cChar >= 0xD800) && (cChar <= 0xDFFF) )
        {
            if( cChar >= 0xDC00 ) // this part of a surrogate pair was already processed
                continue;
            cChar = 0x10000 + ((cChar - 0xD800) << 10)
                  + (rArgs.mpStr[ nCharPos+1 ] - 0xDC00);
        }

        if( bRightToLeft )
            cChar = GetMirroredChar( cChar );
        int nGlyphIndex = rFont.GetGlyphIndex( cChar );
        // when glyph fallback is needed update LayoutArgs
        if( !nGlyphIndex ) {
            rArgs.NeedFallback( nCharPos, bRightToLeft );
        if( cChar >= 0x10000 ) // handle surrogate pairs
                rArgs.NeedFallback( nCharPos+1, bRightToLeft );
    }

        // apply pair kerning to prev glyph if requested
        if( SAL_LAYOUT_KERNING_PAIRS & rArgs.mnFlags )
        {
            int nKernValue = rFont.GetGlyphKernValue( nOldGlyphId, nGlyphIndex );
            nGlyphWidth += nKernValue;
            aPrevItem.mnNewWidth = nGlyphWidth;
        }

        // finish previous glyph
        if( nOldGlyphId >= 0 )
            rLayout.AppendGlyph( aPrevItem );
        aNewPos.X() += nGlyphWidth;

        // prepare GlyphItem for appending it in next round
        nOldGlyphId = nGlyphIndex;
        const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyphIndex );
        nGlyphWidth = rGM.GetCharWidth();
        int nGlyphFlags = bRightToLeft ? GlyphItem::IS_RTL_GLYPH : 0;
        aPrevItem = GlyphItem( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
    }

    // append last glyph item if any
    if( nOldGlyphId >= 0 )
        rLayout.AppendGlyph( aPrevItem );

    return true;
}

// =======================================================================
// bridge to ICU LayoutEngine
// =======================================================================

#ifdef ENABLE_ICU_LAYOUT

#define bool_t signed char

// disable warnings in icu layout headers
#if defined __SUNPRO_CC
#pragma disable_warn
#endif

#include <layout/LayoutEngine.h>
#include <layout/LEFontInstance.h>
#include <layout/LEScripts.h>

// enable warnings again
#if defined __SUNPRO_CC
#pragma enable_warn
#endif

#include <unicode/uscript.h>
#include <unicode/ubidi.h>

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

    virtual const void*     getFontTable(LETag tableTag, size_t &length) const;
    virtual const void*     getFontTable(LETag tableTag) const;
    virtual le_int32        getUnitsPerEM() const;
    virtual float           getXPixelsPerEm() const;
    virtual float           getYPixelsPerEm() const;
    virtual float           getScaleFactorX() const;
    virtual float           getScaleFactorY() const;

    using LEFontInstance::mapCharToGlyph;
    virtual LEGlyphID       mapCharToGlyph( LEUnicode32 ch ) const;

    virtual le_int32        getAscent() const;
    virtual le_int32        getDescent() const;
    virtual le_int32        getLeading() const;

    virtual void            getGlyphAdvance( LEGlyphID glyph, LEPoint &advance ) const;
    virtual le_bool         getGlyphPoint( LEGlyphID glyph, le_int32 pointNumber, LEPoint& point ) const;
};

// -----------------------------------------------------------------------

const void* IcuFontFromServerFont::getFontTable( LETag nICUTableTag, size_t & rLength ) const
{
    char pTagName[5];
    pTagName[0] = (char)(nICUTableTag >> 24);
    pTagName[1] = (char)(nICUTableTag >> 16);
    pTagName[2] = (char)(nICUTableTag >>  8);
    pTagName[3] = (char)(nICUTableTag);
    pTagName[4] = 0;

    sal_uLong nLength = 0;
    const unsigned char* pBuffer = mrServerFont.GetTable( pTagName, &nLength );
    rLength = static_cast<size_t>(nLength);
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"IcuGetTable(\"%s\") => %p\n", pTagName, pBuffer);
    int mnHeight = mrServerFont.GetFontSelData().mnHeight;
    const char* pName = mrServerFont.GetFontFileName()->getStr();
    fprintf(stderr,"font( h=%d, \"%s\" )\n", mnHeight, pName );
#endif
    return (const void*)pBuffer;
}

const void* IcuFontFromServerFont::getFontTable( LETag nICUTableTag ) const
{
    size_t nLength = 0;
    return getFontTable( nICUTableTag, nLength);
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
    le_int32
#if OSL_DEBUG_LEVEL > 1
pointNumber
#endif
    ,
    LEPoint& ) const
{
    //TODO: replace dummy implementation
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr,"getGlyphPoint(%d)\n", pointNumber );
#endif
    return false;
}

// =======================================================================

class IcuLayoutEngine : public ServerFontLayoutEngine
{
private:
    IcuFontFromServerFont   maIcuFont;

    le_int32                meScriptCode;
    LayoutEngine*           mpIcuLE;

public:
                            IcuLayoutEngine( ServerFont& );
    virtual                 ~IcuLayoutEngine();

    virtual bool            operator()( ServerFontLayout&, ImplLayoutArgs& );
};

// -----------------------------------------------------------------------

IcuLayoutEngine::IcuLayoutEngine( ServerFont& rServerFont )
:   maIcuFont( rServerFont ),
    meScriptCode( USCRIPT_INVALID_CODE ),
    mpIcuLE( NULL )
{}

// -----------------------------------------------------------------------

IcuLayoutEngine::~IcuLayoutEngine()
{
    if( mpIcuLE )
        delete mpIcuLE;
}

// -----------------------------------------------------------------------

static bool lcl_CharIsJoiner(sal_Unicode cChar)
{
    return ((cChar == 0x200C) || (cChar == 0x200D));
}

//See https://bugs.freedesktop.org/show_bug.cgi?id=31016
#define ARABIC_BANDAID

bool IcuLayoutEngine::operator()( ServerFontLayout& rLayout, ImplLayoutArgs& rArgs )
{
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
            eScriptCode = uscript_getScript( pIcuChars[i], &rcI18n );
            if( (eScriptCode > 0) && (eScriptCode != latnScriptCode) )
                break;
        }
        if( eScriptCode < 0 )   // TODO: handle errors better
            eScriptCode = latnScriptCode;

        // get layout engine matching to this script
        // no engine change necessary if script is latin
        if( !mpIcuLE || ((eScriptCode != meScriptCode) && (eScriptCode > USCRIPT_INHERITED)) )
        {
            // TODO: cache multiple layout engines when multiple scripts are used
            delete mpIcuLE;
            meScriptCode = eScriptCode;
            le_int32 eLangCode = 0; // TODO: get better value
            mpIcuLE = LayoutEngine::layoutEngineFactory( &maIcuFont, eScriptCode, eLangCode, rcIcu );
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
                    if ( (nCharPos > 0) && lcl_CharIsJoiner(rArgs.mpStr[nCharPos-1]) )
                        rArgs.NeedFallback( nCharPos-1, bRightToLeft );
                    else if ( (nCharPos + 1 < nEndRunPos) && lcl_CharIsJoiner(rArgs.mpStr[nCharPos+1]) )
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

#endif // ENABLE_ICU_LAYOUT

// =======================================================================

ServerFontLayoutEngine* ServerFont::GetLayoutEngine()
{
    // find best layout engine for font, platform, script and language
#ifdef ENABLE_ICU_LAYOUT
    if( !mpLayoutEngine && FT_IS_SFNT( maFaceFT ) )
        mpLayoutEngine = new IcuLayoutEngine( *this );
#endif // ENABLE_ICU_LAYOUT

    return mpLayoutEngine;
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
