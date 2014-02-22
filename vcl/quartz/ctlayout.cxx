/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "tools/debug.hxx"

#include "ctfonts.hxx"

class CTLayout
:   public SalLayout
{
public:
    explicit        CTLayout( const CoreTextStyle* );
    virtual         ~CTLayout( void );

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes,
                        const PhysicalFontFace** pFallbackFonts ) const;

    virtual long    GetTextWidth() const;
    virtual long    FillDXArray( sal_Int32* pDXArray ) const;
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra, int nFactor) const SAL_OVERRIDE;
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

    const CoreTextStyle* const    mpTextStyle;

    
    CFAttributedStringRef mpAttrString;
    CTLineRef mpCTLine;

    int mnCharCount;        

    
    
    mutable double  mfCachedWidth;          

    
    
    mutable double  mfBaseAdv;
};

CTLayout::CTLayout( const CoreTextStyle* pTextStyle )
:   mpTextStyle( pTextStyle )
,   mpAttrString( NULL )
,   mpCTLine( NULL )
,   mnCharCount( 0 )
,   mfCachedWidth( -1 )
,   mfBaseAdv( 0 )
{
}

CTLayout::~CTLayout()
{
    if( mpCTLine )
        CFRelease( mpCTLine );
    if( mpAttrString )
        CFRelease( mpAttrString );
}

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

    
    if( mnCharCount <= 0 )
        return false;

    
    CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL, rArgs.mpStr + mnMinCharPos, mnCharCount, kCFAllocatorNull );
    
    mpAttrString = CFAttributedStringCreate( NULL, aCFText, mpTextStyle->GetStyleDict() );
    mpCTLine = CTLineCreateWithAttributedString( mpAttrString );
    CFRelease( aCFText);

    return true;
}

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
        
        
        nPixelWidth = rArgs.mpDXArray[ mnCharCount - 1 ];
    }

    float fTrailingSpace = CTLineGetTrailingWhitespaceWidth( mpCTLine );
    
    
    if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
        mfBaseAdv = fTrailingSpace;

    
    if( nPixelWidth <= 0 )
        return;

    
    
    if( (nOrigWidth >= nPixelWidth-1) && (nOrigWidth <= nPixelWidth+1) )
        return;

    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, nPixelWidth - fTrailingSpace );
    if( !pNewCTLine ) { 
        
        
        
        
        
        return;
    }
    CFRelease( mpCTLine );
    mpCTLine = pNewCTLine;
    mfCachedWidth = nPixelWidth;
}







//


CGPoint CTLayout::GetTextDrawPosition(void) const
{
    float fPosX, fPosY;

    if (mnLayoutFlags & SAL_LAYOUT_RIGHT_ALIGN)
    {
        
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

    
    if( (mnCharCount <= 0)
    ||  !rAquaGraphics.CheckContext() )
        return;

    
    
    
    CGContextSaveGState( rAquaGraphics.mrContext );
    CGContextScaleCTM( rAquaGraphics.mrContext, 1.0, -1.0 );
    CGContextSetShouldAntialias( rAquaGraphics.mrContext, !rAquaGraphics.mbNonAntialiasedText );

    
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
    
    if( rAquaGraphics.IsWindowGraphics() )
    {
        const CGRect aInkRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );
        const CGRect aRefreshRect = CGContextConvertRectToDeviceSpace( rAquaGraphics.mrContext, aInkRect );
        rAquaGraphics.RefreshRect( aRefreshRect );
    }
#endif
    
    CGContextRestoreGState( rAquaGraphics.mrContext );
}

int CTLayout::GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes,
    const PhysicalFontFace** pFallbackFonts ) const
{
    if( !mpCTLine )
        return 0;

    if( nStart < 0 ) 
        nStart = 0;
    nLen = 1; 

    
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

    
    CFArrayRef aGlyphRuns = CTLineGetGlyphRuns( mpCTLine );
    const int nRunCount = CFArrayGetCount( aGlyphRuns );
    for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex ) {
        CTRunRef pGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex );
        const CFIndex nGlyphsInRun = CTRunGetGlyphCount( pGlyphRun );
        
        if( nSubIndex >= nGlyphsInRun ) {
            nSubIndex -= nGlyphsInRun;
            continue;
        }
        const CFRange aFullRange = CFRangeMake( 0, nGlyphsInRun );

        
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
                pFallbackFont = new CoreTextFontData( rDevFontAttr, (sal_IntPtr)pFontDesc );
            }
        }

        
        
        for(; (--nLen >= 0) && (nSubIndex < nGlyphsInRun); ++nSubIndex, ++nStart )
        {
            
            *(pOutGlyphIds++) = pCGGlyphIdx[ nSubIndex ];
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
        nSubIndex = 0; 
        break; 
    }

    return nCount;
}

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

long CTLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    
    if( !pDXArray )
        return GetTextWidth();

    long nPixWidth = GetTextWidth();
    if( pDXArray ) {
        
        for( int i = 0; i < mnCharCount; ++i)
            pDXArray[i] = 0;
        
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

sal_Int32 CTLayout::GetTextBreak( long nMaxWidth, long /*nCharExtra*/, int nFactor ) const
{
    if( !mpCTLine )
        return -1;

    CTTypesetterRef aCTTypeSetter = CTTypesetterCreateWithAttributedString( mpAttrString );
    const double fCTMaxWidth = (double)nMaxWidth / nFactor;
    CFIndex nIndex = CTTypesetterSuggestClusterBreak( aCTTypeSetter, 0, fCTMaxWidth );
    if( nIndex >= mnCharCount )
        return -1;

    nIndex += mnMinCharPos;
    return nIndex;
}

void CTLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    DBG_ASSERT( ((nMaxIndex>0)&&!(nMaxIndex&1)),
        "CTLayout::GetCaretPositions() : invalid number of caret pairs requested");

    
    for( int i = 0; i < nMaxIndex; ++i )
        pCaretXArray[ i ] = -1;

    for( int n = 0; n <= mnCharCount; ++n )
    {
        
        CGFloat fPos2 = -1;
        const CGFloat fPos1 = CTLineGetOffsetForStringIndex( mpCTLine, n, &fPos2 );
        (void)fPos2; 
        
        if( n > 0 )
            pCaretXArray[ 2*n-1 ] = lrint( fPos1 );
        
        if( 2*n >= nMaxIndex )
            break;
        pCaretXArray[ 2*n+0 ] = lrint( fPos1 );
    }
}

bool CTLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{
    
    
    
    

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



bool CTLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }
void CTLayout::InitFont() const {}
void CTLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}
void CTLayout::DropGlyph( int /*nStart*/ ) {}
void CTLayout::Simplify( bool /*bIsBase*/ ) {}

SalLayout* CoreTextStyle::GetTextLayout( void ) const
{
    return new CTLayout( this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
