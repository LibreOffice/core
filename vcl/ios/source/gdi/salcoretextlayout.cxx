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

#include "tools/debug.hxx"

#include "ios/saldata.hxx"
#include "ios/salgdi.h"
#include "ios/salcoretextfontutils.hxx"

#include "sallayout.hxx"
#include "salgdi.hxx"

#include <math.h>

class CoreTextLayout : public SalLayout
{
public:
                    CoreTextLayout( CTFontSymbolicTraits&, float fFontScale );
    virtual         ~CoreTextLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;

    virtual long    GetTextWidth() const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const;
    virtual bool    GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const;

    const ImplFontData* GetFallbackFontData( sal_GlyphId ) const;

    virtual void    InitFont() const;
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

private:
    // ???
    float               mfFontScale;

private:
    bool    InitGIA( ImplLayoutArgs* pArgs = NULL ) const;
    bool    GetIdealX() const;
    bool    GetDeltaY() const;
    void    InvalidateMeasurements();

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable int         mnGlyphCount;           // glyph count
    mutable Fixed       mnCachedWidth;          // cached value of resulting typographical width
    int                 mnTrailingSpaceWidth;   // in Pixels

    mutable CGGlyph*        mpGlyphIds;
    mutable Fixed*          mpCharWidths;       // map relative charpos to charwidth
    mutable int*            mpChars2Glyphs;     // map relative charpos to absolute glyphpos
    mutable int*            mpGlyphs2Chars;     // map absolute glyphpos to absolute charpos
    mutable bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true if RTL
    mutable Fixed*          mpGlyphAdvances;    // contains glyph widths for the justified layout
    mutable Fixed*          mpGlyphOrigAdvs;    // contains glyph widths for the unjustified layout
    mutable Fixed*          mpDeltaY;           // vertical offset from the baseline

    struct SubPortion { int mnMinCharPos, mnEndCharPos; Fixed mnXOffset; };
    typedef std::vector<SubPortion> SubPortionVector;
    mutable SubPortionVector    maSubPortions;

    // storing details about fonts used in glyph-fallback for this layout
    mutable class FallbackInfo* mpFallbackInfo;

    // x-offset relative to layout origin
    // currently only used in RTL-layouts
    mutable Fixed           mnBaseAdv;
};

class FallbackInfo
{
public:
    FallbackInfo() : mnMaxLevel(0) {}
    int AddFallback( CTFontRef );
    const ImplFontData* GetFallbackFontData( int nLevel ) const;

private:
    const ImplIosFontData* maFontData[ MAX_FALLBACK ];
    CTFontRef              maCTFontRef[ MAX_FALLBACK ];
    int                    mnMaxLevel;
};

CoreTextLayout::CoreTextLayout( CTFontSymbolicTraits& rCoreTextStyle, float fFontScale )
:
    mfFontScale( fFontScale ),
    mnGlyphCount( -1 ),
    mnCachedWidth( 0 ),
    mnTrailingSpaceWidth( 0 ),
    mpGlyphIds( NULL ),
    mpCharWidths( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mpGlyphRTLFlags( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpDeltaY( NULL ),
    mpFallbackInfo( NULL ),
    mnBaseAdv( 0 )
{
    (void) rCoreTextStyle;
}

// -----------------------------------------------------------------------

CoreTextLayout::~CoreTextLayout()
{
    delete[] mpGlyphRTLFlags;
    delete[] mpGlyphs2Chars;
    delete[] mpChars2Glyphs;
    if( mpCharWidths != mpGlyphAdvances )
        delete[] mpCharWidths;
    delete[] mpGlyphIds;
    delete[] mpGlyphOrigAdvs;
    delete[] mpGlyphAdvances;

    delete mpFallbackInfo;
}

bool CoreTextLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    (void) rArgs;
    // Implement...
    return true;
}

void CoreTextLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    (void) rArgs;
    // Implement...
}

