/*************************************************************************
 *
 *  $RCSfile: gcach_layout.cxx,v $
 *
 *  $Revision: 1.30 $
 *  last change: $Author: rt $ $Date: 2004-07-13 16:31:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define ENABLE_ICU_LAYOUT

#ifndef _SV_GCACHFTYP_HXX
#include <gcach_ftyp.hxx>
#endif

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

#if defined(SOLARIS) || defined(IRIX)
  #include <alloca.h>
#else
#if !(defined(MACOSX) || defined(FREEBSD))
  #include <malloc.h>
#endif
#endif

#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

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

    // asian kerning
    if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN)
    && !(rArgs.mnFlags & SAL_LAYOUT_VERTICAL) )
        ApplyAsianKerning( rArgs.mpStr, rArgs.mnLength );

    // kashida justification
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
    FreetypeServerFont& rFont = reinterpret_cast<FreetypeServerFont&>(rLayout.GetServerFont());

    Point aNewPos( 0, 0 );
    int nOldGlyphId = -1;
    int nGlyphWidth = 0;
    GlyphItem aPrevItem;
    bool bRightToLeft;
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
        if( bRightToLeft )
            cChar = GetMirroredChar( cChar );
        int nGlyphIndex = rFont.GetGlyphIndex( cChar );
        // when glyph fallback is needed update LayoutArgs
        if( !nGlyphIndex )
            rArgs.NeedFallback( nCharPos, bRightToLeft );

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
#include <layout/LayoutEngine.h>
#include <layout/LEFontInstance.h>
#include <layout/LEScripts.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>

using namespace U_ICU_NAMESPACE;

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
    const ImplFontSelectData& r = mrServerFont.GetFontSelData();
    float fXScale = r.mnWidth ? r.mnWidth : r.mnHeight;
    fXScale /= mrServerFont.GetEmUnits();
    return fXScale;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getScaleFactorY() const
{
    float fYScale = mrServerFont.GetFontSelData().mnHeight;
    fYScale /= mrServerFont.GetEmUnits();
    return fYScale; //TODO
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
    const GlyphMetric& rGM = mrServerFont.GetGlyphMetric( nGlyphIndex );
    advance.fX = rGM.GetCharWidth();
    advance.fY = 0;
}

// -----------------------------------------------------------------------

le_bool IcuFontFromServerFont::getGlyphPoint( LEGlyphID glyph,
    le_int32 pointNumber, LEPoint& point ) const
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
    LEGlyphID* pIcuGlyphs = (LEGlyphID*)alloca( nGlyphCapacity * nAllocSize + sizeof(IcuPosition) );
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
        le_int32 eScriptCode = uscript_getScript( pIcuChars[nMinRunPos], &rcI18n );
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
        int nRunGlyphCount = mpIcuLE->layoutChars( pIcuChars, nMinRunPos,
            nEndRunPos - nMinRunPos, rArgs.mnLength, bRightToLeft,
            aNewPos.X(), aNewPos.Y(), rcIcu );
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
        const IcuPosition* pPos = pGlyphPositions;
        for( int i = 0; i < nRunGlyphCount; ++i, ++pPos )
        {
            int nCharPos = pCharIndices[i] + nMinRunPos;
            int nGlyphIndex = pIcuGlyphs[i];
            // when glyph fallback is needed update LayoutArgs
            if( !nGlyphIndex )
            {
                rArgs.NeedFallback( nCharPos, bRightToLeft );
                if( SAL_LAYOUT_FOR_FALLBACK & rArgs.mnFlags )
                    continue;
            }

            // apply vertical flags, etc.
            sal_Unicode aChar = rArgs.mpStr[ nCharPos ];
            nGlyphIndex = rFont.FixupGlyphIndex( nGlyphIndex, aChar );

            aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
            const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyphIndex );
            int nGlyphWidth = rGM.GetCharWidth();
            long nGlyphFlags = (nGlyphWidth > 0) ? 0 : GlyphItem::IS_IN_CLUSTER;
            if( bRightToLeft )
                nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
            GlyphItem aGI( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
            rLayout.AppendGlyph( aGI );
        }
        aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
        nGlyphCount += nRunGlyphCount;
    }

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
