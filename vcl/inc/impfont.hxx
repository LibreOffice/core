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

#ifndef _SV_IMPFONT_HXX
#define _SV_IMPFONT_HXX

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <i18nlangtag/lang.h>
#include <tools/color.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <vcl/fntstyle.hxx>
#include <outfont.hxx>

// ------------
// - Impl_Font -
// ------------

typedef sal_uInt32 FontRefCount;

class Impl_Font
{
public:
             Impl_Font();
             Impl_Font( const Impl_Font& );

    bool     operator==( const Impl_Font& ) const;

    FontPitch           GetPitch()      { if(mePitch==PITCH_DONTKNOW)    AskConfig(); return mePitch; }
    FontFamily          GetFamily()     { if(meFamily==FAMILY_DONTKNOW)  AskConfig(); return meFamily; }
    FontItalic          GetItalic()     { if(meItalic==ITALIC_DONTKNOW)  AskConfig(); return meItalic; }
    FontWeight          GetWeight()     { if(meWeight==WEIGHT_DONTKNOW)  AskConfig(); return meWeight; }
    FontWidth           GetWidthType()  { if(meWidthType==WIDTH_DONTKNOW)AskConfig(); return meWidthType; }

private:
    friend class Font;
    void                AskConfig();

    FontRefCount        mnRefCount;
    OUString            maFamilyName;
    OUString            maStyleName;
    Size                maSize;
    Color               maColor;        // compatibility, now on output device
    Color               maFillColor;    // compatibility, now on output device
    rtl_TextEncoding    meCharSet;
    LanguageType        meLanguage;
    LanguageType        meCJKLanguage;
    FontFamily          meFamily;
    FontPitch           mePitch;
    TextAlign           meAlign;
    FontWeight          meWeight;
    FontWidth           meWidthType;
    FontItalic          meItalic;
    FontUnderline       meUnderline;
    FontUnderline       meOverline;
    FontStrikeout       meStrikeout;
    FontRelief          meRelief;
    FontEmphasisMark    meEmphasisMark;
    short               mnOrientation;
    FontKerning         mnKerning;
    sal_Bool                mbWordLine:1,
                        mbOutline:1,
                        mbConfigLookup:1,   // there was a config lookup
                        mbShadow:1,
                        mbVertical:1,
                        mbTransparent:1;    // compatibility, now on output device

    friend SvStream&    operator>>( SvStream& rIStm, Impl_Font& );
    friend SvStream&    operator<<( SvStream& rOStm, const Impl_Font& );
};

// ------------------
// - ImplFontMetric -
// ------------------

class ImplFontMetric
{
    friend class OutputDevice;

private:
    long    mnAscent;      // Ascent
    long    mnDescent;     // Descent
    long    mnIntLeading;  // Internal Leading
    long    mnExtLeading;  // External Leading
    long    mnLineHeight;  // Ascent+Descent+EmphasisMark
    long    mnSlant;       // Slant
    sal_uInt16  mnMiscFlags;   // Misc Flags
    FontRefCount mnRefCount;    // Reference Counter

    enum { DEVICE_FLAG=1, SCALABLE_FLAG=2, LATIN_FLAG=4, CJK_FLAG=8, CTL_FLAG=16 };

public:
            ImplFontMetric();
    void    AddReference();
    void    DeReference();

    long    GetAscent() const       { return mnAscent; }
    long    GetDescent() const      { return mnDescent; }
    long    GetIntLeading() const   { return mnIntLeading; }
    long    GetExtLeading() const   { return mnExtLeading; }
    long    GetLineHeight() const   { return mnLineHeight; }
    long    GetSlant() const        { return mnSlant; }

    bool    IsDeviceFont() const    { return ((mnMiscFlags & DEVICE_FLAG) != 0); }
    bool    IsScalable() const      { return ((mnMiscFlags & SCALABLE_FLAG) != 0); }
    bool    SupportsLatin() const   { return ((mnMiscFlags & LATIN_FLAG) != 0); }
    bool    SupportsCJK() const     { return ((mnMiscFlags & CJK_FLAG) != 0); }
    bool    SupportsCTL() const     { return ((mnMiscFlags & CTL_FLAG) != 0); }

