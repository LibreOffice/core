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

#ifndef INCLUDED_VCL_INC_OUTFONT_HXX
#define INCLUDED_VCL_INC_OUTFONT_HXX

#include <vcl/dllapi.h>
#include "fontattributes.hxx"

#include <unordered_map>

class PhysicalFontFace;
class ImplFontEntry;
class ImplFontCache;
namespace vcl { class Font; }
class ConvertChar;
class OutputDevice;
class Size;

class FontSelectPatternAttributes : public ImplFontAttributes
{
public:
                    FontSelectPatternAttributes( const vcl::Font&, const OUString& rSearchName,
                                                 const Size&, float fExactHeight );
#ifdef WNT
                    FontSelectPatternAttributes( const PhysicalFontFace&, const Size&,
                                                 float fExactHeight, int nOrientation, bool bVertical );
#endif

    size_t          hashCode() const;
    bool operator==(const FontSelectPatternAttributes& rOther) const;
    bool operator!=(const FontSelectPatternAttributes& rOther) const
    {
        return !(*this == rOther);
    }

public:
    OUString        maTargetName;               // name of the font name token that is chosen
    OUString        maSearchName;               // name of the font that matches best
    int             mnWidth;                    // width of font in pixel units
    int             mnHeight;                   // height of font in pixel units
    float           mfExactHeight;              // requested height (in pixels with subpixel details)
    int             mnOrientation;              // text orientation in 3600 system
    LanguageType    meLanguage;                 // text language
    bool            mbVertical;                 // vertical mode of requested font
    bool            mbNonAntialiased;           // true if antialiasing is disabled

    bool            mbEmbolden;                 // Force emboldening
    ItalicMatrix    maItalicMatrix;             // Force matrix for slant
};

class FontSelectPattern : public FontSelectPatternAttributes
{
public:
                    FontSelectPattern( const vcl::Font&, const OUString& rSearchName,
                                       const Size&, float fExactHeight );
#ifdef WNT
// ifdeffed to prevent it going into unusedcode.easy
                    FontSelectPattern( const PhysicalFontFace&, const Size&,
                                       float fExactHeight, int nOrientation, bool bVertical );
#endif

public: // TODO: change to private
    const PhysicalFontFace* mpFontData;         // a matching PhysicalFontFace object
    ImplFontEntry*  mpFontEntry;                // pointer to the resulting FontCache entry

    void            copyAttributes(const FontSelectPatternAttributes &rAttributes);
};

// - ImplFontEntry -

// TODO: rename ImplFontEntry to LogicalFontInstance
// TODO: allow sharing of metrics for related fonts

class VCL_PLUGIN_PUBLIC ImplFontEntry
{
public:
    explicit        ImplFontEntry( const FontSelectPattern& );
    virtual         ~ImplFontEntry();

public: // TODO: make data members private
    ImplFontCache * m_pFontCache;
    FontSelectPattern  maFontSelData;          // FontSelectionData
    ImplFontAttributes  maFontAttributes;       // Font attributes
    const ConvertChar* mpConversion;           // used e.g. for StarBats->StarSymbol

    long            mnLineHeight;
    sal_uLong       mnRefCount;
    sal_uInt16      mnSetFontFlags;         // Flags returned by SalGraphics::SetFont()
    short           mnOwnOrientation;       // text angle if lower layers don't rotate text themselves
    short           mnOrientation;          // text angle in 3600 system
    bool            mbInit;                 // true if maFontAttributes member is valid

    void            AddFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );
    bool            GetFallbackForUnicode( sal_UCS4, FontWeight eWeight, OUString* pFontName ) const;
    void            IgnoreFallbackForUnicode( sal_UCS4, FontWeight eWeight, const OUString& rFontName );

private:
    // cache of Unicode characters and replacement font names
    // TODO: a fallback map can be shared with many other ImplFontEntries
    // TODO: at least the ones which just differ in orientation, stretching or height
    typedef ::std::pair<sal_UCS4,FontWeight> GFBCacheKey;
    struct GFBCacheKey_Hash{ size_t operator()( const GFBCacheKey& ) const; };
    typedef ::std::unordered_map<GFBCacheKey,OUString,GFBCacheKey_Hash> UnicodeFallbackList;
    UnicodeFallbackList* mpUnicodeFallbackList;
};

class ImplTextLineInfo
{
private:
    long        mnWidth;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

public:
   ImplTextLineInfo( long nWidth, sal_Int32 nIndex, sal_Int32 nLen )
   {
       if(nIndex == -1 || nIndex == 0x0FFFF || nLen == -1 || nLen == 0x0FFFF)
       {
            SAL_INFO("sal.rtl.xub",
                     "ImplTextLine Info Suspicious arguments nIndex:" << nIndex << " nLen:" << nLen);
       }
       mnWidth = nWidth;
       mnIndex = nIndex;
       mnLen   = nLen;
   }

    long       GetWidth() const { return mnWidth; }
    sal_Int32  GetIndex() const { return mnIndex; }
    sal_Int32  GetLen() const { return mnLen; }
};

#define MULTITEXTLINEINFO_RESIZE    16

class ImplMultiTextLineInfo
{
public:
                ImplMultiTextLineInfo();
                ~ImplMultiTextLineInfo();

    void        AddLine( ImplTextLineInfo* pLine );
    void        Clear();

    ImplTextLineInfo* GetLine( sal_Int32 nLine ) const
                            { return mpLines[nLine]; }
    sal_Int32   Count() const { return mnLines; }

private:
    ImplMultiTextLineInfo( const ImplMultiTextLineInfo& ) = delete;
    ImplMultiTextLineInfo& operator=( const ImplMultiTextLineInfo& ) = delete;

    ImplTextLineInfo**  mpLines;
    sal_Int32   mnLines;
    sal_Int32   mnSize;

};

#endif // INCLUDED_VCL_INC_OUTFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
