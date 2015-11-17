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

#include <sal/config.h>

#include <boost/ptr_container/ptr_vector.hpp>

#include <sal/types.h>
#include <tools/debug.hxx>

#include "ctfonts.hxx"
#include "CTRunData.hxx"
#include "quartz/utils.h"


class CTLayout : public SalLayout
{
public:
    explicit        CTLayout( const CoreTextStyle* );
    virtual         ~CTLayout();

    virtual bool    LayoutText( ImplLayoutArgs& ) override;
    virtual void    AdjustLayout( ImplLayoutArgs& ) override;
    virtual void    DrawText( SalGraphics& ) const override;
    virtual bool    DrawTextSpecial( SalGraphics& rGraphics, sal_uInt32 flags ) const override;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances, int* pCharIndexes,
                                   const PhysicalFontFace** pFallbackFonts ) const override;

    virtual DeviceCoordinate GetTextWidth() const override;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const override;

    virtual void    InitFont() const override;
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;

private:
    void            drawCTLine(AquaSalGraphics& rAquaGraphics, CTLineRef ctline, const CoreTextStyle* const pStyle) const;
    CGPoint         GetTextDrawPosition() const;
    bool            CacheGlyphLayout() const;

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
    , mpAttrString( nullptr )
    , mpCTLine( nullptr )
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
    mpAttrString = nullptr;
    if( mpCTLine )
    {
        SAL_INFO( "vcl.ct", "CFRelease(" << mpCTLine << ")" );
        CFRelease( mpCTLine );
    }
    mpCTLine = nullptr;

    // initialize the new layout
    SalLayout::AdjustLayout( rArgs );
    mnCharCount = mnEndCharPos - mnMinCharPos;

    // short circuit if there is nothing to do
    if( mnCharCount <= 0 )
        return false;

    const sal_Unicode *pStr = rArgs.mrStr.getStr();

    // create the CoreText line layout
    CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( nullptr,
                                                              reinterpret_cast<UniChar const *>(pStr + mnMinCharPos),
                                                              mnCharCount,
                                                              kCFAllocatorNull );
    // CFAttributedStringCreate copies the attribues parameter
    mpAttrString = CFAttributedStringCreate( nullptr, aCFText, mpTextStyle->GetStyleDict() );
    mpCTLine = CTLineCreateWithAttributedString( mpAttrString );
    SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(\"" << GetOUString(aCFText) << "\") =p " << mpCTLine );
    CFRelease( aCFText);

    mnTrailingSpaceCount = 0;
    // reverse search for first 'non-space'...
    for( int i = mnEndCharPos - 1; i >= mnMinCharPos; i--)
    {
        sal_Unicode nChar = pStr[i];
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
    {
        return;
    }


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
        const sal_Unicode *pStr = rArgs.mrStr.getStr();
        CFStringRef aCFText = CFStringCreateWithCharactersNoCopy( nullptr,
                                                                  reinterpret_cast<UniChar const *>(pStr + mnMinCharPos),
                                                                  mnCharCount - mnTrailingSpaceCount,
                                                                  kCFAllocatorNull );
        CFAttributedStringRef pAttrStr = CFAttributedStringCreate( nullptr,
                                                                   aCFText,
                                                                   mpTextStyle->GetStyleDict() );
        mpCTLine = CTLineCreateWithAttributedString( pAttrStr );
        SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(\"" << GetOUString(aCFText) << "\") = " << mpCTLine );
        CFRelease( pAttrStr );
        CFRelease( aCFText );


        // in RTL-layouts trailing spaces are leftmost
        // TODO: use BiDi-algorithm to thoroughly check this assumption
        if( rArgs.mnFlags & SalLayoutFlags::BiDiRtl)
        {
            mfBaseAdv = mfTrailingSpaceWidth;
        }
    }

    CTLineRef pNewCTLine = CTLineCreateJustifiedLine( mpCTLine, 1.0, nPixelWidth);
    SAL_INFO( "vcl.ct", "CTLineCreateJustifiedLine(" << mpCTLine << ",1.0," << nPixelWidth << ") = " << pNewCTLine );

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
    mfCachedWidth = nPixelWidth + mfTrailingSpaceWidth;
}

// When drawing right aligned text, rounding errors in the position returned by
// GetDrawPosition() cause the right margin of the text to change whenever text
// width changes causing "jumping letters" effect. So here we calculate the
// drawing position relative to the right margin on our own to avoid the
// rounding errors. That is basically a hack, and it should go away if one day
// we managed to get rid of those rounding errors.

