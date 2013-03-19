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

#include "coretext/common.h"
#include "coretext/salcoretextstyle.hxx"
#include "coretext/salgdi.h"

class CoreTextLayout : public SalLayout
{
public:
    CoreTextLayout( QuartzSalGraphics* graphics, CoreTextStyleInfo* style);
    virtual         ~CoreTextLayout();

    virtual void AdjustLayout( ImplLayoutArgs& );
    virtual void DrawText( SalGraphics& ) const;
    virtual void DropGlyph( int nStart );
    virtual long FillDXArray( sal_Int32* pDXArray ) const;
    virtual bool GetBoundRect( SalGraphics&, Rectangle& ) const;
    virtual void GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;
    virtual bool GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual int GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                               sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;
    virtual int GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual long GetTextWidth() const;
    virtual void InitFont() const;
    virtual bool LayoutText( ImplLayoutArgs& );
    virtual void MoveGlyph( int nStart, long nNewXPos );
    virtual void Simplify( bool bIsBase );

private:
    void InvalidateMeasurements();
    bool InitGIA( ImplLayoutArgs &rArgs ) const;

    QuartzSalGraphics* mpGraphics;
    CoreTextStyleInfo* mpStyle;

    int mnCharCount;                    // ==mnEndCharPos-mnMinCharPos

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable int mnGlyphCount;           // glyph count

    mutable CGGlyph* mpGlyphs;          // glyphs
    mutable int* mpCharWidths;          // map relative charpos to charwidth
    mutable int* mpChars2Glyphs;        // map relative charpos to absolute glyphpos
    mutable int* mpGlyphs2Chars;        // map absolute glyphpos to absolute charpos

    mutable int* mpGlyphAdvances;       // glyph widths for the justified layout

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
    mpGraphics(graphics),
    mpStyle(style),
    mnCharCount(-1),
    mnGlyphCount(-1),
    mpGlyphs(NULL),
    mpCharWidths(NULL),
    mpChars2Glyphs(NULL),
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
    SAL_INFO( "vcl.coretext.layout", "CoreTextLayout::CoreTextLayout() " << this );
}

CoreTextLayout::~CoreTextLayout()
{
    InvalidateMeasurements();
    SAL_INFO( "vcl.coretext.layout", "~CoreTextLayout(" << this << ")" );
}

std::ostream &operator <<(std::ostream& s, ImplLayoutArgs &rArgs)
{
#ifndef SAL_LOG_INFO
    (void) rArgs;
    return s;
#else
    s << "ImplLayoutArgs{";

    s << "Flags=";
    if (rArgs.mnFlags == 0)
        s << 0;
    else {
        bool need_or = false;
        s << "{";
#define TEST(x) if (rArgs.mnFlags & SAL_LAYOUT_##x) { if (need_or) s << "|"; s << #x; need_or = true; }
        TEST(BIDI_RTL);
        TEST(BIDI_STRONG);
        TEST(RIGHT_ALIGN);
        TEST(KERNING_PAIRS);
        TEST(KERNING_ASIAN);
        TEST(VERTICAL);
        TEST(COMPLEX_DISABLED);
        TEST(ENABLE_LIGATURES);
        TEST(SUBSTITUTE_DIGITS);
        TEST(KASHIDA_JUSTIFICATON);
        TEST(DISABLE_GLYPH_PROCESSING);
        TEST(FOR_FALLBACK);
#undef TEST
        s << "}";
    }

    s << ",Length=" << rArgs.mnLength;
    s << ",MinCharPos=" << rArgs.mnMinCharPos;
    s << ",EndCharPos=" << rArgs.mnEndCharPos;

    s << ",Str=\"";
    int lim = rArgs.mnLength;
    if (lim > 10)
        lim = 7;
    for (int i = 0; i < lim; i++) {
        if (rArgs.mpStr[i] == '\n')
            s << "\\n";
        else if (rArgs.mpStr[i] < ' ' || (rArgs.mpStr[i] >= 0x7F && rArgs.mpStr[i] <= 0xFF))
            s << "\\0x" << std::hex << std::setw(2) << std::setfill('0') << (int) rArgs.mpStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
        else if (rArgs.mpStr[i] < 0x7F)
            s << (char) rArgs.mpStr[i];
        else
            s << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int) rArgs.mpStr[i] << std::setfill(' ') << std::setw(1) << std::dec;
    }
    if (rArgs.mnLength > lim)
        s << "...";
    s << "\"";

    s << ",LayoutWidth=" << rArgs.mnLayoutWidth;

    s << "}";

    return s;
