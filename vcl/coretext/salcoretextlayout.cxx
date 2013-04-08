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

#include <iostream>
#include <iomanip>

#include "quartz/utils.h"
#include "coretext/common.h"
#include "coretext/salcoretextstyle.hxx"

#ifdef MACOSX
#include "coretext/salgdi.h"
#else
#include "headless/svpgdi.hxx"
#endif

class CoreTextLayout : public SalLayout
{
public:
    CoreTextLayout( QuartzSalGraphics* graphics, CoreTextStyleInfo* style);
    ~CoreTextLayout();

    // Overrides in same order as in base class, without "virtual" and
    // with explicit SAL_OVERRIDE. Just a question of taste;)
    bool LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;
    void AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    void DrawText( SalGraphics& ) const SAL_OVERRIDE;

    int GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const SAL_OVERRIDE;
    long FillDXArray( sal_Int32* pDXArray ) const SAL_OVERRIDE;
    long GetTextWidth() const SAL_OVERRIDE;
    void GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const SAL_OVERRIDE;

    int GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                       sal_Int32* pGlyphAdvances, int* pCharIndexes ) const SAL_OVERRIDE;
    bool GetBoundRect( SalGraphics&, Rectangle& ) const SAL_OVERRIDE;

    void MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    void DropGlyph( int nStart ) SAL_OVERRIDE;
    void Simplify( bool bIsBase ) SAL_OVERRIDE;

private:
    void GetMeasurements();
    void InvalidateMeasurements();
    void ApplyDXArray( ImplLayoutArgs& );
    void Justify( long );

#ifndef NDEBUG
    int mnSavedMinCharPos;
    int mnSavedEndCharPos;
    sal_Unicode *mpSavedStr;
#endif

    QuartzSalGraphics* mpGraphics;
    CoreTextStyleInfo* mpStyle;

    int mnCharCount;                    // ==mnEndCharPos-mnMinCharPos

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable int mnGlyphCount;

    mutable CGGlyph* mpGlyphs;
    mutable CGFloat* mpCharWidths;
    mutable int* mpGlyphs2Chars;

    mutable CGSize* mpGlyphAdvances;

    mutable CGPoint* mpGlyphPositions;
    mutable CTTypesetterRef mpTypesetter;
    mutable CTLineRef mpLine;
    mutable bool mbHasBoundRectangle;
    mutable Rectangle maBoundRectangle;

    // x-offset relative to layout origin
    // currently only used in RTL-layouts
    CGFloat mnBaseAdvance;

    mutable CFIndex mnCurrentRunIndex;
    mutable CFIndex mnCurrentGlyphIndex;
    mutable CFIndex mnCurrentGlyphRunIndex;
    mutable CFArrayRef mpRuns;
};

CoreTextLayout::CoreTextLayout(QuartzSalGraphics* graphics, CoreTextStyleInfo* style) :
#ifndef NDEBUG
    mpSavedStr(NULL),
#endif
    mpGraphics(graphics),
    mpStyle(style),
    mnCharCount(-1),
    mnGlyphCount(-1),
    mpGlyphs(NULL),
    mpCharWidths(NULL),
    mpGlyphs2Chars(NULL),
    mpGlyphAdvances(NULL),
    mpGlyphPositions(NULL),
    mpTypesetter(NULL),
    mpLine(NULL),
    mbHasBoundRectangle(false),
    mnBaseAdvance(0),
    mnCurrentRunIndex(0),
    mnCurrentGlyphIndex(0),
    mnCurrentGlyphRunIndex(0),
    mpRuns(NULL)
{
    SAL_INFO( "vcl.coretext.layout", "CoreTextLayout::CoreTextLayout() " << this << ", style=" << *style);
}

CoreTextLayout::~CoreTextLayout()
{
    InvalidateMeasurements();
    SafeCFRelease(mpTypesetter);
    SafeCFRelease(mpLine);
#ifndef NDEBUG
    delete[] mpSavedStr;
#endif
    SAL_INFO( "vcl.coretext.layout", "~CoreTextLayout(" << this << ")" );
}

void CoreTextLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SAL_INFO( "vcl.coretext.layout", "AdjustLayout(" << this << ",rArgs=" << rArgs << ")" );

