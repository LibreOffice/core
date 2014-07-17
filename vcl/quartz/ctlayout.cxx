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

#include <sal/types.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include "tools/debug.hxx"

#include "quartz/utils.h"

#include "ctfonts.hxx"
#include "CTRunData.hxx"


class CTLayout : public SalLayout
{
public:
    explicit        CTLayout( const CoreTextStyle* );
    virtual         ~CTLayout( void );

    virtual bool    LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    DrawText( SalGraphics& ) const SAL_OVERRIDE;
    virtual bool    DrawTextSpecial( SalGraphics& rGraphics, sal_uInt32 flags ) const SAL_OVERRIDE;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int&,
                                   sal_Int32* pGlyphAdvances, int* pCharIndexes,
                                   const PhysicalFontFace** pFallbackFonts ) const SAL_OVERRIDE;

    virtual long    GetTextWidth() const SAL_OVERRIDE;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;
    virtual sal_Int32 GetTextBreak(long nMaxWidth, long nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const SAL_OVERRIDE;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const SAL_OVERRIDE;

    virtual void    InitFont( void) const SAL_OVERRIDE;
    virtual void    MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    virtual void    DropGlyph( int nStart ) SAL_OVERRIDE;
    virtual void    Simplify( bool bIsBase ) SAL_OVERRIDE;

private:
    void            drawCTLine(AquaSalGraphics& rAquaGraphics, CTLineRef ctline, const CoreTextStyle* const pStyle) const;
    CGPoint         GetTextDrawPosition(void) const;
    double          GetWidth(void) const;
    bool            CacheGlyphLayout(void) const;

    const CoreTextStyle* const    mpTextStyle;

    // CoreText specific objects
    CFAttributedStringRef mpAttrString;
    CTLineRef mpCTLine;

    int mnCharCount;        // ==mnEndCharPos-mnMinCharPos
    int mnTrailingSpaceCount;
    double mfTrailingSpaceWidth;

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable double  mfCachedWidth;          // cached value of resulting typographical width

    // x-offset relative to layout origin
    // currently only used in RTL-layouts
    mutable double  mfBaseAdv;

    mutable bool  bLayouted; // true if the glyph layout information are cached and current;
    mutable boost::ptr_vector<CTRunData> m_vRunData;

};

CTLayout::CTLayout( const CoreTextStyle* pTextStyle )
    : mpTextStyle( pTextStyle )
    , mpAttrString( NULL )
    , mpCTLine( NULL )
    , mnCharCount( 0 )
    , mnTrailingSpaceCount( 0 )
    , mfTrailingSpaceWidth( 0.0 )
    , mfCachedWidth( -1 )
    , mfBaseAdv( 0 )
    , bLayouted( false )
{
}

CTLayout::~CTLayout()
{
    if( mpCTLine )
    {
        SAL_INFO( "vcl.ct", "CFRelease(" << mpCTLine << ")" );
        CFRelease( mpCTLine );
    }
    if( mpAttrString )
        CFRelease( mpAttrString );
}

bool CTLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    m_vRunData.release();
    bLayouted = false;

    // release an eventual older layout
    if( mpAttrString )
        CFRelease( mpAttrString );
    mpAttrString = NULL;
    if( mpCTLine )
    {
        SAL_INFO( "vcl.ct", "CFRelease(" << mpCTLine << ")" );
        CFRelease( mpCTLine );
    }
    mpCTLine = NULL;

    // initialize the new layout
    SalLayout::AdjustLayout( rArgs );
    mnCharCount = mnEndCharPos - mnMinCharPos;

    // short circuit if there is nothing to do
    if( mnCharCount <= 0 )
        return false;

    // create the CoreText line layout
    CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL,
                                                              rArgs.mpStr + mnMinCharPos,
                                                              mnCharCount,
                                                              kCFAllocatorNull );
    // CFAttributedStringCreate copies the attribues parameter
    mpAttrString = CFAttributedStringCreate( NULL, aCFText, mpTextStyle->GetStyleDict() );
    mpCTLine = CTLineCreateWithAttributedString( mpAttrString );
    SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(\"" << GetOUString(aCFText) << "\") = " << mpCTLine );
    CFRelease( aCFText);

    mnTrailingSpaceCount = 0;
    // reverse search for first 'non-space'...
    for( int i = mnEndCharPos - 1; i >= mnMinCharPos; i--)
    {
        sal_Unicode nChar = rArgs.mpStr[i];
        if ((nChar <= 0x0020) ||                  // blank
            (nChar == 0x00A0) ||                  // non breaking space
            (nChar >= 0x2000 && nChar <= 0x200F) || // whitespace
            (nChar == 0x3000))                   // ideographic space
        {
            mnTrailingSpaceCount += 1;
        }
        else
        {
            break;
        }
    }
    return true;
}

void CTLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    if( !mpCTLine)
        return;

    int nPixelWidth = rArgs.mpDXArray ? rArgs.mpDXArray[ mnCharCount - 1 ] : rArgs.mnLayoutWidth;
    if( nPixelWidth <= 0)
        return;

    // HACK: justification requests which change the width by just one pixel are probably
    // #i86038# introduced by lossy conversions between integer based coordinate system
    int fuzz =  (nPixelWidth - GetTextWidth()) / 2;
    if (!fuzz)
    {
        return;
    }

    // if the text to be justified has whitespace in it then
    // - Writer goes crazy with its HalfSpace magic
    // - CoreText handles spaces specially (in particular at the text end)
    if( mnTrailingSpaceCount )
    {
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060
        // don't recreate line layout here, because this can lead to problems
        // (looks like internal issues inside early CoreText versions)
        mfTrailingSpaceWidth = CTLineGetTrailingWhitespaceWidth( mpCTLine );
#else
        if(rArgs.mpDXArray)
        {
            int nFullPixelWidth = nPixelWidth;
            nPixelWidth = mnTrailingSpaceCount == mnCharCount
                ? 0 : rArgs.mpDXArray[ mnCharCount - mnTrailingSpaceCount - 1];
            mfTrailingSpaceWidth = nFullPixelWidth - nPixelWidth;
        }
        else
        {
            if(mfTrailingSpaceWidth <= 0.0)
            {
                mfTrailingSpaceWidth = CTLineGetTrailingWhitespaceWidth( mpCTLine );
                nPixelWidth -= rint(mfTrailingSpaceWidth);
            }
        }
        if(nPixelWidth <= 0)
        {
            return;
        }
        // recreate the CoreText line layout without trailing spaces
        SAL_INFO( "vcl.ct", "CFRelease(" << mpCTLine << ")" );
        CFRelease( mpCTLine );
        CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL,
                                                                  rArgs.mpStr + mnMinCharPos,
                                                                  mnCharCount - mnTrailingSpaceCount,
                                                                  kCFAllocatorNull );
        CFAttributedStringRef pAttrStr = CFAttributedStringCreate( NULL,
                                                                   aCFText,
                                                                   mpTextStyle->GetStyleDict() );
        mpCTLine = CTLineCreateWithAttributedString( pAttrStr );
        SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(\"" << GetOUString(aCFText) << "\") = " << mpCTLine );
        CFRelease( pAttrStr );
        CFRelease( aCFText );
#endif

        // in RTL-layouts trailing spaces are leftmost
        // TODO: use BiDi-algorithm to thoroughly check this assumption
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
        {
            mfBaseAdv = mfTrailingSpaceWidth;
        }
    }

#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060
    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, nPixelWidth - mfTrailingSpaceWidth );
#else
    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, nPixelWidth);
    SAL_INFO( "vcl.ct", "CTLineCreateJustifiedLine(" << mpCTLine << ",1.0," << nPixelWidth << ") = " << pNewCTLine );
#endif
    if( !pNewCTLine )
    {
        // CTLineCreateJustifiedLine can and does fail
        // handle failure by keeping the unjustified layout
        // TODO: a better solution such as
        // - forcing glyph overlap
        // - changing the font size
        // - changing the CTM matrix
        return;
    }
    SAL_INFO( "vcl.ct", "CFRelease(" << mpCTLine << ")" );
    CFRelease( mpCTLine );
    mpCTLine = pNewCTLine;
#if MAC_OS_X_VERSION_MAX_ALLOWED <= 1060
    mfCachedWidth = nPixelWidth;
