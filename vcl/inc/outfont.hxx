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

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <i18nlangtag/lang.h>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <unotools/fontdefs.hxx>
#include <vcl/vclenum.hxx>

#include <unordered_map>

class PhysicalFontFace;
class ImplFontEntry;
class ImplFontCache;
namespace vcl { class Font; }
class ConvertChar;
class OutputDevice;
class Size;
class FontSelectPattern;

class ImplFontAttributes
{
public:
    explicit        ImplFontAttributes() {}
    explicit        ImplFontAttributes( const FontSelectPattern& );

    // device independent font functions
    const OUString& GetFamilyName() const   { return maFamilyName; }
    FontFamily      GetFamilyType() const   { return meFamily; }
    const OUString& GetStyleName() const    { return maStyleName; }

    FontWeight      GetWeight() const       { return meWeight; }
    FontItalic      GetSlantType() const    { return meItalic; }
    FontPitch       GetPitch() const        { return mePitch; }
    FontWidth       GetWidthType() const    { return meWidthType; }

    bool            IsSymbolFont() const    { return mbSymbolFlag; }

    void            SetFamilyName(const OUString& sFamilyName)  { maFamilyName = sFamilyName; }
    void            SetStyleName( const OUString& sStyleName)   { maStyleName = sStyleName; }
    void            SetFamilyType(const FontFamily eFontFamily) { meFamily = eFontFamily; }

    void            SetPitch(const FontPitch ePitch )          { mePitch = ePitch; }
    void            SetItalic(const FontItalic eItalic )       { meItalic = eItalic; }
    void            SetWeight(const FontWeight eWeight )       { meWeight = eWeight; }
    void            SetWidthType(const FontWidth eWidthType)   { meWidthType = eWidthType; }

    void            SetSymbolFlag(const bool bSymbolFlag )     { mbSymbolFlag = bSymbolFlag; }

    bool            CompareDeviceIndependentFontAttributes(const ImplFontAttributes& rOther) const;

    // Device dependent functions
    int             GetQuality() const      { return mnQuality; }
    OUString        GetMapNames() const     { return maMapNames; }

    bool            IsBuiltInFont() const   { return mbDevice; }
    bool            CanEmbed() const        { return mbEmbeddable; }
    bool            CanSubset() const       { return mbSubsettable; }
    bool            CanRotate() const       { return mbOrientation; }
    bool            HasMapNames() const     { return (maMapNames.getLength() > 0); }

    void            SetQuality( int nQuality )                  { mnQuality = nQuality; }
    void            IncreaseQualityBy( int nQualityAmount )     { mnQuality += nQualityAmount; }
    void            DecreaseQualityBy( int nQualityAmount )     { mnQuality -= nQualityAmount; }
    void            SetMapNames( OUString const & aMapNames ) { maMapNames = aMapNames; }
    void            AddMapName( OUString const & aMapName )
                    {
                        if( maMapNames.getLength() > 0 )
                        {
                            maMapNames += ";";
                        }

                        if (aMapName.getLength() == 0)
                        {
                            SAL_WARN("vcl.fonts", "New map name is empty");
                            return;
                        }

                        maMapNames += aMapName;
                    }

    void            SetBuiltInFontFlag( bool bIsBuiltInFont )   { mbDevice = bIsBuiltInFont; }
    void            SetEmbeddableFlag ( bool bEmbeddable )      { mbEmbeddable = bEmbeddable; }
    void            SetSubsettableFlag( bool bSubsettable )     { mbSubsettable = bSubsettable; }
    void            SetOrientationFlag( bool bCanRotate )       { mbOrientation = bCanRotate; }

    // Font metrics below

    // font instance attributes from the font request
    long            GetWidth()              { return mnWidth; }
    short           GetOrientation()        { return mnOrientation; }

    void            SetWidth(long nWidth)   { mnWidth=nWidth; }
    void            SetOrientation(short nOrientation) { mnOrientation=nOrientation; }

