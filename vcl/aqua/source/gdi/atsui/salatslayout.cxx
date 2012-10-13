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

#include "aqua/saldata.hxx"
#include "aqua/atsui/salgdi.h"
#include "aqua/atsui/salatsuifontutils.hxx"

#include "sallayout.hxx"
#include "salgdi.hxx"

#include <math.h>

// ATSUI is deprecated in 10.6 (or already 10.5?)
#if defined LIBO_WERROR && defined __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40201
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif
#endif

// =======================================================================

class ATSLayout : public SalLayout
{
public:
                    ATSLayout( ATSUStyle&, float fFontScale );
    virtual         ~ATSLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;

    virtual long    GetTextWidth() const;
    virtual long    FillDXArray( sal_Int32* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const;
    virtual bool    GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const;
    virtual bool    GetBoundRect( SalGraphics&, Rectangle& ) const;

    const PhysicalFontFace* GetFallbackFontData( sal_GlyphId ) const;

    virtual void    InitFont() const;
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

private:
    ATSUStyle&          mrATSUStyle;
    ATSUTextLayout      maATSULayout;
    int                 mnCharCount;        // ==mnEndCharPos-mnMinCharPos
    // to prevent ATS overflowing the Fixed16.16 values
    // ATS font requests get size limited by downscaling huge fonts
    // in these cases the font scale becomes something bigger than 1.0
    float               mfFontScale;

private:
    bool    InitGIA( ImplLayoutArgs* pArgs = NULL ) const;
    bool    GetIdealX() const;
    bool    GetDeltaY() const;
    void    InvalidateMeasurements();

    int Fixed2Vcl( Fixed ) const;       // convert ATSU-Fixed units to VCL units
    int AtsuPix2Vcl( int ) const;       // convert ATSU-Pixel units to VCL units
    Fixed   Vcl2Fixed( int ) const;     // convert VCL units to ATSU-Fixed units

    // cached details about the resulting layout
    // mutable members since these details are all lazy initialized
    mutable int         mnGlyphCount;           // glyph count
    mutable Fixed       mnCachedWidth;          // cached value of resulting typographical width
    int                 mnTrailingSpaceWidth;   // in Pixels

    mutable ATSGlyphRef*    mpGlyphIds;         // ATSU glyph ids
    mutable Fixed*          mpCharWidths;       // map relative charpos to charwidth
    mutable int*            mpChars2Glyphs;     // map relative charpos to absolute glyphpos
    mutable int*            mpGlyphs2Chars;     // map absolute glyphpos to absolute charpos
    mutable bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true if RTL
    mutable Fixed*          mpGlyphAdvances;    // contains glyph widths for the justified layout
    mutable Fixed*          mpGlyphOrigAdvs;    // contains glyph widths for the unjustified layout
    mutable Fixed*          mpDeltaY;           // vertical offset from the baseline

    struct SubPortion { int mnMinCharPos, mnEndCharPos; Fixed mnXOffset; };
    typedef std::vector<SubPortion> SubPortionVector;
    mutable SubPortionVector    maSubPortions;      // Writer&ATSUI layouts can differ quite a bit...

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
    int AddFallback( ATSUFontID );
    const PhysicalFontFace* GetFallbackFontData( int nLevel ) const;

private:
    const ImplMacFontData* maFontData[ MAX_FALLBACK ];
    ATSUFontID             maATSUFontId[ MAX_FALLBACK ];
    int                    mnMaxLevel;
};

// =======================================================================

ATSLayout::ATSLayout( ATSUStyle& rATSUStyle, float fFontScale )
:   mrATSUStyle( rATSUStyle ),
    maATSULayout( NULL ),
    mnCharCount( 0 ),
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
{}

// -----------------------------------------------------------------------

ATSLayout::~ATSLayout()
{
    if( mpDeltaY )
        ATSUDirectReleaseLayoutDataArrayPtr( NULL,
            kATSUDirectDataBaselineDeltaFixedArray, (void**)&mpDeltaY );

    if( maATSULayout )
        ATSUDisposeTextLayout( maATSULayout );

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

// -----------------------------------------------------------------------

inline int ATSLayout::Fixed2Vcl( Fixed nFixed ) const
{
    float fFloat = mfFontScale * FixedToFloat( nFixed );
    return static_cast<int>(fFloat + 0.5);
}

// -----------------------------------------------------------------------

inline int ATSLayout::AtsuPix2Vcl( int nAtsuPixel) const
{
    float fVclPixel = mfFontScale * nAtsuPixel;
    fVclPixel += (fVclPixel>=0) ? +0.5 : -0.5;  // prepare rounding to int
    int nVclPixel = static_cast<int>( fVclPixel);
    return nVclPixel;
}

// -----------------------------------------------------------------------

inline Fixed ATSLayout::Vcl2Fixed( int nPixel ) const
{
    return FloatToFixed( nPixel / mfFontScale );
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::LayoutText : Manage text layouting
 *
 * @param rArgs: contains array of char to be layouted, starting and ending position of the text to layout
 *
 * Typographic layout of text by using the style maATSUStyle
 *
 * @return : true if everything is ok
**/
bool ATSLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    if( maATSULayout )
        ATSUDisposeTextLayout( maATSULayout );

    maATSULayout = NULL;

    // Layout text
    // set up our locals, verify parameters...
    DBG_ASSERT( (rArgs.mpStr!=NULL), "ATSLayout::LayoutText() with rArgs.mpStr==NULL !!!");
    DBG_ASSERT( (mrATSUStyle!=NULL), "ATSLayout::LayoutText() with ATSUStyle==NULL !!!");

    SalLayout::AdjustLayout( rArgs );
    mnCharCount = mnEndCharPos - mnMinCharPos;

    // Workaround a bug in ATSUI with empty string
    if( mnCharCount<=0 )
        return false;

#if (OSL_DEBUG_LEVEL > 3)
    Fixed fFontSize = 0;
    ByteCount nDummy;
    ATSUGetAttribute( mrATSUStyle, kATSUSizeTag, sizeof(fFontSize), &fFontSize, &nDummy);
    String aUniName( &rArgs.mpStr[rArgs.mnMinCharPos], mnCharCount );
    rtl::OString aCName(rtl::OUStringToOString(aUniName, RTL_TEXTENCODING_UTF8));
    fprintf( stderr, "ATSLayout( \"%s\" %d..%d of %d) with h=%4.1f\n",
        aCName.getStr(),rArgs.mnMinCharPos,rArgs.mnEndCharPos,rArgs.mnLength,Fix2X(fFontSize) );
#endif

    // create the ATSUI layout
    UniCharCount nRunLengths[1] = { mnCharCount };
    const int nRunCount = sizeof(nRunLengths)/sizeof(*nRunLengths);
    OSStatus eStatus = ATSUCreateTextLayoutWithTextPtr( rArgs.mpStr,
        rArgs.mnMinCharPos, mnCharCount, rArgs.mnLength,
        nRunCount, &nRunLengths[0], &mrATSUStyle,
        &maATSULayout);

    DBG_ASSERT( (eStatus==noErr), "ATSUCreateTextLayoutWithTextPtr failed\n");
    if( eStatus != noErr )
        return false;

    // prepare setting of layout controls
    static const int nMaxTagCount = 1;
    ATSUAttributeTag aTagAttrs[ nMaxTagCount ];
    ByteCount aTagSizes[ nMaxTagCount ];
    ATSUAttributeValuePtr aTagValues[ nMaxTagCount ];

    // prepare control of "glyph fallback"
    const SalData* pSalData = GetSalData();
    ATSUFontFallbacks aFontFallbacks = pSalData->mpFontList->maFontFallbacks;
    aTagAttrs[0]  = kATSULineFontFallbacksTag;
    aTagSizes[0]  = sizeof( ATSUFontFallbacks );
    aTagValues[0] = &aFontFallbacks;

    // set paragraph layout controls
    ATSUSetLayoutControls( maATSULayout, 1, aTagAttrs, aTagSizes, aTagValues );

    // enable "glyph fallback"
    ATSUSetTransientFontMatching( maATSULayout, true );

    // control run-specific layout controls
    if( (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) != 0 )
    {
        // control BiDi defaults
        BOOL nLineDirTag = kATSULeftToRightBaseDirection;
        if( (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) != 0 )
            nLineDirTag = kATSURightToLeftBaseDirection;
        aTagAttrs[0] = kATSULineDirectionTag;
        aTagSizes[0] = sizeof( nLineDirTag );
        aTagValues[0] = &nLineDirTag;
        // set run-specific layout controls
        ATSUSetLayoutControls( maATSULayout, 1, aTagAttrs, aTagSizes, aTagValues );
    }

    return true;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::AdjustLayout : Adjust layout style
 *
 * @param rArgs: contains attributes relevant to do a text specific layout
 *
 * Adjust text layout by moving glyphs to match the requested logical widths
 *
 * @return : none
**/
void ATSLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    int nOrigWidth = GetTextWidth();
    int nPixelWidth = rArgs.mnLayoutWidth;
    if( !nPixelWidth && rArgs.mpDXArray ) {
        // for now we are only interested in the layout width
        // TODO: use all mpDXArray elements for layouting
        nPixelWidth = rArgs.mpDXArray[ mnCharCount - 1 ];

        // workaround for ATSUI not using trailing spaces for justification
        int i = mnCharCount;
        while( (--i >= 0) && IsSpacingGlyph( rArgs.mpStr[mnMinCharPos+i]|GF_ISCHAR ) ) {}
        if( i < 0 ) // nothing to do if the text is all spaces
            return;
        // #i91685# trailing letters are left aligned (right aligned for RTL)
        mnTrailingSpaceWidth = rArgs.mpDXArray[ mnCharCount-1 ];
        if( i > 0 )
            mnTrailingSpaceWidth -= rArgs.mpDXArray[ i-1 ];
        InitGIA(); // ensure valid mpCharWidths[], TODO: use GetIdealX() instead?
        mnTrailingSpaceWidth -= Fixed2Vcl( mpCharWidths[i] );
        // ignore trailing space for calculating the available width
        nOrigWidth -= mnTrailingSpaceWidth;
        nPixelWidth -= mnTrailingSpaceWidth;
        // in RTL-layouts trailing spaces are leftmost
        // TODO: use BiDi-algorithm to thoroughly check this assumption
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
            mnBaseAdv = mnTrailingSpaceWidth;
    }
    // return early if there is nothing to do
    if( !nPixelWidth )
        return;

    // HACK: justification requests which change the width by just one pixel were probably
    // #i86038# introduced by lossy conversions between integer based coordinate system
    // => ignoring such requests has many more benefits than eventual drawbacks
    if( (nOrigWidth >= nPixelWidth-1) && (nOrigWidth <= nPixelWidth+1) )
        return;

    // changing the layout will make all previous measurements invalid
    InvalidateMeasurements();

    ATSUAttributeTag nTags[3];
    ATSUAttributeValuePtr nVals[3];
    ByteCount nBytes[3];

    Fixed nFixedWidth = Vcl2Fixed( nPixelWidth );
    mnCachedWidth = nFixedWidth;
    Fract nFractFactor = kATSUFullJustification;
    ATSLineLayoutOptions nLineLayoutOptions = kATSLineHasNoHangers | kATSLineHasNoOpticalAlignment | kATSLineBreakToNearestCharacter;

    nTags[0] = kATSULineWidthTag;
    nVals[0] = &nFixedWidth;
    nBytes[0] = sizeof(Fixed);
    nTags[1] = kATSULineLayoutOptionsTag;
    nVals[1] = &nLineLayoutOptions;
    nBytes[1] = sizeof(ATSLineLayoutOptions);
    nTags[2] = kATSULineJustificationFactorTag;
    nVals[2] = &nFractFactor;
    nBytes[2] = sizeof(Fract);

    OSStatus eStatus = ATSUSetLayoutControls( maATSULayout, 3, nTags, nBytes, nVals );
    if( eStatus != noErr )
        return;

    // update the measurements of the justified layout to match the justification request
    if( rArgs.mpDXArray )
        InitGIA( &rArgs );
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::DrawText : Draw text to screen
 *
 * @param rGraphics: device to draw to
 *
 * Draw the layouted text to the CGContext
 *
 * @return : none
**/
void ATSLayout::DrawText( SalGraphics& rGraphics ) const
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

    // prepare ATSUI drawing attributes
    static const ItemCount nMaxControls = 8;
    ATSUAttributeTag theTags[ nMaxControls ];
    ByteCount theSizes[ nMaxControls];
    ATSUAttributeValuePtr theValues[ nMaxControls ];
    ItemCount numcontrols = 0;

    // Tell ATSUI to use CoreGraphics
    theTags[numcontrols] = kATSUCGContextTag;
    theSizes[numcontrols] = sizeof( CGContextRef );
    theValues[numcontrols++] = &rAquaGraphics.mrContext;

    // Rotate if necessary
    if( rAquaGraphics.mnATSUIRotation != 0 )
    {
        Fixed theAngle = rAquaGraphics.mnATSUIRotation;
        theTags[numcontrols] = kATSULineRotationTag;
        theSizes[numcontrols] = sizeof( Fixed );
        theValues[numcontrols++] = &theAngle;
    }

    DBG_ASSERT( (numcontrols <= nMaxControls), "ATSLayout::DrawText() numcontrols overflow" );
    OSStatus theErr = ATSUSetLayoutControls (maATSULayout, numcontrols, theTags, theSizes, theValues);
    (void) theErr;
    DBG_ASSERT( (theErr==noErr), "ATSLayout::DrawText ATSUSetLayoutControls failed!\n" );

    // Draw the text
    const Point aPos = GetDrawPosition( Point(mnBaseAdv,0) );
    const Fixed nFixedX = Vcl2Fixed( +aPos.X() );
    const Fixed nFixedY = Vcl2Fixed( -aPos.Y() ); // adjusted for y-mirroring
    if( maSubPortions.empty() )
        ATSUDrawText( maATSULayout, mnMinCharPos, mnCharCount, nFixedX, nFixedY );
    else
    {
        // draw the sub-portions and apply individual adjustments
        SubPortionVector::const_iterator it = maSubPortions.begin();
        for(; it != maSubPortions.end(); ++it )
        {
            const SubPortion& rSubPortion = *it;
            // calculate sub-portion offset for rotated text
            Fixed nXOfsFixed = rSubPortion.mnXOffset, nYOfsFixed = 0;
            if( rAquaGraphics.mnATSUIRotation != 0 )
            {
                const double fRadians = rAquaGraphics.mnATSUIRotation * (M_PI/0xB40000);
                nXOfsFixed = static_cast<Fixed>(static_cast<double>(+rSubPortion.mnXOffset) * cos( fRadians ));
                nYOfsFixed = static_cast<Fixed>(static_cast<double>(+rSubPortion.mnXOffset) * sin( fRadians ));
            }

            // draw sub-portions
            ATSUDrawText( maATSULayout,
                rSubPortion.mnMinCharPos, rSubPortion.mnEndCharPos - rSubPortion.mnMinCharPos,
                nFixedX + nXOfsFixed, nFixedY + nYOfsFixed );
        }
    }

    // request an update of the changed window area
    if( rAquaGraphics.IsWindowGraphics() )
    {
        Rect drawRect; // rectangle of the changed area
        theErr = ATSUMeasureTextImage( maATSULayout,
            mnMinCharPos, mnCharCount, nFixedX, nFixedY, &drawRect );
        if( theErr == noErr )
        {
            // FIXME: transformation from baseline to top left
            // with the simple approach below we invalidate too much
            short d = drawRect.bottom - drawRect.top;
            drawRect.top -= d;
            drawRect.bottom += d;
            CGRect aRect = CGRectMake( drawRect.left, drawRect.top,
                                       drawRect.right - drawRect.left,
                                       drawRect.bottom - drawRect.top );
            aRect = CGContextConvertRectToDeviceSpace( rAquaGraphics.mrContext, aRect );
            rAquaGraphics.RefreshRect( aRect );
        }
    }

    // restore the original graphic context transformations
    CGContextRestoreGState( rAquaGraphics.mrContext );
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::GetNextGlyphs : Get info about next glyphs in the layout
 *
 * @param nLen: max number of char
 * @param pGlyphs: returned array of glyph ids
 * @param rPos: returned x starting position
 * @param nStart: index of the first requested glyph
 * @param pGlyphAdvances: returned array of glyphs advances
 * @param pCharIndexes: returned array of char indexes
 *
 * Returns infos about the next glyphs in the text layout
 *
 * @return : number of glyph details that were provided
**/
int ATSLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphIDs, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    if( nStart < 0 )                // first glyph requested?
        nStart = 0;

    // get glyph measurements
    InitGIA();
    // some measurements are only needed for multi-glyph results
    if( nLen > 1 )
    {
        GetIdealX();
        GetDeltaY();
    }

    if( nStart >= mnGlyphCount )    // no glyph left?
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    Fixed nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];
    // if sub-portion offsets are involved there is an additional x-offset
    if( !maSubPortions.empty() )
    {
        // prepare to find the sub-portion
        int nCharPos = nStart + mnMinCharPos;
        if( mpGlyphs2Chars )
            nCharPos = mpGlyphs2Chars[nStart];

        // find the matching subportion
        // TODO: is a non-linear search worth it?
        SubPortionVector::const_iterator it = maSubPortions.begin();
        for(; it != maSubPortions.end(); ++it) {
            const SubPortion& r = *it;
            if( nCharPos < r.mnMinCharPos )
                continue;
            if( nCharPos >= r.mnEndCharPos )
                continue;
            // apply the sub-portion xoffset
            nXOffset += r.mnXOffset;
            break;
        }
    }

    Fixed nYOffset = 0;
    if( mpDeltaY )
        nYOffset = mpDeltaY[ nStart ];

    // calculate absolute position in pixel units
    const Point aRelativePos( Fix2Long(static_cast<Fixed>(nXOffset*mfFontScale)), Fix2Long(static_cast<Fixed>(nYOffset*mfFontScale)) );
    rPos = GetDrawPosition( aRelativePos );

    // update return values
    int nCount = 0;
    while( nCount < nLen )
    {
        ++nCount;
        sal_GlyphId nGlyphId = mpGlyphIds[nStart];

           // check if glyph fallback is needed for this glyph
        // TODO: use ATSUDirectGetLayoutDataArrayPtrFromTextLayout(kATSUDirectDataStyleIndex) API instead?
        const int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[nStart] : nStart + mnMinCharPos;
        ATSUFontID nFallbackFontID = kATSUInvalidFontID;
        UniCharArrayOffset nChangedOffset = 0;
        UniCharCount nChangedLength = 0;
        OSStatus eStatus = ATSUMatchFontsToText( maATSULayout, nCharPos, kATSUToTextEnd,
                      &nFallbackFontID, &nChangedOffset, &nChangedLength );
        if( (eStatus == kATSUFontsMatched) && ((int)nChangedOffset == nCharPos) )
        {
            // fallback is needed
            if( !mpFallbackInfo )
                mpFallbackInfo = new FallbackInfo;
            // register fallback font
            const int nLevel = mpFallbackInfo->AddFallback( nFallbackFontID );
               // update sal_GlyphId with fallback level
            nGlyphId |= (nLevel << GF_FONTSHIFT);
        }

        // update resulting glyphid array
        *(pGlyphIDs++) = nGlyphId;

        // update returned glyph advance array
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = Fixed2Vcl( mpGlyphAdvances[nStart] );

        // update returned index-into-string array
        if( pCharIndexes )
        {
            int nCharPos;
            if( mpGlyphs2Chars )
                nCharPos = mpGlyphs2Chars[nStart];
            else
                nCharPos = nStart + mnMinCharPos;
            *(pCharIndexes++) = nCharPos;
        }

        // stop at last glyph
        if( ++nStart >= mnGlyphCount )
            break;

        // stop when next the x-position is unexpected
        if( !maSubPortions.empty() )
            break;   // TODO: finish the complete sub-portion
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart-1] != mpGlyphOrigAdvs[nStart-1] )
                break;

