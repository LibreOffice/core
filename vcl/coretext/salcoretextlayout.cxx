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
    void Clean();
    bool InitGIA() const;

    QuartzSalGraphics* mpGraphics;
    CoreTextStyleInfo* mpStyle;

    int mnCharCount;                    // ==mnEndCharPos-mnMinCharPos

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable int mnGlyphCount;           // glyph count
    mutable CGFloat mnCachedWidth;      // cached value of resulting typographical width

    mutable CGGlyph* mpGlyphs;          // glyphs
    mutable int* mpCharWidths;          // map relative charpos to charwidth
    mutable int* mpChars2Glyphs;        // map relative charpos to absolute glyphpos
    mutable int* mpGlyphs2Chars;        // map absolute glyphpos to absolute charpos

    mutable int* mpGlyphAdvances;       // glyph widths for the justified layout

    mutable CGPoint* mpGlyphPositions; 
    CTTypesetterRef mpTypesetter;
    CTLineRef mpLine;
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
    mnCachedWidth(0.0F),
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
}

CoreTextLayout::~CoreTextLayout()
{
    Clean();
}

void CoreTextLayout::AdjustLayout( ImplLayoutArgs& /*rArgs*/ )
{
    SAL_INFO( "vcl.coretext.layout", "-->" );
    SAL_INFO( "vcl.coretext.layout", "<--" );
    /* TODO */
}

void CoreTextLayout::Clean()
{
    SAL_INFO( "vcl.coretext.layout", "-->" );
    if(mpGlyphs)
    {
        delete[] mpGlyphs;
        mpGlyphs = NULL;
    }
    if(mpChars2Glyphs)
    {
        delete[] mpChars2Glyphs;
        mpChars2Glyphs = NULL;
    }
    if(mpGlyphs2Chars)
    {
        delete[] mpGlyphs2Chars;
        mpGlyphs2Chars = NULL;
    }
    if(mpCharWidths)
    {
        delete[] mpCharWidths;
        mpCharWidths = NULL;
    }
    if(mpGlyphAdvances)
    {
        delete[] mpGlyphAdvances;
        mpGlyphAdvances = NULL;
    }
    if(mpGlyphPositions)
    {
        delete[] mpGlyphPositions;
        mpGlyphPositions = NULL;
    }
    SafeCFRelease(mpTypesetter);
    SafeCFRelease(mpLine);
    mbHasBoundRectangle = false;
    SAL_INFO( "vcl.coretext.layout", "<--" );
}

void CoreTextLayout::DrawText( SalGraphics& rGraphics ) const
{
    SAL_INFO( "vcl.coretext.layout", "-->" );
    QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);
    if(mnCharCount <= 0 || !gr.CheckContext())
    {
        return;
    }
    CGContextSaveGState( gr.mrContext );
    Point pos = GetDrawPosition(Point(0,0));
#if 0
    SAL_INFO( "vcl.coretext.layout", "at pos (" << pos.X() << "," << pos.Y() << ")" );
    CGContextSetTextMatrix(gr.mrContext, CGAffineTransformMakeScale(1.0, -1.0));
    CGContextSetShouldAntialias( gr.mrContext, !gr.mbNonAntialiasedText );
    CGContextSetTextPosition(gr.mrContext, pos.X(), pos.Y());
    CTLineDraw(mpLine, gr.mrContext);
#else
    InitGIA();
    SAL_INFO( "vcl.coretext.layout", "at pos (" << pos.X() << "," << pos.Y() <<") ctfont=" << mpStyle->GetFont() );
    CGFontRef cg_font = CTFontCopyGraphicsFont(mpStyle->GetFont(), NULL);
    if(!cg_font)
    {
        SAL_INFO( "vcl.coretext.layout", "Error cg_font is NULL" );
        return;
    }
    CGContextSetFont(gr.mrContext, cg_font);
    CGContextSetFontSize(gr.mrContext, CTFontGetSize(mpStyle->GetFont()));
    CGContextSetTextDrawingMode(gr.mrContext, kCGTextFill);
    CGContextSetShouldAntialias( gr.mrContext, true );
    if(mpStyle->GetColor())
    {
        CGContextSetFillColorWithColor(gr.mrContext, mpStyle->GetColor());
        CGContextSetStrokeColorWithColor(gr.mrContext, mpStyle->GetColor());
    }
    else
    {
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
    SAL_INFO( "vcl.coretext.layout", "<--" );

}