// We continue using GetDrawPosition() for non-right aligned text, to minimize
// any unforeseen side effects.
CGPoint CTLayout::GetTextDrawPosition() const
{
    CGFloat fPosX, fPosY;

    if (mnLayoutFlags & SalLayoutFlags::RightAlign)
    {
        // text is always drawn at its leftmost point
        const Point aPos = DrawBase();
        fPosX = aPos.X() + mfBaseAdv - GetTextWidth();
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
#if 0 /* temporarely revert to the 'old way' */
    if (flags & DRAWTEXT_F_OUTLINE)
    {
        CFMutableDictionaryRef styledict = CFDictionaryCreateMutableCopy(
                CFAllocatorGetDefault(),
                CFDictionaryGetCount(mpTextStyle->GetStyleDict()),
                mpTextStyle->GetStyleDict());

        int nStroke = 2;
        CFNumberRef rStroke = CFNumberCreate(NULL, kCFNumberSInt32Type, &nStroke);
        CFDictionarySetValue(styledict, kCTStrokeWidthAttributeName, rStroke);

        CFAttributedStringRef pAttrStr = CFAttributedStringCreate(
                NULL,
                CFAttributedStringGetString(mpAttrString),
                styledict);
        CTLineRef pCTLine = CTLineCreateWithAttributedString( pAttrStr );
        SAL_INFO( "vcl.ct", "CTLineCreateWithAttributedString(" << GetOUString(CFAttributedStringGetString(mpAttrSring)) << ") = " << pCTLine );
        CFRelease( pAttrStr );

        /* draw the text in 'outline' */
        drawCTLine(rAquaGraphics, pCTLine, mpTextStyle);
        SAL_INFO( "vcl.ct", "CFRelease(" << pCTLine << ")" );
        CFRelease(pCTLine);
        return true;
    }
    else
#endif
    {
        return SalLayout::DrawTextSpecial(rGraphics, flags);
    }
}

void CTLayout::drawCTLine(AquaSalGraphics& rAquaGraphics, CTLineRef ctline, const CoreTextStyle* const pStyle) const
{
    // the view is vertically flipped => flipped glyphs
    // so apply a temporary transformation that it flips back
    // also compensate if the font was size limited
    CGContextRef context = rAquaGraphics.mrContext;
    SAL_INFO( "vcl.ct", "CGContextSaveGState(" << context << ")" );
    CGContextSaveGState( context );
    SAL_INFO( "vcl.ct", "CGContextScaleCTM(" << context << ",1.0,-1.0)" );
    CGContextScaleCTM( context, 1.0, -1.0 );
    CGContextSetShouldAntialias( context, !rAquaGraphics.mbNonAntialiasedText );

    // set the text transformation (e.g. position)
    CGPoint aTextPos = GetTextDrawPosition();

    if( pStyle->mfFontRotation != 0.0 )
    {
        const CGFloat fRadians = pStyle->mfFontRotation;
        SAL_INFO( "vcl.ct", "CGContextRotateCTM(" << context << "," << +fRadians << ")" );
        CGContextRotateCTM( context, +fRadians );

        const CGAffineTransform aInvMatrix = CGAffineTransformMakeRotation( -fRadians );
        aTextPos = CGPointApplyAffineTransform( aTextPos, aInvMatrix );
    }

    SAL_INFO( "vcl.ct", "CGContextSetTextPosition(" << context << "," << aTextPos << ")" );
    CGContextSetTextPosition( context, aTextPos.x, aTextPos.y );

#ifndef IOS
    // request an update of the to-be-changed window area
    if( rAquaGraphics.IsWindowGraphics() )
    {
        const CGRect aInkRect = CTLineGetImageBounds( mpCTLine, context );
        const CGRect aRefreshRect = CGContextConvertRectToDeviceSpace( context, aInkRect );
        rAquaGraphics.RefreshRect( aRefreshRect );
    }
#endif

    // set the text color as fill color (see kCTForegroundColorFromContextAttributeName)
    CGContextSetFillColor( context, rAquaGraphics.maTextColor.AsArray() );

    SAL_INFO( "vcl.ct", "CTLineDraw(" << ctline << "," << context << ")" );
    // draw the text
    CTLineDraw( ctline, context );

    if(mnLayoutFlags & SalLayoutFlags::DrawBullet)
    {
        CFArrayRef runArray = CTLineGetGlyphRuns(ctline);
        CFIndex runCount = CFArrayGetCount(runArray);

        for (CFIndex runIndex = 0; runIndex < runCount; runIndex++)
        {
            CTRunRef run = static_cast<CTRunRef>(CFArrayGetValueAtIndex(runArray, runIndex));
            CFIndex runGlyphCount = CTRunGetGlyphCount(run);
            CGPoint position;
            CGSize advance;
            CFIndex runGlyphIndex = 0;
            CFIndex stringIndice = 0;

            for (; runGlyphIndex < runGlyphCount; runGlyphIndex++)
            {
                CFRange glyphRange = CFRangeMake(runGlyphIndex, 1);
                CTRunGetStringIndices( run, glyphRange, &stringIndice );
                UniChar curChar = CFStringGetCharacterAtIndex (CFAttributedStringGetString(mpAttrString), stringIndice);
                if(curChar == ' ')
                {
                    CGFloat ascent;
                    CGFloat descent;
                    CGFloat leading;
                    CTFontRef runFont = static_cast<CTFontRef>(CFDictionaryGetValue(CTRunGetAttributes(run),
                                                                        kCTFontAttributeName));
                    CGFloat baseSize = CTFontGetSize(runFont);
                    CTRunGetTypographicBounds ( run, glyphRange,
                                                &ascent, &descent, &leading);
                    CTRunGetPositions(run, glyphRange, &position);
                    CTRunGetAdvances(run, glyphRange, &advance);
                    CGRect bulletRect;
                    if(mnLayoutFlags & SalLayoutFlags::Vertical)
                    {
                        bulletRect = CGRectMake(position.x - advance.width / 4,
                                                -position.y,  baseSize / 5, baseSize / 5 );
                    }
                    else
                    {
                        bulletRect = CGRectMake(position.x + advance.width / 4,
                                                position.y + ascent / 3 - baseSize / 5,  baseSize / 5, baseSize / 5 );
                    }

                    CGContextSaveGState(context);
                    CGContextTranslateCTM(context, aTextPos.x, aTextPos.y);
                    RGBAColor bulletColor(MAKE_SALCOLOR(0x26, 0x8b, 0xd2 )); // NON_PRINTING_CHARACTER_COLOR
                    CGContextSetFillColor( context, bulletColor.AsArray() );
                    CGContextSetStrokeColor(context, bulletColor.AsArray());
                    if(mnLayoutFlags & SalLayoutFlags::Vertical)
                    {
                        CGContextRotateCTM( context,  -F_PI/2 );
                    }

                    CGContextBeginPath(context);
                    CGContextAddEllipseInRect(context, bulletRect);
                    CGContextDrawPath(context, kCGPathFillStroke); // Or kCGPathFill
                    CGContextRestoreGState(context);
                }
            }
        }
    }
    // restore the original graphic context transformations
    SAL_INFO( "vcl.ct", "CGContextRestoreGState(" << context << ")" );
    CGContextRestoreGState( context );
}

void CTLayout::DrawText( SalGraphics& rGraphics ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    // short circuit if there is nothing to do
    if( (mnCharCount <= 0) || !rAquaGraphics.CheckContext() )
        return;

    drawCTLine(rAquaGraphics, mpCTLine, mpTextStyle);
}

bool CTLayout::CacheGlyphLayout() const // eew!
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
        CTRunRef pRun = static_cast<CTRunRef>(CFArrayGetValueAtIndex( aRuns, i ));
        CTRunData* pRunData = new CTRunData(pRun, nPos);
        m_vRunData.push_back(pRunData);
        nPos += pRunData->m_nGlyphs;
    }
    bLayouted = true;
    return true;
}

