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

#include "tools/debug.hxx"

#include "ctfonts.hxx"

// =======================================================================

class CTLayout
:   public SalLayout
{
public:
    explicit        CTLayout( const CTTextStyle* );
    virtual         ~CTLayout( void );

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes,
                        const PhysicalFontFace** pFallbackFonts ) const;

    virtual long    GetTextWidth() const;
    virtual long    FillDXArray( sal_Int32* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;
    virtual bool    GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const;

    virtual void    InitFont( void) const;
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

private:
    CGPoint         GetTextDrawPosition(void) const;
    double          GetWidth(void) const;

    const CTTextStyle* const    mpTextStyle;

    // CoreText specific objects
    CFAttributedStringRef mpAttrString;
    CTLineRef mpCTLine;

    int mnCharCount;        // ==mnEndCharPos-mnMinCharPos

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable double  mfCachedWidth;          // cached value of resulting typographical width

    // x-offset relative to layout origin
    // currently only used in RTL-layouts
    mutable double  mfBaseAdv;
};

// =======================================================================

CTLayout::CTLayout( const CTTextStyle* pTextStyle )
:   mpTextStyle( pTextStyle )
,   mpAttrString( NULL )
,   mpCTLine( NULL )
,   mnCharCount( 0 )
,   mfCachedWidth( -1 )
,   mfBaseAdv( 0 )
{
}

// -----------------------------------------------------------------------

CTLayout::~CTLayout()
{
    if( mpCTLine )
        CFRelease( mpCTLine );
    if( mpAttrString )
        CFRelease( mpAttrString );
}

// -----------------------------------------------------------------------

bool CTLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    if( mpAttrString )
        CFRelease( mpAttrString );
    mpAttrString = NULL;
    if( mpCTLine )
        CFRelease( mpCTLine );
    mpCTLine = NULL;

    SalLayout::AdjustLayout( rArgs );
    mnCharCount = mnEndCharPos - mnMinCharPos;

    // short circuit if there is nothing to do
    if( mnCharCount <= 0 )
        return false;

    // create the CoreText line layout
    CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL, rArgs.mpStr + mnMinCharPos, mnCharCount, kCFAllocatorNull );
    // CFAttributedStringCreate copies the attribues parameter
    mpAttrString = CFAttributedStringCreate( NULL, aCFText, mpTextStyle->GetStyleDict() );
    mpCTLine = CTLineCreateWithAttributedString( mpAttrString );
    CFRelease( aCFText);

    return true;
}

// -----------------------------------------------------------------------

void CTLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    if( !mpCTLine)
        return;

    int nOrigWidth = GetTextWidth();
    int nPixelWidth = rArgs.mnLayoutWidth;
    if( nPixelWidth )
    {
        if( nPixelWidth <= 0)
            return;
    }
    else if( rArgs.mpDXArray )
    {
        // for now we are only interested in the layout width
        // TODO: use all mpDXArray elements for layouting
        nPixelWidth = rArgs.mpDXArray[ mnCharCount - 1 ];
    }

    float fTrailingSpace = CTLineGetTrailingWhitespaceWidth( mpCTLine );
    // in RTL-layouts trailing spaces are leftmost
    // TODO: use BiDi-algorithm to thoroughly check this assumption
    if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
        mfBaseAdv = fTrailingSpace;

    // return early if there is nothing to do
    if( nPixelWidth <= 0 )
        return;

    // HACK: justification requests which change the width by just one pixel are probably
    // #i86038# introduced by lossy conversions between integer based coordinate system
    if( (nOrigWidth >= nPixelWidth-1) && (nOrigWidth <= nPixelWidth+1) )
        return;

    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, nPixelWidth - fTrailingSpace );
    if( !pNewCTLine ) { // CTLineCreateJustifiedLine can and does fail
        // handle failure by keeping the unjustified layout
        // TODO: a better solution such as
        // - forcing glyph overlap
        // - changing the font size
        // - changing the CTM matrix
        return;
    }
    CFRelease( mpCTLine );
    mpCTLine = pNewCTLine;
    mfCachedWidth = nPixelWidth;
}

// -----------------------------------------------------------------------

