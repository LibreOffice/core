#define ENABLE_ICU_LAYOUT

/*************************************************************************
 *
 *  $RCSfile: gcach_layout.cxx,v $
 *
 *  $Revision: 1.2 $
 *  last change: $Author: hdu $ $Date: 2002-02-26 13:43:51 $
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

#ifndef _SV_SALUNX_HXX
#include <salunx.h>
#endif

#ifndef _SV_GCACHFTYP_HXX
#include <gcach_ftyp.hxx>
#endif

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

#if !( defined(FREEBSD) || defined(NETBSD) )
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
    for( int i = 0; i < nGlyphCount; ++i )
    {
        int nLogicalIndex = bRightToLeft ? rArgs.mnEndCharIndex-1-i : rArgs.mnFirstCharIndex+i;
        int nGlyphIndex = pFont->GetGlyphIndex( rArgs.mpStr[ nLogicalIndex ] );
        if( !nGlyphIndex )
            bWantFallback = true;

        if( SAL_LAYOUT_KERNING_PAIRS & rArgs.mnFlags )
        {
            int nKern = pFont->GetGlyphKernValue( nOldGlyphId, nGlyphIndex );
            aNewPos += Point( nKern, 0 );
            nOldGlyphId = nGlyphIndex;
        }
        // TODO: apply asian kerning if requested too

        const GlyphMetric& rGM = pFont->GetGlyphMetric( nGlyphIndex );
        int nGlyphWidth = rGM.GetCharWidth();
        pGlyphBuffer[i] = GlyphItem( nLogicalIndex, nGlyphIndex, aNewPos,
            GlyphItem::CLUSTER_START, nGlyphWidth );

        aNewPos += Point( nGlyphWidth, 0 );
    }

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
fprintf(stderr,"IcuGetTable(\"%s\") => %p\n",pTagName,pBuffer);
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

void IcuFontFromServerFont::mapCharsToGlyphs( const LEUnicode chars[],
    le_int32 offset, le_int32 count, le_bool reverse,
    const LECharMapper* mapper, LEGlyphID glyphs[] ) const
{
    for( int i = 0; i < count; ++i )
        glyphs[i] = mapCharToGlyph( chars[offset+i], mapper );

    if( reverse )
    {
        for( LEGlyphID *p1=glyphs, *p2=p1+count; p1 < --p2; ++p1 )
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
#ifdef DEBUG
    fprintf(stderr,"mnWidth %d\n",(int)fX);
#endif
    return fX;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::getYPixelsPerEm() const
{
    float fY = mpServerFont->GetFontSelData().mnHeight;
fprintf(stderr,"mnHeight %d\n",(int)fY);
    return fY;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::xUnitsToPoints( float xUnits ) const
{
fprintf(stderr,"xu2p\n");
    // assumption: pixels==points
    float fPoints = xUnits;
    fPoints *= mpServerFont->GetFontSelData().mnWidth;
    fPoints /= mpServerFont->GetEmUnits();
    return fPoints;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::yUnitsToPoints( float yUnits ) const
{
fprintf(stderr,"yu2p\n");
    // assumption: pixels==points
    float fPoints = yUnits;
    fPoints *= mpServerFont->GetFontSelData().mnHeight;
    fPoints /= mpServerFont->GetEmUnits();
    return fPoints;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::unitsToPoints( LEPoint &units, LEPoint &points ) const
{
fprintf(stderr,"pu2p\n");
    points.fX = xUnitsToPoints( units.fX );
    points.fY = yUnitsToPoints( units.fY );
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::xPixelsToUnits( float xPixels ) const
{
fprintf(stderr,"xp2u\n");
    float fPixels = xPixels;
    fPixels *= mpServerFont->GetEmUnits();
    fPixels /= mpServerFont->GetFontSelData().mnWidth;
    return fPixels;
}

// -----------------------------------------------------------------------

float IcuFontFromServerFont::yPixelsToUnits( float yPixels ) const
{
fprintf(stderr,"yp2u\n");
    float fPixels = yPixels;
    fPixels *= mpServerFont->GetEmUnits();
    fPixels /= mpServerFont->GetFontSelData().mnHeight;
    return fPixels;
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::pixelsToUnits( LEPoint &pixels, LEPoint &units ) const
{
fprintf(stderr,"pp2u\n");
    units.fX = xPixelsToUnits( pixels.fX );
    units.fY = yPixelsToUnits( pixels.fY );
}

// -----------------------------------------------------------------------

void IcuFontFromServerFont::transformFunits( float xFunits, float yFunits, LEPoint &pixels ) const
{
fprintf(stderr,"tfu\n");
    //TODO: replace dummy implementation
    Point aOrig( (int)(xFunits + 0.5), (int)(yFunits + 0.5) );
    Point aDest = mpServerFont->TransformPoint( aOrig );
    pixels.fX = aDest.X();
    pixels.fY = aDest.Y();
}

// =======================================================================

class IcuLayoutEngine : public ServerFontLayoutEngine
{
private:
    LayoutEngine*           mpIcuLE;
    IcuFontFromServerFont   maIcuFont;

public:
                            IcuLayoutEngine( FreetypeServerFont* pServerFont );
    virtual                 ~IcuLayoutEngine();
    bool                    IsReady() const { return (mpIcuLE != NULL); }

    virtual ServerFontLayout* operator()( ServerFont*, const ImplLayoutArgs& );
};

// -----------------------------------------------------------------------

IcuLayoutEngine::IcuLayoutEngine( FreetypeServerFont* pServerFont )
:   mpIcuLE( NULL ),
    maIcuFont( pServerFont )
{
    le_int32 eLangCode = 0;
    le_int32 eScriptCode = /*TODO*/ thaiScriptCode;
    //le_int32 eScriptCode = arabScriptCode;

    LEErrorCode rcIcu = LE_NO_ERROR;
    mpIcuLE = LayoutEngine::layoutEngineFactory( &maIcuFont, eScriptCode, eLangCode, rcIcu );
    if( LE_FAILURE(rcIcu) )
    {
        delete mpIcuLE;
        mpIcuLE = NULL;
    }
}

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

    le_bool bRightToLeft = (SAL_LAYOUT_BIDI_RTL & rArgs.mnFlags) != 0;

    // forward to ICU layout engine
    LEErrorCode rcIcu = LE_NO_ERROR;
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
        pGlyphBuffer[i] = GlyphItem( nCharIndex, nGlyphIndex, aNewPos,
            GlyphItem::CLUSTER_START, nGlyphWidth );
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
    ULONG nLength;
    if( GetTable("GSUB",&nLength) || GetTable("GPOS",&nLength) )
    {
        IcuLayoutEngine* pLE = new IcuLayoutEngine( this );
        if( !pLE->IsReady() )
        {
            delete pLE;
            pLE = NULL;
        }
        mpLayoutData = (void*)pLE;
    }
#endif // ENABLE_ICU_LAYOUT

    return (mpLayoutData != NULL);
}

// =======================================================================
