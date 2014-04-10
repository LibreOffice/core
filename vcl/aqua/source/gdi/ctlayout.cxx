/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;

    virtual long    GetTextWidth() const;
    virtual long    FillDXArray( sal_Int32* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;
    virtual bool    GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const;

    const ImplFontData* GetFallbackFontData( sal_GlyphId ) const;

    virtual void    InitFont( void) const;
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

private:
    // CoreText specific objects
    CFMutableDictionaryRef mpStyleDict;
    CFAttributedStringRef mpAttrString;
    CTLineRef mpCTLine;

    int mnCharCount;        // ==mnEndCharPos-mnMinCharPos
    int mnTrailingSpaceCount;
    double mfTrailingSpaceWidth;    // preserves the width of stripped-off trailing space

    // to prevent overflows
    // font requests get size limited by downscaling huge fonts
    // in these cases the font scale becomes something bigger than 1.0
    float mfFontScale; // TODO: does CoreText have a font size limit?

    CGFloat mfFontRotation; // text direction angle (in radians)
    CGFloat mfFontStretch;  <1.0: font is squeezed, >1.0 font is stretched

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable double  mfCachedWidth;          // cached value of resulting typographical width

    // x-offset relative to layout origin
    // currently only used in RTL-layouts
    mutable long    mnBaseAdv;
};

// =======================================================================

CTLayout::CTLayout( const CTTextStyle* pTextStyle )
:   mpStyleDict( pTextStyle->GetStyleDict() )
,   mpAttrString( NULL )
,   mpCTLine( NULL )
,   mnCharCount( 0 )
,   mnTrailingSpaceCount( 0 )
,   mfTrailingSpaceWidth( 0.0 )
,   mfFontScale( pTextStyle->mfFontScale )
,   mfFontRotation( pTextStyle->mfFontRotation )
,   mfFontStretch( pTextStyle->mfFontStretch )
,   mfCachedWidth( -1 )
,   mnBaseAdv( 0 )
{
    CFRetain( mpStyleDict );
}

// -----------------------------------------------------------------------

CTLayout::~CTLayout()
{
    if( mpCTLine )
        CFRelease( mpCTLine );
    if( mpAttrString )
        CFRelease( mpAttrString );
    CFRelease( mpStyleDict );
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

    // prepare the string to be layouted by CoreText
    CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL, rArgs.mpStr + mnMinCharPos, mnCharCount, kCFAllocatorNull );
    // #i124375# force soft-hyphen visibility to meet the expectations of Writer+EditEngine
    if( CFStringFind( aCFText, (CFStringRef)@"\u00AD", 0).length > 0 )
    {
        NSString* pDashStr = [(NSString*)aCFText stringByReplacingOccurrencesOfString: @"\u00AD" withString: @"-"];
        aCFText = CFStringCreateCopy( NULL, (CFStringRef)pDashStr );
    }

    // create the CoreText line layout using the requested text style
    mpAttrString = CFAttributedStringCreate( NULL, aCFText, mpStyleDict );
    mpCTLine = CTLineCreateWithAttributedString( mpAttrString );
    CFRelease( aCFText);

    // get info about trailing whitespace to prepare for text justification in AdjustLayout()
    mnTrailingSpaceCount = 0;
    for( int i = mnEndCharPos; --i >= mnMinCharPos; ++mnTrailingSpaceCount )
        if( !IsSpacingGlyph( rArgs.mpStr[i] | GF_ISCHAR )
        &&  (rArgs.mpStr[i] != 0x00A0) )
            break;
    return true;
}

// -----------------------------------------------------------------------

void CTLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    if( !mpCTLine)
        return;

    int nPixelWidth = rArgs.mnLayoutWidth;
    if( rArgs.mpDXArray )
    {
        // for now we are only interested in the layout width
        // TODO: use all mpDXArray elements for layouting
        nPixelWidth = rArgs.mpDXArray[ mnCharCount-1 ];
    }
    else if( !nPixelWidth ) // short-circuit if there is nothing to adjust
        return;

    // short-circuit when justifying an all-whitespace string
    if( mnTrailingSpaceCount >= mnCharCount)
    {
        mfCachedWidth = nPixelWidth / mfFontScale;
        return;
    }

    // return early if there is nothing to do
    if( nPixelWidth <= 0 )
        return;

    // HACK: justification requests which change the width by just one pixel are probably
    // #i86038# introduced by lossy conversions between integer based coordinate system
    const int nOrigWidth = GetTextWidth();
    if( (nOrigWidth >= nPixelWidth-1) && (nOrigWidth <= nPixelWidth+1) )
        return;

    // if the text to be justified has whitespace in it then
    // - Writer goes crazy with its HalfSpace magic
    // - CoreText handles spaces specially (in particular at the text end)
    if( mnTrailingSpaceCount ) {
        int nTrailingSpaceWidth = 0;
        if( rArgs.mpDXArray) {
            const int nFullPixWidth = nPixelWidth;
            nPixelWidth = rArgs.mpDXArray[ mnCharCount-1-mnTrailingSpaceCount ];
            nTrailingSpaceWidth = nFullPixWidth - nPixelWidth;
            mfTrailingSpaceWidth = nTrailingSpaceWidth;
        } else {
            if( mfTrailingSpaceWidth <= 0.0 )
                mfTrailingSpaceWidth = CTLineGetTrailingWhitespaceWidth( mpCTLine );
            nTrailingSpaceWidth = rint( mfTrailingSpaceWidth );
            nPixelWidth -= nTrailingSpaceWidth;
        }
        if( nPixelWidth <= 0 )
            return;

        // recreate the CoreText line layout without trailing spaces
        CFRelease( mpCTLine );
        CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL, rArgs.mpStr + mnMinCharPos,
            mnCharCount - mnTrailingSpaceCount, kCFAllocatorNull );
        CFAttributedStringRef pAttrStr = CFAttributedStringCreate( NULL, aCFText, mpStyleDict );
        mpCTLine = CTLineCreateWithAttributedString( pAttrStr );
        CFRelease( aCFText);
        CFRelease( pAttrStr );

        // in RTL-layouts trailing spaces are leftmost
        // TODO: use BiDi-algorithm to thoroughly check this assumption
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
            mnBaseAdv = nTrailingSpaceWidth;
    }

    const double fAdjustedWidth = nPixelWidth / mfFontScale;
    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, fAdjustedWidth );
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
    mfCachedWidth = fAdjustedWidth + mfTrailingSpaceWidth;
}

// -----------------------------------------------------------------------

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
    CGContextScaleCTM( rAquaGraphics.mrContext, +mfFontScale, -mfFontScale );
    CGContextSetShouldAntialias( rAquaGraphics.mrContext, !rAquaGraphics.mbNonAntialiasedText );

    // Draw the text
    const Point aVclPos = GetDrawPosition( Point(mnBaseAdv,0) );
    CGPoint aTextPos = { +aVclPos.X()/mfFontScale, -aVclPos.Y()/mfFontScale };

    if( mfFontRotation != 0.0 )
    {
        CGContextRotateCTM( rAquaGraphics.mrContext, +mfFontRotation );

        const CGAffineTransform aInvMatrix = CGAffineTransformMakeRotation( -mfFontRotation );
        aTextPos = CGPointApplyAffineTransform( aTextPos, aInvMatrix );
    }

    CGContextSetTextPosition( rAquaGraphics.mrContext, aTextPos.x, aTextPos.y );
    CTLineDraw( mpCTLine, rAquaGraphics.mrContext );

    // request an update of the changed window area
    if( rAquaGraphics.IsWindowGraphics() )
    {
        const CGRect aInkRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );
        const CGRect aRefreshRect = CGContextConvertRectToDeviceSpace( rAquaGraphics.mrContext, aInkRect );
        rAquaGraphics.RefreshRect( aRefreshRect );
    }

    // restore the original graphic context transformations
    CGContextRestoreGState( rAquaGraphics.mrContext );
}

// -----------------------------------------------------------------------

