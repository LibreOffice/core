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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWFONT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWFONT_HXX

#include <memory>
#include <i18nlangtag/lang.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <editeng/svxfont.hxx>
#include <swtypes.hxx>
#include "drawfont.hxx"
#include <editeng/borderline.hxx>
#include <boost/optional.hpp>
#include <o3tl/enumarray.hxx>

class SfxItemSet;
class SwAttrSet;
class SwDoCapitals;     // DoCapitals
class SwDrawTextInfo;   // DrawText_
class SwScriptInfo;     // GetTextSize_
class SwViewShell;
class IDocumentSettingAccess;
enum class SvxShadowItemSide;

const sal_Unicode CH_BLANK = ' ';   // ' ' blank spaces
const sal_Unicode CH_BREAK = 0x0A;
const sal_Unicode CH_TAB   = '\t';  // \t
const sal_Unicode CH_PAR    = 0xB6;     // paragraph
const sal_Unicode CH_BULLET = 0xB7;     // centered dot

sal_uInt16 UnMapDirection( sal_uInt16 nDir, const bool bVertFormat, const bool bVertFormatLRBT );

class SwSubFont : public SvxFont
{
    friend class SwFont;
    const void* m_nFontCacheId;     // "MagicNumber" within the font cache
    Size        m_aSize;          // foreigners only see this size
    sal_uInt16      m_nFontIndex;      // index in the font cache
    sal_uInt16      m_nOrgHeight;     // height including escapement/proportion
    sal_uInt16      m_nOrgAscent;     // ascent including escapement/proportion
    sal_uInt16      m_nProportionalWidth;     // proportional width
    bool m_bSmallCapsPercentage66;

    sal_uInt16 CalcEscAscent( const sal_uInt16 nOldAscent ) const;
    sal_uInt16 CalcEscHeight( const sal_uInt16 nOldHeight,
                          const sal_uInt16 nOldAscent ) const;
    void CalcEsc( SwDrawTextInfo const & rInf, Point& rPos );

    short CheckKerning_( );

    bool ChgFnt( SwViewShell const *pSh, OutputDevice& rOut );
    bool IsSymbol( SwViewShell const *pSh );
    sal_uInt16 GetAscent( SwViewShell const *pSh, const OutputDevice& rOut );
    sal_uInt16 GetHeight( SwViewShell const *pSh, const OutputDevice& rOut );
    Size GetTextSize_( SwDrawTextInfo& rInf );
    Size GetCapitalSize( SwDrawTextInfo& rInf );
    void DrawText_( SwDrawTextInfo &rInf, const bool bGrey );
    void DrawCapital( SwDrawTextInfo &rInf );
    void DrawStretchCapital( SwDrawTextInfo &rInf );
    void DoOnCapitals( SwDoCapitals &rDo );
    void DrawStretchText_( SwDrawTextInfo &rInf );
    TextFrameIndex GetCursorOfst_( SwDrawTextInfo& rInf );
    TextFrameIndex GetCapitalCursorOfst( SwDrawTextInfo& rInf );

    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetCharSet( const rtl_TextEncoding eCharSet );
    inline void SetPitch( const FontPitch ePitch );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontLineStyle eUnderline );
    inline void SetOverline( const FontLineStyle eOverline );
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetItalic( const FontItalic eItalic );
    inline void SetOutline( const bool bOutline );
    inline void SetVertical( const sal_uInt16 nDir, const bool bVertFormat );
    inline void SetShadow( const bool bShadow );
    inline void SetAutoKern( FontKerning nAutoKern );
    inline void SetWordLineMode( const bool bWordLineMode );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );
    inline void SetRelief( const FontRelief eNew );

    // methods for sub-/superscript
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const sal_uInt8 nNewPropr );

    inline void SetFamily( const FontFamily eFamily );
    inline void SetName( const OUString& rName );
    inline void SetStyleName( const OUString& rStyleName );
    inline void SetSize( const Size& rSize );
    inline void SetWeight( const FontWeight eWeight );
    inline void SetLanguage( LanguageType eNewLang );
    short CheckKerning()
    {   return GetFixKerning() >= 0 ? GetFixKerning() : CheckKerning_( ); }
    void SetPropWidth( const sal_uInt16 nNew )
        { m_nFontCacheId = nullptr; m_nProportionalWidth = nNew; }
public:
    SwSubFont() : m_aSize(0,0)
    { m_nFontCacheId = nullptr; m_nFontIndex = m_nOrgHeight = m_nOrgAscent = 0; m_nProportionalWidth =100; m_bSmallCapsPercentage66 = false; }
    sal_uInt16 GetPropWidth() const { return m_nProportionalWidth; }
};