void CoreTextLayout::DrawText( SalGraphics& rGraphics ) const
{
    (void) rGraphics;
    // Implement...
}

int CoreTextLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    (void) nLen;
    (void) pGlyphIDs;
    (void) rPos;
    (void) nStart;
    (void) pGlyphAdvances;
    (void) pCharIndexes;

    if( nStart < 0 )                // first glyph requested?
        nStart = 0;

    // Implement...

    return 0;
}

long CoreTextLayout::GetTextWidth() const
{
    // Implement...

    return 0;
}

long CoreTextLayout::FillDXArray( long* pDXArray ) const
{
    // short circuit requests which don't need full details
    if( !pDXArray )
        return GetTextWidth();

    // Implement...

    return 0;
}

int CoreTextLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    (void) nMaxWidth;
    (void) nCharExtra;
    (void) nFactor;
    // Implement...
    return 0;
}

void CoreTextLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
{
    (void) nMaxIndex;
    (void) pCaretXArray;
    // Implement...
}

bool CoreTextLayout::GetBoundRect( SalGraphics&, Rectangle& rVCLRect ) const
{
    (void) rVCLRect;
    // Implement;
    return true;
}

bool CoreTextLayout::InitGIA( ImplLayoutArgs* pArgs ) const
{
    (void) pArgs;
    // no need to run InitGIA more than once on the same CoreTextLayout object
    if( mnGlyphCount >= 0 )
        return true;
    mnGlyphCount = 0;

    // Implement...

    return true;
}

// -----------------------------------------------------------------------

bool CoreTextLayout::GetIdealX() const
{
    // compute the ideal advance widths only once
    if( mpGlyphOrigAdvs != NULL )
        return true;

    // Implement...

    return true;
}

// -----------------------------------------------------------------------

bool CoreTextLayout::GetDeltaY() const
{
    return true;
}

// -----------------------------------------------------------------------

#define DELETEAZ( X ) { delete[] X; X = NULL; }

void CoreTextLayout::InvalidateMeasurements()
{
    mnGlyphCount = -1;
    DELETEAZ( mpGlyphIds );
    DELETEAZ( mpCharWidths );
    DELETEAZ( mpChars2Glyphs );
    DELETEAZ( mpGlyphs2Chars );
    DELETEAZ( mpGlyphRTLFlags );
    DELETEAZ( mpGlyphAdvances );
    DELETEAZ( mpGlyphOrigAdvs );
    DELETEAZ( mpDeltaY );
}

// glyph fallback is supported directly by Ios
// so methods used only by MultiSalLayout can be dummy implementated
bool CoreTextLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }
void CoreTextLayout::InitFont() const {}
void CoreTextLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}
void CoreTextLayout::DropGlyph( int /*nStart*/ ) {}
void CoreTextLayout::Simplify( bool /*bIsBase*/ ) {}

// get the ImplFontData for a glyph fallback font
// for a glyphid that was returned by CoreTextLayout::GetNextGlyphs()
const ImplFontData* CoreTextLayout::GetFallbackFontData( sal_GlyphId nGlyphId ) const
{
    // check if any fallback fonts were needed
    if( !mpFallbackInfo )
        return NULL;
    // check if the current glyph needs a fallback font
    int nFallbackLevel = (nGlyphId & GF_FONTMASK) >> GF_FONTSHIFT;
    if( !nFallbackLevel )
        return NULL;
    return mpFallbackInfo->GetFallbackFontData( nFallbackLevel );
}

int FallbackInfo::AddFallback( CTFontRef  nFontId )
{
    (void) nFontId;
    // Implement...
    return 0;
}

const ImplFontData* FallbackInfo::GetFallbackFontData( int nFallbackLevel ) const
{
    const ImplIosFontData* pFallbackFont = maFontData[ nFallbackLevel-1 ];
    return pFallbackFont;
}

SalLayout* IosSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
