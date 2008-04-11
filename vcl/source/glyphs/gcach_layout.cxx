/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gcach_layout.cxx,v $
 * $Revision: 1.46 $
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

#define ENABLE_ICU_LAYOUT
#include <gcach_ftyp.hxx>
#include <vcl/sallayout.hxx>
#include <vcl/salgdi.hxx>

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

//--------------------------------------------------------------------------

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
    FreetypeServerFont& rFont = static_cast<FreetypeServerFont&>(rLayout.GetServerFont());

    Point aNewPos( 0, 0 );
    int nOldGlyphId = -1;
    int nGlyphWidth = 0;
    GlyphItem aPrevItem;
    bool bRightToLeft;
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
    FreetypeServerFont&     mrServerFont;

public:
                            IcuFontFromServerFont( FreetypeServerFont& rFont )
                            : mrServerFont( rFont )
                            {}

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

const void* IcuFontFromServerFont::getFontTable( LETag nICUTableTag ) const
{
    char pTagName[5];
    pTagName[0] = (char)(nICUTableTag >> 24);
    pTagName[1] = (char)(nICUTableTag >> 16);
    pTagName[2] = (char)(nICUTableTag >>  8);
    pTagName[3] = (char)(nICUTableTag);
    pTagName[4] = 0;

    ULONG nLength;
    const unsigned char* pBuffer = mrServerFont.GetTable( pTagName, &nLength );
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"IcuGetTable(\"%s\") => %p\n", pTagName, pBuffer);
    int mnHeight = mrServerFont.GetFontSelData().mnHeight;
    const char* pName = mrServerFont.GetFontFileName()->getStr();
    fprintf(stderr,"font( h=%d, \"%s\" )\n", mnHeight, pName );
#endif
    return (const void*)pBuffer;
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getUnitsPerEM() const
{
    return mrServerFont.GetEmUnits();
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getXPixelsPerEm() const
{
    const ImplFontSelectData& r = mrServerFont.GetFontSelData();
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
                            IcuLayoutEngine( FreetypeServerFont& );
    virtual                 ~IcuLayoutEngine();

    virtual bool            operator()( ServerFontLayout&, ImplLayoutArgs& );
};

// -----------------------------------------------------------------------

IcuLayoutEngine::IcuLayoutEngine( FreetypeServerFont& rServerFont )
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

    struct IcuPosition{ float fX, fY; };
    const int nAllocSize = sizeof(LEGlyphID) + sizeof(le_int32) + sizeof(IcuPosition);
    LEGlyphID* pIcuGlyphs = (LEGlyphID*)alloca( (nGlyphCapacity * nAllocSize) + sizeof(IcuPosition) );
    le_int32* pCharIndices = (le_int32*)((char*)pIcuGlyphs + nGlyphCapacity * sizeof(LEGlyphID) );
    IcuPosition* pGlyphPositions = (IcuPosition*)((char*)pCharIndices + nGlyphCapacity * sizeof(le_int32) );

    FreetypeServerFont& rFont = reinterpret_cast<FreetypeServerFont&>(rLayout.GetServerFont());

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
        mpIcuLE->reset(); // TODO: get rid of this, PROBLEM: crash at exit when removed
        if( LE_FAILURE(rcIcu) )
            return false;

        // layout bidi/script runs and export them to a ServerFontLayout
        // convert results to GlyphItems
        int nLastCharPos = -1;
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

            // if ICU feeds us a character index sequence like [1,0,1] (which
            // is completely valid), smooth out the sequence so that our cluster
            // detection routines work better. The best knowledge where the
            // cluster boundaries are should be provided by the layout engine...
            if( nLastCharPos != -1 )
                if( (nCharPos < nLastCharPos) ^ bRightToLeft )
                    nCharPos = nLastCharPos;

            // apply vertical flags, etc.
            if( nCharPos >= 0 )
            {
                sal_UCS4 aChar = rArgs.mpStr[ nCharPos ];
#if 0 // TODO: enable if some unicodes>0xFFFF should need glyph flags!=0
                if( (aChar >= 0xD800) && (aChar <= 0xDFFF) )
                {
                    if( cChar >= 0xDC00 ) // this part of a surrogate pair was already processed
                        continue;
                    // calculate unicode scalar value of surrogate pair
                    aChar = 0x10000 + ((aChar - 0xD800) << 10);
                    sal_UCS4 aLow = rArgs.mpStr[ nCharPos+1 ];
                    aChar += aLow & 0x03FF;
                }
#endif
                nGlyphIndex = rFont.FixupGlyphIndex( nGlyphIndex, aChar );
            }

            // get glyph position and its metrics
            aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
            const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyphIndex );
            int nGlyphWidth = rGM.GetCharWidth();

            // heuristic to detect group clusters using "smoothed" char positions
            long nGlyphFlags = 0;
            if( nLastCharPos != -1 )
                if( (nCharPos == nLastCharPos) || (nGlyphWidth <= 0) )
                    nGlyphFlags = GlyphItem::IS_IN_CLUSTER;
            if( bRightToLeft )
                nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;

            // add resulting glyph item to layout
            GlyphItem aGI( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
            rLayout.AppendGlyph( aGI );
            ++nFilteredRunGlyphCount;
            nLastCharPos = nCharPos;
        }
        aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
        nGlyphCount += nFilteredRunGlyphCount;
    }

    // sort glyphs in visual order
    rLayout.SortGlyphItems();

    // determine need for kashida justification
    if( (rArgs.mpDXArray || rArgs.mnLayoutWidth)
    &&  ((meScriptCode == arabScriptCode) || (meScriptCode == syrcScriptCode)) )
        rArgs.mnFlags |= SAL_LAYOUT_KASHIDA_JUSTIFICATON;

    return true;
}

#endif // ENABLE_ICU_LAYOUT

// =======================================================================

ServerFontLayoutEngine* FreetypeServerFont::GetLayoutEngine()
{
    // find best layout engine for font, platform, script and language
#ifdef ENABLE_ICU_LAYOUT
    if( !mpLayoutEngine && FT_IS_SFNT( maFaceFT ) )
        mpLayoutEngine = new IcuLayoutEngine( *this );
#endif // ENABLE_ICU_LAYOUT

    return mpLayoutEngine;
}

// =======================================================================
