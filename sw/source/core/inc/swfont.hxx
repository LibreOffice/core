/*************************************************************************
 *
 *  $RCSfile: swfont.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2000-11-24 15:55:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWFONT_HXX
#define _SWFONT_HXX


#ifndef _TOOLS_LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _TOOLS_STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX //autogen
#include <svx/svxfont.hxx>
#endif

// Hiermit kann man einen Kerning-Test mit CodeView in ITRPAINT.CXX
// und FNTCACHE durchfuehren, solange die UI dies nicht anbietet.
#ifdef DEBUG
#define KERNINGTEST
#endif

class LinguBase;        // SetLingu()
class SfxItemSet;
class SwAttrSet;
class SwDoCapitals;     // DoCapitals
class SwWrongList;
class SwDrawTextInfo;   // _DrawText
class ViewShell;

const xub_Unicode CH_BLANK = ' ';   // ' ' Leerzeichen
const xub_Unicode CH_BREAK = 0x0A;  //
const xub_Unicode CH_TAB   = '\t';  // \t
const xub_Unicode CH_PAR_ALTER = 0xDE;      // Paragraph (StarBats)

#if defined(WIN) || defined(WNT) || defined(UNX)
const xub_Unicode CH_PAR    = 0xB6;     // Paragraph
const xub_Unicode CH_BULLET = 0xB7;     // mittiger Punkt
#endif

#ifdef PM2
const xub_Unicode CH_PAR    = 0xF4;     // Paragraph
const xub_Unicode CH_BULLET = 0xFA;         // mittiger Punkt
#endif

#ifdef MAC
const xub_Unicode CH_PAR    = 0xA6;     // Paragraph
const xub_Unicode CH_BULLET = 0xE1;     // mittiger Punkt
#endif

LanguageType GetSystemLang();

class SwSubFont : public SvxFont
{
    friend class SwFont;
    const void *pMagic;         // "MagicNumber" innerhalb des Fontcaches
    Size        aSize;          // Fremde kriegen nur diese Size zu sehen
    USHORT      nFntIndex;      // Index im Fontcache
    USHORT      nOrgHeight;     // Hoehe inkl. Escapement/Proportion
    USHORT      nOrgAscent;     // Ascent inkl. Escapement/Proportion

    inline SwSubFont() : aSize(0,0)
        { pMagic = NULL; nFntIndex = 0; nOrgHeight = nOrgAscent = 0; }

    USHORT CalcEscAscent( const USHORT nOldAscent ) const;
    USHORT CalcEscHeight( const USHORT nOldHeight,
                          const USHORT nOldAscent ) const;
    inline void CopyMagic( const SwSubFont& rFnt )
        { pMagic = rFnt.pMagic; nFntIndex = rFnt.nFntIndex; }

    BOOL operator==( const SwFont &rFnt ) const;
    SwSubFont& operator=( const SwSubFont &rFont );

    short _CheckKerning( );
    void SetFnt( const SvxFont &rFont );

    BOOL ChgFnt( ViewShell *pSh, OutputDevice *pOut );
    BOOL IsSymbol( ViewShell *pSh );
    USHORT GetAscent( ViewShell *pSh, const OutputDevice *pOut );
    USHORT GetHeight( ViewShell *pSh, const OutputDevice *pOut );
    Size _GetTxtSize( ViewShell *pSh, const OutputDevice *pOut,
        const XubString &rTxt, const xub_StrLen nIdx, const xub_StrLen nLen );
    Size GetCapitalSize( ViewShell *pSh, const OutputDevice *pOut,
        const XubString &rTxt, const xub_StrLen nIdx, const xub_StrLen nLen );
    void _DrawText( SwDrawTextInfo &rInf, const BOOL bGrey );
    void DrawCapital( SwDrawTextInfo &rInf );
    void DrawStretchCapital( SwDrawTextInfo &rInf );
    void DoOnCapitals( SwDoCapitals &rDo );
    void _DrawStretchText( SwDrawTextInfo &rInf );
    xub_StrLen _GetCrsrOfst( ViewShell *pSh, OutputDevice *pOut,
            const XubString &rTxt, const USHORT nOfst, const xub_StrLen nIdx,
            const xub_StrLen nLen, const short nSpaceAdd );
    xub_StrLen GetCapitalCrsrOfst( ViewShell *pSh, OutputDevice *pOut,
            const XubString &rTxt, const USHORT nOfst, const xub_StrLen nIdx,
            const xub_StrLen nLen, const short nSpaceAdd );

    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetCharSet( const CharSet eCharSet );
    inline void SetPitch( const FontPitch ePitch );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontUnderline eUnderline );
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetItalic( const FontItalic eItalic );
    inline void SetOutline( const BOOL bOutline );
    inline void SetShadow( const BOOL bShadow );
    inline void SetAutoKern( const BOOL bAutoKern );
    inline void SetWordLineMode( const BOOL bWordLineMode );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );

    // Methoden fuer die Hoch-/Tiefstellung
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const BYTE nNewPropr );

    inline void SetFamily( const FontFamily eFamily );
    inline void SetName( const XubString& rName );
    inline void SetStyleName( const XubString& rStyleName );
    inline void SetSize( const Size& rSize );
    inline void SetWeight( const FontWeight eWeight );
    inline void SetLanguage( LanguageType eNewLang );
    inline short CheckKerning()
    {   return GetFixKerning() >= 0 ? GetFixKerning() : _CheckKerning( ); }
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
    BYTE        nToxCnt;        // Zaehlt die Schachtelungstiefe der Tox
    BYTE        nRefCnt;        // Zaehlt die Schachtelungstiefe der Refs
    BYTE        nActual;        // actual font (Latin, CJK or CTL)

    // Schalter fuer die Font-Extensions
    BOOL bNoHyph        :1;  // SwTxtNoHyphenHere:    keine Trennstelle
    BOOL bBlink         :1;  // blinkender Font
    BOOL bPaintBlank    :1;  // Blanks nicht mit DrawRect
    BOOL bFntChg        :1;
    BOOL bOrgChg        :1;  // nOrgHeight/Ascent sind invalid
    BOOL bURL           :1;
    BOOL bPaintWrong    :1;  // Flag fuer Rechtschreibfehler
    BOOL bGreyWave      :1;  // Fuers extended TextInput: Graue Wellenlinie
    BOOL bNoColReplace  :1;  // Replacement without colormanipulation


    BOOL operator==( const SwFont &rFnt ) const;

protected:
    inline SwFont() { pBackColor = NULL; nActual = SW_LATIN; }

public:
    SwFont( const SwAttrSet* pSet );
    SwFont( const SwFont& rFont );

    inline void ChgFnt( ViewShell *pSh, OutputDevice *pOut )
        { bPaintBlank = aSub[nActual].ChgFnt( pSh, pOut ); }

    ~SwFont(){ delete pBackColor; }

    SwFont& operator=( const SwFont &rFont );

    inline BYTE GetActual() const { return nActual; }
    inline void SetActual( BYTE nNew ) { nActual = nNew; bFntChg = TRUE; }
    inline const SvxFont& GetActualFont() const { return aSub[nActual]; }

    // holt sich eine MagicNumber ueber SwFntAccess
    void GoMagic( ViewShell *pSh, BYTE nWhich );
    // set background color
    void SetBackColor( Color* pNewColor );
    Color* XChgBackColor( Color* pNewColor );
    inline const Color* GetBackColor() const{ return pBackColor; }

    inline void ChkMagic( ViewShell *pSh, BYTE nWhich )
        { if( !aSub[ nWhich ].pMagic ) GoMagic( pSh, nWhich ); }
    // uebernimmt die MagicNumber eines (hoffentlich ident.) Kollegen
    inline void CopyMagic( const SwFont* pFnt, BYTE nWhich )
        { aSub[nWhich].CopyMagic( pFnt->aSub[nWhich] ); }
    inline void GetMagic( const void* &rMagic, USHORT &rIdx, BYTE nWhich )
        { rMagic = aSub[nWhich].pMagic; rIdx = aSub[nWhich].nFntIndex; }
    inline void SetMagic( const void* pNew, const USHORT nIdx, BYTE nWhich )
        { aSub[nWhich].pMagic = pNew; aSub[nWhich].nFntIndex = nIdx; }
    inline BOOL DifferentMagic( const SwFont* pFnt, BYTE nWhich )
        { return aSub[nWhich].pMagic != pFnt->aSub[nWhich].pMagic ||
          !aSub[nWhich].pMagic || !pFnt->aSub[nWhich].pMagic; }

    inline const Size &GetSize( BYTE nWhich ) const
        { return aSub[nWhich].aSize; }
    inline BOOL IsFntChg() const { return bFntChg; }
    inline void SetFntChg( const BOOL bNew ) { bFntChg = bNew; }

    // die gekapselten SV-Font-Methoden (setzen bFntChg auf TRUE)
    inline void SetColor( const Color& rColor );
    inline void SetFillColor( const Color& rColor );
    inline void SetAlign( const FontAlign eAlign );
    inline void SetUnderline( const FontUnderline eUnderline );
    inline void SetUnderColor( const Color &rColor ) { aUnderColor = rColor; }
    inline void SetStrikeout( const FontStrikeout eStrikeout );
    inline void SetOutline( const BOOL bOutline );
    inline void SetShadow( const BOOL bShadow );
    inline void SetAutoKern( const BOOL bAutoKern );
    inline void SetTransparent( const BOOL bTrans );
    inline void SetWordLineMode( const BOOL bWordLineMode );
    inline void SetFixKerning( const short nNewKern );
    inline void SetCaseMap( const SvxCaseMap eNew );
    inline void SetEmphasisMark( const FontEmphasisMark eValue );

    // Methoden fuer die Hoch-/Tiefstellung
    inline void SetEscapement( const short nNewEsc );
    inline void SetProportion( const BYTE nNewPropr );

    inline void SetFamily( const FontFamily eFamily, const BYTE nWhich );
    inline void SetName( const XubString& rName, const BYTE nWhich );
    inline void SetStyleName( const XubString& rStyleName, const BYTE nWhich );
    inline void SetSize( const Size& rSize, const BYTE nWhich );
    inline void SetWeight( const FontWeight eWeight, const BYTE nWhich );
    inline void SetItalic( const FontItalic eItalic, const BYTE nWhich );
    inline void SetLanguage( LanguageType eNewLang, const BYTE nWhich );
    inline void SetCharSet( const CharSet eCharSet, const BYTE nWhich );
    inline void SetPitch( const FontPitch ePitch, const BYTE nWhich );

    // Get/Set-Methoden fuer die aktuelle Einstellung
    inline void SetNoHyph( const BOOL bNew );
    inline BOOL IsNoHyph() const { return bNoHyph; }
    inline void SetBlink( const BOOL bBlink );
    inline BOOL IsBlink() const { return bBlink; }
    inline BYTE &GetTox() { return nToxCnt; }
    inline BYTE GetTox() const { return nToxCnt; }
    inline BOOL IsTox() const { return ( 0 != nToxCnt ); }
    inline BYTE &GetRef() { return nRefCnt; }
    inline BYTE GetRef() const { return nRefCnt; }
    inline BOOL IsRef() const { return ( 0 != nRefCnt ); }
    inline void SetURL( const BOOL bURL );
    inline BOOL IsURL() const { return bURL; }
    inline void SetGreyWave( const BOOL bNew );
    inline BOOL IsGreyWave() const { return bGreyWave; }
    inline void SetNoCol( const BOOL bNew );
    inline BOOL IsNoCol() const { return bNoColReplace; }

    inline void SetPaintBlank( const BOOL bNew );
    inline BOOL IsPaintBlank() const { return bPaintBlank; }
    inline void SetPaintWrong( const BOOL bNew );
    inline BOOL IsPaintWrong() const { return bPaintWrong; }

    // Setzen der Basisklasse Font fuer SwTxtCharFmt
           void SetDiffFnt( const SfxItemSet* pSet );
    inline void SetFnt( const SvxFont &rFont, const BYTE nWhich )
        { bFntChg = bOrgChg = TRUE; aSub[nWhich].SetFnt( rFont ); }
           void SetFnt( const SwAttrSet* pSet );
    inline void SetSwFnt( const SwFont &rFont )
            { *this = rFont; }
    inline const SvxFont &GetFnt( const BYTE nWhich ) const
        { return aSub[nWhich]; };
    inline SvxFont &GetFnt( const BYTE nWhich )
        { return aSub[nWhich]; };

    void SetSystemLang();

    BOOL IsSymbol( ViewShell *pSh )
        { return aSub[nActual].IsSymbol( pSh ); }
    FontUnderline GetUnderline() const { return aSub[nActual].GetUnderline(); }
    const Color& GetUnderColor() const { return aUnderColor; }
    short GetFixKerning() const { return aSub[nActual].GetFixKerning(); }
    FontStrikeout GetStrikeout() const { return aSub[nActual].GetStrikeout(); }
    const Color& GetColor() const { return aSub[nActual].GetColor(); }
    BOOL IsShadow() const { return aSub[nActual].IsShadow(); }
    BOOL IsWordLineMode() const { return aSub[nActual].IsWordLineMode(); }
    BOOL IsOutline() const { return aSub[nActual].IsOutline(); }
    BOOL IsKerning() const { return aSub[nActual].IsKerning(); }
    short GetEscapement() const { return aSub[nActual].GetEscapement(); }
    SvxCaseMap GetCaseMap() const { return aSub[nActual].GetCaseMap(); }
    BYTE GetPropr() const { return aSub[nActual].GetPropr(); }
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

    inline const XubString& GetName( const BYTE nWhich ) const
        { return aSub[nWhich].GetName(); }
    inline LanguageType GetLanguage( const BYTE nWhich ) const
        { return aSub[nWhich].GetLanguage(); }
    inline const XubString& GetStyleName( const BYTE nWhich ) const
        { return aSub[nWhich].GetStyleName(); }
    inline FontFamily GetFamily( const BYTE nWhich ) const
        { return aSub[nWhich].GetFamily(); }
    inline FontItalic GetItalic( const BYTE nWhich ) const
        { return aSub[nWhich].GetItalic(); }
    inline FontPitch GetPitch( const BYTE nWhich ) const
        { return aSub[nWhich].GetPitch(); }
    inline rtl_TextEncoding GetCharSet( const BYTE nWhich ) const
        { return aSub[nWhich].GetCharSet(); }
    inline long GetHeight( const BYTE nWhich ) const
        { return aSub[nWhich].GetSize().Height(); }
    inline FontWeight GetWeight( const BYTE nWhich ) const
        { return aSub[nWhich].GetWeight(); }
    inline FontEmphasisMark GetEmphasisMark( const BYTE nWhich ) const
        { return aSub[nWhich].GetEmphasisMark(); }

    // Macht den logischen Font im OutputDevice wirksam.
    void ChgPhysFnt( ViewShell *pSh, OutputDevice *pOut );

    Size GetCapitalSize( ViewShell *pSh,
                         const OutputDevice *pOut, const XubString &rTxt,
                         const xub_StrLen nIdx = 0,
                         const xub_StrLen nLen = STRING_LEN )
        { return aSub[nActual].GetCapitalSize( pSh, pOut, rTxt, nIdx, nLen ); }

    xub_StrLen GetCapitalBreak( ViewShell *pSh,
                            const OutputDevice *pOut, const XubString &rTxt,
                            long nTextWidth, xub_StrLen *pExtra = 0,
                            const xub_StrLen nIdx = 0,
                            const xub_StrLen nLen = STRING_LEN );

    xub_StrLen GetCapitalCrsrOfst( ViewShell *pSh,
                    OutputDevice *pOut,
                    const XubString &rTxt, const USHORT nOfst,
                    const xub_StrLen nIdx, const xub_StrLen nLen,
                    const short nSpaceAdd = 0 )
        { return aSub[nActual].GetCapitalCrsrOfst( pSh, pOut, rTxt, nOfst,
                                                   nIdx, nLen, nSpaceAdd ); }


    void DrawCapital( SwDrawTextInfo &rInf )
        { aSub[nActual].DrawCapital( rInf ); }

    void DrawStretchCapital( SwDrawTextInfo &rInf )
        { aSub[nActual].DrawStretchCapital( rInf ); }

    void DoOnCapitals( SwDoCapitals &rDo )
        { aSub[nActual].DoOnCapitals( rDo ); }

    Size _GetTxtSize( ViewShell *pSh,
                     const OutputDevice *pOut, const XubString &rTxt,
                     const xub_StrLen nIdx = 0,
                     const xub_StrLen nLen = STRING_LEN )
        { return aSub[nActual]._GetTxtSize( pSh, pOut, rTxt, nIdx, nLen ); }

    xub_StrLen GetTxtBreak( ViewShell *pSh,
                const OutputDevice *pOut, const XubString &rTxt,
                long nTextWidth, xub_StrLen& rExtraCharPos,
                const xub_StrLen nIdx = 0,
                const xub_StrLen nLen = STRING_LEN );

    xub_StrLen GetTxtBreak( ViewShell *pSh,
                const OutputDevice *pOut, const XubString &rTxt,
                long nTextWidth, const xub_StrLen nIdx = 0,
                const xub_StrLen nLen = STRING_LEN );

    xub_StrLen _GetCrsrOfst( ViewShell *pSh,
                         OutputDevice *pOut, const XubString &rTxt,
                         const USHORT nOfst,
                         const xub_StrLen nIdx = 0,
                         const xub_StrLen nLen = STRING_LEN,
                         const short nSpaceAdd = 0 )
        { return aSub[nActual]._GetCrsrOfst( pSh, pOut, rTxt, nOfst,
                                             nIdx, nLen, nSpaceAdd ); }
    inline void _DrawText( SwDrawTextInfo &rInf )
        { aSub[nActual]._DrawText( rInf, IsGreyWave() ); }

    inline void _DrawStretchText( SwDrawTextInfo &rInf )
        { aSub[nActual]._DrawStretchText( rInf ); }

    inline short CheckKerning()
        { return aSub[nActual].CheckKerning(); }
    inline USHORT GetAscent( ViewShell *pSh, const OutputDevice *pOut )
        { return aSub[nActual].GetAscent( pSh, pOut ); }
    USHORT GetLeading( ViewShell *pSh, const OutputDevice *pOut );
    inline USHORT GetHeight( ViewShell *pSh, const OutputDevice *pOut )
        { return aSub[nActual].GetHeight( pSh, pOut ); }
    inline void Invalidate()
        { bFntChg = bOrgChg = TRUE; }
};

// gekapselte SV-Font-Methode
inline void SwSubFont::SetColor( const Color& rColor )
{
    pMagic = 0;
    Font::SetColor( rColor );
}

inline void SwFont::SetColor( const Color& rColor )
{
    bFntChg = TRUE;
    aSub[0].SetColor( rColor );
    aSub[1].SetColor( rColor );
    aSub[2].SetColor( rColor );
}


// gekapselte SV-Font-Methode
inline void SwSubFont::SetFillColor( const Color& rColor )
{
    pMagic = 0;
    Font::SetFillColor( rColor );
}

inline void SwFont::SetFillColor( const Color& rColor )
{
    bFntChg = TRUE;
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

inline void SwFont::SetFamily( const FontFamily eFamily, const BYTE nWhich )
{
    bFntChg = TRUE;
    aSub[nWhich].SetFamily( eFamily );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetName( const XubString& rName )
{
    pMagic = 0;
    Font::SetName( rName );
}

inline void SwFont::SetName( const XubString& rName, const BYTE nWhich )
{
    bFntChg = TRUE;
    aSub[nWhich].SetName( rName );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetStyleName( const XubString& rStyleName )
{
    pMagic = 0;
    Font::SetStyleName( rStyleName );
}

inline void SwFont::SetStyleName( const XubString& rStyle, const BYTE nWhich )
{
    bFntChg = TRUE;
    aSub[nWhich].SetStyleName( rStyle );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetCharSet( const CharSet eCharSet )
{
    pMagic = 0;
    Font::SetCharSet( eCharSet );
}

inline void SwFont::SetCharSet( const CharSet eCharSet, const BYTE nWhich )
{
    bFntChg = TRUE;
    aSub[nWhich].SetCharSet( eCharSet );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetPitch( const FontPitch ePitch )
{
    pMagic = 0;
    Font::SetPitch( ePitch );
}

// gekapselte SV-Font-Methode
inline void SwFont::SetPitch( const FontPitch ePitch, const BYTE nWhich )
{
    bFntChg = TRUE;
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
    bFntChg = TRUE;
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

inline void SwFont::SetWeight( const FontWeight eWeight, const BYTE nWhich )
{
    bFntChg = TRUE;
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
    bFntChg = TRUE;
    aSub[0].SetUnderline( eUnderline );
    aSub[1].SetUnderline( eUnderline );
    aSub[2].SetUnderline( eUnderline );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    pMagic = 0;
    Font::SetStrikeout( eStrikeout );
}

inline void SwFont::SetStrikeout( const FontStrikeout eStrikeout )
{
    bFntChg = TRUE;
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

inline void SwFont::SetItalic( const FontItalic eItalic, const BYTE nWhich )
{
    bFntChg = TRUE;
    aSub[nWhich].SetItalic( eItalic );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetOutline( const BOOL bOutline )
{
    pMagic = 0;
    Font::SetOutline( bOutline );
}

inline void SwFont::SetOutline( const BOOL bOutline )
{
    bFntChg = TRUE;
    aSub[0].SetOutline( bOutline );
    aSub[1].SetOutline( bOutline );
    aSub[2].SetOutline( bOutline );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetShadow( const BOOL bShadow )
{
    pMagic = 0;
    Font::SetShadow( bShadow );
}

inline void SwFont::SetShadow( const BOOL bShadow )
{
    bFntChg = TRUE;
    aSub[0].SetShadow( bShadow );
    aSub[1].SetShadow( bShadow );
    aSub[2].SetShadow( bShadow );
}

// gekapselte SV-Font-Methode
inline void SwSubFont::SetAutoKern( const BOOL bAutoKern )
{
    pMagic = 0;
    Font::SetKerning( bAutoKern );
}

inline void SwFont::SetAutoKern( const BOOL bAutoKern )
{
    bFntChg = TRUE;
    aSub[0].SetAutoKern( bAutoKern );
    aSub[1].SetAutoKern( bAutoKern );
    aSub[2].SetAutoKern( bAutoKern );
}

inline void SwFont::SetTransparent( const BOOL bTrans )
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
inline void SwSubFont::SetWordLineMode( const BOOL bWordLineMode )
{
    pMagic = 0;
    Font::SetWordLineMode( bWordLineMode );
}

inline void SwFont::SetWordLineMode( const BOOL bWordLineMode )
{
    bFntChg = TRUE;
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
    bFntChg = TRUE;
    aSub[0].SetEmphasisMark( eValue );
    aSub[1].SetEmphasisMark( eValue );
    aSub[2].SetEmphasisMark( eValue );
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

inline void SwFont::SetSize( const Size& rSize, const BYTE nWhich )
{
    if( aSub[nWhich].aSize != rSize )
    {
        aSub[nWhich].SetSize( rSize );
        bFntChg = TRUE;
        bOrgChg = TRUE;
    }
}

inline void SwSubFont::SetProportion( const BYTE nNewPropr )
{
    pMagic = 0;
    Font::SetSize( Size( (long) aSize.Width() * nNewPropr / 100L,
                         (long) aSize.Height() * nNewPropr / 100L ) );
    SvxFont::SetPropr( nNewPropr );
}

inline void SwFont::SetProportion( const BYTE nNewPropr )
{
    if( nNewPropr != aSub[0].GetPropr() )
    {
        bFntChg = TRUE;
        bOrgChg = TRUE;
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
        aSub[0].SetEscapement( nNewEsc );
        aSub[1].SetEscapement( nNewEsc );
        aSub[2].SetEscapement( nNewEsc );
    }
}

inline void SwSubFont::SetLanguage( LanguageType eNewLang )
{
    if( eNewLang == LANGUAGE_SYSTEM )
        eNewLang = ::GetSystemLang();
    SvxFont::SetLanguage( eNewLang );
}

inline void SwFont::SetLanguage( const LanguageType eNewLang, const BYTE nWhich )
{
    aSub[nWhich].SetLanguage( eNewLang );
}

inline void SwFont::SetPaintBlank( const BOOL bNew )
{
    bPaintBlank = bNew;
}

inline void SwFont::SetPaintWrong( const BOOL bNew )
{
    bPaintWrong = bNew;
}

inline void SwFont::SetNoHyph( const BOOL bNew )
{
    bNoHyph = bNew;
}

inline void SwFont::SetBlink( const BOOL bNew )
{
    bBlink = bNew;
}

inline void SwFont::SetURL( const BOOL bNew )
{
    bURL = bNew;
}

inline void SwFont::SetGreyWave( const BOOL bNew )
{
    bGreyWave = bNew;
}

inline void SwFont::SetNoCol( const BOOL bNew )
{
    bNoColReplace = bNew;
}

/*************************************************************************
 *                      class SvStatistics
 *************************************************************************/

#ifdef PRODUCT
#define SV_STAT(nWhich)
#else

class SvStatistics
{
public:
    USHORT nGetTextSize;
    USHORT nDrawText;
    USHORT nGetStretchTextSize;
    USHORT nDrawStretchText;
    USHORT nChangeFont;
    USHORT nGetFontMetric;

    inline void Reset()
    {
        nGetTextSize = nDrawText = nGetStretchTextSize =
        nDrawStretchText = nChangeFont = nGetFontMetric = 0;
    }

    inline SvStatistics() { Reset(); }

    inline void PrintOn( SvStream &rOS ) const; //$ ostream
    inline BOOL IsEmpty() const
    {
        return !( nGetTextSize || nDrawText ||
                  nDrawStretchText || nChangeFont || nGetFontMetric );
    }
};

// globale Variable, implementiert in swfont.cxx
extern SvStatistics aSvStat;

#define SV_STAT(nWhich) ++(aSvStat.nWhich);

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
#endif  /*  PRODUCT */

#endif