#ifndef NDEBUG
    assert( mnSavedMinCharPos == rArgs.mnMinCharPos );
    assert( mnSavedEndCharPos == rArgs.mnEndCharPos );
    assert( mnCharCount == (mnSavedEndCharPos - mnSavedMinCharPos) );
    assert( memcmp( &mpSavedStr[0],
                    &rArgs.mpStr[mnSavedMinCharPos],
                    mnCharCount * sizeof( sal_Unicode ) ) == 0 );
#endif

    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
    else
        return;
}

void CoreTextLayout::ApplyDXArray( ImplLayoutArgs& rArgs )
{
    Justify( rArgs.mpDXArray[mnCharCount-1] );
}

void CoreTextLayout::Justify( long nNewWidth )
{
    CTLineRef justifiedLine = CTLineCreateJustifiedLine( mpLine, 1.0, nNewWidth );
    if ( !justifiedLine ) {
        SAL_INFO( "vcl.coretext.layout", "ApplyDXArray(): CTLineCreateJustifiedLine() failed" );
    } else {
        CFRelease( mpLine );
        mpLine = justifiedLine;
    }

    GetMeasurements();
}

void CoreTextLayout::InvalidateMeasurements()
{
    if( mpGlyphs ) {
        delete[] mpGlyphs;
        mpGlyphs = NULL;
    }
    if( mpGlyphs2Chars ) {
        delete[] mpGlyphs2Chars;
        mpGlyphs2Chars = NULL;
    }
    if( mpCharWidths ) {
        delete[] mpCharWidths;
        mpCharWidths = NULL;
    }
    if( mpGlyphAdvances ) {
        delete[] mpGlyphAdvances;
        mpGlyphAdvances = NULL;
    }
    if( mpGlyphPositions ) {
        delete[] mpGlyphPositions;
        mpGlyphPositions = NULL;
    }
    mbHasBoundRectangle = false;
}

void CoreTextLayout::DrawText( SalGraphics& rGraphics ) const
{
    SAL_INFO( "vcl.coretext.layout", "DrawText(" << this << ")" );

    QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);
    if( mnCharCount <= 0 || !gr.CheckContext() )
        return;

    Point pos = GetDrawPosition(Point(0,0));
    SAL_INFO( "vcl.coretext.layout", "  at pos (" << pos.X() << "," << pos.Y() <<") ctfont=" << mpStyle->GetFont() );

    CGFontRef cg_font = CTFontCopyGraphicsFont(mpStyle->GetFont(), NULL);
    if( !cg_font ) {
        SAL_INFO( "vcl.coretext.layout", "Error cg_font is NULL" );
        return;
    }
    CGContextSaveGState( gr.mrContext );
    CGContextSetFont(gr.mrContext, cg_font);
    CGContextSetFontSize(gr.mrContext, CTFontGetSize(mpStyle->GetFont()));
    CGContextSetTextDrawingMode(gr.mrContext, kCGTextFill);
    CGContextSetShouldAntialias( gr.mrContext, true );
    if( mpStyle->GetColor() ) {
        CGContextSetFillColorWithColor(gr.mrContext, mpStyle->GetColor());
        CGContextSetStrokeColorWithColor(gr.mrContext, mpStyle->GetColor());
    }
    else {
        CGContextSetRGBFillColor(gr.mrContext, 0.0, 0.0, 0.0, 1.0);
    }
    CFRelease(cg_font);
    CGContextSetTextMatrix(gr.mrContext, CGAffineTransformMakeScale(1.0, -1.0));
    CGContextSetShouldAntialias( gr.mrContext, !gr.mbNonAntialiasedText );
    CGContextTranslateCTM(gr.mrContext, pos.X(), pos.Y());

    CGContextShowGlyphsWithAdvances(gr.mrContext, mpGlyphs, mpGlyphAdvances, mnGlyphCount);