#else
    mfCachedWidth = nPixelWidth + mfTrailingSpaceWidth;
#endif
}

// When drawing right aligned text, rounding errors in the position returned by
// GetDrawPosition() cause the right margin of the text to change whenever text
// width changes causing "jumping letters" effect. So here we calculate the
// drawing position relative to the right margin on our own to avoid the
// rounding errors. That is basically a hack, and it should go away if one day
// we managed to get rid of those rounding errors.

// We continue using GetDrawPosition() for non-right aligned text, to minimize
// any unforeseen side effects.
CGPoint CTLayout::GetTextDrawPosition(void) const
{
    CGFloat fPosX, fPosY;

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

/* use to deal with special font decoration like 'outline' drawing
 * return true if it was able to handle the drawing
 * false if not, in which case the caller
 * is supposed to fallback to 'generic' method
 */
bool CTLayout::DrawTextSpecial( SalGraphics& rGraphics, sal_uInt32 flags ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    // short circuit if there is nothing to do
    if( (mnCharCount <= 0) || !rAquaGraphics.CheckContext() )
        return true;

    if (flags & DRAWTEXT_F_OUTLINE)
    {
        CFMutableDictionaryRef styledict = CFDictionaryCreateMutableCopy(
                CFAllocatorGetDefault(),
                CFDictionaryGetCount(mpTextStyle->GetStyleDict()),
                mpTextStyle->GetStyleDict());

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060
        /* just don't do 'kCTStrokeWidthAttributeName' */
#else
        int nStroke = 2;
        CFNumberRef rStroke = CFNumberCreate(NULL, kCFNumberSInt32Type, &nStroke);
        CFDictionarySetValue(styledict, kCTStrokeWidthAttributeName, rStroke);
#endif

        CFAttributedStringRef pAttrStr = CFAttributedStringCreate(
                NULL,
                CFAttributedStringGetString(mpAttrString),
                styledict);
        CTLineRef pCTLine = CTLineCreateWithAttributedString( pAttrStr );
        SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(" << pAttrStr << ") = " << pCTLine );
        CFRelease( pAttrStr );

        /* draw the text in 'outline' */
        drawCTLine(rAquaGraphics, pCTLine, mpTextStyle);
        SAL_INFO( "vcl.ct", "CFRelease(" << pCTLine << ")" );
        CFRelease(pCTLine);
        return true;
    }
    else
    {
        return SalLayout::DrawTextSpecial(rGraphics, flags);
    }
}

void CTLayout::drawCTLine(AquaSalGraphics& rAquaGraphics, CTLineRef ctline, const CoreTextStyle* const pStyle) const
{
    // the view is vertically flipped => flipped glyphs
    // so apply a temporary transformation that it flips back
    // also compensate if the font was size limited
    SAL_INFO( "vcl.ct", "CGContextSaveGState(" << rAquaGraphics.mrContext << ")" );
    CGContextSaveGState( rAquaGraphics.mrContext );
    SAL_INFO( "vcl.ct", "CGContextScaleCTM(" << rAquaGraphics.mrContext << ",1.0,-1.0)" );
    CGContextScaleCTM( rAquaGraphics.mrContext, 1.0, -1.0 );
    CGContextSetShouldAntialias( rAquaGraphics.mrContext, !rAquaGraphics.mbNonAntialiasedText );

    // set the text transformation (e.g. position)
    CGPoint aTextPos = GetTextDrawPosition();

    if( pStyle->mfFontRotation != 0.0 )
    {
        const CGFloat fRadians = pStyle->mfFontRotation;
        SAL_INFO( "vcl.ct", "CGContextRotateCTM(" << rAquaGraphics.mrContext << "," << +fRadians << ")" );
        CGContextRotateCTM( rAquaGraphics.mrContext, +fRadians );

        const CGAffineTransform aInvMatrix = CGAffineTransformMakeRotation( -fRadians );
        aTextPos = CGPointApplyAffineTransform( aTextPos, aInvMatrix );
    }

    SAL_INFO( "vcl.ct", "CGContextSetTextPosition(" << rAquaGraphics.mrContext << "," << aTextPos << ")" );
    CGContextSetTextPosition( rAquaGraphics.mrContext, aTextPos.x, aTextPos.y );

#ifndef IOS
    // request an update of the to-be-changed window area
    if( rAquaGraphics.IsWindowGraphics() )
    {
        const CGRect aInkRect = CTLineGetImageBounds( mpCTLine, rAquaGraphics.mrContext );
        const CGRect aRefreshRect = CGContextConvertRectToDeviceSpace( rAquaGraphics.mrContext, aInkRect );
        rAquaGraphics.RefreshRect( aRefreshRect );
    }
#endif

    // set the text color as fill color (see kCTForegroundColorFromContextAttributeName)
    CGContextSetFillColor( rAquaGraphics.mrContext, rAquaGraphics.maTextColor.AsArray() );

    SAL_INFO( "vcl.ct", "CTLineDraw(" << ctline << "," << rAquaGraphics.mrContext << ")" );
    // draw the text
    CTLineDraw( ctline, rAquaGraphics.mrContext );

    if(mnLayoutFlags & SAL_LAYOUT_DRAW_BULLET)
    {
        CFArrayRef runArray = CTLineGetGlyphRuns(ctline);
        CFIndex runCount = CFArrayGetCount(runArray);

        CFIndex runIndex = 0;
        CTLineRef ctlinebullet = 0;
        OUString sBullet((sal_Unicode)0xb7); // centered bullet

        for (; runIndex < runCount; runIndex++)
        {

            CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runArray, runIndex);
            CFIndex runGlyphCount = CTRunGetGlyphCount(run);

            CGPoint position;
            CFIndex runGlyphIndex = 0;
            CFIndex stringIndice = 0;

            for (; runGlyphIndex < runGlyphCount; runGlyphIndex++)
            {
                CFRange glyphRange = CFRangeMake(runGlyphIndex, 1);

                CTRunGetStringIndices( run, glyphRange, &stringIndice );
                UniChar curChar = CFStringGetCharacterAtIndex (CFAttributedStringGetString(mpAttrString), stringIndice);
                if(curChar == ' ')
                {
                    CTRunGetPositions(run, glyphRange, &position);
                    // print a dot
                    if(!ctlinebullet)
                    {
                        CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( NULL,
                                                                                  sBullet.getStr(),
                                                                                  1,
                                                                                  kCFAllocatorNull );
                        // CFAttributedStringCreate copies the attribues parameter
                        CFAttributedStringRef bulletAttrString = CFAttributedStringCreate( NULL, aCFText, mpTextStyle->GetStyleDict() );
                        ctlinebullet = CTLineCreateWithAttributedString( bulletAttrString );
                        CFRelease( aCFText);
                        CFRelease( bulletAttrString);
                        RGBAColor bulletColor(MAKE_SALCOLOR(0x26, 0x8b, 0xd2 )); // NON_PRINTING_CHARACTER_COLOR
                        CGContextSetFillColor( rAquaGraphics.mrContext, bulletColor.AsArray() );
                    }
                    CGContextSetTextPosition( rAquaGraphics.mrContext, aTextPos.x + position.x, position.y + aTextPos.y );
                    CTLineDraw(ctlinebullet, rAquaGraphics.mrContext);
                }
            }
        }
        if(ctlinebullet)
        {
            CFRelease(ctlinebullet);
        }
    }

    // restore the original graphic context transformations
    SAL_INFO( "vcl.ct", "CGContextRestoreGState(" << rAquaGraphics.mrContext << ")" );
    CGContextRestoreGState( rAquaGraphics.mrContext );
}