int CTLayout::GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    if( !mpCTLine )
        return 0;

    if( nStart < 0 ) // first glyph requested?
        nStart = 0;
    nLen = 1; // TODO: handle nLen>1 below

    // prepare to iterate over the glyph runs
    int nCount = 0;
    int nSubIndex = nStart;

    const DynCoreTextSyms& rCT = DynCoreTextSyms::get();
    typedef std::vector<CGGlyph> CGGlyphVector;
    typedef std::vector<CGPoint> CGPointVector;
    typedef std::vector<CGSize>  CGSizeVector;
    typedef std::vector<CFIndex> CFIndexVector;
    CGGlyphVector aCGGlyphVec;
    CGPointVector aCGPointVec;
    CGSizeVector  aCGSizeVec;
    CFIndexVector aCFIndexVec;

    // TODO: iterate over cached layout
    CFArrayRef aGlyphRuns = rCT.LineGetGlyphRuns( mpCTLine );
    const int nRunCount = CFArrayGetCount( aGlyphRuns );
    for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex ) {
        CTRunRef pGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex );
        const CFIndex nGlyphsInRun = rCT.RunGetGlyphCount( pGlyphRun );
        // skip to the first glyph run of interest
        if( nSubIndex >= nGlyphsInRun ) {
            nSubIndex -= nGlyphsInRun;
            continue;
        }
        const CFRange aFullRange = CFRangeMake( 0, nGlyphsInRun );

        // get glyph run details
        const CGGlyph* pCGGlyphIdx = rCT.RunGetGlyphsPtr( pGlyphRun );
        if( !pCGGlyphIdx ) {
            aCGGlyphVec.reserve( nGlyphsInRun );
            CTRunGetGlyphs( pGlyphRun, aFullRange, &aCGGlyphVec[0] );
            pCGGlyphIdx = &aCGGlyphVec[0];
        }
        const CGPoint* pCGGlyphPos = rCT.RunGetPositionsPtr( pGlyphRun );
        if( !pCGGlyphPos ) {
            aCGPointVec.reserve( nGlyphsInRun );
            CTRunGetPositions( pGlyphRun, aFullRange, &aCGPointVec[0] );
            pCGGlyphPos = &aCGPointVec[0];
        }

        const CGSize* pCGGlyphAdvs = NULL;
        if( pGlyphAdvances) {
            pCGGlyphAdvs = rCT.RunGetAdvancesPtr( pGlyphRun );
            if( !pCGGlyphAdvs) {
                aCGSizeVec.reserve( nGlyphsInRun );
                CTRunGetAdvances( pGlyphRun, aFullRange, &aCGSizeVec[0] );
                pCGGlyphAdvs = &aCGSizeVec[0];
            }
        }

        const CFIndex* pCGGlyphStrIdx = NULL;
        if( pCharIndexes) {
            pCGGlyphStrIdx = rCT.RunGetStringIndicesPtr( pGlyphRun );
            if( !pCGGlyphStrIdx) {
                aCFIndexVec.reserve( nGlyphsInRun );
                CTRunGetStringIndices( pGlyphRun, aFullRange, &aCFIndexVec[0] );
                pCGGlyphStrIdx = &aCFIndexVec[0];
            }
        }

        // get the details for each interesting glyph
        // TODO: handle nLen>1
        for(; (--nLen >= 0) && (nSubIndex < nGlyphsInRun); ++nSubIndex, ++nStart )
        {
            // convert glyph details for VCL
            *(pOutGlyphIds++) = pCGGlyphIdx[ nSubIndex ];
            if( pGlyphAdvances )
                *(pGlyphAdvances++) = mfFontStretch * pCGGlyphAdvs[ nSubIndex ].width;
            if( pCharIndexes )
                *(pCharIndexes++) = pCGGlyphStrIdx[ nSubIndex] + mnMinCharPos;
            if( !nCount++ ) {
                const CGPoint& rCurPos = pCGGlyphPos[ nSubIndex ];
                rPos = GetDrawPosition( Point( mfFontScale * mfFontStretch * rCurPos.x, mfFontScale * rCurPos.y) );
            }
        }
        nSubIndex = 0; // prepare for the next glyph run
        break; // TODO: handle nLen>1
    }

    return nCount;
}

// -----------------------------------------------------------------------

long CTLayout::GetTextWidth() const
{
    if( (mnCharCount <= 0) || !mpCTLine )
        return 0;

    if( mfCachedWidth < 0.0 )
        mfCachedWidth = CTLineGetTypographicBounds( mpCTLine, NULL, NULL, NULL );

    const long nScaledWidth = lrint( mfFontScale * mfCachedWidth );
    return nScaledWidth;
}

// -----------------------------------------------------------------------

long CTLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    // short circuit requests which don't need full details
    if( !pDXArray )
        return GetTextWidth();

    long nPixWidth = GetTextWidth();
    if( pDXArray ) {
        // prepare the sub-pixel accurate logical-width array
        ::std::vector<float> aWidthVector( mnCharCount );
        if( mnTrailingSpaceCount && (mfTrailingSpaceWidth > 0.0) ) {
            const double fOneWidth = mfTrailingSpaceWidth / mnTrailingSpaceCount;
            for( int i = 1; i <= mnTrailingSpaceCount; ++i)
                aWidthVector[ mnCharCount - i ] = fOneWidth;
        }
        // measure advances in each glyph run
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
            aSizeVec.resize( nGlyphCount );
            aIndexVec.resize( nGlyphCount );
            CTRunGetAdvances( pGlyphRun, aFullRange, &aSizeVec[0] );
            CTRunGetStringIndices( pGlyphRun, aFullRange, &aIndexVec[0] );
            for( int i = 0; i != nGlyphCount; ++i ) {
                const int nRelIdx = aIndexVec[i];
                aWidthVector[nRelIdx] += aSizeVec[i].width;
            }
        }

        // convert the sub-pixel accurate array into classic pDXArray integers
        float fWidthSum = 0.0;
        sal_Int32 nOldDX = 0;
        for( int i = 0; i < mnCharCount; ++i) {
            const sal_Int32 nNewDX = rint( fWidthSum += aWidthVector[i]);
            pDXArray[i] = nNewDX - nOldDX;
            nOldDX = nNewDX;
        }
    }

    return nPixWidth;
}

// -----------------------------------------------------------------------

int CTLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    if( !mpCTLine )
        return STRING_LEN;

    CTTypesetterRef aCTTypeSetter = CTTypesetterCreateWithAttributedString( mpAttrString );
    const double fCTMaxWidth = (double)nMaxWidth / (nFactor * mfFontScale);
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

    const DynCoreTextSyms& rCT = DynCoreTextSyms::get();
    for( int n = 0; n <= mnCharCount; ++n )
    {
        // measure the characters cursor position
        CGFloat fPos2 = -1;
        const CGFloat fPos1 = rCT.LineGetOffsetForStringIndex( mpCTLine, n, &fPos2 );
        (void)fPos2; // TODO: split cursor at line direction change
        // update previous trailing position
        if( n > 0 )
            pCaretXArray[ 2*n-1 ] = lrint( fPos1 * mfFontScale );
        // update current leading position
        if( 2*n >= nMaxIndex )
            break;
        pCaretXArray[ 2*n+0 ] = lrint( fPos1 * mfFontScale );
    }
}

// -----------------------------------------------------------------------

bool CTLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);
    CGRect aMacRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );
    CGPoint aMacPos = CGContextGetTextPosition( rAquaGraphics.mrContext );
    aMacRect.origin.x -= aMacPos.x;
    aMacRect.origin.y -= aMacPos.y;

    const Point aPos = GetDrawPosition( Point(mnBaseAdv, 0) );

    // CoreText top-bottom are vertically flipped from a VCL aspect
    rVCLRect.Left()   = aPos.X() + mfFontScale * aMacRect.origin.x;
    rVCLRect.Right()  = aPos.X() + mfFontScale * (aMacRect.origin.x + aMacRect.size.width);
    rVCLRect.Bottom() = aPos.Y() - mfFontScale * aMacRect.origin.y;
    rVCLRect.Top()    = aPos.Y() - mfFontScale * (aMacRect.origin.y + aMacRect.size.height);
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

// get the ImplFontData for a glyph fallback font
// for a glyphid that was returned by CTLayout::GetNextGlyphs()
const ImplFontData* CTLayout::GetFallbackFontData( sal_GlyphId /*aGlyphId*/ ) const
{
#if 0
    // check if any fallback fonts were needed
    if( !mpFallbackInfo )
        return NULL;
    // check if the current glyph needs a fallback font
    int nFallbackLevel = (aGlyphId & GF_FONTMASK) >> GF_FONTSHIFT;
    if( !nFallbackLevel )
        return NULL;
    pFallbackFont = mpFallbackInfo->GetFallbackFontData( nFallbackLevel );
#else
    // let CoreText's font cascading handle glyph fallback
    const ImplFontData* pFallbackFont = NULL;
#endif
    return pFallbackFont;
}

// =======================================================================

SalLayout* CTTextStyle::GetTextLayout( void ) const
{
    return new CTLayout( this);
}

// =======================================================================

