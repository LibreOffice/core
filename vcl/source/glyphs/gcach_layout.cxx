/*************************************************************************
 *
 *  $RCSfile: gcach_layout.cxx,v $
 *
 *  $Revision: 1.10 $
 *  last change: $Author: hdu $ $Date: 2002-06-13 19:21:56 $
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
#ifdef DEBUG
//#define VERBOSE_DEBUG
#endif

#if !defined(_SV_SALUNX_HXX) && !defined(WIN32)
#include <salunx.h>
#endif

#ifndef _SV_GCACHFTYP_HXX
#include <gcach_ftyp.hxx>
#endif

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

#ifdef WIN32
#include <malloc.h>
#define alloca _alloca
#elif !defined(FREEBSD) && !defined(NETBSD)
#include <alloca.h>
#endif

#ifdef DEBUG
#include <cstdio>
#endif

// =======================================================================

ServerFontLayout::ServerFontLayout( ServerFont* pServerFont,
    const ImplLayoutArgs& rArgs )
:   GenericSalLayout( rArgs ),
    mpServerFont( pServerFont )
{}

// =======================================================================
// Layout Engine Abstract Base Class
// =======================================================================

class ServerFontLayoutEngine
{
public:
    virtual ServerFontLayout*   operator()( ServerFont*, const ImplLayoutArgs& rArgs );
};

static ServerFontLayoutEngine aSimpleLayoutEngine;

// =======================================================================
// layout implementation for ServerFont
// =======================================================================

bool ServerFont::InitLayoutEngine()
{
    // setup default layout engine
    mpLayoutData = (void*)&aSimpleLayoutEngine;
    return true;
}

// -----------------------------------------------------------------------

void ServerFont::DoneLayoutEngine()
{
    ServerFontLayoutEngine* pLE = (ServerFontLayoutEngine*)mpLayoutData;
    if( (pLE != NULL) && (pLE != &aSimpleLayoutEngine) )
        delete pLE;
    mpLayoutData = NULL;
}

// -----------------------------------------------------------------------

ServerFontLayout* ServerFont::LayoutText( const ImplLayoutArgs& rLayoutArgs )
{
    ServerFontLayoutEngine* pLE = NULL;
    if( !(rLayoutArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED) )
    {
        if( !mpLayoutData )
            InitLayoutEngine();
        pLE = (ServerFontLayoutEngine*)mpLayoutData;
    }
    if( !pLE )
        pLE = &aSimpleLayoutEngine;

    ServerFontLayout* pSalLayout = (*pLE)( this, rLayoutArgs );
    if( rLayoutArgs.mpDXArray )
        pSalLayout->ApplyDXArray( rLayoutArgs.mpDXArray );
    if( rLayoutArgs.mnLayoutWidth )
        pSalLayout->Justify( rLayoutArgs.mnLayoutWidth );
    return pSalLayout;
}

// -----------------------------------------------------------------------

ServerFontLayout* ServerFontLayoutEngine::operator()( ServerFont* pFont,
    const ImplLayoutArgs& rArgs )
{
    bool bRightToLeft = (SAL_LAYOUT_BIDI_RTL & rArgs.mnFlags) != 0;

    int nGlyphCount = rArgs.mnEndCharIndex - rArgs.mnFirstCharIndex;
    GlyphItem* pGlyphBuffer = new GlyphItem[ nGlyphCount ];

    Point aNewPos( 0, 0 );
    bool bWantFallback = false;
    int nOldGlyphId = -1;
    int nGlyphWidth = 0;
    for( int i = 0; i < nGlyphCount; ++i )
    {
        int nLogicalIndex = bRightToLeft ? rArgs.mnEndCharIndex-1-i : rArgs.mnFirstCharIndex+i;
        int nGlyphIndex = pFont->GetGlyphIndex( rArgs.mpStr[ nLogicalIndex ] );
        if( !nGlyphIndex )
            bWantFallback = true;

        // apply pair kerning if requested
        if( SAL_LAYOUT_KERNING_PAIRS & rArgs.mnFlags )
        {
            int nKern = pFont->GetGlyphKernValue( nOldGlyphId, nGlyphIndex );
            nGlyphWidth += nKern;
            nOldGlyphId = nGlyphIndex;
        }

        // update position of this glyph using all previous info
        aNewPos.X() += nGlyphWidth;

        const GlyphMetric& rGM = pFont->GetGlyphMetric( nGlyphIndex );
        nGlyphWidth = rGM.GetCharWidth();
        pGlyphBuffer[i] = GlyphItem( nLogicalIndex, nGlyphIndex, aNewPos,
            GlyphItem::CLUSTER_START, nGlyphWidth );
    }

    // apply asian kerning if requested
    if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN)
    &&  !rArgs.mpDXArray && !rArgs.mnLayoutWidth && !bRightToLeft )
    {
        bool bVertical = false; // TODO

        const xub_Unicode* pStr = rArgs.mpStr + rArgs.mnFirstCharIndex;
        // #99658# also do asian kerning one beyond substring
        int nLen = nGlyphCount;
        if( rArgs.mnFirstCharIndex + nLen < rArgs.mnLength )
            ++nLen;
        long nOffset = 0;
        for( int i = 1; i < nLen; ++i )
        {
            if( (0x3000 == (0xFF00 & pStr[i-1]))
            &&  (0x3000 == (0xFF00 & pStr[i])) )
            {
                long nKernFirst = +SalLayout::CalcAsianKerning( pStr[i-1], true, bVertical );
                long nKernNext  = -SalLayout::CalcAsianKerning( pStr[i], false, bVertical );

                long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
                if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
                {
                    nGlyphWidth = pGlyphBuffer[i-1].mnOrigWidth;
                    nDelta = (nDelta * nGlyphWidth + 2) / 4;
                    if( i == nGlyphCount )
                        pGlyphBuffer[i].mnNewWidth += nDelta;
                    nOffset += nDelta;
                }
            }

            if( i < nGlyphCount )
                pGlyphBuffer[i].maLinearPos.X() += nOffset;
        }
    }

    // create layout object
    ServerFontLayout* pSalLayout = new ServerFontLayout( pFont, rArgs );
    pSalLayout->SetGlyphItems( pGlyphBuffer, nGlyphCount );
    pSalLayout->SetWantFallback( bWantFallback );
    return pSalLayout;
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

using namespace U_ICU_NAMESPACE;

// -----------------------------------------------------------------------

class IcuFontFromServerFont
: public LEFontInstance
{
private:
    FreetypeServerFont* const mpServerFont;

public:
                            IcuFontFromServerFont( FreetypeServerFont* pFont )
                            : mpServerFont(pFont)
                            {}

    virtual const void*     getFontTable(LETag tableTag) const;
    virtual le_bool         canDisplay(LEUnicode32 ch) const;
    virtual le_int32        getUnitsPerEM() const;

    virtual void            mapCharsToGlyphs( const LEUnicode chars[],
                                le_int32 offset, le_int32 count, le_bool reverse,
                                const LECharMapper* mapper, LEGlyphID glyphs[] ) const;
    virtual LEGlyphID       mapCharToGlyph( LEUnicode32 ch, const LECharMapper* mapper) const;

    virtual le_int32        getName( le_uint16 platformID, le_uint16 scriptID, le_uint16 languageID,
                                le_uint16 nameID, LEUnicode *name ) const;
    virtual void            getGlyphAdvance( LEGlyphID glyph, LEPoint &advance ) const;
    virtual le_bool         getGlyphPoint( LEGlyphID glyph, le_int32 pointNumber, LEPoint& point ) const;
    virtual float           getXPixelsPerEm() const;
    virtual float           getYPixelsPerEm() const;
    virtual float           xUnitsToPoints( float xUnits ) const;
    virtual float           yUnitsToPoints( float xUnits ) const;
    virtual void            unitsToPoints( LEPoint &units, LEPoint &points ) const;
    virtual float           xPixelsToUnits( float xPixels ) const;
    virtual float           yPixelsToUnits( float xPixels ) const;
    virtual void            pixelsToUnits( LEPoint &pixels, LEPoint &units ) const;
    virtual void            transformFunits( float xFunits, float yFunits, LEPoint &pixels ) const;
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
    const unsigned char* pBuffer = mpServerFont->GetTable( pTagName, &nLength );
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"IcuGetTable(\"%s\") => %p\n", pTagName, pBuffer);
    int mnHeight = mpServerFont->GetFontSelData().mnHeight;
    const char* pName = mpServerFont->GetFontFileName()->getStr();
    fprintf(stderr,"font(  h=%d, s=%\"%s\" )\n", mnHeight, pName );
#endif
    return (const void*)pBuffer;
}

// -----------------------------------------------------------------------

le_bool IcuFontFromServerFont::canDisplay( LEUnicode32 ch ) const
{
    le_bool rc = (mpServerFont->GetRawGlyphIndex( ch ) != 0);
    return rc;
}

// -----------------------------------------------------------------------

le_int32 IcuFontFromServerFont::getUnitsPerEM() const
{
    return mpServerFont->GetEmUnits();
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::mapCharsToGlyphs( const LEUnicode pChars[],
    le_int32 nOffset, le_int32 nCount, le_bool bReverse,
    const LECharMapper* pMapper, LEGlyphID pGlyphs[] ) const
{
    for( int i = 0; i < nCount; ++i )
        pGlyphs[i] = mapCharToGlyph( pChars[nOffset+i], pMapper );

    if( bReverse )
    {
        for( LEGlyphID *p1=pGlyphs, *p2=p1+nCount; p1 < --p2; ++p1 )
        {
            LEGlyphID t = *p1; *p1 = *p2; *p2 = t;
        }
    }
}

// -----------------------------------------------------------------------

LEGlyphID IcuFontFromServerFont::mapCharToGlyph( LEUnicode32 ch,
    const LECharMapper* /*TODO: mapper*/ ) const
{
    LEGlyphID nGlyphIndex = mpServerFont->GetRawGlyphIndex( ch );
    return nGlyphIndex;
}