int CTLayout::GetNextGlyphs( int nLen, sal_GlyphId* pOutGlyphIds, Point& rPos, int& nStart,
                             DeviceCoordinate* pGlyphAdvances, int* pCharIndexes,
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
    const PhysicalFontFace* pFallbackFont = nullptr;
    CTFontRef pFont = nullptr;
    CTFontDescriptorRef pFontDesc = nullptr;
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
            pFont = static_cast<CTFontRef>(CFDictionaryGetValue( mpTextStyle->GetStyleDict(), kCTFontAttributeName ));
            pFontDesc = CTFontCopyFontDescriptor( iter->m_pFont );
            rDevFontAttr = DevFontFromCTFontDescriptor( pFontDesc, nullptr );
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
                pFallbackFont = new CoreTextFontData( rDevFontAttr, reinterpret_cast<sal_IntPtr>(pFontDesc) );
                *(pFallbackFonts++) = pFallbackFont;
            }
            else
            {
                *(pFallbackFonts++) = nullptr;
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
                pFont = static_cast<CTFontRef>(CFDictionaryGetValue( mpTextStyle->GetStyleDict(), kCTFontAttributeName ));
                pFontDesc = CTFontCopyFontDescriptor( iter->m_pFont );
                rDevFontAttr = DevFontFromCTFontDescriptor( pFontDesc, nullptr );
            }
        }
    }
    nStart = i;

    return count;

}