    // font metrics measured for the font instance
    long            GetAscent()             { return mnAscent; }
    long            GetDescent()            { return mnDescent; }
    long            GetInternalLeading()    { return mnIntLeading; }
    long            GetExternalLeading()    { return mnExtLeading; }
    int             GetSlant()              { return mnSlant; }
    long            GetMinKashida()         { return mnMinKashida; }

    void            SetAscent(long nAscent) { mnAscent=nAscent; }
    void            SetDescent(long nDescent) { mnDescent=nDescent; }
    void            SetInternalLeading(long nIntLeading) { mnIntLeading = nIntLeading; }
    void            SetExternalLeading(long nExtLeading) { mnExtLeading = nExtLeading; }
    void            SetSlant(int nSlant)    { mnSlant=nSlant; }
    void            SetMinKashida( long nMinKashida ) { mnMinKashida=nMinKashida; }

    // font attributes queried from the font instance
    bool            IsScalable()            { return mbScalableFont; }
    bool            IsTrueTypeFont()        { return mbTrueTypeFont; }
    bool            IsKernable()            { return mbKernableFont; }
    bool            IsFullstopCentered()    { return mbFullstopCentered; }
    long            GetBulletOffset()       { return mnBulletOffset; }

    void            SetScalableFlag(bool bScalable) { mbScalableFont = bScalable; }
    void            SetTrueTypeFlag(bool bTrueTypeFont) { mbTrueTypeFont = bTrueTypeFont; }
    void            SetKernableFlag(bool bKernable) { mbKernableFont = bKernable; }
    void            SetFullstopCenteredFlag(bool bFullstopCentered) { mbFullstopCentered = bFullstopCentered; }
    void            SetBulletOffset(long nBulletOffset) { mnBulletOffset = nBulletOffset; }

    // font metrics that are usually derived from the measurements
    long            GetUnderlineSize()                  { return mnUnderlineSize; }
    long            GetUnderlineOffset()                { return mnUnderlineOffset; }
    long            GetBoldUnderlineSize()              { return mnBUnderlineSize; }
    long            GetBoldUnderlineOffset()            { return mnBUnderlineOffset; }
    long            GetDoubleUnderlineSize()            { return mnDUnderlineSize; }
    long            GetDoubleUnderlineOffset1()         { return mnDUnderlineOffset1; }
    long            GetDoubleUnderlineOffset2()         { return mnDUnderlineOffset2; }
    long            GetWavelineUnderlineSize()          { return mnWUnderlineSize; }
    long            GetWavelineUnderlineOffset()        { return mnWUnderlineOffset; }
    long            GetAboveUnderlineSize()             { return mnAboveUnderlineSize; }
    long            GetAboveUnderlineOffset()           { return mnAboveUnderlineOffset; }
    long            GetAboveBoldUnderlineSize()         { return mnAboveBUnderlineSize; }
    long            GetAboveBoldUnderlineOffset()       { return mnAboveBUnderlineOffset; }
    long            GetAboveDoubleUnderlineSize()       { return mnAboveDUnderlineSize; }
    long            GetAboveDoubleUnderlineOffset1()    { return mnAboveDUnderlineOffset1; }
    long            GetAboveDoubleUnderlineOffset2()    { return mnAboveDUnderlineOffset2; }
    long            GetAboveWavelineUnderlineSize()     { return mnAboveWUnderlineSize; }
    long            GetAboveWavelineUnderlineOffset()   { return mnAboveWUnderlineOffset; }
    long            GetStrikeoutSize()                  { return mnStrikeoutSize; }
    long            GetStrikeoutOffset()                { return mnStrikeoutOffset; }
    long            GetBoldStrikeoutSize()              { return mnBStrikeoutSize; }
    long            GetBoldStrikeoutOffset()            { return mnBStrikeoutOffset; }
    long            GetDoubleStrikeoutSize()            { return mnDStrikeoutSize; }
    long            GetDoubleStrikeoutOffset1()         { return mnDStrikeoutOffset1; }
    long            GetDoubleStrikeoutOffset2()         { return mnDStrikeoutOffset2; }