#endif
}

void CoreTextLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SAL_INFO( "vcl.coretext.layout", "AdjustLayout(" << this << ",rArgs=" << rArgs << ")" );

    InvalidateMeasurements();
    SalLayout::AdjustLayout( rArgs );
    mnCharCount = mnEndCharPos - mnMinCharPos;
    InitGIA( rArgs );
}

void CoreTextLayout::InvalidateMeasurements()
{
    SAL_INFO( "vcl.coretext.layout", "InvalidateMeasurements(" << this << ")" );

    if( mpGlyphs ) {
        delete[] mpGlyphs;
        mpGlyphs = NULL;
    }
    if( mpChars2Glyphs ) {
        delete[] mpChars2Glyphs;
        mpChars2Glyphs = NULL;
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
    SafeCFRelease(mpTypesetter);
    SafeCFRelease(mpLine);
    mbHasBoundRectangle = false;
}

void CoreTextLayout::DrawText( SalGraphics& rGraphics ) const
{
    SAL_INFO( "vcl.coretext.layout", "DrawText(" << this << ")" );

    QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);
    if( mnCharCount <= 0 || !gr.CheckContext() )
        return;

    CGContextSaveGState( gr.mrContext );
    Point pos = GetDrawPosition(Point(0,0));
#if 0
    SAL_INFO( "vcl.coretext.layout", "at pos (" << pos.X() << "," << pos.Y() << ")" );

    CGContextSetTextMatrix(gr.mrContext, CGAffineTransformMakeScale(1.0, -1.0));
    CGContextSetShouldAntialias( gr.mrContext, !gr.mbNonAntialiasedText );
    CGContextSetTextPosition(gr.mrContext, pos.X(), pos.Y());
    CTLineDraw(mpLine, gr.mrContext);
#else
    SAL_INFO( "vcl.coretext.layout", "at pos (" << pos.X() << "," << pos.Y() <<") ctfont=" << mpStyle->GetFont() );

    CGFontRef cg_font = CTFontCopyGraphicsFont(mpStyle->GetFont(), NULL);
    if( !cg_font ) {
        SAL_INFO( "vcl.coretext.layout", "Error cg_font is NULL" );
        return;
    }
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
    CGContextShowGlyphs(gr.mrContext, mpGlyphs, mnGlyphCount);
#endif
    // restore the original graphic context transformations
    CGContextRestoreGState( gr.mrContext );
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::DropGlyph( int /*nStart*/ )
{
}

long CoreTextLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    SAL_INFO( "vcl.coretext.layout", "FillDXArray(" << this << ")" );

    // short circuit requests which don't need full details
    if( !pDXArray ) {
        SAL_INFO( "vcl.coretext.layout", "FillDXArray() returning GetTextWidth()" );
        return GetTextWidth();
    }

    // distribute the widths among the string elements
    long width = 0;
    float scale = mpStyle->GetFontStretchFactor();
    CGFloat accumulated_width = 0;

    for( int i = 0; i < mnCharCount; ++i ) {
        // convert and adjust for accumulated rounding errors
        accumulated_width += mpCharWidths[i];
        const long old_width = width;
        width = round_to_long(accumulated_width * scale);
        pDXArray[i] = width - old_width;
    }

    SAL_INFO( "vcl.coretext.layout", "FillDXArrar() returning " << width );

    return width;
}