DeviceCoordinate CTLayout::GetTextWidth() const
{
    if( (mnCharCount <= 0) || !mpCTLine )
        return 0;

    if( mfCachedWidth < 0.0 )
    {
        mfCachedWidth = CTLineGetTypographicBounds( mpCTLine, nullptr, nullptr, nullptr);
    }

    return mfCachedWidth;
}

DeviceCoordinate CTLayout::FillDXArray( DeviceCoordinate* pDXArray ) const
{
    DeviceCoordinate nPixelWidth = GetTextWidth();

    // short circuit requests which don't need full details
    if( !pDXArray )
    {
        return nPixelWidth;
    }

    for(int i = 0; i < mnCharCount; i++)
    {
        pDXArray[i] = 0.0;
    }

    // prepare the sub-pixel accurate logical-width array
    ::std::vector<float> aWidthVector( mnCharCount );
    if( mnTrailingSpaceCount && (mfTrailingSpaceWidth > 0.0) )
    {
        const double fOneWidth = mfTrailingSpaceWidth / mnTrailingSpaceCount;
        for(int i = mnCharCount - mnTrailingSpaceCount; i < mnCharCount; i++)
        {
            aWidthVector[i] = fOneWidth;
        }
    }

    // handle each glyph run
    CFArrayRef aGlyphRuns = CTLineGetGlyphRuns( mpCTLine );
    const int nRunCount = CFArrayGetCount( aGlyphRuns );
    typedef std::vector<CGSize> CGSizeVector;
    CGSizeVector aSizeVector;
    typedef std::vector<CFIndex> CFIndexVector;
    CFIndexVector aIndexVector;

    for( int nRunIndex = 0; nRunIndex < nRunCount; ++nRunIndex )
    {
        CTRunRef pGlyphRun = static_cast<CTRunRef>(CFArrayGetValueAtIndex( aGlyphRuns, nRunIndex ));
        const CFIndex nGlyphCount = CTRunGetGlyphCount( pGlyphRun );
        const CFRange aFullRange = CFRangeMake( 0, nGlyphCount );

        aSizeVector.resize( nGlyphCount );
        aIndexVector.resize( nGlyphCount );
        CTRunGetAdvances( pGlyphRun, aFullRange, &aSizeVector[0] );
        CTRunGetStringIndices( pGlyphRun, aFullRange, &aIndexVector[0] );

        for( int i = 0; i != nGlyphCount; ++i )
        {
            const int nRelIndex = aIndexVector[i];
            SAL_INFO( "vcl.ct", "aWidthVector[ g:" << i << "-> c:" << nRelIndex << " ] = " <<
                      aWidthVector[nRelIndex] << " + " << aSizeVector[i].width << " = " <<
                      aWidthVector[nRelIndex] + aSizeVector[i].width);
            aWidthVector[nRelIndex] += aSizeVector[i].width;
        }
    }

    // convert the sub-pixel accurate array into classic pDXArray integers
    float fWidthSum = 0.0;
    sal_Int32 nOldDX = 0;
    for( int i = 0; i < mnCharCount; ++i)
    {
        const sal_Int32 nNewDX = rint( fWidthSum += aWidthVector[i]);
        pDXArray[i] = nNewDX - nOldDX;
        nOldDX = nNewDX;
    }
    return nPixelWidth;
}

sal_Int32 CTLayout::GetTextBreak( DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor ) const
{
    if( !mpCTLine )
    {
        SAL_INFO("vcl.ct", "GetTextBreak mpCTLine == NULL");
        return -1;
    }
    CTTypesetterRef aCTTypeSetter = CTTypesetterCreateWithAttributedString( mpAttrString );
    CFIndex nBestGuess = (nCharExtra >= 0) ? 0 : mnCharCount;
    for( int i = 1; i <= mnCharCount; i *= 2 )
    {
        // guess the target width considering char-extra expansion/condensation
        const double nTargetWidth = nMaxWidth - nBestGuess * nCharExtra;
        const double fCTMaxWidth = nTargetWidth / nFactor;
        // calculate the breaking index for the guessed target width
        const CFIndex nNewIndex = CTTypesetterSuggestClusterBreak( aCTTypeSetter, 0, fCTMaxWidth );
        if( nNewIndex >= mnCharCount )
        {
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
        {
            nNewGuess += (nNewIndex > nBestGuess) ? +1 : -1;
        }
        nBestGuess = nNewGuess;
    }

    // suggest the best fitting cluster break as breaking position
    CFRelease( aCTTypeSetter );

    const int nIndex = nBestGuess + mnMinCharPos;
    SAL_INFO("vcl.ct", "GetTextBreak nIndex:" << nIndex);

    return nIndex;
}

void CTLayout::GetCaretPositions( int nMaxIndex, long* pCaretXArray ) const
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

SalLayout* CoreTextStyle::GetTextLayout() const
{
    return new CTLayout( this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