    void            ImplInitTextLineSize( const OutputDevice* pDev );
    void            ImplInitAboveTextLineSize();

private:
    // device independent variables
    OUString        maFamilyName;               // Font Family Name
    OUString        maStyleName;                // Font Style Name
    FontWeight      meWeight;                   // Weight Type
    FontItalic      meItalic;                   // Slant Type
    FontFamily      meFamily;                   // Family Type
    FontPitch       mePitch;                    // Pitch Type
    FontWidth       meWidthType;                // Width Type
    bool            mbSymbolFlag;               // Is font a symbol?

    // device dependent variables
    OUString        maMapNames;                 // List of family name aliases separated with ';'
    int             mnQuality;                  // Quality (used when similar fonts compete)
    bool            mbOrientation;              // true: physical font can be rotated
    bool            mbDevice;                   // true: built in font
    bool            mbSubsettable;              // true: a subset of the font can be created
    bool            mbEmbeddable;               // true: the font can be embedded

    // Font metrics below

    // font instance attributes from the font request
    long            mnWidth;                    // Reference Width
    short           mnOrientation;              // Rotation in 1/10 degrees

    // font metrics measured for the font instance
    long            mnAscent;                   // Ascent
    long            mnDescent;                  // Descent
    long            mnIntLeading;               // Internal Leading
    long            mnExtLeading;               // External Leading
    int             mnSlant;                    // Slant (Italic/Oblique)
    long            mnMinKashida;               // Minimal width of kashida (Arabic)

    // font attributes queried from the font instance
    bool            mbScalableFont;
    bool            mbTrueTypeFont;
    bool            mbKernableFont;
    bool            mbFullstopCentered;
    long            mnBulletOffset;             // Offset to position non-print character

    // font metrics that are usually derived from the measurements
    long            mnUnderlineSize;            // Lineheight of Underline
    long            mnUnderlineOffset;          // Offset from Underline to Baseline
    long            mnBUnderlineSize;           // Height of bold underline
    long            mnBUnderlineOffset;         // Offset from bold underline to baseline
    long            mnDUnderlineSize;           // Height of double underline
    long            mnDUnderlineOffset1;        // Offset from double underline to baseline
    long            mnDUnderlineOffset2;        // Offset from double underline to baseline
    long            mnWUnderlineSize;           // Height of WaveLine underline
    long            mnWUnderlineOffset;         // Offset from WaveLine underline to baseline, but centrered to WaveLine
    long            mnAboveUnderlineSize;       // Height of single underline (for Vertical Right)
    long            mnAboveUnderlineOffset;     // Offset from single underline to baseline (for Vertical Right)
    long            mnAboveBUnderlineSize;      // Height of bold underline (for Vertical Right)
    long            mnAboveBUnderlineOffset;    // Offset from bold underline to baseline (for Vertical Right)
    long            mnAboveDUnderlineSize;      // Height of double underline (for Vertical Right)
    long            mnAboveDUnderlineOffset1;   // Offset from double underline to baseline (for Vertical Right)
    long            mnAboveDUnderlineOffset2;   // Offset from double underline to baseline (for Vertical Right)
    long            mnAboveWUnderlineSize;      // Height of WaveLine-strike-out (for Vertical Right)
    long            mnAboveWUnderlineOffset;    // Offset from WaveLine-strike-out to baseline, but centrered to the WaveLine (for Vertical Right)
    long            mnStrikeoutSize;            // Height of single strike-out
    long            mnStrikeoutOffset;          // Offset from single strike-out to baseline
    long            mnBStrikeoutSize;           // Height of bold strike-out
    long            mnBStrikeoutOffset;         // Offset of bold strike-out to baseline
    long            mnDStrikeoutSize;           // Height of double strike-out
    long            mnDStrikeoutOffset1;        // Offset of double strike-out to baseline
    long            mnDStrikeoutOffset2;        // Offset of double strike-out to baseline

};

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
