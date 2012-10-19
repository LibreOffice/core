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
#ifndef _SWFONT_HXX
#define _SWFONT_HXX

#include <i18npool/lang.h>
#include <tools/color.hxx>
#include <tools/stream.hxx>
#include <editeng/svxfont.hxx>
#include <swtypes.hxx>
#include <drawfont.hxx>     // SwDrawTextInfo

class SfxItemSet;
class SwAttrSet;
class SwDoCapitals;     // DoCapitals
class SwDrawTextInfo;   // _DrawText
class SwScriptInfo;     // _GetTxtSize
class ViewShell;
class IDocumentSettingAccess;

const sal_Unicode CH_BLANK = ' ';   // ' ' Leerzeichen
const sal_Unicode CH_BREAK = 0x0A;  //
const sal_Unicode CH_TAB   = '\t';  // \t
const sal_Unicode CH_PAR    = 0xB6;     // Paragraph
const sal_Unicode CH_BULLET = 0xB7;     // mittiger Punkt

class SwSubFont : public SvxFont
{
    friend class SwFont;
    const void *pMagic;         // "MagicNumber" innerhalb des Fontcaches
    Size        aSize;          // Fremde kriegen nur diese Size zu sehen
    sal_uInt16      nFntIndex;      // Index im Fontcache
    sal_uInt16      nOrgHeight;     // Hoehe inkl. Escapement/Proportion
    sal_uInt16      nOrgAscent;     // Ascent inkl. Escapement/Proportion
    sal_uInt16      nPropWidth;     // proportional width
    bool smallCapsPercentage66;
    inline SwSubFont() : aSize(0,0)
    { pMagic = NULL; nFntIndex = nOrgHeight = nOrgAscent = 0; nPropWidth =100; smallCapsPercentage66 = false; }

    sal_uInt16 CalcEscAscent( const sal_uInt16 nOldAscent ) const;
    sal_uInt16 CalcEscHeight( const sal_uInt16 nOldHeight,
                          const sal_uInt16 nOldAscent ) const;
    void CalcEsc( SwDrawTextInfo& rInf, Point& rPos );

    inline void CopyMagic( const SwSubFont& rFnt )
        { pMagic = rFnt.pMagic; nFntIndex = rFnt.nFntIndex; }

    sal_Bool operator==( const SwFont &rFnt ) const;
    SwSubFont& operator=( const SwSubFont &rFont );

    short _CheckKerning( );

    sal_Bool ChgFnt( ViewShell *pSh, OutputDevice& rOut );
    sal_Bool IsSymbol( ViewShell *pSh );
    sal_uInt16 GetAscent( ViewShell *pSh, const OutputDevice& rOut );
    sal_uInt16 GetHeight( ViewShell *pSh, const OutputDevice& rOut );
    Size _GetTxtSize( SwDrawTextInfo& rInf );
    Size GetCapitalSize( SwDrawTextInfo& rInf );
    void _DrawText( SwDrawTextInfo &rInf, const sal_Bool bGrey );
    void DrawCapital( SwDrawTextInfo &rInf );
    void DrawStretchCapital( SwDrawTextInfo &rInf );
    void DoOnCapitals( SwDoCapitals &rDo );
    void _DrawStretchText( SwDrawTextInfo &rInf );
    xub_StrLen _GetCrsrOfst( SwDrawTextInfo& rInf );
    xub_StrLen GetCapitalCrsrOfst( SwDrawTextInfo& rInf );

    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetCharSet( const CharSet eCharSet );
    inline void SetPitch( const FontPitch ePitch );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontUnderline eUnderline );
    inline void SetOverline( const FontUnderline eOverline );
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetItalic( const FontItalic eItalic );
    inline void SetOutline( const sal_Bool bOutline );
    inline void SetVertical( const sal_uInt16 nDir, const sal_Bool bVertFormat );
    inline void SetShadow( const sal_Bool bShadow );
    inline void SetAutoKern( const sal_uInt8 nAutoKern );
    inline void SetWordLineMode( const sal_Bool bWordLineMode );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );
    inline void SetRelief( const FontRelief eNew );

    // Methoden fuer die Hoch-/Tiefstellung
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const sal_uInt8 nNewPropr );

    inline void SetFamily( const FontFamily eFamily );
    inline void SetName( const XubString& rName );
    inline void SetStyleName( const XubString& rStyleName );
    inline void SetSize( const Size& rSize );
    inline void SetWeight( const FontWeight eWeight );
    inline void SetLanguage( LanguageType eNewLang );
    inline short CheckKerning()
    {   return GetFixKerning() >= 0 ? GetFixKerning() : _CheckKerning( ); }
    inline void SetPropWidth( const sal_uInt16 nNew )
        { pMagic = 0; nPropWidth = nNew; }