enum class SwFontScript
{
    Latin, CJK, CTL, LAST = CTL
};

// mostly used as a "unknown script" marker
#define SW_SCRIPTS (SwFontScript(int(SwFontScript::LAST)+1))

class SW_DLLPUBLIC SwFont
{
    // CJK == Chinese, Japanese, Korean
    // CTL == Complex text layout ( Hebrew, Arabic )
    o3tl::enumarray<SwFontScript, SwSubFont>   m_aSub; // Latin-, CJK- and CTL-font

    std::unique_ptr<Color>
                m_pBackColor;     // background color (i.e. at character styles)
    Color       m_aHighlightColor;   // highlight color
    Color       m_aUnderColor;    // color of the underlining
    Color       m_aOverColor;     // color of the overlining

    // character borders
    boost::optional<editeng::SvxBorderLine>     m_aTopBorder;
    boost::optional<editeng::SvxBorderLine>     m_aBottomBorder;
    boost::optional<editeng::SvxBorderLine>     m_aRightBorder;
    boost::optional<editeng::SvxBorderLine>     m_aLeftBorder;

    // border distance
    sal_uInt16 m_nTopBorderDist;
    sal_uInt16 m_nBottomBorderDist;
    sal_uInt16 m_nRightBorderDist;
    sal_uInt16 m_nLeftBorderDist;

    Color               m_aShadowColor;
    sal_uInt16          m_nShadowWidth;
    SvxShadowLocation   m_aShadowLocation;

    sal_uInt8   m_nToxCount;        // counts the nesting depth of the Tox
    sal_uInt8   m_nRefCount;        // counts the nesting depth of the Refs
    sal_uInt8   m_nMetaCount;   // count META/METAFIELD
    sal_uInt8   m_nInputFieldCount; // count INPUTFIELD

    SwFontScript m_nActual;        // actual font (Latin, CJK or CTL)

    // switch for the font-extensions
    bool m_bBlink         :1;  // blinking font
    bool m_bPaintBlank    :1;  // blanks not with DrawRect
    bool m_bFontChg       :1;
    bool m_bOrgChg        :1;  // nOrgHeight/Ascent are invalid
    bool m_bGreyWave      :1;  // for the extended TextInput: gray waveline

public:
    SwFont( const SwAttrSet* pSet, const IDocumentSettingAccess* pIDocumentSettingAccess );
    SwFont( const SwFont& rFont );

    void ChgFnt( SwViewShell const *pSh, OutputDevice& rOut )
        { m_bPaintBlank = m_aSub[m_nActual].ChgFnt( pSh, rOut ); }

    ~SwFont();

    SwFont& operator=( const SwFont &rFont );

    SwFontScript GetActual() const {    return m_nActual; }
    inline void SetActual( SwFontScript nNew );
    const SvxFont& GetActualFont() const { return m_aSub[m_nActual]; }

    // gets a font cache id via SwFntAccess
    void AllocFontCacheId( SwViewShell const *pSh, SwFontScript nWhich );
    // set background color
    void SetBackColor( Color* pNewColor );
    const Color* GetBackColor() const{ return m_pBackColor.get(); }
    void SetHighlightColor( const Color& aNewColor );
    const Color& GetHighlightColor() const { return m_aHighlightColor; }

    void CheckFontCacheId( SwViewShell const *pSh, SwFontScript nWhich )
        { if( !m_aSub[ nWhich ].m_nFontCacheId ) AllocFontCacheId( pSh, nWhich ); }
    void GetFontCacheId( const void* &rnFontCacheId, sal_uInt16 &rIdx, SwFontScript nWhich )
        { rnFontCacheId = m_aSub[nWhich].m_nFontCacheId; rIdx = m_aSub[nWhich].m_nFontIndex; }
    void SetFontCacheId( const void* nNewFontCacheId, const sal_uInt16 nIdx, SwFontScript nWhich )
        { m_aSub[nWhich].m_nFontCacheId = nNewFontCacheId; m_aSub[nWhich].m_nFontIndex = nIdx; }
    bool DifferentFontCacheId( const SwFont* pFnt, SwFontScript nWhich )
        { return m_aSub[nWhich].m_nFontCacheId != pFnt->m_aSub[nWhich].m_nFontCacheId ||
          !m_aSub[nWhich].m_nFontCacheId || !pFnt->m_aSub[nWhich].m_nFontCacheId; }

    const Size &GetSize( SwFontScript nWhich ) const
        { return m_aSub[nWhich].m_aSize; }
    bool IsFntChg() const { return m_bFontChg; }
    void SetFntChg( const bool bNew ) { m_bFontChg = bNew; }