// When drawing right aligned text, rounding errors in the position returned by
// GetDrawPosition() cause the right margin of the text to change whenever text
// width changes causing "jumping letters" effect. So here we calculate the
// drawing position relative to the right margin on our own to avoid the
// rounding errors. That is basically a hack, and it should go away if one day
// we managed to get rid of those rounding errors.
//
// We continue using GetDrawPosition() for non-right aligned text, to minimize
// any unforeseen side effects.
CGPoint CTLayout::GetTextDrawPosition(void) const
{
    float fPosX, fPosY;

    if (mnLayoutFlags & SAL_LAYOUT_RIGHT_ALIGN)
    {
        // text is always drawn at its leftmost point
        const Point aPos = DrawBase();
        fPosX = aPos.X() + mfBaseAdv - GetWidth();
        fPosY = aPos.Y();
    }
    else
    {
        const Point aPos = GetDrawPosition(Point(mfBaseAdv, 0));
        fPosX = aPos.X();
        fPosY = aPos.Y();
    }

    CGPoint aTextPos = { +fPosX, -fPosY };
    return aTextPos;
}

void CTLayout::DrawText( SalGraphics& rGraphics ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    // short circuit if there is nothing to do
    if( (mnCharCount <= 0)
    ||  !rAquaGraphics.CheckContext() )
        return;

    // the view is vertically flipped => flipped glyphs
    // so apply a temporary transformation that it flips back
    // also compensate if the font was size limited
    CGContextSaveGState( rAquaGraphics.mrContext );
    CGContextScaleCTM( rAquaGraphics.mrContext, 1.0, -1.0 );
    CGContextSetShouldAntialias( rAquaGraphics.mrContext, !rAquaGraphics.mbNonAntialiasedText );

    // Draw the text
    CGPoint aTextPos = GetTextDrawPosition();

    if( mpTextStyle->mfFontRotation != 0.0 )
    {
        const CGFloat fRadians = mpTextStyle->mfFontRotation;
        CGContextRotateCTM( rAquaGraphics.mrContext, +fRadians );

        const CGAffineTransform aInvMatrix = CGAffineTransformMakeRotation( -fRadians );
        aTextPos = CGPointApplyAffineTransform( aTextPos, aInvMatrix );
    }

    CGContextSetTextPosition( rAquaGraphics.mrContext, aTextPos.x, aTextPos.y );
    CTLineDraw( mpCTLine, rAquaGraphics.mrContext );
#ifndef IOS
    // request an update of the changed window area
    if( rAquaGraphics.IsWindowGraphics() )
    {
        const CGRect aInkRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );
        const CGRect aRefreshRect = CGContextConvertRectToDeviceSpace( rAquaGraphics.mrContext, aInkRect );
        rAquaGraphics.RefreshRect( aRefreshRect );
    }
#endif
    // restore the original graphic context transformations
    CGContextRestoreGState( rAquaGraphics.mrContext );
}

// -----------------------------------------------------------------------

int CTLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes,
    const PhysicalFontFace** pFallbackFonts ) const
{
    if( !mpCTLine )
        return 0;

    if( nStart < 0 ) // first glyph requested?
        nStart = 0;
    nLen = 1; // TODO: handle nLen>1 below

    // prepare to iterate over the glyph runs
    int nCount = 0;
    int nSubIndex = nStart;

    typedef std::vector<CGGlyph> CGGlyphVector;
    typedef std::vector<CGPoint> CGPointVector;
    typedef std::vector<CGSize>  CGSizeVector;
    typedef std::vector<CFIndex> CFIndexVector;
    CGGlyphVector aCGGlyphVec;
    CGPointVector aCGPointVec;
    CGSizeVector  aCGSizeVec;
    CFIndexVector aCFIndexVec;

    // TODO: iterate over cached layout
    CFArrayRef aGlyphRuns = CTLineGetGlyphRuns( mpCTLine );
    const int nRunCount = CFArrayGetCount( aGlyphRuns );
    for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex ) {
        CTRunRef pGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex );
        const CFIndex nGlyphsInRun = CTRunGetGlyphCount( pGlyphRun );
        // skip to the first glyph run of interest
        if( nSubIndex >= nGlyphsInRun ) {
            nSubIndex -= nGlyphsInRun;
            continue;
        }
        const CFRange aFullRange = CFRangeMake( 0, nGlyphsInRun );

        // get glyph run details
        const CGGlyph* pCGGlyphIdx = CTRunGetGlyphsPtr( pGlyphRun );
        if( !pCGGlyphIdx ) {
            aCGGlyphVec.reserve( nGlyphsInRun );
            CTRunGetGlyphs( pGlyphRun, aFullRange, &aCGGlyphVec[0] );
            pCGGlyphIdx = &aCGGlyphVec[0];
        }
        const CGPoint* pCGGlyphPos = CTRunGetPositionsPtr( pGlyphRun );
        if( !pCGGlyphPos ) {
            aCGPointVec.reserve( nGlyphsInRun );
            CTRunGetPositions( pGlyphRun, aFullRange, &aCGPointVec[0] );
            pCGGlyphPos = &aCGPointVec[0];
        }

        const CGSize* pCGGlyphAdvs = NULL;
        if( pGlyphAdvances) {
            pCGGlyphAdvs = CTRunGetAdvancesPtr( pGlyphRun );
            if( !pCGGlyphAdvs) {
                aCGSizeVec.reserve( nGlyphsInRun );
                CTRunGetAdvances( pGlyphRun, aFullRange, &aCGSizeVec[0] );
                pCGGlyphAdvs = &aCGSizeVec[0];
            }
        }

        const CFIndex* pCGGlyphStrIdx = NULL;
        if( pCharIndexes) {
            pCGGlyphStrIdx = CTRunGetStringIndicesPtr( pGlyphRun );
            if( !pCGGlyphStrIdx) {
                aCFIndexVec.reserve( nGlyphsInRun );
                CTRunGetStringIndices( pGlyphRun, aFullRange, &aCFIndexVec[0] );
                pCGGlyphStrIdx = &aCFIndexVec[0];
            }
        }

        const PhysicalFontFace* pFallbackFont = NULL;
        if( pFallbackFonts ) {
            CFDictionaryRef pRunAttributes = CTRunGetAttributes( pGlyphRun );
            CTFontRef pRunFont = (CTFontRef)CFDictionaryGetValue( pRunAttributes, kCTFontAttributeName );

            CFDictionaryRef pAttributes = mpTextStyle->GetStyleDict();
            CTFontRef pFont = (CTFontRef)CFDictionaryGetValue( pAttributes, kCTFontAttributeName );
            if ( !CFEqual( pRunFont,  pFont ) ) {
                CTFontDescriptorRef pFontDesc = CTFontCopyFontDescriptor( pRunFont );
                ImplDevFontAttributes rDevFontAttr = DevFontFromCTFontDescriptor( pFontDesc, NULL );
                pFallbackFont = new CTFontData( rDevFontAttr, (sal_IntPtr)pFontDesc );
            }
        }

        // get the details for each interesting glyph
        // TODO: handle nLen>1
        for(; (--nLen >= 0) && (nSubIndex < nGlyphsInRun); ++nSubIndex, ++nStart )
        {
            // convert glyph details for VCL
            *(pGlyphIDs++) = pCGGlyphIdx[ nSubIndex ];
            if( pGlyphAdvances )
                *(pGlyphAdvances++) = lrint(pCGGlyphAdvs[ nSubIndex ].width);
            if( pCharIndexes )
                *(pCharIndexes++) = pCGGlyphStrIdx[ nSubIndex] + mnMinCharPos;
            if( pFallbackFonts )
                *(pFallbackFonts++) = pFallbackFont;
            if( !nCount++ ) {
                const CGPoint& rCurPos = pCGGlyphPos[ nSubIndex ];
                rPos = GetDrawPosition( Point( rCurPos.x, rCurPos.y) );
            }
        }
        nSubIndex = 0; // prepare for the next glyph run
        break; // TODO: handle nLen>1
    }

    return nCount;
}

// -----------------------------------------------------------------------

double CTLayout::GetWidth() const
{
    if( (mnCharCount <= 0) || !mpCTLine )
        return 0;

    if( mfCachedWidth < 0.0 ) {
        mfCachedWidth = CTLineGetTypographicBounds( mpCTLine, NULL, NULL, NULL);
    }

    return mfCachedWidth;
}

long CTLayout::GetTextWidth() const
{
    return lrint(GetWidth());
}
// -----------------------------------------------------------------------

long CTLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    // short circuit requests which don't need full details
    if( !pDXArray )
        return GetTextWidth();

    long nPixWidth = GetTextWidth();
    if( pDXArray ) {
        // initialize the result array
        for( int i = 0; i < mnCharCount; ++i)
            pDXArray[i] = 0;
        // handle each glyph run
        CFArrayRef aGlyphRuns = CTLineGetGlyphRuns( mpCTLine );
        const int nRunCount = CFArrayGetCount( aGlyphRuns );
        typedef std::vector<CGSize> CGSizeVector;
        CGSizeVector aSizeVec;
        typedef std::vector<CFIndex> CFIndexVector;
        CFIndexVector aIndexVec;
        for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex ) {
            CTRunRef pGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex );
            const CFIndex nGlyphCount = CTRunGetGlyphCount( pGlyphRun );
            const CFRange aFullRange = CFRangeMake( 0, nGlyphCount );
            aSizeVec.reserve( nGlyphCount );
            aIndexVec.reserve( nGlyphCount );
            CTRunGetAdvances( pGlyphRun, aFullRange, &aSizeVec[0] );
            CTRunGetStringIndices( pGlyphRun, aFullRange, &aIndexVec[0] );
            for( int i = 0; i != nGlyphCount; ++i ) {
                const int nRelIdx = aIndexVec[i];
                pDXArray[ nRelIdx ] += lrint(aSizeVec[i].width);
            }
        }
    }

    return nPixWidth;
}