public:
    sal_uInt16 GetPropWidth() const { return nPropWidth; }
};

#define SW_LATIN 0
#define SW_CJK 1
#define SW_CTL 2
#define SW_SCRIPTS 3

class SwFont
{                               // CJK == Chinese, Japanese, Korean
                                // CTL == Complex text layout ( Hebrew, Arabic )
    SwSubFont   aSub[SW_SCRIPTS]; // Latin-, CJK- and CTL-font
    Color*      pBackColor;     // background color (i.e. at character styles)
    Color       aUnderColor;    // color of the underlining
    Color       aOverColor;     // color of the overlining
    sal_uInt8       nToxCnt;        // Zaehlt die Schachtelungstiefe der Tox
    sal_uInt8       nRefCnt;        // Zaehlt die Schachtelungstiefe der Refs
    sal_uInt8        m_nMetaCount;   // count META/METAFIELD
    sal_uInt8       nActual;        // actual font (Latin, CJK or CTL)

    // Schalter fuer die Font-Extensions
    sal_Bool bNoHyph        :1;  // SwTxtNoHyphenHere:    keine Trennstelle
    sal_Bool bBlink         :1;  // blinkender Font
    sal_Bool bPaintBlank    :1;  // Blanks nicht mit DrawRect
    sal_Bool bFntChg        :1;
    sal_Bool bOrgChg        :1;  // nOrgHeight/Ascent sind invalid
    sal_Bool bURL           :1;
    sal_Bool bPaintWrong    :1;  // Flag fuer Rechtschreibfehler
    sal_Bool bGreyWave      :1;  // Fuers extended TextInput: Graue Wellenlinie
    sal_Bool bNoColReplace  :1;  // Replacement without colormanipulation

    sal_Bool operator==( const SwFont &rFnt ) const;

protected:
    inline SwFont() { pBackColor = NULL; nActual = SW_LATIN; }

public:
    SwFont( const SwAttrSet* pSet, const IDocumentSettingAccess* pIDocumentSettingAccess );
    SwFont( const SwFont& rFont );

    inline void ChgFnt( ViewShell *pSh, OutputDevice& rOut )
        { bPaintBlank = aSub[nActual].ChgFnt( pSh, rOut ); }

    ~SwFont(){ delete pBackColor; }

    SwFont& operator=( const SwFont &rFont );

    inline sal_uInt8 GetActual() const {    return nActual; }
    inline void SetActual( sal_uInt8 nNew );
    inline const SvxFont& GetActualFont() const { return aSub[nActual]; }

    // holt sich eine MagicNumber ueber SwFntAccess
    void GoMagic( ViewShell *pSh, sal_uInt8 nWhich );
    // set background color
    void SetBackColor( Color* pNewColor );
    inline const Color* GetBackColor() const{ return pBackColor; }

    inline void ChkMagic( ViewShell *pSh, sal_uInt8 nWhich )
        { if( !aSub[ nWhich ].pMagic ) GoMagic( pSh, nWhich ); }
    // uebernimmt die MagicNumber eines (hoffentlich ident.) Kollegen
    inline void CopyMagic( const SwFont* pFnt, sal_uInt8 nWhich )
        { aSub[nWhich].CopyMagic( pFnt->aSub[nWhich] ); }
    inline void GetMagic( const void* &rMagic, sal_uInt16 &rIdx, sal_uInt8 nWhich )
        { rMagic = aSub[nWhich].pMagic; rIdx = aSub[nWhich].nFntIndex; }
    inline void SetMagic( const void* pNew, const sal_uInt16 nIdx, sal_uInt8 nWhich )
        { aSub[nWhich].pMagic = pNew; aSub[nWhich].nFntIndex = nIdx; }
    inline sal_Bool DifferentMagic( const SwFont* pFnt, sal_uInt8 nWhich )
        { return aSub[nWhich].pMagic != pFnt->aSub[nWhich].pMagic ||
          !aSub[nWhich].pMagic || !pFnt->aSub[nWhich].pMagic; }

