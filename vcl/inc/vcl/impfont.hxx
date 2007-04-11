
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impfont.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:57:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_IMPFONT_HXX
#define _SV_IMPFONT_HXX

#ifndef _TOOLS_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif
#ifndef _VCL_FNTSTYLE_HXX
#include <vcl/fntstyle.hxx>
#endif

// ------------
// - Impl_Font -
// ------------

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

    int                 mnRefCount;
    String              maFamilyName;
    String              maStyleName;
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
    FontStrikeout       meStrikeout;
    FontRelief          meRelief;
    FontEmphasisMark    meEmphasisMark;
    FontType            meType;         // used by metrics only
    short               mnOrientation;
    FontKerning         mnKerning;
    BOOL                mbWordLine:1,
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
    USHORT  mnMiscFlags;   // Misc Flags
    UINT32  mnRefCount;    // Reference Counter

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

// -------------------
// - ImplFontCharMap -
// -------------------

class VCL_DLLPUBLIC ImplFontCharMap
{
public:
                        ImplFontCharMap( int nRangePairs,
                            const sal_uInt32* pRangeCodes,
                            const int* pStartGlyphs = NULL );

static ImplFontCharMap* GetDefaultMap();
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

    void                AddReference();
    void                DeReference();

    int                 GetGlyphIndex( sal_uInt32 );

private:
                        ~ImplFontCharMap();
    int                 ImplFindRangeIndex( sal_uInt32 ) const;

    // prevent assignment and copy construction
                        ImplFontCharMap( const ImplFontCharMap& );
    void                operator=( const ImplFontCharMap& );

private:
    const sal_uInt32*   mpRangeCodes;     // pairs of StartCode/(EndCode+1)
    const int*          mpStartGlyphs;    // index of the first glyph of the ranges
    int                 mnRangeCount;
    int                 mnCharCount;
    int                 mnRefCount;
};

// CmapResult is a normalized version of the many CMAP formats
struct CmapResult
{
    sal_uInt32* mpPairCodes;
    int*        mpStartGlyphs;
    int         mnPairCount;
    bool        mbRecoded;
    bool        mbSymbolic;
};

bool ParseCMAP( const unsigned char* pRawData, int nRawLength, CmapResult& );

#endif // _SV_IMPFONT_HXX