    bool    operator==( const ImplFontMetric& ) const;
};

// ------------------
// - ImplFontOptions -
// ------------------

class ImplFontOptions
{
public:
    FontEmbeddedBitmap meEmbeddedBitmap; // whether the embedded bitmaps should be used
    FontAntiAlias      meAntiAlias;      // whether the font should be antialiased
    FontAutoHint       meAutoHint;       // whether the font should be autohinted
    FontHinting        meHinting;        // whether the font should be hinted
    FontHintStyle      meHintStyle;      // type of font hinting to be used
public:
    ImplFontOptions() :
        meEmbeddedBitmap(EMBEDDEDBITMAP_DONTKNOW),
        meAntiAlias(ANTIALIAS_DONTKNOW),
        meAutoHint(AUTOHINT_DONTKNOW),
        meHinting(HINTING_DONTKNOW),
        meHintStyle(HINT_SLIGHT)
    {}
    virtual ~ImplFontOptions()
    {}
    FontAutoHint GetUseAutoHint() const
        { return meAutoHint; }
    FontHintStyle GetHintStyle() const
        { return meHintStyle; }
    bool DontUseEmbeddedBitmaps() const
        { return meEmbeddedBitmap == EMBEDDEDBITMAP_FALSE; }
    bool DontUseAntiAlias() const
        { return meAntiAlias == ANTIALIAS_FALSE; }
    bool DontUseHinting() const
        { return (meHinting == HINTING_FALSE) || (GetHintStyle() == HINT_NONE); }
    virtual void *GetPattern(void * /*pFace*/, bool /*bEmbolden*/, bool /*bVerticalMetrics*/) const
        { return NULL; }
};

// -------------------
// - ImplFontCharMap -
// -------------------

class CmapResult;

class VCL_PLUGIN_PUBLIC ImplFontCharMap
{
public:
    explicit             ImplFontCharMap( const CmapResult& );
    virtual              ~ImplFontCharMap();

    static ImplFontCharMap* GetDefaultMap( bool bSymbols=false);

    bool                IsDefaultMap() const;
    bool                HasChar( sal_uInt32 ) const;
    int                 CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const;
    int                 GetCharCount() const;

    sal_uInt32          GetFirstChar() const;
    sal_uInt32          GetLastChar() const;

    sal_uInt32          GetNextChar( sal_uInt32 ) const;
    sal_uInt32          GetPrevChar( sal_uInt32 ) const;

    int                 GetIndexFromChar( sal_uInt32 ) const;
    sal_uInt32          GetCharFromIndex( int ) const;

    void                AddReference() const;
    void                DeReference() const;

    int                 GetGlyphIndex( sal_uInt32 ) const;

private:
    int                 ImplFindRangeIndex( sal_uInt32 ) const;

    // prevent assignment and copy construction
    explicit            ImplFontCharMap( const ImplFontCharMap& );
    void                operator=( const ImplFontCharMap& );

private:
    const sal_uInt32*   mpRangeCodes;     // pairs of StartCode/(EndCode+1)
    const int*          mpStartGlyphs;    // range-specific mapper to glyphs
    const sal_uInt16*       mpGlyphIds;       // individual glyphid mappings
    int                 mnRangeCount;
    int                 mnCharCount;      // covered codepoints
    mutable FontRefCount    mnRefCount;
};

// CmapResult is a normalized version of the many CMAP formats
class VCL_PLUGIN_PUBLIC CmapResult
{
public:
    explicit    CmapResult( bool bSymbolic = false,
                    const sal_uInt32* pRangeCodes = NULL, int nRangeCount = 0,
                    const int* pStartGlyphs = 0, const sal_uInt16* pGlyphIds = NULL );

    const sal_uInt32* mpRangeCodes;
    const int*        mpStartGlyphs;
    const sal_uInt16*     mpGlyphIds;
    int               mnRangeCount;
    bool              mbSymbolic;
    bool              mbRecoded;
};

bool ParseCMAP( const unsigned char* pRawData, int nRawLength, CmapResult& );

void UpdateAttributesFromPSName( const OUString& rPSName, ImplDevFontAttributes& );

#endif // _SV_IMPFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