bool CoreTextLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{

    SAL_INFO( "vcl.coretext.layout", "GetBoundRect(" << this << ")" );

    if ( !mbHasBoundRectangle ) {
        QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);
        CGRect bound_rect = CTLineGetImageBounds( mpLine, gr.mrContext );
        if ( !CGRectIsNull( bound_rect ) ) {
            maBoundRectangle = Rectangle(
                Point( round_to_long(bound_rect.origin.x * mpStyle->GetFontStretchFactor()),
                       round_to_long(bound_rect.origin.y - bound_rect.size.height )),
                Size( round_to_long(bound_rect.size.width * mpStyle->GetFontStretchFactor()), round_to_long(bound_rect.size.height)));
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

bool CoreTextLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const
{
    return false;
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
            pGlyphAdvances[i] = mpGlyphAdvances[mnCurrentGlyphIndex];
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
    CGRect bound_rect = CTLineGetImageBounds(mpLine, mpGraphics->GetContext());
    long w = round_to_long(bound_rect.size.width * mpStyle->GetFontStretchFactor());

    SAL_INFO( "vcl.coretext.layout", "GetTextWidth(" << this << ") returning " << w );

    return w;
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::InitFont() const
{
    SAL_INFO( "vcl.coretext.layout", "InitFont(" << this << ")" );
}

bool CoreTextLayout::InitGIA( ImplLayoutArgs& rArgs ) const
{
    SAL_INFO( "vcl.coretext.layout", "InitGIA(" << this << "): " << mnCharCount << ":" << rArgs.mnMinCharPos << "--" << mnEndCharPos );

    if( mnCharCount <= 0) {
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): mnCharCount is non-positive, returning false" );
        return false;
    }

    if( mpGlyphs ) {
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): mpGlyphs is non-NULL, returning true" );
        return true;
    }

    // Note that unlike the ATSUI code, we store only the part of the
    // buffer addressed by mnMinCharPos--mnEndCharPos. Not the whole
    // buffer. I.e. all indexing of the string as referenced to by
    // mpTypesetter should be relative to mnMinCharPos.
    CFStringRef string = CFStringCreateWithCharacters( NULL, &(rArgs.mpStr[rArgs.mnMinCharPos]), mnCharCount );
    if ( !string ) {
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): CFStringCreateWithCharacter() returned NULL, returning false" );
        return false;
    }

    CFStringRef keys[6];
    CFTypeRef values[6];

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
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): CFAttributedStringCreate() returned NULL, returning false" );
        return false;
    }

    mpTypesetter = CTTypesetterCreateWithAttributedString( attributed_string );
    CFRelease( attributed_string );
    if( !mpTypesetter ) {
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): CTTypesetterCreateWithAttributedString() returned NULL, returning false" );
        return false;
    }

    mpLine = CTTypesetterCreateLine( mpTypesetter, CFRangeMake( 0, 0 ) );
    if( !mpLine ) {
        SAL_INFO( "vcl.coretext.layout", "InitGIA(): CTTypesetterCreateLine() returned NULL, returning false" );
        return false;
    }

    mnGlyphCount = CTLineGetGlyphCount( mpLine );
    SAL_INFO( "vcl.coretext.layout", "InitGIA(): CTLineGetGlyphCount() returned " << mnGlyphCount );

    mpGlyphs = new CGGlyph[ mnGlyphCount ];
    mpCharWidths = new int[ mnCharCount ];
    mpChars2Glyphs = new int[ mnCharCount ];
    for( int i = 0; i < mnCharCount; ++i) {
        mpCharWidths[i] = 0.0;
        mpChars2Glyphs[i] = -1;
    }
    mpGlyphs2Chars = new int[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];
    mpGlyphPositions = new CGPoint[ mnGlyphCount ];

    CFArrayRef runs = CTLineGetGlyphRuns( mpLine );
    CFIndex nb_runs = CFArrayGetCount( runs );

    int p = 0;
    for( CFIndex i = 0; i < nb_runs; ++i ) {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( runs, i );
        if( run ) {
            std::ostringstream glyph_info_line;
			CFIndex nb_glyphs = CTRunGetGlyphCount( run );
            if( nb_glyphs ) {
                CFRange text_range = CTRunGetStringRange( run );
                if( text_range.location != kCFNotFound && text_range.length > 0 ) {
                    CFIndex indices[ nb_glyphs ];
                    CGGlyph glyphs[ nb_glyphs ];
                    CTRunGetStringIndices( run, CFRangeMake( 0, 0 ), indices );
                    CTRunGetGlyphs( run, CFRangeMake( 0, 0 ), glyphs );
                    CTRunGetPositions( run, CFRangeMake( 0, 0 ), &mpGlyphPositions[p] );
                    bool is_vertical_run = false;
                    CFDictionaryRef aDict = CTRunGetAttributes( run );
                    if ( aDict ) {
                        const CFBooleanRef aValue = (const CFBooleanRef)CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName );
                        is_vertical_run =  (aValue == kCFBooleanTrue) ? true : false;
                    }

                    for (CFIndex j = 0 ; j < nb_glyphs; ++p, ++j ) {
                        assert ( p < mnGlyphCount );
                        mpGlyphs[ p ] = glyphs[ j ];
#ifdef SAL_LOG_INFO
                        if (j < 7)
                            glyph_info_line << " " << glyphs[j] << "@(" << mpGlyphPositions[p].x << "," << mpGlyphPositions[p].y << ")";
                        else if (j == 7)
                            glyph_info_line << "...";
#endif
                        CFIndex k = indices[ j ];
                        mpGlyphs2Chars[p] = k;
                        assert( k < mnCharCount );
                        mpChars2Glyphs[k] = p;

                        if ( j < nb_glyphs - 1 )
                        {
                            mpCharWidths[ k ] += mpGlyphPositions[ p + 1 ].x - mpGlyphPositions[ p ].x;
                        }
                        if( p > 0)
                        {
                            mpGlyphAdvances[p - 1] = mpGlyphPositions[ p ].x - mpGlyphPositions[p - 1].x;
                        }
                    }
                }
			}
            SAL_INFO( "vcl.coretext.layout", "  run " << run << " glyphs:" << glyph_info_line.str() );
        }
    }

    SAL_INFO( "vcl.coretext.layout", "InitGIA() returning normally true" );
    return true;
}