// not needed. CoreText manage fallback directly
void CoreTextLayout::DropGlyph( int /*nStart*/ ) {}

long CoreTextLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    SAL_INFO( "vcl.coretext.layout", "-->" );
    // short circuit requests which don't need full details
    if( !pDXArray )
    {
        return GetTextWidth();
    }

    // initialize details about the resulting layout
    InitGIA();

    // distribute the widths among the string elements
    long width = 0;
    float scale = mpStyle->GetFontStretchFactor();
    mnCachedWidth = 0;

    for( int i = 0; i < mnCharCount; ++i )
    {
        // convert and adjust for accumulated rounding errors
        mnCachedWidth += mpCharWidths[i];
        const long old_width = width;
        width = round_to_long(mnCachedWidth * scale);
        pDXArray[i] = width - old_width;
    }
    SAL_INFO( "vcl.coretext.layout", " width=" << width << " <--" );
    return width;
}

bool CoreTextLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{

    SAL_INFO( "vcl.coretext.layout", "-->" );
    if ( !mbHasBoundRectangle )
    {
        QuartzSalGraphics& gr = static_cast<QuartzSalGraphics&>(rGraphics);
        CGRect bound_rect = CTLineGetImageBounds( mpLine, gr.mrContext );
        if ( !CGRectIsNull( bound_rect ) )
        {
            maBoundRectangle = Rectangle(
                Point( round_to_long(bound_rect.origin.x * mpStyle->GetFontStretchFactor()),
                       round_to_long(bound_rect.origin.y - bound_rect.size.height )),
                Size( round_to_long(bound_rect.size.width * mpStyle->GetFontStretchFactor()), round_to_long(bound_rect.size.height)));
            maBoundRectangle.Justify();
        }
        mbHasBoundRectangle = true;
    }
    rVCLRect = maBoundRectangle;
    SAL_INFO( "vcl.coretext.layout", "<--" );
    return true;
}

void CoreTextLayout::GetCaretPositions( int max_index, sal_Int32* caret_position) const
{
    SAL_INFO( "vcl.coretext.layout", "max_index " << max_index << " -->" );
    int local_max = max_index < mnCharCount * 2 ? max_index : mnCharCount;
    for(int i = 0 ; i < max_index - 1; i+=2)
    {
        CGFloat primary, secondary;
        primary = CTLineGetOffsetForStringIndex(mpLine, i >> 1, &secondary);
        caret_position[i] = round_to_long(mnBaseAdvance + primary);
        caret_position[i+1] = round_to_long(mnBaseAdvance + secondary);
        i += 2;
    }
    for(int i = local_max ; i < max_index ; ++i)
    {
        caret_position[i] = -1;
    }
    SAL_INFO( "vcl.coretext.layout", "<--" );
}

bool CoreTextLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }

int CoreTextLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
                                   sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    SAL_INFO( "vcl.coretext.layout", "nLen=" << nLen << " nStart=" << nStart << " -->");
    // get glyph measurements
    InitGIA();

    if( nStart < 0 )                // first glyph requested?
    {
        nStart = 0;
        mnCurrentRunIndex = 0;
        mnCurrentGlyphIndex = 0;
        mnCurrentGlyphRunIndex = 0;
    }
    else if(nStart >= mnGlyphCount)
    {
        mnCurrentRunIndex = 0;
        mnCurrentGlyphIndex = 0;
        mnCurrentGlyphRunIndex = 0;
        return 0;
    }
    if(!mpRuns)
    {
        mpRuns = CTLineGetGlyphRuns(mpLine);
    }
    CFIndex nb_runs = CFArrayGetCount( mpRuns );
    CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( mpRuns, mnCurrentRunIndex );
    CFIndex nb_glyphs = CTRunGetGlyphCount( run );

    int i = 0;
    bool first = true;
    while(i < nLen)
    {
        if(mnCurrentGlyphRunIndex >= nb_glyphs)
        {
            mnCurrentRunIndex += 1;
            if(mnCurrentRunIndex >= nb_runs)
            {
                break;
            }
            run = (CTRunRef)CFArrayGetValueAtIndex( mpRuns, mnCurrentRunIndex );
            nb_glyphs = CTRunGetGlyphCount( run );
            mnCurrentGlyphRunIndex = 0;
        }
        if(first)
        {
            CGPoint first_pos;
            CTRunGetPositions(run, CFRangeMake(mnCurrentGlyphRunIndex,1), &first_pos);
            Point pos(first_pos.x, first_pos.y);
            rPos = GetDrawPosition(pos);
            SAL_INFO( "vcl.coretext.layout", "rPos(" << rPos.X() << "," << rPos.Y() << ")" );
            first = false;
        }
        pGlyphIDs[i] = mpGlyphs[mnCurrentGlyphIndex];
        if(pGlyphAdvances)
        {
            pGlyphAdvances[i] = mpGlyphAdvances[mnCurrentGlyphIndex];
        }
        if(pCharIndexes)
        {
            pCharIndexes[i] = mpGlyphs2Chars[mnCurrentGlyphIndex];
        }
        mnCurrentGlyphIndex += 1;
        mnCurrentGlyphRunIndex += 1;
        i += 1;
        nStart += 1;
    }
    SAL_INFO( "vcl.coretext.layout", "i=" << i << " <--" );
    return i;
}

int CoreTextLayout::GetTextBreak( long /*nMaxWidth*/, long /*nCharExtra*/, int /*nFactor*/ ) const
{
    /* TODO */
    return false;
}