#ifndef IOS
    // Request an update of the changed window area. Like in the ATSUI
    // code, I am not sure if this is actually necessary. Once this
    // seems to work fine otherwise, let's try removing this.
    if( gr.IsWindowGraphics() )
    {
        CGRect drawRect = CTLineGetImageBounds( mpLine, gr.mrContext );
        SAL_INFO( "vcl.coretext.layout", "drawRect=" << drawRect );
        if( !CGRectIsNull( drawRect ) ) {
#if 1
            // For kicks, try the same silly (?) enlarging of the
            // rectangle as in the ATSUI code
            drawRect.origin.y -= drawRect.size.height;
            drawRect.size.height += 2*drawRect.size.height;
            SAL_INFO( "vcl.coretext.layout", "after enlarging drawRect=" << drawRect );
#endif
            drawRect = CGContextConvertRectToDeviceSpace( gr.mrContext, drawRect );
            SAL_INFO( "vcl.coretext.layout", "after convert: drawRect=" << drawRect );
            gr.RefreshRect( drawRect );
        }
    }
#endif

    // restore the original graphic context transformations
    CGContextRestoreGState( gr.mrContext );
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::DropGlyph( int /*nStart*/ )
{
}

// Note that the "DX array" here is filled with individual character
// widths, while ImplLayoutArgs::mpDXArray contains cumulative
// character positions. Consistency is over-rated.

long CoreTextLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    // Short circuit requests which don't need full details
    if( !pDXArray ) {
        return GetTextWidth();
    }

    // Distribute the widths among the string elements
    long width = 0;
    float scale = mpStyle->GetFontStretchFactor();
    CGFloat accumulatedWidth = 0;

    std::ostringstream DXArrayInfo;
    for( int i = 0; i < mnCharCount; ++i ) {
        // Convert and adjust for accumulated rounding errors
        accumulatedWidth += mpCharWidths[ i ];
        const long old_width = width;
        width = round_to_long( accumulatedWidth * scale );
        pDXArray[i] = width - old_width;
#ifdef SAL_LOG_INFO
        if ( i < 7 )
            DXArrayInfo << " " << pDXArray[i];
        else if ( i == 7 )
            DXArrayInfo << "...";
#endif
    }

    SAL_INFO( "vcl.coretext.layout", "FillDXArray(" << this << "):" << DXArrayInfo.str() << ", result=" << width );

    return width;
}

bool CoreTextLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{
    SAL_INFO( "vcl.coretext.layout", "GetBoundRect(" << this << ")" );

    QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);

    if( !gr.CheckContext() )
        return false;

    if ( !mbHasBoundRectangle ) {
        CGRect bound_rect = CTLineGetImageBounds( mpLine, gr.mrContext );
        if ( !CGRectIsNull( bound_rect ) ) {
            maBoundRectangle = Rectangle(
                Point( round_to_long(bound_rect.origin.x * mpStyle->GetFontStretchFactor()),
                       round_to_long(bound_rect.origin.y - bound_rect.size.height )),
                Size( round_to_long((bound_rect.size.width + CTLineGetTrailingWhitespaceWidth( mpLine )) * mpStyle->GetFontStretchFactor()),
                      round_to_long(bound_rect.size.height)));
            maBoundRectangle.Justify();
        } else {
            maBoundRectangle = Rectangle(
                Point( 0, 0 ),
                Size( round_to_long(CTLineGetTrailingWhitespaceWidth( mpLine ) * mpStyle->GetFontStretchFactor()),
                      0 ) );
            maBoundRectangle.Justify();
        }
        mbHasBoundRectangle = true;
    }

    rVCLRect = maBoundRectangle;
    SAL_INFO( "vcl.coretext.layout", "GetBoundRect() returning with rVCLRect={" << rVCLRect << "}" );

    return true;
}

void CoreTextLayout::GetCaretPositions( int max_index, sal_Int32* caret_position ) const
{
    SAL_INFO( "vcl.coretext.layout", "GetCaretPositions(" << this << ",max_index=" << max_index << ")" );

    int local_max = max_index < mnCharCount * 2 ? max_index : mnCharCount;
    for( int i = 0 ; i < max_index - 1; i+=2 ) {
        CGFloat primary, secondary;
        primary = CTLineGetOffsetForStringIndex(mpLine, i >> 1, &secondary);
        caret_position[i] = round_to_long(mnBaseAdvance + primary);
        caret_position[i+1] = round_to_long(mnBaseAdvance + secondary);
        i += 2;
    }
    for( int i = local_max ; i < max_index ; ++i ) {
        caret_position[i] = -1;
    }
}

int CoreTextLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
                                   sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    SAL_INFO( "vcl.coretext.layout", "GetNextGlyphs(" << this << ",nLen=" << nLen << ",nStart=" << nStart << ")" );

    if( nStart < 0 ) {              // first glyph requested?
        nStart = 0;
        mnCurrentRunIndex = 0;
        mnCurrentGlyphIndex = 0;
        mnCurrentGlyphRunIndex = 0;
    }
    else if( nStart >= mnGlyphCount ) {
        mnCurrentRunIndex = 0;
        mnCurrentGlyphIndex = 0;
        mnCurrentGlyphRunIndex = 0;
        return 0;
    }
    if( !mpRuns ) {
        mpRuns = CTLineGetGlyphRuns(mpLine);
    }
    CFIndex nb_runs = CFArrayGetCount( mpRuns );
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( mpRuns, mnCurrentRunIndex );
    CFIndex nb_glyphs = CTRunGetGlyphCount( run );

    int i = 0;
    bool first = true;
    while( i < nLen ) {
        if( mnCurrentGlyphRunIndex >= nb_glyphs ) {
            mnCurrentRunIndex += 1;
            if( mnCurrentRunIndex >= nb_runs ) {
                break;
            }
            run = (CTRunRef)CFArrayGetValueAtIndex( mpRuns, mnCurrentRunIndex );
            nb_glyphs = CTRunGetGlyphCount( run );
            mnCurrentGlyphRunIndex = 0;
        }
        if( first ) {
            CGPoint first_pos;
            CTRunGetPositions(run, CFRangeMake(mnCurrentGlyphRunIndex,1), &first_pos);
            Point pos(first_pos.x, first_pos.y);
            rPos = GetDrawPosition(pos);
            SAL_INFO( "vcl.coretext.layout", "rPos(" << rPos.X() << "," << rPos.Y() << ")" );
            first = false;
        }
        pGlyphIDs[i] = mpGlyphs[mnCurrentGlyphIndex];
        if( pGlyphAdvances ) {
            pGlyphAdvances[i] = mpGlyphAdvances[mnCurrentGlyphIndex].width;
        }
        if( pCharIndexes ) {
            pCharIndexes[i] = mpGlyphs2Chars[mnCurrentGlyphIndex];
        }
        mnCurrentGlyphIndex += 1;
        mnCurrentGlyphRunIndex += 1;
        i += 1;
        nStart += 1;
    }

    SAL_INFO( "vcl.coretext.layout", "GetNextGlyphs() returning " << i );

    return i;
}

int CoreTextLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    SAL_INFO( "vcl.coretext.layout", "GetTextBreak(" << this << ",nMaxWidth=" << nMaxWidth << ",nCharExtra=" << nCharExtra << ",nFactor=" << nFactor << ")" );

    if( !mpLine ) {
        SAL_INFO( "vcl.coretext.layout", "GetTextBreak() returning STRING_LEN" );
        return STRING_LEN;
    }

    // the semantics of the legacy use case (nCharExtra!=0) cannot be mapped to ATSUBreakLine()
    if( nCharExtra != 0 )
    {
#if 0
        // prepare the measurement by layouting and measuring the un-expanded/un-condensed text
        if( !InitGIA() )
            return STRING_LEN;

        // TODO: use a better way than by testing each the char position
        ATSUTextMeasurement nATSUSumWidth = 0;
        const ATSUTextMeasurement nATSUMaxWidth = Vcl2Fixed( nMaxWidth / nFactor );
        const ATSUTextMeasurement nATSUExtraWidth = Vcl2Fixed( nCharExtra ) / nFactor;
        for( int i = 0; i < mnCharCount; ++i ) {
            nATSUSumWidth += mpCharWidths[i];
            if( nATSUSumWidth >= nATSUMaxWidth )
                return (mnMinCharPos + i);
            nATSUSumWidth += nATSUExtraWidth;
            if( nATSUSumWidth >= nATSUMaxWidth )
                if( i+1 < mnCharCount )
                    return (mnMinCharPos + i);
        }

        return STRING_LEN;
#endif
    }

    // get a quick overview on what could fit
    const CGFloat nPixelWidth = (nMaxWidth - (nCharExtra * mnCharCount)) / nFactor;
    if( nPixelWidth <= 0 ) {
        SAL_INFO( "vcl.coretext.layout", "GetTextBreak(): nPixelWidth=" << nPixelWidth << ", returning mnMinCharPos=" << mnMinCharPos );
        return mnMinCharPos;
    }

    CFIndex nBreakPos = CTTypesetterSuggestLineBreak( mpTypesetter, 0, nPixelWidth ) + mnMinCharPos;

    // upper layers expect STRING_LEN if everything fits
    if( nBreakPos >= mnEndCharPos ) {
        SAL_INFO( "vcl.coretext.layout", "GetTextBreak(): nBreakPos=" << nBreakPos << " >= mnEndCharPos=" << mnEndCharPos << ", returning STRING_LEN" );
        return STRING_LEN;
    }

    SAL_INFO( "vcl.coretext.layout", "GetTextBreak() returning nBreakPos=" << nBreakPos );

    return nBreakPos;
}