        // stop when the next y-position is unexpected
        if( mpDeltaY )
            if( mpDeltaY[nStart-1] != mpDeltaY[nStart] )
                break;
    }

    return nCount;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::GetTextWidth : Get typographic width of layouted text
 *
 * Get typographic bounds of the text
 *
 * @return : text width
**/
long ATSLayout::GetTextWidth() const
{
    if( mnCharCount <= 0 )
        return 0;

    DBG_ASSERT( (maATSULayout!=NULL), "ATSLayout::GetTextWidth() with maATSULayout==NULL !\n");
    if( !maATSULayout )
        return 0;

    if( !mnCachedWidth )
    {
        // prepare precise measurements on pixel based or reference-device
        const UInt16 eTypeOfBounds = kATSUseFractionalOrigins;

        // determine number of needed measurement trapezoids
        ItemCount nMaxBounds = 0;
        OSStatus err = ATSUGetGlyphBounds( maATSULayout, 0, 0, mnMinCharPos, mnCharCount,
            eTypeOfBounds, 0, NULL, &nMaxBounds );
        if( (err != noErr)
        ||  (nMaxBounds <= 0) )
            return 0;

        // get the trapezoids
        typedef std::vector<ATSTrapezoid> TrapezoidVector;
        TrapezoidVector aTrapezoidVector( nMaxBounds );
        ItemCount nBoundsCount = 0;
        err = ATSUGetGlyphBounds( maATSULayout, 0, 0, mnMinCharPos, mnCharCount,
            eTypeOfBounds, nMaxBounds, &aTrapezoidVector[0], &nBoundsCount );
        if( err != noErr )
            return 0;

        DBG_ASSERT( (nBoundsCount <= nMaxBounds), "ATSLayout::GetTextWidth() : too many trapezoids !\n");

        // find the bound extremas
        Fixed nLeftBound = 0;
        Fixed nRightBound = 0;
        for( ItemCount i = 0; i < nBoundsCount; ++i )
        {
            const ATSTrapezoid& rTrap = aTrapezoidVector[i];
            if( (i == 0) || (nLeftBound < rTrap.lowerLeft.x) )
                nLeftBound = rTrap.lowerLeft.x;
            if( (i == 0) || (nRightBound > rTrap.lowerRight.x) )
                nRightBound = rTrap.lowerRight.x;
        }

        // measure the bound extremas
        mnCachedWidth = nRightBound - nLeftBound;
        // adjust for eliminated trailing space widths
    }

    int nScaledWidth = Fixed2Vcl( mnCachedWidth );
    nScaledWidth += mnTrailingSpaceWidth;
    return nScaledWidth;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::FillDXArray : Get Char widths
 *
 * @param pDXArray: array to be filled with x-advances
 *
 * Fill the pDXArray with horizontal deltas : CharWidths
 *
 * @return : typographical width of the complete text layout
**/
long ATSLayout::FillDXArray( sal_Int32* pDXArray ) const
{
    // short circuit requests which don't need full details
    if( !pDXArray )
        return GetTextWidth();

    // check assumptions
    DBG_ASSERT( !mnTrailingSpaceWidth, "ATSLayout::FillDXArray() with nTSW!=0" );

    // initialize details about the resulting layout
    InitGIA();

    // distribute the widths among the string elements
    int nPixWidth = 0;
    mnCachedWidth = 0;
    for( int i = 0; i < mnCharCount; ++i )
    {
        // convert and adjust for accumulated rounding errors
        mnCachedWidth += mpCharWidths[i];
        const int nOldPixWidth = nPixWidth;
        nPixWidth = Fixed2Vcl( mnCachedWidth );
        pDXArray[i] = nPixWidth - nOldPixWidth;
    }

    return nPixWidth;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::GetTextBreak : Find line break depending on width
 *
 * @param nMaxWidth : maximal logical text width in subpixel units
 * @param nCharExtra: expanded/condensed spacing in subpixel units
 * @param nFactor:    number of subpixel units per pixel
 *
 * Measure the layouted text to find the typographical line break
 * the result is needed by the language specific line breaking
 *
 * @return : string index corresponding to the suggested line break
**/
int ATSLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    if( !maATSULayout )
        return STRING_LEN;

    // the semantics of the legacy use case (nCharExtra!=0) cannot be mapped to ATSUBreakLine()
    if( nCharExtra != 0 )
    {
        // prepare the measurement by layouting and measuring the un-expanded/un-condensed text
        if( !InitGIA() )
            return STRING_LEN;

        // TODO: use a better way than by testing each the char position
        ATSUTextMeasurement nATSUSumWidth = 0;
        const ATSUTextMeasurement nATSUMaxWidth = Vcl2Fixed( nMaxWidth / nFactor );
        const ATSUTextMeasurement nATSUExtraWidth = Vcl2Fixed( nCharExtra ) / nFactor;
        for( int i = 0; i < mnCharCount; ++i )
        {
            nATSUSumWidth += mpCharWidths[i];
            if( nATSUSumWidth >= nATSUMaxWidth )
                return (mnMinCharPos + i);
            nATSUSumWidth += nATSUExtraWidth;
            if( nATSUSumWidth >= nATSUMaxWidth )
                if( i+1 < mnCharCount )
                    return (mnMinCharPos + i);
        }

        return STRING_LEN;
    }

    // get a quick overview on what could fit
    const long nPixelWidth = (nMaxWidth - (nCharExtra * mnCharCount)) / nFactor;
    if( nPixelWidth <= 0 )
        return mnMinCharPos;

    // check assumptions
    DBG_ASSERT( !mnTrailingSpaceWidth, "ATSLayout::GetTextBreak() with nTSW!=0" );

    // initial measurement of text break position
    UniCharArrayOffset nBreakPos = mnMinCharPos;
    const ATSUTextMeasurement nATSUMaxWidth = Vcl2Fixed( nPixelWidth );
    if( nATSUMaxWidth <= 0xFFFF ) // #i108584# avoid ATSU rejecting the parameter
        return mnMinCharPos;      //           or do ATSUMaxWidth=0x10000;
    OSStatus eStatus = ATSUBreakLine( maATSULayout, mnMinCharPos,
        nATSUMaxWidth, false, &nBreakPos );
    if( (eStatus != noErr) && (eStatus != kATSULineBreakInWord) )
        return STRING_LEN;

    // the result from ATSUBreakLine() doesn't match the semantics expected by its
    // application layer callers from SW+SVX+I18N. Adjust the results to the expectations:

    // ATSU reports that everything fits even when trailing spaces would break the line
    // #i89789# OOo's application layers expect STRING_LEN if everything fits
    if( nBreakPos >= static_cast<UniCharArrayOffset>(mnEndCharPos) )
        return STRING_LEN;

    // GetTextBreak()'s callers expect it to return the "stupid visual line break".
    // Returning anything else result.s in subtle problems in the application layers.
    static const bool bInWord = true; // TODO: add as argument to GetTextBreak() method
    if( !bInWord )
        return nBreakPos;

    // emulate stupid visual line breaking by line breaking for the remaining width
    ATSUTextMeasurement nLeft, nRight, nDummy;
    eStatus = ATSUGetUnjustifiedBounds( maATSULayout, mnMinCharPos, nBreakPos-mnMinCharPos,
        &nLeft, &nRight, &nDummy, &nDummy );
    if( eStatus != noErr )
        return nBreakPos;
    const ATSUTextMeasurement nATSURemWidth = nATSUMaxWidth - (nRight - nLeft);
    if( nATSURemWidth <= 0xFFFF ) // #i108584# avoid ATSU rejecting the parameter
        return nBreakPos;
    UniCharArrayOffset nBreakPosInWord = nBreakPos;
    eStatus = ATSUBreakLine( maATSULayout, nBreakPos, nATSURemWidth, false, &nBreakPosInWord );
    return nBreakPosInWord;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::GetCaretPositions : Find positions of carets
 *
 * @param nMaxIndex position to which we want to find the carets
 *
 * Fill the array of positions of carets (for cursors and selections)
 *
 * @return : none
**/
void ATSLayout::GetCaretPositions( int nMaxIndex, sal_Int32* pCaretXArray ) const
{
    DBG_ASSERT( ((nMaxIndex>0)&&!(nMaxIndex&1)),
        "ATSLayout::GetCaretPositions() : invalid number of caret pairs requested");

    // initialize the caret positions
    for( int i = 0; i < nMaxIndex; ++i )
        pCaretXArray[ i ] = -1;

    for( int n = 0; n <= mnCharCount; ++n )
    {
        // measure the characters cursor position
        typedef unsigned char Boolean;
        const Boolean bIsLeading = true;
        ATSUCaret aCaret0, aCaret1;
        Boolean bIsSplit;
        OSStatus eStatus = ATSUOffsetToCursorPosition( maATSULayout,
            mnMinCharPos + n, bIsLeading, kATSUByCharacter,
            &aCaret0, &aCaret1, &bIsSplit );
        if( eStatus != noErr )
            continue;
        const Fixed nFixedPos = mnBaseAdv + aCaret0.fX;
        // convert the measurement to pixel units
        const int nPixelPos = Fixed2Vcl( nFixedPos );
        // update previous trailing position
        if( n > 0 )
            pCaretXArray[2*n-1] = nPixelPos;
        // update current leading position
        if( 2*n >= nMaxIndex )
            break;
        pCaretXArray[2*n+0] = nPixelPos;
    }
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::GetBoundRect : Get rectangle dim containing the layouted text
 *
 * @param rVCLRect: rectangle of text image (layout) measures
 *
 * Get ink bounds of the text
 *
 * @return : measurement valid
**/
bool ATSLayout::GetBoundRect( SalGraphics& rGraphics, Rectangle& rVCLRect ) const
{
    AquaSalGraphics& rAquaGraphics = static_cast<AquaSalGraphics&>(rGraphics);

    const Point aPos = GetDrawPosition( Point(mnBaseAdv, 0) );
    const Fixed nFixedX = Vcl2Fixed( +aPos.X() );
    const Fixed nFixedY = Vcl2Fixed( +aPos.Y() );

    // prepare ATSUI drawing attributes
    static const ItemCount nMaxControls = 8;
    ATSUAttributeTag theTags[ nMaxControls ];
    ByteCount theSizes[ nMaxControls];
    ATSUAttributeValuePtr theValues[ nMaxControls ];
    ItemCount numcontrols = 0;

    // Tell ATSUI to use CoreGraphics
    theTags[numcontrols] = kATSUCGContextTag;
    theSizes[numcontrols] = sizeof( CGContextRef );
    theValues[numcontrols++] = &rAquaGraphics.mrContext;

    // Rotate if necessary
    if( rAquaGraphics.mnATSUIRotation != 0 )
    {
        Fixed theAngle = rAquaGraphics.mnATSUIRotation;
        theTags[numcontrols] = kATSULineRotationTag;
        theSizes[numcontrols] = sizeof( Fixed );
        theValues[numcontrols++] = &theAngle;
    }

    DBG_ASSERT( (numcontrols <= nMaxControls), "ATSLayout::GetBoundRect() numcontrols overflow" );
    OSStatus theErr = ATSUSetLayoutControls (maATSULayout, numcontrols, theTags, theSizes, theValues);
    (void) theErr;
    DBG_ASSERT( (theErr==noErr), "ATSLayout::GetBoundRect ATSUSetLayoutControls failed!\n" );

    Rect aMacRect;
    OSStatus eStatus = ATSUMeasureTextImage( maATSULayout,
        mnMinCharPos, mnCharCount, nFixedX, nFixedY, &aMacRect );
    if( eStatus != noErr )
        return false;

    // ATSU top-bottom are vertically flipped from a VCL aspect
    rVCLRect.Left()   = AtsuPix2Vcl( aMacRect.left );
    rVCLRect.Top()    = AtsuPix2Vcl( aMacRect.top );
    rVCLRect.Right()  = AtsuPix2Vcl( aMacRect.right );
    rVCLRect.Bottom() = AtsuPix2Vcl( aMacRect.bottom );
    return true;
}

// -----------------------------------------------------------------------
/**
 * ATSLayout::InitGIA() : get many informations about layouted text
 *
 * Fills arrays of information about the gylph layout previously done
 *  in ASTLayout::LayoutText() : glyph advance (width), glyph delta Y (from baseline),
 *  mapping between glyph index and character index, chars widths
 *
 * @return : true if everything could be computed, otherwise false
**/
bool ATSLayout::InitGIA( ImplLayoutArgs* pArgs ) const
{
    // no need to run InitGIA more than once on the same ATSLayout object
    if( mnGlyphCount >= 0 )
        return true;
    mnGlyphCount = 0;

    // Workaround a bug in ATSUI with empty string
    if( mnCharCount <=  0 )
        return false;

    // initialize character details
    mpCharWidths    = new Fixed[ mnCharCount ];
    mpChars2Glyphs  = new int[ mnCharCount ];
    for( int n = 0; n < mnCharCount; ++n )
    {
        mpCharWidths[ n ] = 0;
        mpChars2Glyphs[ n ] = -1;
    }

    // get details about the glyph layout
    ItemCount iLayoutDataCount;
    const ATSLayoutRecord* pALR;
    OSStatus eStatus = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(
        maATSULayout, mnMinCharPos, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
        (void**)&pALR, &iLayoutDataCount );
    DBG_ASSERT( (eStatus==noErr), "ATSLayout::InitGIA() : no ATSLayoutRecords!\n");
    if( (eStatus != noErr)
    || (iLayoutDataCount <= 1) )
        return false;

    // initialize glyph details
    mpGlyphIds      = new ATSGlyphRef[ iLayoutDataCount ];
    mpGlyphAdvances = new Fixed[ iLayoutDataCount ];
    mpGlyphs2Chars  = new int[ iLayoutDataCount ];

    // measure details of the glyph layout
    Fixed nLeftPos = 0;
    for( ItemCount i = 0; i < iLayoutDataCount; ++i )
    {
        const ATSLayoutRecord& rALR = pALR[i];

        // distribute the widths as fairly as possible among the chars
        const int nRelativeIdx = (rALR.originalOffset / 2);
        if( i+1 < iLayoutDataCount )
            mpCharWidths[ nRelativeIdx ] += pALR[i+1].realPos - rALR.realPos;

        // new glyph is available => finish measurement of old glyph
        if( mnGlyphCount > 0 )
            mpGlyphAdvances[ mnGlyphCount-1 ] = rALR.realPos - nLeftPos;

        // ignore marker or deleted glyphs
        enum { MARKED_OUTGLYPH=0xFFFE, DROPPED_OUTGLYPH=0xFFFF};
        if( rALR.glyphID >= MARKED_OUTGLYPH )
            continue;

        DBG_ASSERT( !(rALR.flags & kATSGlyphInfoTerminatorGlyph),
            "ATSLayout::InitGIA(): terminator glyph not marked as deleted!" );

        // store details of the visible glyphs
        nLeftPos = rALR.realPos;
        mpGlyphIds[ mnGlyphCount ] = rALR.glyphID;

        // map visible glyphs to their counterparts in the UTF16-character array
        mpGlyphs2Chars[ mnGlyphCount ] = nRelativeIdx + mnMinCharPos;
        mpChars2Glyphs[ nRelativeIdx ] = mnGlyphCount;

        ++mnGlyphCount;
    }

    // measure complete width
    mnCachedWidth = mnBaseAdv;
    mnCachedWidth += pALR[iLayoutDataCount-1].realPos - pALR[0].realPos;

#if (OSL_DEBUG_LEVEL > 1)
    Fixed nWidthSum = mnBaseAdv;
    for( int n = 0; n < mnCharCount; ++n )
        nWidthSum += mpCharWidths[ n ];
    DBG_ASSERT( (nWidthSum==mnCachedWidth),
        "ATSLayout::InitGIA(): measured widths do not match!\n" );
#endif

    // #i91183# we need to split up the portion into sub-portions
    // if the ATSU-layout differs too much from the requested layout
    if( pArgs && pArgs->mpDXArray )
    {
        // TODO: non-strong-LTR case cases should be handled too
        if( (pArgs->mnFlags & TEXT_LAYOUT_BIDI_STRONG)
        && !(pArgs->mnFlags & TEXT_LAYOUT_BIDI_RTL) )
        {
            Fixed nSumCharWidths = 0;
            SubPortion aSubPortion = { mnMinCharPos, 0, 0 };
            for( int i = 0; i < mnCharCount; ++i )
            {
                // calculate related logical position
                nSumCharWidths += mpCharWidths[i];

                // start new sub-portion if needed
                const Fixed nNextXPos = Vcl2Fixed(pArgs->mpDXArray[i]);
                const Fixed nNextXOffset = nNextXPos - nSumCharWidths;
                const Fixed nFixedDiff = aSubPortion.mnXOffset - nNextXOffset;
                if( (nFixedDiff < -0xC000) || (nFixedDiff > +0xC000) ) {
                    // get to the end of the current sub-portion
                    // prevent splitting up at diacritics etc.
                    int j = i;
                    while( (++j < mnCharCount) && !mpCharWidths[j] )
                        ;
                    aSubPortion.mnEndCharPos = mnMinCharPos + j;
                    // emit current sub-portion
                    maSubPortions.push_back( aSubPortion );
                    // prepare next sub-portion
                    aSubPortion.mnMinCharPos = aSubPortion.mnEndCharPos;
                    aSubPortion.mnXOffset = nNextXOffset;
                }
            }

            // emit the remaining sub-portion
            if( !maSubPortions.empty() )
            {
                aSubPortion.mnEndCharPos = mnEndCharPos;
                if( aSubPortion.mnEndCharPos != aSubPortion.mnMinCharPos )
                    maSubPortions.push_back( aSubPortion );
            }
        }

        // override layouted charwidths with requested charwidths
        for( int n = 0; n < mnCharCount; ++n )
            mpCharWidths[ n ] = pArgs->mpDXArray[ n ];
    }

    // release the ATSU layout records
    ATSUDirectReleaseLayoutDataArrayPtr(NULL,
        kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void**)&pALR );

    return true;
}

// -----------------------------------------------------------------------

bool ATSLayout::GetIdealX() const
{
    // compute the ideal advance widths only once
    if( mpGlyphOrigAdvs != NULL )
        return true;

    DBG_ASSERT( (mpGlyphIds!=NULL), "GetIdealX() called with mpGlyphIds==NULL !" );
    DBG_ASSERT( (mrATSUStyle!=NULL), "GetIdealX called with mrATSUStyle==NULL !" );

    // TODO: cache ideal metrics per glyph?
    std::vector<ATSGlyphIdealMetrics> aIdealMetrics;
    aIdealMetrics.resize( mnGlyphCount );
    OSStatus theErr = ATSUGlyphGetIdealMetrics( mrATSUStyle,
        mnGlyphCount, &mpGlyphIds[0], sizeof(*mpGlyphIds), &aIdealMetrics[0] );
    DBG_ASSERT( (theErr==noErr), "ATSUGlyphGetIdealMetrics failed!");
    if( theErr != noErr )
        return false;

    mpGlyphOrigAdvs = new Fixed[ mnGlyphCount ];
    for( int i = 0;i < mnGlyphCount;++i )
        mpGlyphOrigAdvs[i] = FloatToFixed( aIdealMetrics[i].advance.x );

    return true;
}

// -----------------------------------------------------------------------

bool ATSLayout::GetDeltaY() const
{
    // don't bother to get the same delta-y-array more than once
    if( mpDeltaY != NULL )
        return true;

    if( !maATSULayout )
        return false;

    // get and keep the y-deltas in the mpDeltaY member variable
    // => release it in the destructor
    ItemCount nDeltaCount = 0;
    OSStatus theErr = ATSUDirectGetLayoutDataArrayPtrFromTextLayout(
        maATSULayout, mnMinCharPos, kATSUDirectDataBaselineDeltaFixedArray,
        (void**)&mpDeltaY, &nDeltaCount );

    DBG_ASSERT( (theErr==noErr ), "mpDeltaY - ATSUDirectGetLayoutDataArrayPtrFromTextLayout failed!\n");
    if( theErr != noErr )
        return false;

    if( mpDeltaY == NULL )
        return true;

    if( nDeltaCount != (ItemCount)mnGlyphCount )
    {
        DBG_WARNING( "ATSLayout::GetDeltaY() : wrong deltaY count!" );
        ATSUDirectReleaseLayoutDataArrayPtr( NULL,
            kATSUDirectDataBaselineDeltaFixedArray, (void**)&mpDeltaY );
        mpDeltaY = NULL;
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------

#define DELETEAZ( X ) { delete[] X; X = NULL; }

void ATSLayout::InvalidateMeasurements()
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

// =======================================================================

// glyph fallback is supported directly by Aqua
// so methods used only by MultiSalLayout can be dummy implementated
bool ATSLayout::GetGlyphOutlines( SalGraphics&, PolyPolyVector& ) const { return false; }
void ATSLayout::InitFont() const {}
void ATSLayout::MoveGlyph( int /*nStart*/, long /*nNewXPos*/ ) {}
void ATSLayout::DropGlyph( int /*nStart*/ ) {}
void ATSLayout::Simplify( bool /*bIsBase*/ ) {}

// get the PhysicalFontFace for a glyph fallback font
// for a glyphid that was returned by ATSLayout::GetNextGlyphs()
const PhysicalFontFace* ATSLayout::GetFallbackFontData( sal_GlyphId nGlyphId ) const
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

// =======================================================================

int FallbackInfo::AddFallback( ATSUFontID nFontId )
{
    // check if the fallback font is already known
    for( int nLevel = 0; nLevel < mnMaxLevel; ++nLevel )
        if( maATSUFontId[ nLevel ] == nFontId )
            return (nLevel + 1);

    // append new fallback font if possible
    if( mnMaxLevel >= MAX_FALLBACK-1 )
        return 0;
    // keep ATSU font id of fallback font
    maATSUFontId[ mnMaxLevel ] = nFontId;
    // find and cache the corresponding PhysicalFontFace pointer
    const SystemFontList* pSFL = GetSalData()->mpFontList;
    const ImplMacFontData* pFontData = pSFL->GetFontDataFromId( nFontId );
    maFontData[ mnMaxLevel ] = pFontData;
    // increase fallback level by one
    return (++mnMaxLevel);
}

// -----------------------------------------------------------------------

const PhysicalFontFace* FallbackInfo::GetFallbackFontData( int nFallbackLevel ) const
{
    const ImplMacFontData* pFallbackFont = maFontData[ nFallbackLevel-1 ];
    return pFallbackFont;
}

// =======================================================================

SalLayout* AquaSalGraphics::GetTextLayout( ImplLayoutArgs&, int /*nFallbackLevel*/ )
{
    ATSLayout* pATSLayout = new ATSLayout( maATSUStyle, mfFontScale );
    return pATSLayout;
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