    // the encapsulated SV-Font-methods (set bFntChg to true)
    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontLineStyle eUnderline );
    void SetUnderColor( const Color &rColor ) { m_aUnderColor = rColor; }
    inline void SetOverline( const FontLineStyle eOverline );
    void SetOverColor( const Color &rColor ) { m_aOverColor = rColor; }
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetOutline( const bool bOutline );
    void SetVertical(sal_uInt16 nDir, const bool bVertLayout = false,
                     const bool bVertLayoutLRBT = false);
    inline void SetShadow( const bool bShadow );
    inline void SetAutoKern( FontKerning nAutoKern );
    inline void SetTransparent( const bool bTrans );
    inline void SetWordLineMode( const bool bWordLineMode );
    inline void SetFixKerning( const short nNewKern );
    inline void SetCaseMap( const SvxCaseMap eNew );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );

    // methods for sub-/superscript
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const sal_uInt8 nNewPropr );

    inline void SetPropWidth( const sal_uInt16 nNew );

    inline void SetFamily( const FontFamily eFamily, const SwFontScript nWhich );
    inline void SetName( const OUString& rName, const SwFontScript nWhich );
    inline void SetStyleName( const OUString& rStyleName, const SwFontScript nWhich );
    inline void SetSize( const Size& rSize, const SwFontScript nWhich );
    inline void SetWeight( const FontWeight eWeight, const SwFontScript nWhich );
    inline void SetItalic( const FontItalic eItalic, const SwFontScript nWhich );
    inline void SetLanguage( LanguageType eNewLang, const SwFontScript nWhich );
    inline void SetCharSet( const rtl_TextEncoding eCharSet, const SwFontScript nWhich );
    inline void SetPitch( const FontPitch ePitch, const SwFontScript nWhich );
    inline void SetRelief( const FontRelief eNew );

    // Get/Set-methods for the current setting
    inline void SetBlink( const bool bBlink );
    bool IsBlink() const { return m_bBlink; }
    sal_uInt8 &GetTox() { return m_nToxCount; }
    bool IsTox() const { return ( 0 != m_nToxCount ); }
    sal_uInt8 &GetRef() { return m_nRefCount; }
    bool IsRef() const { return ( 0 != m_nRefCount ); }
    sal_uInt8 &GetMeta() { return m_nMetaCount; }
    bool IsMeta() const { return (0 != m_nMetaCount); }
    sal_uInt8 &GetInputField() { return m_nInputFieldCount; }
    bool IsInputField() const { return (0 != m_nInputFieldCount); }
    inline void SetGreyWave( const bool bNew );
    bool IsGreyWave() const { return m_bGreyWave; }
    bool IsPaintBlank() const { return m_bPaintBlank; }

    // setting of the base class font for SwTextCharFormat
    void SetDiffFnt( const SfxItemSet* pSet,
                     const IDocumentSettingAccess* pIDocumentSettingAccess );

    const SvxFont &GetFnt( const SwFontScript nWhich ) const
        { return m_aSub[nWhich]; };

    bool IsSymbol( SwViewShell const *pSh )
        { return m_aSub[m_nActual].IsSymbol( pSh ); }
    FontLineStyle GetUnderline() const { return m_aSub[m_nActual].GetUnderline(); }
    const Color& GetUnderColor() const { return m_aUnderColor; }
    FontLineStyle GetOverline() const { return m_aSub[m_nActual].GetOverline(); }
    const Color& GetOverColor() const { return m_aOverColor; }
    FontStrikeout GetStrikeout() const { return m_aSub[m_nActual].GetStrikeout(); }
    const Color& GetColor() const { return m_aSub[m_nActual].GetColor(); }
    bool IsWordLineMode() const { return m_aSub[m_nActual].IsWordLineMode(); }
    short GetEscapement() const { return m_aSub[m_nActual].GetEscapement(); }
    SvxCaseMap GetCaseMap() const { return m_aSub[m_nActual].GetCaseMap(); }
    sal_uInt8 GetPropr() const { return m_aSub[m_nActual].GetPropr(); }
    FontItalic GetItalic() const { return m_aSub[m_nActual].GetItalic(); }
    LanguageType GetLanguage() const { return m_aSub[m_nActual].GetLanguage(); }
    long GetHeight() const { return m_aSub[m_nActual].GetFontSize().Height(); }
    FontWeight GetWeight() const { return m_aSub[m_nActual].GetWeight(); }
    FontEmphasisMark GetEmphasisMark() const
        { return m_aSub[m_nActual].GetEmphasisMark(); }
    sal_uInt16 GetOrientation(const bool bVertLayout = false,
                              const bool bVertFormatLRBT = false) const;

    const OUString& GetName( const SwFontScript nWhich ) const
        { return m_aSub[nWhich].GetFamilyName(); }
    LanguageType GetLanguage( const SwFontScript nWhich ) const
        { return m_aSub[nWhich].GetLanguage(); }
    rtl_TextEncoding GetCharSet( const SwFontScript nWhich ) const
        { return m_aSub[nWhich].GetCharSet(); }
    long GetHeight( const SwFontScript nWhich ) const
        { return m_aSub[nWhich].GetFontSize().Height(); }

    // makes the logical font be effective in the OutputDevice
    void ChgPhysFnt( SwViewShell const *pSh, OutputDevice& rOut );

    TextFrameIndex GetCapitalBreak( SwViewShell const* pSh,  const OutputDevice* pOut,
        const SwScriptInfo* pScript, const OUString& rText,
        long nTextWidth, TextFrameIndex nIdx, TextFrameIndex nLen);

    void DoOnCapitals( SwDoCapitals &rDo )
        { m_aSub[m_nActual].DoOnCapitals( rDo ); }

    Size GetTextSize_( SwDrawTextInfo& rInf )
        { rInf.SetFont( this ); return m_aSub[m_nActual].GetTextSize_( rInf ); }

    TextFrameIndex GetTextBreak( SwDrawTextInfo const & rInf, long nTextWidth );

    TextFrameIndex GetCursorOfst_( SwDrawTextInfo& rInf )
        { return m_aSub[m_nActual].GetCursorOfst_( rInf ); }

    void DrawText_( SwDrawTextInfo &rInf )
        { m_aSub[m_nActual].DrawText_( rInf, IsGreyWave() ); }

    void DrawStretchText_( SwDrawTextInfo &rInf )
        { m_aSub[m_nActual].DrawStretchText_( rInf ); }

    short CheckKerning()
        { return m_aSub[m_nActual].CheckKerning(); }

    sal_uInt16 GetAscent( SwViewShell const *pSh, const OutputDevice& rOut )
        { return m_aSub[m_nActual].GetAscent( pSh, rOut ); }
    sal_uInt16 GetHeight( SwViewShell const *pSh, const OutputDevice& rOut )
        { return m_aSub[m_nActual].GetHeight( pSh, rOut ); }

    void Invalidate()
        { m_bFontChg = m_bOrgChg = true; }

    void SetTopBorder( const editeng::SvxBorderLine* pTopBorder );
    void SetBottomBorder( const editeng::SvxBorderLine* pBottomBorder );
    void SetRightBorder( const editeng::SvxBorderLine* pRightBorder );
    void SetLeftBorder( const editeng::SvxBorderLine* pLeftBorder );

    const boost::optional<editeng::SvxBorderLine>& GetTopBorder() const { return m_aTopBorder; }
    const boost::optional<editeng::SvxBorderLine>& GetBottomBorder() const { return m_aBottomBorder; }
    const boost::optional<editeng::SvxBorderLine>& GetRightBorder() const { return m_aRightBorder; }
    const boost::optional<editeng::SvxBorderLine>& GetLeftBorder() const { return m_aLeftBorder; }

    // Get absolute border correspond to the layout verticality and orientation.
    const boost::optional<editeng::SvxBorderLine>&
    GetAbsTopBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const;
    const boost::optional<editeng::SvxBorderLine>&
    GetAbsBottomBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const;
    const boost::optional<editeng::SvxBorderLine>&
    GetAbsRightBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const;
    const boost::optional<editeng::SvxBorderLine>&
    GetAbsLeftBorder(const bool bVertLayout, const bool bVertLayoutLRBT) const;

    void SetTopBorderDist( const sal_uInt16 nTopDist );
    void SetBottomBorderDist( const sal_uInt16 nBottomDist );
    void SetRightBorderDist( const sal_uInt16 nRightDist );
    void SetLeftBorderDist( const sal_uInt16 nLeftDist );

    sal_uInt16 GetTopBorderDist() const { return m_nTopBorderDist; }
    sal_uInt16 GetBottomBorderDist() const { return m_nBottomBorderDist; }
    sal_uInt16 GetRightBorderDist() const { return m_nRightBorderDist; }
    sal_uInt16 GetLeftBorderDist() const { return m_nLeftBorderDist; }

    // Return with the whole space which border holed (border width, spacing and shadow width)
    sal_uInt16 GetTopBorderSpace() const;
    sal_uInt16 GetBottomBorderSpace() const;
    sal_uInt16 GetRightBorderSpace() const;
    sal_uInt16 GetLeftBorderSpace() const;

    /// Check whether font has any border on any side
    bool HasBorder() const;

    void SetShadowColor( const Color& rColor );
    void SetShadowWidth( const sal_uInt16 nWidth );
    void SetShadowLocation( const SvxShadowLocation aLocation );

    const Color&        GetShadowColor() const { return m_aShadowColor; }
    sal_uInt16          GetShadowWidth() const { return m_nShadowWidth; }
    SvxShadowLocation   GetShadowLocation() const { return m_aShadowLocation; }

    /**
     * Get the absolute shadow location dependent from orientation.
     *
     * @param[in]   bVertLayout true, if the container layout is vertical
     *                          false, otherwise
     * @param[in]   bVertLayoutLRBT true if the container layout is vertical
     *                          (bottom to top, left to right), false otherwise
     * @return      absolute location
    **/
    SvxShadowLocation GetAbsShadowLocation(const bool bVertLayout,
                                           const bool bVertLayoutLRBT) const;

    /**
     * Calculate the shadow space on the specified side dependent from
     * the orientation and connection with neighbours.
     * @see shaditem.hxx for integer constants of sides
     *
     * @param[in]   nShadow     specify the side
     * @param[in]   bVertLayout true, if the container layout is vertical
     *                          false, otherwise
     * @param[in]   bVertLayoutLRBT true if the container layout is vertical
     *                          (bottom to top, left to right), false otherwise
     * @param[in]   bSkipLeft   relative left shadow space is skipped
     * @param[in]   bSkipRight  relative right shadow space is skipped
     * @return      the shadow space
    **/
    sal_uInt16 CalcShadowSpace(
        const SvxShadowItemSide nShadow, const bool bVertLayout, const bool bVertLayoutLRBT,
        const bool bSkipLeft, const bool bSkipRight ) const;

    void dumpAsXml( xmlTextWriterPtr writer ) const;
};