// -----------------------------------------------------------------------

int CTLayout::GetTextBreak( long nMaxWidth, long /*nCharExtra*/, int nFactor ) const
{
    if( !mpCTLine )
        return STRING_LEN;

    CTTypesetterRef aCTTypeSetter = CTTypesetterCreateWithAttributedString( mpAttrString );
    const double fCTMaxWidth = (double)nMaxWidth / nFactor;
    CFIndex nIndex = CTTypesetterSuggestClusterBreak( aCTTypeSetter, 0, fCTMaxWidth );
    if( nIndex >= mnCharCount )
        return STRING_LEN;

    nIndex += mnMinCharPos;
    return (int)nIndex;
}

// -----------------------------------------------------------------------

void CTLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    DBG_ASSERT( ((nMaxIndex>0)&&!(nMaxIndex&1)),
        "CTLayout::GetCaretPositions() : invalid number of caret pairs requested");

    // initialize the caret positions
    for( int i = 0; i < nMaxIndex; ++i )
        pCaretXArray[ i ] = -1;

    for( int n = 0; n <= mnCharCount; ++n )
    {
        // measure the characters cursor position
        CGFloat fPos2 = -1;
        const CGFloat fPos1 = CTLineGetOffsetForStringIndex( mpCTLine, n, &fPos2 );
        (void)fPos2; // TODO: split cursor at line direction change
        // update previous trailing position
        if( n > 0 )
            pCaretXArray[ 2*n-1 ] = lrint( fPos1 );
        // update current leading position
        if( 2*n >= nMaxIndex )
            break;
        pCaretXArray[ 2*n+0 ] = lrint( fPos1 );
    }
}

// -----------------------------------------------------------------------

bool CTLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{
    // Closely mimic DrawText(), except that instead of calling
    // CTLineDraw() to draw the line, we call CTLineGetImageBounds()
    // to get its bounds. But all the coordinate system manipulation
    // before that is the same => should be factored out?

    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    if( !rAquaGraphics.CheckContext() )
        return false;

    CGContextSaveGState( rAquaGraphics.mrContext );
    CGContextScaleCTM( rAquaGraphics.mrContext, 1.0, -1.0 );
    CGContextSetShouldAntialias( rAquaGraphics.mrContext, !rAquaGraphics.mbNonAntialiasedText );

    const CGPoint aVclPos = GetTextDrawPosition();
    CGPoint aTextPos = GetTextDrawPosition();

    if( mpTextStyle->mfFontRotation != 0.0 )
    {
        const CGFloat fRadians = mpTextStyle->mfFontRotation;
        CGContextRotateCTM( rAquaGraphics.mrContext, +fRadians );

        const CGAffineTransform aInvMatrix = CGAffineTransformMakeRotation( -fRadians );
        aTextPos = CGPointApplyAffineTransform( aTextPos, aInvMatrix );
    }

    CGContextSetTextPosition( rAquaGraphics.mrContext, aTextPos.x, aTextPos.y );
    CGRect aMacRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );

    if( mpTextStyle->mfFontRotation != 0.0 )
    {
        const CGFloat fRadians = mpTextStyle->mfFontRotation;
        const CGAffineTransform aMatrix = CGAffineTransformMakeRotation( +fRadians );
        aMacRect = CGRectApplyAffineTransform( aMacRect, aMatrix );
    }

    CGContextRestoreGState( rAquaGraphics.mrContext );

    rVCLRect.Left()   = aVclPos.x + lrint(aMacRect.origin.x);
    rVCLRect.Right()  = aVclPos.x + lrint(aMacRect.origin.x + aMacRect.size.width);
    rVCLRect.Bottom() = aVclPos.x - lrint(aMacRect.origin.y);
    rVCLRect.Top()    = aVclPos.x - lrint(aMacRect.origin.y + aMacRect.size.height);

    return true;
}

// =======================================================================

// glyph fallback is supported directly by Aqua
// so methods used only by MultiSalLayout can be dummy implementated
bool CTLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }
void CTLayout::InitFont() const {}
void CTLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}
void CTLayout::DropGlyph( int /*nStart*/ ) {}
void CTLayout::Simplify( bool /*bIsBase*/ ) {}

// =======================================================================

SalLayout* CTTextStyle::GetTextLayout( void ) const
{
    return new CTLayout( this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