// -----------------------------------------------------------------------
le_int32 IcuFontFromServerFont::getName(
    le_uint16 platformID, le_uint16 scriptID, le_uint16 languageID,
    le_uint16 nameID, LEUnicode *name ) const
{
    //TODO: replace dummy implementation
    if( name )
        name[0] = '#';
    return 1;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::getGlyphAdvance( LEGlyphID nGlyphIndex,
    LEPoint &advance ) const
{
    const GlyphMetric& rGM = mpServerFont->GetGlyphMetric( nGlyphIndex );
    advance.fX = rGM.GetCharWidth();
    advance.fY = 0;
}

// -----------------------------------------------------------------------

le_bool IcuFontFromServerFont::getGlyphPoint( LEGlyphID glyph,
    le_int32 pointNumber, LEPoint& point ) const
{
    //TODO: replace dummy implementation
#ifdef DEBUG
    fprintf(stderr,"getGlyphPoint\n");
#endif
    return false;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getXPixelsPerEm() const
{
    const ImplFontSelectData& r = mpServerFont->GetFontSelData();
    float fX = r.mnWidth ? r.mnWidth : r.mnHeight;
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"mnXPixel4EM %f\n", fX );
#endif
    return fX;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getYPixelsPerEm() const
{
    float fY = mpServerFont->GetFontSelData().mnHeight;
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"mnYPixel4EM %f\n", fY );
#endif
    return fY;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::xUnitsToPoints( float xUnits ) const
{
    // TODO: avoid assumption: pixels==points
    float fPoints = xUnits;
    const ImplFontSelectData& r = mpServerFont->GetFontSelData();
    fPoints *= r.mnWidth ? r.mnWidth : r.mnHeight;
    fPoints /= mpServerFont->GetEmUnits();
    return fPoints;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::yUnitsToPoints( float yUnits ) const
{
    // TODO: avoid assumption pixels==points
    float fPoints = yUnits;
    fPoints *= mpServerFont->GetFontSelData().mnHeight;
    fPoints /= mpServerFont->GetEmUnits();
    return fPoints;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::unitsToPoints( LEPoint &units, LEPoint &points ) const
{
    points.fX = xUnitsToPoints( units.fX );
    points.fY = yUnitsToPoints( units.fY );
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"u2p( %f, %f ) => ( %f, %f )\n", units.fX, units.fY, points.fX, points.fY );
#endif
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::xPixelsToUnits( float xPixels ) const
{
    float fPixels = xPixels;
    fPixels *= mpServerFont->GetEmUnits();
    const ImplFontSelectData& r = mpServerFont->GetFontSelData();
    fPixels /= r.mnWidth ? r.mnWidth : r.mnHeight;
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"xp2u( %f ) => %f\n", xPixels, fPixels );
#endif
    return fPixels;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::yPixelsToUnits( float yPixels ) const
{
    float fPixels = yPixels;
    fPixels *= mpServerFont->GetEmUnits();
    fPixels /= mpServerFont->GetFontSelData().mnHeight;
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"p2u( %f ) => %f\n", yPixels, fPixels );
#endif
    return fPixels;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::pixelsToUnits( LEPoint &pixels, LEPoint &units ) const
{
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"p2u( %f, %f )\n", pixels.fX, pixels.fY );
#endif
    units.fX = xPixelsToUnits( pixels.fX );
    units.fY = yPixelsToUnits( pixels.fY );
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::transformFunits( float xFunits, float yFunits, LEPoint &pixels ) const
{
    // TODO: avoid assumption pixels==points
    LEPoint units = { xFunits, yFunits };
    unitsToPoints( units, pixels );
#ifdef VERBOSE_DEBUG
    fprintf(stderr,"tfu( %f, %f ) => ( %f, %f )\n", xFunits, yFunits, pixels.fX, pixels.fY );
#endif
}

// =======================================================================

class IcuLayoutEngine : public ServerFontLayoutEngine
{
private:
    IcuFontFromServerFont   maIcuFont;

    le_int32                meScriptCode;
    LayoutEngine*           mpIcuLE;

public:
                            IcuLayoutEngine( FreetypeServerFont* pServerFont );
    virtual                 ~IcuLayoutEngine();
    bool                    IsReady() const { return (mpIcuLE != NULL); }

    virtual ServerFontLayout* operator()( ServerFont*, const ImplLayoutArgs& );
};

// -----------------------------------------------------------------------

IcuLayoutEngine::IcuLayoutEngine( FreetypeServerFont* pServerFont )
:   maIcuFont( pServerFont ),
    mpIcuLE( NULL ),
    meScriptCode( USCRIPT_INVALID_CODE )
{}

// -----------------------------------------------------------------------

IcuLayoutEngine::~IcuLayoutEngine()
{
    if( mpIcuLE )
        delete mpIcuLE;
}

// -----------------------------------------------------------------------

ServerFontLayout* IcuLayoutEngine::operator()( ServerFont* pFont,
    const ImplLayoutArgs& rArgs )
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

    UErrorCode rcI18n = U_ZERO_ERROR;
    LEErrorCode rcIcu = LE_NO_ERROR;

    // find matching script
    // TODO: handle errors better
    // TODO: consider script changes
    le_int32 eScriptCode = uscript_getScript( pIcuChars[rArgs.mnFirstCharIndex], &rcI18n );

    // get layout engine matching to this script
    // no engine change necessary if script is latin
    if( (meScriptCode != eScriptCode)
    &&   (eScriptCode > USCRIPT_INHERITED)
    &&   (eScriptCode != latnScriptCode) )
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
        return aSimpleLayoutEngine( pFont, rArgs );

    // TODO: split up BiDi runs
    le_bool bRightToLeft = (SAL_LAYOUT_BIDI_RTL & rArgs.mnFlags) != 0;

    // run ICU layout engine
    int nGlyphCount = mpIcuLE->layoutChars( pIcuChars, rArgs.mnFirstCharIndex,
        rArgs.mnEndCharIndex - rArgs.mnFirstCharIndex, rArgs.mnLength,
        bRightToLeft, 0.0, 0.0, rcIcu );
    if( LE_FAILURE(rcIcu) )
        return NULL;

    // import layout info from icu
    struct IcuPosition{ float fX, fY; };
    LEGlyphID* pIcuGlyphs = (LEGlyphID*)alloca( nGlyphCount * sizeof(LEGlyphID) );
    le_int32* pCharIndices = (le_int32*)alloca( nGlyphCount * sizeof(le_int32) );
    IcuPosition* pGlyphPositions = (IcuPosition*)alloca( (nGlyphCount+1) * sizeof(IcuPosition) );

    mpIcuLE->getGlyphs( pIcuGlyphs, rcIcu );
    mpIcuLE->getCharIndices( pCharIndices, rcIcu );
    mpIcuLE->getGlyphPositions( (float*)pGlyphPositions, rcIcu );
    mpIcuLE->reset();
    if( LE_FAILURE(rcIcu) )
        return NULL;

    // export layout info to ServerFontLayout
    GlyphItem* pGlyphBuffer = new GlyphItem[ nGlyphCount ];
    bool bWantFallback = false;

    Point aNewPos;
    const IcuPosition* pPos = pGlyphPositions;
    FreetypeServerFont* pFtFont = reinterpret_cast<FreetypeServerFont*>(pFont);
    for( int i = 0; i < nGlyphCount; ++i, ++pPos )
    {
        //TODO: remove workaround below for ICU getting pCharIndex wrong
        int nCharIndex = rArgs.mnFirstCharIndex + pCharIndices[i];
        int nGlyphIndex = pIcuGlyphs[i];
        if( nGlyphIndex != 0 )
        {
            // apply vertical flags, etc.
            sal_Unicode aChar = rArgs.mpStr[ pCharIndices[ i ] ];
            nGlyphIndex = pFtFont->FixupGlyphIndex( nGlyphIndex, aChar );
        }
        else
            bWantFallback = true;

        aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );
        const GlyphMetric& rGM = pFont->GetGlyphMetric( nGlyphIndex );
        int nGlyphWidth = rGM.GetCharWidth();
        long nGlyphFlags = (nGlyphWidth > 0) ? GlyphItem::CLUSTER_START : 0;
        pGlyphBuffer[i] = GlyphItem( nCharIndex, nGlyphIndex, aNewPos,
            nGlyphFlags, nGlyphWidth );
        //TODO: apply kerning if requested, set MOVED flag
    }
    aNewPos = Point( (int)(pPos->fX+0.5), (int)(pPos->fY+0.5) );

    ServerFontLayout* pSalLayout = new ServerFontLayout( pFont, rArgs );
    pSalLayout->SetGlyphItems( pGlyphBuffer, nGlyphCount );
    pSalLayout->SetWantFallback( bWantFallback );
    return pSalLayout;
}

#endif // ENABLE_ICU_LAYOUT

// =======================================================================

bool FreetypeServerFont::InitLayoutEngine()
{
    // find best layout engine for font, platform, script and language
#ifdef ENABLE_ICU_LAYOUT
    IcuLayoutEngine* pLE = new IcuLayoutEngine( this );
    mpLayoutData = (void*)pLE;
#endif // ENABLE_ICU_LAYOUT

    return (mpLayoutData != NULL);
}

// =======================================================================