void CTLayout::DrawText( SalGraphics& rGraphics ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    // short circuit if there is nothing to do
    if( (mnCharCount <= 0) || !rAquaGraphics.CheckContext() )
        return;

    drawCTLine(rAquaGraphics, mpCTLine, mpTextStyle);
}

bool CTLayout::CacheGlyphLayout(void) const // eew!
{
    m_vRunData.release();
    if(!mpCTLine)
    {
        return false;
    }
    CFArrayRef aRuns = CTLineGetGlyphRuns( mpCTLine );
    const int nRun = CFArrayGetCount( aRuns );
    int nPos = 0;

    for( int i = 0; i < nRun; ++i )
    {
        CTRunRef pRun = (CTRunRef)CFArrayGetValueAtIndex( aRuns, i );
        CTRunData* pRunData = new CTRunData(pRun, nPos);
        m_vRunData.push_back(pRunData);
        nPos += pRunData->m_nGlyphs;
    }
    bLayouted = true;
    return true;
}

int CTLayout::GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int& nStart,
                             sal_Int32* pGlyphAdvances, int* pCharIndexes,
                             const PhysicalFontFace** pFallbackFonts ) const
{
    if( !mpCTLine )
    {
        return 0;
    }
    if(!bLayouted)
    {
        CacheGlyphLayout();
    }

    if( nStart < 0 ) // first glyph requested?
    {
        nStart = 0;
    }
    const PhysicalFontFace* pFallbackFont = NULL;
    CTFontRef pFont = NULL;
    CTFontDescriptorRef pFontDesc = NULL;
    ImplDevFontAttributes rDevFontAttr;

    boost::ptr_vector<CTRunData>::const_iterator iter = m_vRunData.begin();

    while(iter != m_vRunData.end() && iter->m_EndPos <= nStart)
    {
        ++iter;
    }
    if(iter == m_vRunData.end())
    {
        return 0;
    }
    else
    {
        if( pFallbackFonts )
        {
            pFont = (CTFontRef)CFDictionaryGetValue( mpTextStyle->GetStyleDict(), kCTFontAttributeName );
            pFontDesc = CTFontCopyFontDescriptor( iter->m_pFont );
            rDevFontAttr = DevFontFromCTFontDescriptor( pFontDesc, NULL );
        }
    }
    int i = nStart;
    int count = 0;
    while( i < nStart + nLen )
    {
            // convert glyph details for VCL
        int j = i - iter->m_StartPos;
        *(pOutGlyphIds++) = iter->m_pGlyphs[ j ];
        if( pGlyphAdvances )
        {
            *(pGlyphAdvances++) = lrint(iter->m_pAdvances[ j ].width);
        }
        if( pCharIndexes )
        {
            *(pCharIndexes++) = iter->m_pStringIndices[ j ] + mnMinCharPos;
        }
        if( pFallbackFonts )
        {
            if ( !CFEqual( iter->m_pFont,  pFont ) )
            {
                pFallbackFont = new CoreTextFontData( rDevFontAttr, (sal_IntPtr)pFontDesc );
                *(pFallbackFonts++) = pFallbackFont;
            }
            else
            {
                *(pFallbackFonts++) = NULL;
            }
        }
        if( i == nStart )
        {
            const CGPoint& rFirstPos = iter->m_pPositions[ j ];
            rPos = GetDrawPosition( Point( rFirstPos.x, rFirstPos.y) );
        }
        i += 1;
        count += 1;
        if(i == iter->m_EndPos)
        {
            // note: we assume that we do not have empty runs in the middle of things
            ++iter;
            if( iter == m_vRunData.end())
            {
                break;
            }
            if( pFallbackFonts )
            {
                pFont = (CTFontRef)CFDictionaryGetValue( mpTextStyle->GetStyleDict(), kCTFontAttributeName );
                pFontDesc = CTFontCopyFontDescriptor( iter->m_pFont );
                rDevFontAttr = DevFontFromCTFontDescriptor( pFontDesc, NULL );
            }
        }
    }
    nStart = i;

    return count;

}