    inline const Size &GetSize( sal_uInt8 nWhich ) const
        { return aSub[nWhich].aSize; }
    inline sal_Bool IsFntChg() const { return bFntChg; }
    inline void SetFntChg( const sal_Bool bNew ) { bFntChg = bNew; }

    // die gekapselten SV-Font-Methoden (setzen bFntChg auf sal_True)
    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontUnderline eUnderline );
    inline void SetUnderColor( const Color &rColor ) { aUnderColor = rColor; }
    inline void SetOverline( const FontUnderline eOverline );
    inline void SetOverColor( const Color &rColor ) { aOverColor = rColor; }
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetOutline( const sal_Bool bOutline );
           void SetVertical( sal_uInt16 nDir, const sal_Bool nVertLayout = sal_False );
    inline void SetShadow( const sal_Bool bShadow );
    inline void SetAutoKern( sal_uInt8 nAutoKern );
    inline void SetTransparent( const sal_Bool bTrans );
    inline void SetWordLineMode( const sal_Bool bWordLineMode );
    inline void SetFixKerning( const short nNewKern );
    inline void SetCaseMap( const SvxCaseMap eNew );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );

    // Methoden fuer die Hoch-/Tiefstellung
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const sal_uInt8 nNewPropr );

    inline void SetPropWidth( const sal_uInt16 nNew );

    inline void SetFamily( const FontFamily eFamily, const sal_uInt8 nWhich );
    inline void SetName( const XubString& rName, const sal_uInt8 nWhich );
    inline void SetStyleName( const XubString& rStyleName, const sal_uInt8 nWhich );
    inline void SetSize( const Size& rSize, const sal_uInt8 nWhich );
    inline void SetWeight( const FontWeight eWeight, const sal_uInt8 nWhich );
    inline void SetItalic( const FontItalic eItalic, const sal_uInt8 nWhich );
    inline void SetLanguage( LanguageType eNewLang, const sal_uInt8 nWhich );
    inline void SetCharSet( const CharSet eCharSet, const sal_uInt8 nWhich );
    inline void SetPitch( const FontPitch ePitch, const sal_uInt8 nWhich );
    inline void SetRelief( const FontRelief eNew );

    // Get/Set-Methoden fuer die aktuelle Einstellung
    inline void SetNoHyph( const sal_Bool bNew );
    inline sal_Bool IsNoHyph() const { return bNoHyph; }
    inline void SetBlink( const sal_Bool bBlink );
    inline sal_Bool IsBlink() const { return bBlink; }
    inline sal_uInt8 &GetTox() { return nToxCnt; }
    inline sal_uInt8 GetTox() const { return nToxCnt; }
    inline sal_Bool IsTox() const { return ( 0 != nToxCnt ); }
    inline sal_uInt8 &GetRef() { return nRefCnt; }
    inline sal_uInt8 GetRef() const { return nRefCnt; }
    inline sal_Bool IsRef() const { return ( 0 != nRefCnt ); }
    inline sal_uInt8 &GetMeta() { return m_nMetaCount; }
    inline sal_uInt8 GetMeta() const { return m_nMetaCount; }
    inline bool IsMeta() const { return (0 != m_nMetaCount); }
    inline void SetURL( const sal_Bool bURL );
    inline sal_Bool IsURL() const { return bURL; }
    inline void SetGreyWave( const sal_Bool bNew );
    inline sal_Bool IsGreyWave() const { return bGreyWave; }
    inline void SetNoCol( const sal_Bool bNew );
    inline sal_Bool IsNoCol() const { return bNoColReplace; }

    inline void SetPaintBlank( const sal_Bool bNew );
    inline sal_Bool IsPaintBlank() const { return bPaintBlank; }
    inline void SetPaintWrong( const sal_Bool bNew );
    inline sal_Bool IsPaintWrong() const { return bPaintWrong; }

    // Setzen der Basisklasse Font fuer SwTxtCharFmt
    void SetDiffFnt( const SfxItemSet* pSet,
                     const IDocumentSettingAccess* pIDocumentSettingAccess );

    inline const SvxFont &GetFnt( const sal_uInt8 nWhich ) const
        { return aSub[nWhich]; };

    sal_Bool IsSymbol( ViewShell *pSh )
        { return aSub[nActual].IsSymbol( pSh ); }
    FontUnderline GetUnderline() const { return aSub[nActual].GetUnderline(); }
    const Color& GetUnderColor() const { return aUnderColor; }
    FontUnderline GetOverline() const { return aSub[nActual].GetOverline(); }
    const Color& GetOverColor() const { return aOverColor; }
    short GetFixKerning() const { return aSub[nActual].GetFixKerning(); }
    FontStrikeout GetStrikeout() const { return aSub[nActual].GetStrikeout(); }
    const Color& GetColor() const { return aSub[nActual].GetColor(); }
    sal_Bool IsShadow() const { return aSub[nActual].IsShadow(); }
    sal_Bool IsWordLineMode() const { return aSub[nActual].IsWordLineMode(); }
    sal_Bool IsOutline() const { return aSub[nActual].IsOutline(); }
    sal_Bool IsKerning() const { return aSub[nActual].IsKerning(); }
    short GetEscapement() const { return aSub[nActual].GetEscapement(); }
    SvxCaseMap GetCaseMap() const { return aSub[nActual].GetCaseMap(); }
    sal_uInt8 GetPropr() const { return aSub[nActual].GetPropr(); }
    FontItalic GetItalic() const { return aSub[nActual].GetItalic(); }
    LanguageType GetLanguage() const { return aSub[nActual].GetLanguage(); }
    FontAlign GetAlign() const { return aSub[nActual].GetAlign(); }
    const XubString& GetName() const { return aSub[nActual].GetName(); }
    const XubString& GetStyleName() const {return aSub[nActual].GetStyleName();}
    FontFamily GetFamily() const { return aSub[nActual].GetFamily(); }
    FontPitch GetPitch() const { return aSub[nActual].GetPitch(); }
    rtl_TextEncoding GetCharSet() const { return aSub[nActual].GetCharSet(); }
    long GetHeight() const { return aSub[nActual].GetSize().Height(); }
    FontWeight GetWeight() const { return aSub[nActual].GetWeight(); }
    FontEmphasisMark GetEmphasisMark() const
        { return aSub[nActual].GetEmphasisMark(); }
    sal_uInt16 GetPropWidth() const { return aSub[nActual].GetPropWidth(); }
    sal_uInt16 GetOrientation( const sal_Bool nVertLayout = sal_False ) const;

    inline const XubString& GetName( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetName(); }
    inline LanguageType GetLanguage( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetLanguage(); }
    inline const XubString& GetStyleName( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetStyleName(); }
    inline FontFamily GetFamily( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetFamily(); }
    inline FontItalic GetItalic( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetItalic(); }
    inline FontPitch GetPitch( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetPitch(); }
    inline rtl_TextEncoding GetCharSet( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetCharSet(); }
    inline long GetHeight( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetSize().Height(); }
    inline FontWeight GetWeight( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetWeight(); }
    inline FontEmphasisMark GetEmphasisMark( const sal_uInt8 nWhich ) const
        { return aSub[nWhich].GetEmphasisMark(); }

    // Macht den logischen Font im OutputDevice wirksam.
    void ChgPhysFnt( ViewShell *pSh, OutputDevice& rOut );

    Size GetCapitalSize( SwDrawTextInfo& rInf )
        { return aSub[nActual].GetCapitalSize( rInf ); }

    xub_StrLen GetCapitalBreak( ViewShell* pSh,  const OutputDevice* pOut,
        const SwScriptInfo* pScript, const XubString& rTxt,
        long nTextWidth, xub_StrLen* pExtra, const xub_StrLen nIdx,
        const xub_StrLen nLen );

    xub_StrLen GetCapitalCrsrOfst( SwDrawTextInfo& rInf )
        { return aSub[nActual].GetCapitalCrsrOfst( rInf ); }

    void DrawCapital( SwDrawTextInfo &rInf )
        { aSub[nActual].DrawCapital( rInf ); }

    void DrawStretchCapital( SwDrawTextInfo &rInf )
        { aSub[nActual].DrawStretchCapital( rInf ); }

    void DoOnCapitals( SwDoCapitals &rDo )
        { aSub[nActual].DoOnCapitals( rDo ); }

    Size _GetTxtSize( SwDrawTextInfo& rInf )
        { rInf.SetFont( this ); return aSub[nActual]._GetTxtSize( rInf ); }

    xub_StrLen GetTxtBreak( SwDrawTextInfo& rInf, long nTextWidth );

    xub_StrLen _GetCrsrOfst( SwDrawTextInfo& rInf )
        { return aSub[nActual]._GetCrsrOfst( rInf ); }

    inline void _DrawText( SwDrawTextInfo &rInf )
        { aSub[nActual]._DrawText( rInf, IsGreyWave() ); }

    inline void _DrawStretchText( SwDrawTextInfo &rInf )
        { aSub[nActual]._DrawStretchText( rInf ); }

    inline short CheckKerning()
        { return aSub[nActual].CheckKerning(); }

    inline sal_uInt16 GetAscent( ViewShell *pSh, const OutputDevice& rOut )
        { return aSub[nActual].GetAscent( pSh, rOut ); }
    inline sal_uInt16 GetHeight( ViewShell *pSh, const OutputDevice& rOut )
        { return aSub[nActual].GetHeight( pSh, rOut ); }

    inline void Invalidate()
        { bFntChg = bOrgChg = sal_True; }
};