long CoreTextLayout::GetTextWidth() const
{
    SAL_INFO( "vcl.coretext.layout", "-->" );

    CGRect bound_rect = CTLineGetImageBounds(mpLine, mpGraphics->GetContext());
    long w = round_to_long(bound_rect.size.width * mpStyle->GetFontStretchFactor());
    SAL_INFO( "vcl.coretext.layout", "w=" << w << " <--" );
    return w;
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::InitFont() const
{
    SAL_INFO( "vcl.coretext.layout", "<-->" );
}

bool CoreTextLayout::InitGIA() const
{
    SAL_INFO( "vcl.coretext.layout", "count=" << mnCharCount << "-->" );

    if( mnCharCount <= 0)
    {
        return false;
    }
    if(mpGlyphs)
    {
        return true;
    }

    mpGlyphs = new CGGlyph[mnGlyphCount];
    mpCharWidths = new int[ mnCharCount ];
    mpChars2Glyphs = new int[ mnCharCount ];
    for( int i = 0; i < mnCharCount; ++i)
    {
        mpCharWidths[i] = 0.0;
        mpChars2Glyphs[i] = -1;
    }
    mpGlyphs2Chars = new int[mnGlyphCount];
    mpGlyphAdvances = new int[mnGlyphCount];
    mpGlyphPositions = new CGPoint[mnGlyphCount];


    CFArrayRef runs = CTLineGetGlyphRuns( mpLine );
    CFIndex nb_runs = CFArrayGetCount( runs );
    int p = 0;
    for( CFIndex i = 0; i < nb_runs; ++i )
    {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex( runs, i );
        if( run )
        {
			CFIndex nb_glyphs = CTRunGetGlyphCount( run );
            if(nb_glyphs)
            {
                CFRange text_range = CTRunGetStringRange( run );
                if( text_range.location != kCFNotFound && text_range.length > 0 )
                {
                    CFIndex indices[ nb_glyphs ];
                    CGGlyph glyphs[ nb_glyphs ];
                    CTRunGetStringIndices( run, CFRangeMake( 0, 0 ), indices );
                    CTRunGetGlyphs( run, CFRangeMake( 0, 0 ), glyphs );
                    CTRunGetPositions( run, CFRangeMake( 0, 0 ), &mpGlyphPositions[p] );
                    bool is_vertical_run = false;
                    CFDictionaryRef aDict = CTRunGetAttributes( run );
                    if ( aDict )
                    {
                        const CFBooleanRef aValue = (const CFBooleanRef)CFDictionaryGetValue( aDict, kCTVerticalFormsAttributeName );
                        is_vertical_run =  (aValue == kCFBooleanTrue) ? true : false;
                    }

                    for (CFIndex j = 0 ; j < nb_glyphs; ++p, ++j )
                    {
                        mpGlyphs[ p ] = glyphs[ j ];
                        SAL_INFO( "vcl.coretext.layout", "m_glyphys[" << p << "]=glyphs[" << j << "] run " << i << " : 0x" << std::hex << glyphs[j] << std::dec );
                        CFIndex k = indices[ j ];
                        mpGlyphs2Chars[p] = k;
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
        }
    }
    SAL_INFO( "vcl.coretext.layout", "<--" );
    return true;
}

bool CoreTextLayout::LayoutText(ImplLayoutArgs& args)
{
    SAL_INFO( "vcl.coretext.layout", "mpStyle=" << mpStyle << " font=" << mpStyle->GetFont() << " -->" );
    Clean();
    mpStyle->SetColor();
    /* retreive MinCharPos EndCharPos Flags and Orientation */
    SalLayout::AdjustLayout(args);
    mnCharCount = mnEndCharPos - mnMinCharPos;

    /* don't layout emptty (or worse negative size) strings */
    if(mnCharCount <= 0)
    {
        return false;
    }

#ifdef IOS
    // This might be caused by some red herring and be unnecessary
    // once the CoreText code actually works;)

    // If the string contains U+FFFD ("REPLACEMENT CHARACTER"), which
    // happens at least for the ooo80484-1.slk document in
    // sc_filters_test, the CTTypesetterCreateWithAttributedString()
    // call below crashes, at least in the iOS simulator. Go figure.
    // (In that case the string consists of *only* such characters,
    // but play it safe.)
    for (int i = 0; i < mnCharCount; i++)
    {
        if (args.mpStr[args.mnMinCharPos+i] == 0xFFFD)
            return false;
    }
#endif

    /* c0 and c1 are construction objects */
    CFStringRef c0 = CFStringCreateWithCharactersNoCopy( NULL, &(args.mpStr[args.mnMinCharPos]), mnCharCount, kCFAllocatorNull );
    if ( !c0 )
    {
        Clean();
        return false;
    }

    CFStringRef keys[6];
    CFTypeRef   values[6];
    int nb_attributes = 0;

    keys[nb_attributes]= kCTFontAttributeName;
    values[nb_attributes] = mpStyle->GetFont();
    nb_attributes += 1;

    CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault,
                                                    (const void**)&keys,
                                                    (const void**)&values,
                                                    nb_attributes,
                                                    &kCFTypeDictionaryKeyCallBacks,
                                                    &kCFTypeDictionaryValueCallBacks);


    CFAttributedStringRef string = CFAttributedStringCreate( NULL, c0, attributes );
    CFRelease( c0 );
    CFRelease( attributes );
    if ( !string )
    {
        Clean();
        return false;
    }
    mpTypesetter = CTTypesetterCreateWithAttributedString(string);
    CFRelease(string);
    if(!mpTypesetter)
    {
        Clean();
        return false;
    }
    mpLine = CTTypesetterCreateLine(mpTypesetter, CFRangeMake(0, 0));
    if(!mpLine)
    {
        Clean();
        return false;
    }
    mnGlyphCount = CTLineGetGlyphCount(mpLine);

    SAL_INFO( "vcl.coretext.layout", "glyph_count=" << mnGlyphCount << " <--" );
    return true;
}

// not needed. CoreText manage fallback directly
void CoreTextLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}

// not needed. CoreText manage fallback directly
void CoreTextLayout::Simplify( bool /*bIsBase*/ ) {}

SalLayout* QuartzSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    SAL_INFO( "vcl.coretext.gr", "-->" );
    CoreTextLayout* layout = new CoreTextLayout( this, m_style );
    SAL_INFO( "vcl.coretext.gr", "layout:" << layout << " <--" );
    return layout;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