double CTLayout::GetWidth() const
{
    if( (mnCharCount <= 0) || !mpCTLine )
        return 0;

    if( mfCachedWidth < 0.0 )
    {
        mfCachedWidth = CTLineGetTypographicBounds( mpCTLine, NULL, NULL, NULL);
    }

    return mfCachedWidth;
}

long CTLayout::GetTextWidth() const
{
    return lrint(GetWidth());
}

DeviceCoordinate CTLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    DeviceCoordinate nPixWidth = GetWidth();
    // short circuit requests which don't need full details
    if( !pDXArray )
        return nPixWidth;

    // prepare the sub-pixel accurate logical-width array
    ::std::vector<double> aWidthVector( mnCharCount );
    if( mnTrailingSpaceCount && (mfTrailingSpaceWidth > 0.0) )
    {
        const double fOneWidth = mfTrailingSpaceWidth / mnTrailingSpaceCount;
        ::std::fill_n(aWidthVector.begin() + (mnCharCount - mnTrailingSpaceCount),
                      mnTrailingSpaceCount,
                      fOneWidth);
    }

    // handle each glyph run
    CFArrayRef aGlyphRuns = CTLineGetGlyphRuns( mpCTLine );
    const int nRunCount = CFArrayGetCount( aGlyphRuns );
    typedef std::vector<CGSize> CGSizeVector;
    CGSizeVector aSizeVec;
    typedef std::vector<CFIndex> CFIndexVector;
    CFIndexVector aIndexVec;

    for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex )
    {
        CTRunRef pGlyphRun = (CTRunRef)CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex );
        const CFIndex nGlyphCount = CTRunGetGlyphCount( pGlyphRun );
        const CFRange aFullRange = CFRangeMake( 0, nGlyphCount );

        aSizeVec.resize( nGlyphCount );
        aIndexVec.resize( nGlyphCount );
        CTRunGetAdvances( pGlyphRun, aFullRange, &aSizeVec[0] );
        CTRunGetStringIndices( pGlyphRun, aFullRange, &aIndexVec[0] );

        for( int i = 0; i != nGlyphCount; ++i )
        {
            const int nRelIdx = aIndexVec[i];
            aWidthVector[nRelIdx] += aSizeVec[i].width;
        }
    }

    // convert the sub-pixel accurate array into classic pDXArray integers
    double fWidthSum = 0.0;
    sal_Int32 nOldDX = 0;
    for( int i = 0; i < mnCharCount; ++i)
    {
        const sal_Int32 nNewDX = rint( fWidthSum += aWidthVector[i]);
        pDXArray[i] = nNewDX - nOldDX;
        nOldDX = nNewDX;
    }

    return nPixWidth;
}