inline void SwFont::SetColor( const Color& rColor )
{
    bFntChg = sal_True;
    aSub[0].SetColor( rColor );
    aSub[1].SetColor( rColor );
    aSub[2].SetColor( rColor );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetColor( const Color& rColor )
{
    pMagic = 0;
    Font::SetColor( rColor );
}


// gekapselte SV-Font-Methode
inline void SwSubFont::SetFillColor( const Color& rColor )
{
    pMagic = 0;
    Font::SetFillColor( rColor );
}

inline void SwFont::SetFillColor( const Color& rColor )
{
    bFntChg = sal_True;
    aSub[0].SetFillColor( rColor );
    aSub[1].SetFillColor( rColor );
    aSub[2].SetFillColor( rColor );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetFamily( const FontFamily eFamily )
{
    pMagic = 0;
    Font::SetFamily( eFamily );
}

inline void SwFont::SetFamily( const FontFamily eFamily, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetFamily( eFamily );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetName( const XubString& rName )
{
    pMagic = 0;
    Font::SetName( rName );
}

inline void SwFont::SetName( const XubString& rName, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetName( rName );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetStyleName( const XubString& rStyleName )
{
    pMagic = 0;
    Font::SetStyleName( rStyleName );
}

inline void SwFont::SetStyleName( const XubString& rStyle, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetStyleName( rStyle );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetCharSet( const CharSet eCharSet )
{
    pMagic = 0;
    Font::SetCharSet( eCharSet );
}

inline void SwFont::SetCharSet( const CharSet eCharSet, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetCharSet( eCharSet );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetPitch( const FontPitch ePitch )
{
    pMagic = 0;
    Font::SetPitch( ePitch );
}

// gekapselte SV-Font-Methode
inline void SwFont::SetPitch( const FontPitch ePitch, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetPitch( ePitch );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetAlign( const FontAlign eAlign )
{
    pMagic = 0;
    Font::SetAlign( eAlign );
}

inline void SwFont::SetAlign( const FontAlign eAlign )
{
    bFntChg = sal_True;
    aSub[0].SetAlign( eAlign );
    aSub[1].SetAlign( eAlign );
    aSub[2].SetAlign( eAlign );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetWeight( const FontWeight eWeight )
{
    pMagic = 0;
    Font::SetWeight( eWeight );
}

inline void SwFont::SetWeight( const FontWeight eWeight, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetWeight( eWeight );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetUnderline( const FontUnderline eUnderline )
{
    pMagic = 0;
    Font::SetUnderline( eUnderline );
}

inline void SwFont::SetUnderline( const FontUnderline eUnderline )
{
    bFntChg = sal_True;
    aSub[0].SetUnderline( eUnderline );
    aSub[1].SetUnderline( eUnderline );
    aSub[2].SetUnderline( eUnderline );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetOverline( const FontUnderline eOverline )
{
    pMagic = 0;
    Font::SetOverline( eOverline );
}

inline void SwFont::SetOverline( const FontUnderline eOverline )
{
    bFntChg = sal_True;
    aSub[0].SetOverline( eOverline );
    aSub[1].SetOverline( eOverline );
    aSub[2].SetOverline( eOverline );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    pMagic = 0;
    Font::SetStrikeout( eStrikeout );
}

inline void SwFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    bFntChg = sal_True;
    aSub[0].SetStrikeout( eStrikeout );
    aSub[1].SetStrikeout( eStrikeout );
    aSub[2].SetStrikeout( eStrikeout );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetItalic( const FontItalic eItalic )
{
    pMagic = 0;
    Font::SetItalic( eItalic );
}

inline void SwFont::SetItalic( const FontItalic eItalic, const sal_uInt8 nWhich )
{
    bFntChg = sal_True;
    aSub[nWhich].SetItalic( eItalic );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetOutline( const sal_Bool bOutline )
{
    pMagic = 0;
    Font::SetOutline( bOutline );
}

inline void SwFont::SetOutline( const sal_Bool bOutline )
{
    bFntChg = sal_True;
    aSub[0].SetOutline( bOutline );
    aSub[1].SetOutline( bOutline );
    aSub[2].SetOutline( bOutline );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetShadow( const sal_Bool bShadow )
{
    pMagic = 0;
    Font::SetShadow( bShadow );
}

inline void SwFont::SetShadow( const sal_Bool bShadow )
{
    bFntChg = sal_True;
    aSub[0].SetShadow( bShadow );
    aSub[1].SetShadow( bShadow );
    aSub[2].SetShadow( bShadow );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetAutoKern( const sal_uInt8 nAutoKern )
{
    pMagic = 0;
    Font::SetKerning( nAutoKern );
}

inline void SwFont::SetAutoKern( sal_uInt8 nAutoKern )
{
    bFntChg = sal_True;
    aSub[1].SetAutoKern( nAutoKern );
    if( nAutoKern )
        nAutoKern = KERNING_FONTSPECIFIC;
    aSub[0].SetAutoKern( nAutoKern );
    aSub[2].SetAutoKern( nAutoKern );
}

inline void SwFont::SetTransparent( const sal_Bool bTrans )
{
    aSub[0].SetTransparent( bTrans );
    aSub[1].SetTransparent( bTrans );
    aSub[2].SetTransparent( bTrans );
}

inline void SwFont::SetFixKerning( const short nNewKern )
{
    aSub[SW_LATIN].SetFixKerning( nNewKern );
    aSub[SW_CJK].SetFixKerning( nNewKern );
    aSub[SW_CTL].SetFixKerning( nNewKern );
}

inline void SwFont::SetCaseMap( const SvxCaseMap eNew )
{
    aSub[SW_LATIN].SetCaseMap( eNew );
    aSub[SW_CJK].SetCaseMap( eNew );
    aSub[SW_CTL].SetCaseMap( eNew );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetWordLineMode( const sal_Bool bWordLineMode )
{
    pMagic = 0;
    Font::SetWordLineMode( bWordLineMode );
}

inline void SwFont::SetWordLineMode( const sal_Bool bWordLineMode )
{
    bFntChg = sal_True;
    aSub[0].SetWordLineMode( bWordLineMode );
    aSub[1].SetWordLineMode( bWordLineMode );
    aSub[2].SetWordLineMode( bWordLineMode );
}
// gekapselte SV-Font-Methode
inline void SwSubFont::SetEmphasisMark( const FontEmphasisMark eValue )
{
    pMagic = 0;
    Font::SetEmphasisMark( eValue );
}

inline void SwFont::SetEmphasisMark( const FontEmphasisMark eValue )
{
    bFntChg = sal_True;
    aSub[0].SetEmphasisMark( eValue );
    aSub[1].SetEmphasisMark( eValue );
    aSub[2].SetEmphasisMark( eValue );
}

inline void SwFont::SetPropWidth( const sal_uInt16 nNew )
{
    if( nNew != aSub[0].GetPropWidth() )
    {
        bFntChg = sal_True;
        aSub[0].SetPropWidth( nNew );
        aSub[1].SetPropWidth( nNew );
        aSub[2].SetPropWidth( nNew );
    }
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetRelief( const FontRelief eNew )
{
    pMagic = 0;
    Font::SetRelief( eNew );
}

inline void SwFont::SetRelief( const FontRelief eNew )
{
    if( eNew != aSub[0].GetRelief() )
    {
        bFntChg = sal_True;
        aSub[0].SetRelief( eNew );
        aSub[1].SetRelief( eNew );
        aSub[2].SetRelief( eNew );
    }
}

// ueberladene Font-Methode
inline void SwSubFont::SetSize( const Size& rSize )
{
    aSize = rSize;
    if ( GetPropr() == 100 )
        Font::SetSize( aSize );
    else
    {
        Font::SetSize( Size(
            (long) aSize.Width() * GetPropr() / 100L,
            (long) aSize.Height() * GetPropr() / 100L ) );
    }
    pMagic = 0;
}

inline void SwFont::SetSize( const Size& rSize, const sal_uInt8 nWhich )
{
    if( aSub[nWhich].aSize != rSize )
    {
        aSub[nWhich].SetSize( rSize );
        bFntChg = sal_True;
        bOrgChg = sal_True;
    }
}

inline void SwFont::SetActual( sal_uInt8 nNew )
{
     if ( nActual != nNew )
     {
        bFntChg = sal_True;
        bOrgChg = sal_True;
        nActual = nNew;
     }
}

inline void SwSubFont::SetProportion( const sal_uInt8 nNewPropr )
{
    pMagic = 0;
    Font::SetSize( Size( (long) aSize.Width() * nNewPropr / 100L,
                         (long) aSize.Height() * nNewPropr / 100L ) );
    SvxFont::SetPropr( nNewPropr );
}

inline void SwFont::SetProportion( const sal_uInt8 nNewPropr )
{
    if( nNewPropr != aSub[0].GetPropr() )
    {
        bFntChg = sal_True;
        bOrgChg = sal_True;

        aSub[0].SetProportion( nNewPropr );
        aSub[1].SetProportion( nNewPropr );
        aSub[2].SetProportion( nNewPropr );
    }
}

inline void SwSubFont::SetEscapement( const short nNewEsc )
{
    pMagic = 0;
    SvxFont::SetEscapement( nNewEsc );
}

inline void SwFont::SetEscapement( const short nNewEsc )
{
    if( nNewEsc != aSub[0].GetEscapement() )
    {
        // these have to be set, otherwise nOrgHeight and nOrgAscent will not
        // be calculated
        bFntChg = sal_True;
        bOrgChg = sal_True;

        aSub[0].SetEscapement( nNewEsc );
        aSub[1].SetEscapement( nNewEsc );
        aSub[2].SetEscapement( nNewEsc );
    }
}

inline void SwSubFont::SetLanguage( LanguageType eNewLang )
{
    if( eNewLang == LANGUAGE_SYSTEM )
        eNewLang = (LanguageType)GetAppLanguage();
    SvxFont::SetLanguage( eNewLang );
}

inline void SwFont::SetLanguage( const LanguageType eNewLang, const sal_uInt8 nWhich )
{
    aSub[nWhich].SetLanguage( eNewLang );
    if( SW_CJK == nWhich )
    {
        aSub[SW_LATIN].SetCJKContextLanguage( eNewLang );
        aSub[SW_CJK].SetCJKContextLanguage( eNewLang );
        aSub[SW_CTL].SetCJKContextLanguage( eNewLang );
    }
}

inline void SwFont::SetPaintBlank( const sal_Bool bNew )
{
    bPaintBlank = bNew;
}

inline void SwFont::SetPaintWrong( const sal_Bool bNew )
{
    bPaintWrong = bNew;
}

inline void SwFont::SetNoHyph( const sal_Bool bNew )
{
    bNoHyph = bNew;
}

inline void SwFont::SetBlink( const sal_Bool bNew )
{
    bBlink = bNew;
}

inline void SwFont::SetURL( const sal_Bool bNew )
{
    bURL = bNew;
}

inline void SwFont::SetGreyWave( const sal_Bool bNew )
{
    bGreyWave = bNew;
}

inline void SwFont::SetNoCol( const sal_Bool bNew )
{
    bNoColReplace = bNew;
}

inline void SwSubFont::SetVertical( const sal_uInt16 nDir, const sal_Bool bVertFormat )
{
    pMagic = 0;
    Font::SetVertical( bVertFormat );
    Font::SetOrientation( nDir );
}


/*************************************************************************
 *                      class SwUnderlineFont
 *
 * Used for the "continuous underline" feature.
 *************************************************************************/

class SwUnderlineFont
{
    Point aPos;
    SwFont* pFnt;

public:
    // sets the font which should paint the common baseline
    // and the starting point of the common baseline
    SwUnderlineFont( SwFont& rFnt, const Point& rPoint );
    ~SwUnderlineFont();

    SwFont& GetFont()
    {
        OSL_ENSURE( pFnt, "No underline font" );
        return *pFnt;
    }
    const Point& GetPos() const { return aPos; }
    // the x coordinate of the starting point has to be set for each portion
    void SetPos( const Point& rPoint ) { aPos = rPoint;  }
};


/*************************************************************************
 *                      class SvStatistics
 *************************************************************************/

#ifdef DBG_UTIL

class SvStatistics
{
public:
    sal_uInt16 nGetTextSize;
    sal_uInt16 nDrawText;
    sal_uInt16 nGetStretchTextSize;
    sal_uInt16 nDrawStretchText;
    sal_uInt16 nChangeFont;
    sal_uInt16 nGetFontMetric;

    inline void Reset()
    {
        nGetTextSize = nDrawText = nGetStretchTextSize =
        nDrawStretchText = nChangeFont = nGetFontMetric = 0;
    }

    inline SvStatistics() { Reset(); }

    inline void PrintOn( SvStream &rOS ) const; //$ ostream
    inline sal_Bool IsEmpty() const
    {
        return !( nGetTextSize || nDrawText ||
                  nDrawStretchText || nChangeFont || nGetFontMetric );
    }
};

// globale Variable, implementiert in swfont.cxx
extern SvStatistics g_SvStat;

#define SV_STAT(nWhich) ++(g_SvStat.nWhich);

inline void SvStatistics::PrintOn( SvStream &rOS ) const //$ ostream
{
    if( IsEmpty() )
        return;

    rOS << "{   SV called:" << '\n';
    if( nGetTextSize )
        rOS << "\tnGetTextSize: " <<    nGetTextSize    << '\n';        if( nDrawText   )
        rOS << "\tnDrawText: "  << nDrawText    << '\n';        if( nGetStretchTextSize )
        rOS << "\tnGetStretchTextSize: "    << nGetStretchTextSize  << '\n';        if( nDrawStretchText    )
        rOS << "\tnDrawStretchText: "   << nDrawStretchText << '\n';        if( nChangeFont )
        rOS << "\tnChangeFont: "    << nChangeFont  << '\n';        if( nGetFontMetric  )
        rOS << "\tnGetFontMetric: " << nGetFontMetric   << '\n';        rOS << "}"  << '\n';    }
#else
#define SV_STAT(nWhich)
#endif  /* DBG_UTIL */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