long CoreTextLayout::GetTextWidth() const
{
    CGContextRef context = mpGraphics->GetContext();
    if (!context) {
        SAL_INFO( "vcl.coretext.layout", "GetTextWidth(): no context!?");
        return 0;
    }
    CGRect bound_rect = CTLineGetImageBounds(mpLine, context);
    long w = round_to_long((bound_rect.size.width + CTLineGetTrailingWhitespaceWidth(mpLine)) * mpStyle->GetFontStretchFactor());

    SAL_INFO( "vcl.coretext.layout", "GetTextWidth(" << this << ") returning " << w );

    return w;
}

bool CoreTextLayout::LayoutText( ImplLayoutArgs& rArgs)
{
    SAL_INFO( "vcl.coretext.layout", "LayoutText(" << this << ",rArgs=" << rArgs << ")" );

    mnCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

    /* don't layout empty (or worse negative size) strings */
    if(mnCharCount <= 0)
        return false;

#ifndef NDEBUG
    mnSavedMinCharPos = rArgs.mnMinCharPos;
    mnSavedEndCharPos = rArgs.mnEndCharPos;
    mpSavedStr = new sal_Unicode[mnCharCount];
    memcpy( mpSavedStr, &rArgs.mpStr[mnSavedMinCharPos], mnCharCount * sizeof( sal_Unicode ) );
#endif

    // Note that unlike the ATSUI code, we store only the part of the
    // buffer addressed by mnMinCharPos--mnEndCharPos. Not the whole
    // buffer. I.e. all indexing of the string as referenced to by
    // mpTypesetter should be relative to mnMinCharPos.
    CFStringRef string = CFStringCreateWithCharacters( NULL, &(rArgs.mpStr[rArgs.mnMinCharPos]), mnCharCount );
    if ( !string ) {
        SAL_INFO( "vcl.coretext.layout", "  CFStringCreateWithCharacter() returned NULL, returning false" );
        return false;
    }

    CFStringRef keys[1];
    CFTypeRef values[1];

    keys[0] = kCTFontAttributeName;
    values[0] = CFRetain( mpStyle->GetFont() );

    CFDictionaryRef attributes = CFDictionaryCreate( kCFAllocatorDefault,
                                                     (const void**)&keys,
                                                     (const void**)&values,
                                                     1,
                                                     &kCFTypeDictionaryKeyCallBacks,
                                                     &kCFTypeDictionaryValueCallBacks );

    CFAttributedStringRef attributed_string = CFAttributedStringCreate( NULL, string, attributes );
    CFRelease( string );
    CFRelease( attributes );
    if ( !attributed_string ) {
        SAL_INFO( "vcl.coretext.layout", "  CFAttributedStringCreate() returned NULL, returning false" );
        return false;
    }

    mpTypesetter = CTTypesetterCreateWithAttributedString( attributed_string );
    CFRelease( attributed_string );
    if ( !mpTypesetter ) {
        SAL_INFO( "vcl.coretext.layout", "  CTTypesetterCreateWithAttributedString() returned NULL, returning false" );
        return false;
    }

    mpLine = CTTypesetterCreateLine( mpTypesetter, CFRangeMake( 0, 0 ) );
    if ( !mpLine ) {
        SAL_INFO( "vcl.coretext.layout", "  CTTypesetterCreateLine() returned NULL, returning false" );
        return false;
    }

    mnGlyphCount = CTLineGetGlyphCount( mpLine );

    GetMeasurements();

    SAL_INFO( "vcl.coretext.layout", "LayoutText() returning,  mnGlyphCount=" << mnGlyphCount );

    return true;
}