bool CoreTextLayout::LayoutText( ImplLayoutArgs& rArgs)
{
    SAL_INFO( "vcl.coretext.layout", "LayoutText(" << this << ",rArgs=" << rArgs << ")" );

    mpStyle->SetColor();

    AdjustLayout( rArgs );

    /* don't layout empty (or worse negative size) strings */
    if(mnCharCount <= 0) {
        SAL_INFO( "vcl.coretext.layout", "LayoutText(): mnCharCount non-positive, returning false!" );
        return false;
    }

#ifdef IOS
    // This might be a red herring and unnecessary once the CoreText
    // code actually works;)

    // If the string contains U+FFFD ("REPLACEMENT CHARACTER"), which
    // happens at least for the ooo80484-1.slk document in
    // sc_filters_test, the CTTypesetterCreateWithAttributedString()
    // call below crashes, at least in the iOS simulator. Go figure.
    // (In that case the string consists of *only* such characters,
    // but play it safe.)
    for (int i = 0; i < mnCharCount; i++)
    {
        if ( rArgs.mpStr[rArgs.mnMinCharPos+i] == 0xFFFD)
            return false;
    }
#endif

    SAL_INFO( "vcl.coretext.layout", "LayoutText() returning,  mnGlyphCount=" << mnGlyphCount );

    return true;
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