inline void SwFont::SetColor( const Color& rColor )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetColor( rColor );
    m_aSub[SwFontScript::CJK].SetColor( rColor );
    m_aSub[SwFontScript::CTL].SetColor( rColor );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetColor( const Color& rColor )
{
    m_nFontCacheId = nullptr;
    Font::SetColor( rColor );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetFillColor( const Color& rColor )
{
    m_nFontCacheId = nullptr;
    Font::SetFillColor( rColor );
}

inline void SwFont::SetFillColor( const Color& rColor )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetFillColor( rColor );
    m_aSub[SwFontScript::CJK].SetFillColor( rColor );
    m_aSub[SwFontScript::CTL].SetFillColor( rColor );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetFamily( const FontFamily eFamily )
{
    m_nFontCacheId = nullptr;
    Font::SetFamily( eFamily );
}

inline void SwFont::SetFamily( const FontFamily eFamily, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetFamily( eFamily );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetName( const OUString& rName )
{
    m_nFontCacheId = nullptr;
    Font::SetFamilyName( rName );
}

inline void SwFont::SetName( const OUString& rName, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetName( rName );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetStyleName( const OUString& rStyleName )
{
    m_nFontCacheId = nullptr;
    Font::SetStyleName( rStyleName );
}

inline void SwFont::SetStyleName( const OUString& rStyle, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetStyleName( rStyle );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetCharSet( const rtl_TextEncoding eCharSet )
{
    m_nFontCacheId = nullptr;
    Font::SetCharSet( eCharSet );
}

inline void SwFont::SetCharSet( const rtl_TextEncoding eCharSet, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetCharSet( eCharSet );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetPitch( const FontPitch ePitch )
{
    m_nFontCacheId = nullptr;
    Font::SetPitch( ePitch );
}

// encapsulated SV-Font-method
inline void SwFont::SetPitch( const FontPitch ePitch, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetPitch( ePitch );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetAlign( const FontAlign eAlign )
{
    m_nFontCacheId = nullptr;
    Font::SetAlignment( eAlign );
}

inline void SwFont::SetAlign( const FontAlign eAlign )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetAlign( eAlign );
    m_aSub[SwFontScript::CJK].SetAlign( eAlign );
    m_aSub[SwFontScript::CTL].SetAlign( eAlign );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetWeight( const FontWeight eWeight )
{
    m_nFontCacheId = nullptr;
    Font::SetWeight( eWeight );
}

inline void SwFont::SetWeight( const FontWeight eWeight, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetWeight( eWeight );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetUnderline( const FontLineStyle eUnderline )
{
    m_nFontCacheId = nullptr;
    Font::SetUnderline( eUnderline );
}

inline void SwFont::SetUnderline( const FontLineStyle eUnderline )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetUnderline( eUnderline );
    m_aSub[SwFontScript::CJK].SetUnderline( eUnderline );
    m_aSub[SwFontScript::CTL].SetUnderline( eUnderline );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetOverline( const FontLineStyle eOverline )
{
    m_nFontCacheId = nullptr;
    Font::SetOverline( eOverline );
}

inline void SwFont::SetOverline( const FontLineStyle eOverline )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetOverline( eOverline );
    m_aSub[SwFontScript::CJK].SetOverline( eOverline );
    m_aSub[SwFontScript::CTL].SetOverline( eOverline );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    m_nFontCacheId = nullptr;
    Font::SetStrikeout( eStrikeout );
}

inline void SwFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetStrikeout( eStrikeout );
    m_aSub[SwFontScript::CJK].SetStrikeout( eStrikeout );
    m_aSub[SwFontScript::CTL].SetStrikeout( eStrikeout );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetItalic( const FontItalic eItalic )
{
    m_nFontCacheId = nullptr;
    Font::SetItalic( eItalic );
}

inline void SwFont::SetItalic( const FontItalic eItalic, const SwFontScript nWhich )
{
    m_bFontChg = true;
    m_aSub[nWhich].SetItalic( eItalic );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetOutline( const bool bOutline )
{
    m_nFontCacheId = nullptr;
    Font::SetOutline( bOutline );
}

inline void SwFont::SetOutline( const bool bOutline )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetOutline( bOutline );
    m_aSub[SwFontScript::CJK].SetOutline( bOutline );
    m_aSub[SwFontScript::CTL].SetOutline( bOutline );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetShadow( const bool bShadow )
{
    m_nFontCacheId = nullptr;
    Font::SetShadow( bShadow );
}

inline void SwFont::SetShadow( const bool bShadow )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetShadow( bShadow );
    m_aSub[SwFontScript::CJK].SetShadow( bShadow );
    m_aSub[SwFontScript::CTL].SetShadow( bShadow );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetAutoKern( FontKerning nAutoKern )
{
    m_nFontCacheId = nullptr;
    Font::SetKerning( nAutoKern );
}

inline void SwFont::SetAutoKern( FontKerning nAutoKern )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::CJK].SetAutoKern( nAutoKern );
    if( nAutoKern != FontKerning::NONE )
        nAutoKern = FontKerning::FontSpecific;
    m_aSub[SwFontScript::Latin].SetAutoKern( nAutoKern );
    m_aSub[SwFontScript::CTL].SetAutoKern( nAutoKern );
}

inline void SwFont::SetTransparent( const bool bTrans )
{
    m_aSub[SwFontScript::Latin].SetTransparent( bTrans );
    m_aSub[SwFontScript::CJK].SetTransparent( bTrans );
    m_aSub[SwFontScript::CTL].SetTransparent( bTrans );
}

inline void SwFont::SetFixKerning( const short nNewKern )
{
    m_aSub[SwFontScript::Latin].SetFixKerning( nNewKern );
    m_aSub[SwFontScript::CJK].SetFixKerning( nNewKern );
    m_aSub[SwFontScript::CTL].SetFixKerning( nNewKern );
}

inline void SwFont::SetCaseMap( const SvxCaseMap eNew )
{
    m_aSub[SwFontScript::Latin].SetCaseMap( eNew );
    m_aSub[SwFontScript::CJK].SetCaseMap( eNew );
    m_aSub[SwFontScript::CTL].SetCaseMap( eNew );
}

// encapsulated SV-Font-method
inline void SwSubFont::SetWordLineMode( const bool bWordLineMode )
{
    m_nFontCacheId = nullptr;
    Font::SetWordLineMode( bWordLineMode );
}

inline void SwFont::SetWordLineMode( const bool bWordLineMode )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetWordLineMode( bWordLineMode );
    m_aSub[SwFontScript::CJK].SetWordLineMode( bWordLineMode );
    m_aSub[SwFontScript::CTL].SetWordLineMode( bWordLineMode );
}
// encapsulated SV-Font-method
inline void SwSubFont::SetEmphasisMark( const FontEmphasisMark eValue )
{
    m_nFontCacheId = nullptr;
    Font::SetEmphasisMark( eValue );
}

inline void SwFont::SetEmphasisMark( const FontEmphasisMark eValue )
{
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].SetEmphasisMark( eValue );
    m_aSub[SwFontScript::CJK].SetEmphasisMark( eValue );
    m_aSub[SwFontScript::CTL].SetEmphasisMark( eValue );
}

inline void SwFont::SetPropWidth( const sal_uInt16 nNew )
{
    if( nNew != m_aSub[SwFontScript::Latin].GetPropWidth() )
    {
        m_bFontChg = true;
        m_aSub[SwFontScript::Latin].SetPropWidth( nNew );
        m_aSub[SwFontScript::CJK].SetPropWidth( nNew );
        m_aSub[SwFontScript::CTL].SetPropWidth( nNew );
    }
}

// encapsulated SV-Font-method
inline void SwSubFont::SetRelief( const FontRelief eNew )
{
    m_nFontCacheId = nullptr;
    Font::SetRelief( eNew );
}

inline void SwFont::SetRelief( const FontRelief eNew )
{
    if( eNew != m_aSub[SwFontScript::Latin].GetRelief() )
    {
        m_bFontChg = true;
        m_aSub[SwFontScript::Latin].SetRelief( eNew );
        m_aSub[SwFontScript::CJK].SetRelief( eNew );
        m_aSub[SwFontScript::CTL].SetRelief( eNew );
    }
}

// overloaded font-method
inline void SwSubFont::SetSize( const Size& rSize )
{
    m_aSize = rSize;
    if ( GetPropr() == 100 )
        Font::SetFontSize( m_aSize );
    else
    {
        Font::SetFontSize( Size(
            m_aSize.Width() * GetPropr() / 100,
            m_aSize.Height() * GetPropr() / 100 ) );
    }
    m_nFontCacheId = nullptr;
}

inline void SwFont::SetSize( const Size& rSize, const SwFontScript nWhich )
{
    if( m_aSub[nWhich].m_aSize != rSize )
    {
        m_aSub[nWhich].SetSize( rSize );
        m_bFontChg = true;
        m_bOrgChg = true;
    }
}

inline void SwFont::SetActual( SwFontScript nNew )
{
     if ( m_nActual != nNew )
     {
        m_bFontChg = true;
        m_bOrgChg = true;
        m_nActual = nNew;
     }
}

inline void SwSubFont::SetProportion( const sal_uInt8 nNewPropr )
{
    m_nFontCacheId = nullptr;
    Font::SetFontSize( Size( m_aSize.Width() * nNewPropr / 100,
                             m_aSize.Height() * nNewPropr / 100 ) );
    SvxFont::SetPropr( nNewPropr );
}

inline void SwFont::SetProportion( const sal_uInt8 nNewPropr )
{
    if( nNewPropr != m_aSub[SwFontScript::Latin].GetPropr() )
    {
        m_bFontChg = true;
        m_bOrgChg = true;

        m_aSub[SwFontScript::Latin].SetProportion( nNewPropr );
        m_aSub[SwFontScript::CJK].SetProportion( nNewPropr );
        m_aSub[SwFontScript::CTL].SetProportion( nNewPropr );
    }
}

inline void SwSubFont::SetEscapement( const short nNewEsc )
{
    m_nFontCacheId = nullptr;
    SvxFont::SetEscapement( nNewEsc );
}

inline void SwFont::SetEscapement( const short nNewEsc )
{
    if( nNewEsc != m_aSub[SwFontScript::Latin].GetEscapement() )
    {
        // these have to be set, otherwise nOrgHeight and nOrgAscent will not
        // be calculated
        m_bFontChg = true;
        m_bOrgChg = true;

        m_aSub[SwFontScript::Latin].SetEscapement( nNewEsc );
        m_aSub[SwFontScript::CJK].SetEscapement( nNewEsc );
        m_aSub[SwFontScript::CTL].SetEscapement( nNewEsc );
    }
}

inline void SwSubFont::SetLanguage( LanguageType eNewLang )
{
    m_nFontCacheId = nullptr;
    if( eNewLang == LANGUAGE_SYSTEM )
        eNewLang = GetAppLanguage();
    SvxFont::SetLanguage( eNewLang );
}

inline void SwFont::SetLanguage( const LanguageType eNewLang, const SwFontScript nWhich )
{
    m_aSub[nWhich].SetLanguage( eNewLang );
    if( SwFontScript::CJK == nWhich )
    {
        m_aSub[SwFontScript::Latin].SetCJKContextLanguage( eNewLang );
        m_aSub[SwFontScript::CJK].SetCJKContextLanguage( eNewLang );
        m_aSub[SwFontScript::CTL].SetCJKContextLanguage( eNewLang );
    }
}

inline void SwFont::SetBlink( const bool bNew )
{
    m_bBlink = bNew;
}

inline void SwFont::SetGreyWave( const bool bNew )
{
    m_bGreyWave = bNew;
}

inline void SwSubFont::SetVertical( const sal_uInt16 nDir, const bool bVertFormat )
{
    m_nFontCacheId = nullptr;
    Font::SetVertical( bVertFormat );
    Font::SetOrientation( nDir );
}

inline void SwFont::SetTopBorderDist( const sal_uInt16 nTopDist )
{
    m_nTopBorderDist = nTopDist;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetBottomBorderDist( const sal_uInt16 nBottomDist )
{
    m_nBottomBorderDist = nBottomDist;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetRightBorderDist( const sal_uInt16 nRightDist )
{
    m_nRightBorderDist = nRightDist;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetLeftBorderDist( const sal_uInt16 nLeftDist )
{
    m_nLeftBorderDist = nLeftDist;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline sal_uInt16 SwFont::GetTopBorderSpace() const
{
    sal_uInt16 nRet = 0;
    if( m_aTopBorder )
    {
        nRet += m_aTopBorder.get().GetScaledWidth() + m_nTopBorderDist;
    }
    if( m_aShadowLocation == SvxShadowLocation::TopLeft ||
        m_aShadowLocation == SvxShadowLocation::TopRight )
    {
        nRet += m_nShadowWidth;
    }
    return nRet;
}

inline sal_uInt16 SwFont::GetBottomBorderSpace() const
{
    sal_uInt16 nRet = 0;
    if( m_aBottomBorder )
    {
        nRet += m_aBottomBorder.get().GetScaledWidth() + m_nBottomBorderDist;
    }
    if( m_aShadowLocation == SvxShadowLocation::BottomLeft ||
        m_aShadowLocation == SvxShadowLocation::BottomRight )
    {
        nRet += m_nShadowWidth;
    }
    return nRet;
}

inline sal_uInt16 SwFont::GetRightBorderSpace() const
{
    sal_uInt16 nRet = 0;
    if( m_aRightBorder )
    {
        nRet += m_aRightBorder.get().GetScaledWidth() + m_nRightBorderDist;
    }
    if( m_aShadowLocation == SvxShadowLocation::TopRight ||
        m_aShadowLocation == SvxShadowLocation::BottomRight )
    {
        nRet += m_nShadowWidth;
    }
    return nRet;
}

inline sal_uInt16 SwFont::GetLeftBorderSpace() const
{
    sal_uInt16 nRet = 0;
    if( m_aLeftBorder )
    {
        nRet += m_aLeftBorder.get().GetScaledWidth() + m_nLeftBorderDist;
    }
    if( m_aShadowLocation == SvxShadowLocation::TopLeft ||
        m_aShadowLocation == SvxShadowLocation::BottomLeft )
    {
        nRet += m_nShadowWidth;
    }
    return nRet;
}

inline bool SwFont::HasBorder() const
{
    return m_aTopBorder || m_aBottomBorder || m_aLeftBorder || m_aRightBorder;
}

inline void SwFont::SetShadowColor( const Color& rColor )
{
    m_aShadowColor = rColor;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetShadowWidth( const sal_uInt16 nWidth )
{
    m_nShadowWidth = nWidth;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetShadowLocation( const SvxShadowLocation aLocation )
{
    m_aShadowLocation = aLocation;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

inline void SwFont::SetHighlightColor( const Color& aNewColor )
{
    m_aHighlightColor = aNewColor;
    m_bFontChg = true;
    m_aSub[SwFontScript::Latin].m_nFontCacheId = m_aSub[SwFontScript::CJK].m_nFontCacheId = m_aSub[SwFontScript::CTL].m_nFontCacheId = nullptr;
}

// Used for the "continuous underline" feature.
class SwUnderlineFont
{
    Point m_aPos;
    TextFrameIndex const m_nEnd;
    std::unique_ptr<SwFont> m_pFont;

public:
    // sets the font which should paint the common baseline,
    // index where continuous underline ends,
    // and the starting point of the common baseline
    SwUnderlineFont(SwFont& rFnt, TextFrameIndex nEnd, const Point& rPoint);
    ~SwUnderlineFont();

    SwFont& GetFont()
    {
        OSL_ENSURE( m_pFont, "No underline font" );
        return *m_pFont;
    }
    const Point& GetPos() const { return m_aPos; }
    TextFrameIndex GetEnd() const { return m_nEnd; }
    // the x coordinate of the starting point has to be set for each portion
    void SetPos( const Point& rPoint ) { m_aPos = rPoint;  }
};

#ifdef DBG_UTIL

class SvStatistics
{
public:
    sal_uInt16 nGetTextSize;
    sal_uInt16 nDrawText;
    sal_uInt16 nGetStretchTextSize;
    sal_uInt16 nDrawStretchText;
    sal_uInt16 nChangeFont;

    SvStatistics()
    { nGetTextSize = nDrawText = nGetStretchTextSize = nDrawStretchText = nChangeFont = 0; }
};

// global variable, implemented in swfont.cxx
extern SvStatistics g_SvStat;

#define SV_STAT(nWhich) ++(g_SvStat.nWhich);


#else
#define SV_STAT(nWhich)
#endif  /* DBG_UTIL */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