void CoreTextLayout::GetMeasurements()
{
    InvalidateMeasurements();

    mpGlyphs = new CGGlyph[ mnGlyphCount ];
    mpCharWidths = new CGFloat[ mnCharCount ];
    mpGlyphs2Chars = new int[ mnGlyphCount ];
    mpGlyphAdvances = new CGSize[ mnGlyphCount ];
    mpGlyphPositions = new CGPoint[ mnGlyphCount ];

    CFArrayRef runs = CTLineGetGlyphRuns( mpLine );
    CFIndex nb_runs = CFArrayGetCount( runs );

    CFIndex lineGlyphIx = 0;
    for ( CFIndex runIx = 0; runIx < nb_runs; runIx++ )
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( runs, runIx );
        if ( !run )
            continue;

        std::ostringstream glyphPositionInfo;
        std::ostringstream glyphAdvancesInfo;
        std::ostringstream charWidthInfo;

        const CFIndex runGlyphCount = CTRunGetGlyphCount( run );
        if ( runGlyphCount )
        {
            assert( lineGlyphIx + runGlyphCount <= mnGlyphCount );

            const CFIndex lineRunGlyphStartIx = lineGlyphIx;

            CFIndex runStringIndices[ runGlyphCount ];
            CTRunGetStringIndices( run, CFRangeMake( 0, 0 ), runStringIndices );

            CTRunGetGlyphs( run, CFRangeMake( 0, 0 ), &mpGlyphs[ lineGlyphIx ] );

            CTRunGetPositions( run, CFRangeMake( 0, 0 ), &mpGlyphPositions[ lineGlyphIx ] );
            CTRunGetAdvances( run, CFRangeMake( 0, 0 ), &mpGlyphAdvances[ lineGlyphIx ] );

            bool isVerticalRun = false;
            CFDictionaryRef aDict = CTRunGetAttributes( run );
            if ( aDict ) {
                const CFBooleanRef aValue = (const CFBooleanRef)CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName );
                isVerticalRun = (aValue == kCFBooleanTrue);
            }

            for ( CFIndex runGlyphIx = 0 ; runGlyphIx < runGlyphCount; lineGlyphIx++, runGlyphIx++ )
            {
                const CFIndex charIx = runStringIndices[ runGlyphIx ];
                assert( charIx < mnCharCount );
                mpGlyphs2Chars[ lineGlyphIx ] = charIx;

                mpCharWidths[ charIx ] = mpGlyphAdvances[ lineGlyphIx ].width;
            }
#ifdef SAL_LOG_INFO
            for ( int i = 0; i < runGlyphCount; i++ ) {
                const int ix = lineRunGlyphStartIx + i;
                if ( i < 7 ) {
                    glyphPositionInfo << " " << mpGlyphs[ ix ] << "@" << mpGlyphPositions[ ix ];
                    glyphAdvancesInfo << " " << mpGlyphAdvances[ ix ];
                } else if (i == 7 ) {
                    glyphPositionInfo << "...";
                    glyphAdvancesInfo << "...";
                }
            }
            SAL_INFO( "vcl.coretext.layout", "  run " << runIx << ": " << runGlyphCount << " glyphs:" << glyphPositionInfo.str() );
            SAL_INFO( "vcl.coretext.layout", "  run " << runIx << ": advances:" << glyphAdvancesInfo.str() );
#endif
        }
    }

#ifdef SAL_LOG_INFO
    std::ostringstream charWidthInfo;

    for ( int ix = 0; ix < mnCharCount; ix++ ) {
        if ( ix < 7 )
            charWidthInfo << " " << mpCharWidths[ ix ];
        else if ( ix == 7 )
            charWidthInfo << "...";
    }
    SAL_INFO( "vcl.coretext.layout", "  char widths:" << charWidthInfo.str() );
#endif
}


// not needed. CoreText manage fallback directly
void CoreTextLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ )
{
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::Simplify( bool /*bIsBase*/ )
{
}

SalLayout* QuartzSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    CoreTextLayout* layout = new CoreTextLayout( this, m_style );

    return layout;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