sal_Int32 CTLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    if( !mpCTLine )
        return -1;

    CTTypesetterRef aCTTypeSetter = CTTypesetterCreateWithAttributedString( mpAttrString );
    CFIndex nBestGuess = (nCharExtra >= 0) ? 0 : mnCharCount;
    for( int i = 1; i <= mnCharCount; i *= 2 )
    {
        // guess the target width considering char-extra expansion/condensation
        const double nTargetWidth = nMaxWidth - nBestGuess * nCharExtra;
        const double fCTMaxWidth = nTargetWidth / nFactor;
        // calculate the breaking index for the guessed target width
        const CFIndex nNewIndex = CTTypesetterSuggestClusterBreak( aCTTypeSetter, 0, fCTMaxWidth );
        if( nNewIndex >= mnCharCount ) {
            CFRelease( aCTTypeSetter );
            return -1;
        }
        // check if the original extra-width guess was good
        if( !nCharExtra )
            nBestGuess = nNewIndex;
        if( nBestGuess == nNewIndex )
            break;
        // prepare another round for a different number of characters
        CFIndex nNewGuess = (nNewIndex + nBestGuess + 1) / 2;
        if( nNewGuess == nBestGuess )
            nNewGuess += (nNewIndex > nBestGuess) ? +1 : -1;
        nBestGuess = nNewGuess;
    }

    // suggest the best fitting cluster break as breaking position
    CFRelease( aCTTypeSetter );
    const int nIndex = nBestGuess + mnMinCharPos;
    return nIndex;
}

void CTLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    DBG_ASSERT( ((nMaxIndex>0)&&!(nMaxIndex&1)),
        "CTLayout::GetCaretPositions() : invalid number of caret pairs requested");

    // initialize the caret positions
    for( int i = 0; i < nMaxIndex; ++i )
    {
        pCaretXArray[ i ] = -1;
    }
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

// glyph fallback is supported directly by Aqua
// so methods used only by MultiSalLayout can be dummy implementated
void CTLayout::InitFont() const {}
void CTLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}
void CTLayout::DropGlyph( int /*nStart*/ ) {}
void CTLayout::Simplify( bool /*bIsBase*/ ) {}

SalLayout* CoreTextStyle::GetTextLayout( void ) const
{
    return new CTLayout( this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
