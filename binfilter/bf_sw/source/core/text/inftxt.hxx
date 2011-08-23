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
#ifndef _INFTXT_HXX
#define _INFTXT_HXX

#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#include "swtypes.hxx"
#include "txttypes.hxx"
#include "swrect.hxx"

#include "txtfly.hxx"
#include "swfont.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "ndtxt.hxx"
#include "txttypes.hxx"

#ifndef _SVX_PARAVERTALIGNITEM_HXX //autogen
#include <bf_svx/paravertalignitem.hxx>
#endif
class Font; 
class OutputDevice; 
namespace binfilter {


class SvxBrushItem;
class SvxLineSpacingItem;
class SvxTabStop;
class SvxTabStopItem;
class SwAttrSet;
class SwField;
class SwFldPortion;
class SwFlyPortion;
class SwFmtDrop;
class SwFtnPortion;
class SwLineHeightRule;
class SwLineLayout;
class SwLinePortion;
class SwLineSpaceRule;
class SwParaPortion;
class SwTabPortion;
class SwTxtFrm;
class SwTxtSizeInfo;
class SwViewOption;
class ViewShell;
class SwTxtFtn;
class SwAttrIter;
struct SwMultiCreator;

#ifdef BIDI
class SwMultiPortion;
#endif

/* Minimum: Prozentwert fuers kernen */
#define MINKERNPERCENT 5
#define ARROW_WIDTH	200
#define DIR_LEFT2RIGHT 0
#define DIR_BOTTOM2TOP 1
#define DIR_RIGHT2LEFT 2
#define DIR_TOP2BOTTOM 3

#ifdef DBG_UTIL
#define OPTCALM( rInf )  (rInf).IsOptCalm()
#define OPTLOW( rInf )	 (rInf).IsOptLow()
#define OPTDBG( rInf )	 (rInf).IsOptDbg()
#else
#define OPTCALM( rInf )  sal_True
#define OPTLOW( rInf )	 sal_False
#define OPTDBG( rInf )	 sal_False
#endif

/*************************************************************************
 *						class SwLineInfo
 *************************************************************************/

// Beruecksichtigt das Attribut LineSpace bei der Hoehen/Ascentberechnung.

class SwLineInfo
{
    friend class SwTxtIter;

    const SvxTabStopItem	*pRuler;
    const SvxLineSpacingItem *pSpace;
    USHORT nVertAlign;
    KSHORT nDefTabStop;
    void CtorInit( const SwAttrSet& rAttrSet );
    inline SwLineInfo() {}
public:
    inline SwLineInfo( const SwAttrSet& rAttrSet )
           { CtorInit( rAttrSet ); }
    // Liefert den Tabstop, der auf LinePos folgt, oder 0.
    const SvxTabStop *GetTabStop( const SwTwips nLinePos,
                                 const SwTwips nLeft,
                                 const SwTwips nRight ) const;
    inline const SvxLineSpacingItem *GetLineSpacing() const { return pSpace; }
    inline KSHORT GetDefTabStop() const { return nDefTabStop; }
    inline void SetDefTabStop( KSHORT nNew ) const
        { ( (SwLineInfo*)this )->nDefTabStop = nNew; }

    // vertical alignment
    inline USHORT GetVertAlign() const { return nVertAlign; }
#ifdef BIDI
    inline sal_Bool HasSpecialAlign( sal_Bool bVert ) const
        { return bVert ?
                 ( SvxParaVertAlignItem::BASELINE  != nVertAlign ) :
                 ( SvxParaVertAlignItem::BASELINE  != nVertAlign &&
                   SvxParaVertAlignItem::AUTOMATIC != nVertAlign ); }
#else
    inline sal_Bool HasSpecialAlign() const
        { return SvxParaVertAlignItem::BASELINE  != nVertAlign ||
                 SvxParaVertAlignItem::AUTOMATIC != nVertAlign; }
#endif

//  friend ostream &operator<<( ostream &rOS, const SwLineInfo &rInf );
    friend SvStream &operator<<( SvStream &rOS, const SwLineInfo &rInf );
};

/*************************************************************************
 *						class SwTxtInfo
 *************************************************************************/

class SwTxtInfo
{
    // Implementation in txthyph.cxx
    friend void SetParaPortion( SwTxtInfo *pInf, SwParaPortion *pRoot );
    SwParaPortion *pPara;
    xub_StrLen nTxtStart;				  // TxtOfst bei Follows

protected:
    inline SwTxtInfo() { }
public:
    void CtorInit( SwTxtFrm *pFrm );
    SwTxtInfo( const SwTxtInfo &rInf );
    inline SwTxtInfo( SwTxtFrm *pFrm ) { CtorInit( pFrm ); }
    inline SwParaPortion *GetParaPortion() { return pPara; }
    inline const SwParaPortion *GetParaPortion() const { return pPara; }
    inline xub_StrLen GetTxtStart() const { return nTxtStart; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtInfo &rInf );
};

/*************************************************************************
 *						class SwTxtSizeInfo
 *************************************************************************/

DECLARE_TABLE( SwTxtPortionTable, sal_IntPtr )

class SwTxtSizeInfo : public SwTxtInfo
{
protected:
    // during formatting, a small database is built, mapping portion pointers
    // to their maximum size (used for kana compression)
    SwTxtPortionTable aMaxWidth;
    // for each line, an array of compression values is calculated
    // this array is passed over to the info structure
    SvUShorts* pKanaComp;

    ViewShell	 *pVsh;

    // pOut is the output device, pRef is the device used for formatting
    OutputDevice *pOut;
    OutputDevice *pRef;

    SwFont *pFnt;
    SwUnderlineFont *pUnderFnt; // Font for underlining
    SwTxtFrm *pFrm;
    const SwViewOption *pOpt;
    const XubString *pTxt;
    xub_StrLen nIdx, nLen;
    USHORT nKanaIdx;
    sal_Bool bOnWin     : 1;
    sal_Bool bNotEOL	: 1;
    sal_Bool bURLNotify : 1;
    sal_Bool bStopUnderFlow : 1;// Underflow gestoppt z.B. von einer FlyPortion
    sal_Bool bArrowDone : 1;	// Pfeil nach links bei gescrollten Absaetzen
    sal_Bool bFtnInside : 1;	// the current line contains a footnote
    sal_Bool bMulti : 1;		// inside a multiportion
    sal_Bool bFirstMulti : 1;   // this flag is used for two purposes:
                                // - the multiportion is the first lineportion
                                // - indicates, if we are currently in second
                                //   line of multi portion
    sal_Bool bRuby : 1;			// during the formatting of a phonetic line
    sal_Bool bHanging : 1;		// formatting of hanging punctuation allowed
    sal_Bool bScriptSpace : 1;	// space between different scripts (Asian/Latin)
    sal_Bool bForbiddenChars : 1; // Forbidden start/endline characters
#ifdef VERTICAL_LAYOUT
    sal_Bool bSnapToGrid : 1;   // paragraph snaps to grid
#endif
    sal_uInt8 nDirection : 2;		// writing direction: 0/90/180/270 degree

protected:
    void CtorInit( SwTxtFrm *pFrm, SwFont *pFnt = 0,
                   const xub_StrLen nIdx = 0,
                   const xub_StrLen nLen = STRING_LEN );
    SwTxtSizeInfo() {}
public:
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf );
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf, const XubString &rTxt,
                   const xub_StrLen nIdx = 0,
                   const xub_StrLen nLen = STRING_LEN );

    inline SwTxtSizeInfo( SwTxtFrm *pFrm, SwFont *pFnt = 0,
                   const xub_StrLen nIdx = 0,
                   const xub_StrLen nLen = STRING_LEN )
           { CtorInit( pFrm, pFnt, nIdx, nLen ); }

    // GetMultiAttr returns the text attribute of the multiportion,
    // if rPos is inside any multi-line part.
    // rPos will set to the end of the multi-line part.
#ifdef BIDI
    SwMultiCreator* GetMultiCreator( xub_StrLen &rPos, SwMultiPortion* pM ) const;
#else
    SwMultiCreator* GetMultiCreator( xub_StrLen &rPos ) const;
#endif

    inline sal_Bool OnWin() const { return bOnWin; }
    inline void SetOnWin( const sal_Bool bNew ) { bOnWin = bNew; }
    inline sal_Bool NotEOL() const { return bNotEOL; }
    inline void SetNotEOL( const sal_Bool bNew ) { bNotEOL = bNew; }
    inline sal_Bool URLNotify() const { return bURLNotify; }
    inline void SetURLNotify( const sal_Bool bNew ) { bURLNotify = bNew; }
    inline sal_Bool StopUnderFlow() const { return bStopUnderFlow; }
    inline void SetStopUnderFlow( const sal_Bool bNew ) { bStopUnderFlow = bNew; }
    inline sal_Bool IsFtnInside() const { return bFtnInside; }
    inline void SetFtnInside( const sal_Bool bNew ) { bFtnInside = bNew; }
    inline sal_Bool IsMulti() const { return bMulti; }
    inline void SetMulti( const sal_Bool bNew ) { bMulti = bNew; }
    inline sal_Bool IsFirstMulti() const { return bFirstMulti; }
    inline void SetFirstMulti( const sal_Bool bNew ) { bFirstMulti = bNew; }
    inline sal_Bool IsRuby() const { return bRuby; }
    inline void SetRuby( const sal_Bool bNew ) { bRuby = bNew; }
    inline sal_Bool IsHanging() const { return bHanging; }
    inline void SetHanging( const sal_Bool bNew ) { bHanging = bNew; }
    inline sal_Bool HasScriptSpace() const { return bScriptSpace; }
    inline void SetScriptSpace( const sal_Bool bNew ) { bScriptSpace = bNew; }
    inline sal_Bool HasForbiddenChars() const { return bForbiddenChars; }
    inline void SetForbiddenChars( const sal_Bool bN ) { bForbiddenChars = bN; }
#ifdef VERTICAL_LAYOUT
    inline sal_Bool SnapToGrid() const { return bSnapToGrid; }
    inline void SetSnapToGrid( const sal_Bool bN ) { bSnapToGrid = bN; }
#endif
    inline sal_uInt8 GetDirection() const { return nDirection; }
    inline void SetDirection( const sal_uInt8 nNew ) { nDirection = nNew; }
    inline sal_Bool IsRotated() const { return 0 != ( 1 & nDirection ); }

    inline ViewShell *GetVsh() { return pVsh; }
    inline const ViewShell *GetVsh() const { return pVsh; }

    inline OutputDevice *GetOut() { return pOut; }
    inline const OutputDevice *GetOut() const { return pOut; }
    inline void SetOut( OutputDevice* pNewOut ) { pOut = pNewOut; }

    inline OutputDevice *GetRefDev() { return pRef; }
    inline const OutputDevice *GetRefDev() const { return pRef; }

    inline SwFont *GetFont() { return pFnt; }
    inline const SwFont *GetFont() const { return pFnt; }
    inline void SetFont( SwFont *pNew ) { pFnt = pNew; }
    void SelectFont();
    inline void SetUnderFnt( SwUnderlineFont* pNew ) { pUnderFnt = pNew; }
    inline SwUnderlineFont* GetUnderFnt() const { return pUnderFnt; }

    inline const  SwViewOption &GetOpt() const { return *pOpt; }
    inline const XubString &GetTxt() const { return *pTxt; }
    inline xub_Unicode GetChar( const xub_StrLen nPos ) const
        { return pTxt->GetChar( nPos ); }

    inline KSHORT	   GetTxtHeight() const;

    //
    // GetTxtSize
    //
    SwPosSize GetTxtSize( OutputDevice *pOut, const SwScriptInfo* pSI,
                          const XubString& rTxt, const xub_StrLen nIdx,
                          const xub_StrLen nLen, const USHORT nComp ) const;
    SwPosSize GetTxtSize() const;
    void GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
                      const xub_StrLen nLen, const USHORT nComp,
                      USHORT& nMinSize, USHORT& nMaxSizeDiff ) const;
    inline SwPosSize GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
                                 const xub_StrLen nLen, const USHORT nComp ) const;
    inline SwPosSize GetTxtSize( const XubString &rTxt ) const;

    //
    // GetTxtBreak
    //
    xub_StrLen GetTxtBreak( const long nLineWidth,
                                           const xub_StrLen nMaxLen,
                                           const USHORT nComp ) const;
    xub_StrLen GetTxtBreak( const long nLineWidth,
                                           const xub_StrLen nMaxLen,
                                           const USHORT nComp,
                                           xub_StrLen& rExtraCharPos ) const;

    inline KSHORT GetAscent() const;

    inline xub_StrLen GetIdx() const { return nIdx; }
    inline void SetIdx( const xub_StrLen nNew ) { nIdx = nNew; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline void SetLen( const xub_StrLen nNew ) { nLen = nNew; }
    inline void SetTxt( const XubString &rNew ){ pTxt = &rNew; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtSizeInfo &rInf );

// 7780: Keine Bullets beim Symbol-Zeichensatz!
    inline sal_Bool IsNoSymbol() const
    { return RTL_TEXTENCODING_SYMBOL != pFnt->GetCharSet( pFnt->GetActual() ); }


    // Home is where Your heart is...
    inline SwTxtFrm *GetTxtFrm() { return pFrm; }
    inline const SwTxtFrm *GetTxtFrm() const { return pFrm; }

    inline sal_Bool HasHint( xub_StrLen nPos ) const
        { return _HasHint( pFrm->GetTxtNode(), nPos ); }
    static sal_Bool _HasHint( const SwTxtNode* pTxtNode, xub_StrLen nPos );

    inline SwDoc* GetDoc() const { return pFrm->GetNode()->GetDoc(); };

    //
    // If Kana Compression is enabled, a minimum and maximum portion width
    // is calculated. We format lines with minimal size and share remaining
    // space among compressed kanas.
    // During formatting, the maximum values of compressable portions are
    // stored in aMaxWidth and discarded after a line has been formatted.
    inline void SetMaxWidthDiff( ULONG nKey, USHORT nVal )
    {
        aMaxWidth.Insert( nKey, nVal );
    };
    inline USHORT GetMaxWidthDiff( ULONG nKey )
    {
        return (USHORT)aMaxWidth.Get( nKey );
    };
    inline void ResetMaxWidthDiff()
    {
        aMaxWidth.Clear();
    };
    inline sal_Bool CompressLine()
    {
        return (sal_Bool)aMaxWidth.Count();
    };

    //
    // Feature: Kana Compression
    //
    inline MSHORT GetKanaIdx() const { return nKanaIdx; }
    inline void ResetKanaIdx(){ nKanaIdx = 0; }
    inline void SetKanaIdx( MSHORT nNew ) { nKanaIdx = nNew; }
    inline void IncKanaIdx() { ++nKanaIdx; }
    inline void SetKanaComp( SvUShorts *pNew ){ pKanaComp = pNew; }
    inline SvUShorts* GetpKanaComp() const { return pKanaComp; }
    inline USHORT GetKanaComp() const
        { return ( pKanaComp && nKanaIdx < pKanaComp->Count() )
                   ? (*pKanaComp)[nKanaIdx] : 0; }

#ifdef DBG_UTIL
#endif
};

/*************************************************************************
 *						class SwTxtPaintInfo
 *************************************************************************/

class SwTxtPaintInfo : public SwTxtSizeInfo
{
    SwWrongList *pWrongList;
    SvShorts 	*pSpaceAdd;
    const SvxBrushItem *pBrushItem; // Fuer den Hintergrund
    SwRect      aItemRect;			// ebenfalls fuer den Hintergrund
    SwTxtFly	aTxtFly;	// FlyFrm-Berechnung
    Point		aPos;		// Ausgabeposition
    SwRect		aPaintRect;	// Original Ausgaberechteck (aus Layout-Paint)

    MSHORT nSpaceIdx;

    SwTxtPaintInfo &operator=(const SwTxtPaintInfo&);

protected:
#ifndef DBG_UTIL
    SwTxtPaintInfo() { pFrm = 0; pWrongList = 0; pSpaceAdd = 0; pBrushItem = 0;}
#else
    SwTxtPaintInfo() { pFrm = 0; pWrongList = 0; pSpaceAdd = 0;
                       pBrushItem = ((SvxBrushItem*)-1);}
#endif
public:
    SwTxtPaintInfo( const SwTxtPaintInfo &rInf );

    void CtorInit( SwTxtFrm *pFrame, const SwRect &rPaint );

    void SetBack( const SvxBrushItem *pItem,
                  const SwRect &rRect ) { pBrushItem = pItem; aItemRect = rRect;}
    const SvxBrushItem *GetBrushItem() const { return pBrushItem; }
    const SwRect       &GetBrushRect() const { return aItemRect;  }

    inline SwTxtPaintInfo( SwTxtFrm *pFrame, const SwRect &rPaint )
           { CtorInit( pFrame, rPaint ); }

    inline SwTwips X() const { return aPos.X(); }
    inline void X( const long nNew ) { aPos.X() = nNew; }
    inline SwTwips Y() const { return aPos.Y(); }
    inline void Y( const SwTwips nNew ) { aPos.Y() = nNew; }

    inline SwTxtFly *GetTxtFly() { return &aTxtFly; }
    inline const SwTxtFly *GetTxtFly() const { return &aTxtFly; }
    inline void DrawText( const XubString &rText, const SwLinePortion &rPor,
                          const xub_StrLen nIdx = 0,
                          const xub_StrLen nLen = STRING_LEN,
                          const sal_Bool bKern = sal_False) const;
    inline void DrawText( const SwLinePortion &rPor, const xub_StrLen nLen,
                          const sal_Bool bKern = sal_False ) const;
    inline void DrawWrongText( const SwLinePortion &rPor, const xub_StrLen nLen,
                          const sal_Bool bKern = sal_False ) const;
#ifdef VERTICAL_LAYOUT
#endif


    inline SwTwips GetPaintOfst() const;
    inline void SetPaintOfst( const SwTwips nNew );
    inline const Point &GetPos() const { return aPos; }
    inline void SetPos( const Point &rNew ) { aPos = rNew; }

    inline const SwRect &GetPaintRect() const { return aPaintRect; }
    inline void SetPaintRect( const SwRect &rNew ) { aPaintRect = rNew; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtPaintInfo &rInf );

    inline MSHORT GetSpaceIdx() const { return nSpaceIdx; }
    inline void ResetSpaceIdx(){nSpaceIdx = 0; }
    inline void SetSpaceIdx( MSHORT nNew ) { nSpaceIdx = nNew; }
    inline void IncSpaceIdx() { ++nSpaceIdx; }
    inline void SetSpaceAdd( SvShorts *pNew ){ pSpaceAdd = pNew; }
    inline SvShorts* GetpSpaceAdd() const { return pSpaceAdd; }
    inline short GetSpaceAdd() const
        { return ( pSpaceAdd && nSpaceIdx < pSpaceAdd->Count() )
                   ? (*pSpaceAdd)[nSpaceIdx] : 0; }

    inline void SetWrongList( SwWrongList *pNew ){ pWrongList = pNew; }
    inline SwWrongList* GetpWrongList() const { return pWrongList; }

};

/*************************************************************************
 *						class SwTxtFormatInfo
 *************************************************************************/

class SwTxtFormatInfo : public SwTxtPaintInfo
{
    // temporary arguments for hyphenation
    ::com::sun::star::beans::PropertyValues	aHyphVals;

    SwLineLayout	*pRoot; 	  // die Root der aktuellen Zeile (pCurr)
    SwLinePortion	*pLast; 	  // die letzte Portion
    SwFlyPortion	*pFly;		  // die nachfolgende FlyPortion
    SwFldPortion	*pLastFld;	  // umgebrochenes Feld
    SwLinePortion	*pUnderFlow;  // Unterlaufsituation: letzte Portion
    SwLinePortion	*pRest; 	  // Rest ist der Beginn der naechsten Zeile

    SwTabPortion	*pLastTab;	   // die _letzte_ TabPortion

    xub_StrLen nSoftHyphPos;    // SoftHyphPos fuer Hyphenate
    xub_StrLen nHyphStart;	    // TxtPos, an der die interakt.Tr.z.Z. steht
    xub_StrLen nHyphWrdStart;   // gefundene Wort-Position
    xub_StrLen nHyphWrdLen;     // gefundene Wort-Laenge
    xub_StrLen nLineStart;		// aktueller Zeilenbeginn im rTxt
    xub_StrLen nUnderScorePos;  // enlarge repaint if underscore has been found
    KSHORT nLeft;			// linker Rand
    KSHORT nRight;			// rechter Rand
    KSHORT nFirst;			// EZE
    KSHORT nRealWidth;		// "echte" Zeilenbreite
    KSHORT nWidth;			// "virtuelle" Zeilenbreite
    KSHORT nLineHeight; 	// endgueltige Hoehe nach CalcLine
    KSHORT nLineNettoHeight;   // line height without spacing
    KSHORT nForcedLeftMargin;	// Verschiebung des linken Rands wg. Rahmen

    INT16  nMinLeading;		// minimum number of chars before hyphenation point
    INT16  nMinTrailing;	// minimum number of chars after hyphenation point
    INT16  nMinWordLength;	// minimum length of word to be hyphenated

    sal_Bool bFull	 : 1;	   // Zeile ist voll
    sal_Bool bFtnDone  : 1;	   // Ftn bereits formatiert
    sal_Bool bErgoDone : 1;	   // ErgoDone bereits formatiert
    sal_Bool bNumDone  : 1;	   // bNumDone bereits formatiert
    sal_Bool bStop	 : 1;	   // Sofort abbrechen, Zeile verwerfen.
    sal_Bool bNewLine  : 1;	   // Noch eine weitere Zeile formatieren.
    sal_Bool bShift	 : 1;	   // Positionsaend.: Repaint bis auf Weiteres
    sal_Bool bUnderFlow : 1;	   // Kontext: UnderFlow() ?
    sal_Bool bInterHyph: 1;	   // interaktive Trennung ?
    sal_Bool bAutoHyph : 1;	   // automatische Trennung ?
    sal_Bool bDropInit : 1;	   // DropWidth einstellen.
    sal_Bool bQuick	 : 1;	   // FormatQuick()
    sal_Bool bNoEndHyph	 : 1;  // Trennung am Zeilenende abgeschaltet wg. MaxHyphens
    sal_Bool bNoMidHyph	 : 1;  // Trennung vor Flies abgeschaltet wg. MaxHyphens
    sal_Bool bIgnoreFly: 1;	   // FitToContent ignoriert Flies
    sal_Bool bFakeLineStart: 1; // String has been replaced by field portion
                                // info structure only pretends that we are at
                                // the beginning of a line

    xub_Unicode   cTabDecimal;	// das _aktuelle_ Dezimalzeichen
    xub_Unicode   cHookChar;	// fuer Tabs in Feldern etc.
    sal_uInt8   nMaxHyph;		// max. Zeilenanz. aufeinanderfolg. Trenn.
    sal_Bool   bTestFormat;		// Testformatierung aus WouldFit, keine Benachrichtigungen etc.

    // Hyphenating ...
    sal_Bool InitHyph( const sal_Bool bAuto = sal_False );
    sal_Bool _CheckFtnPortion( SwLineLayout* pCurr );

public:
    void CtorInit( SwTxtFrm *pFrm, const sal_Bool bInterHyph = sal_False,
        const sal_Bool bQuick = sal_False, const sal_Bool bTst = sal_False );
    inline SwTxtFormatInfo(SwTxtFrm *pFrame,const sal_Bool bInterHyph=sal_False,
            const sal_Bool bQuick = sal_False, const sal_Bool bTst = sal_False )
           { CtorInit( pFrame, bInterHyph, bQuick, bTst ); }

    // For the formatting inside a double line in a line (multi-line portion)
    // we need a modified text-format-info:

    inline KSHORT Width() const { return nWidth; }
    inline void Width( const KSHORT nNew ) { nWidth = nNew; }
           void Init();

    // liefert die erste veraenderte Position im Absatz zurueck
    inline xub_StrLen GetReformatStart() const;

    // Raender
    inline KSHORT Left() const { return nLeft; }
    inline void Left( const KSHORT nNew ) { nLeft = nNew; }
    inline KSHORT Right() const { return nRight; }
    inline void Right( const KSHORT nNew ) { nRight = nNew; }
    inline KSHORT First() const { return nFirst; }
    inline void First( const KSHORT nNew ) { nFirst = nNew; }
    inline KSHORT CurrLeft() const { return (nLineStart ? nLeft : nFirst); }
    inline KSHORT RealWidth() const { return nRealWidth; }
    inline void RealWidth( const KSHORT nNew ) { nRealWidth = nNew; }
    inline KSHORT ForcedLeftMargin() const { return nForcedLeftMargin; }
    inline void ForcedLeftMargin( const KSHORT nN ) { nForcedLeftMargin = nN; }

    inline sal_uInt8 &MaxHyph() { return nMaxHyph; }
    inline const sal_uInt8 &MaxHyph() const { return nMaxHyph; }

    inline SwLineLayout *GetRoot() { return pRoot; }
    inline const SwLineLayout *GetRoot() const { return pRoot; }

    inline void SetRoot( SwLineLayout *pNew ) { pRoot = pNew; }
    inline SwLinePortion *GetLast() { return pLast; }
    inline void SetLast( SwLinePortion *pNewLast ) { pLast = pNewLast; }
    inline sal_Bool IsFull() const { return bFull; }
    inline void SetFull( const sal_Bool bNew ) { bFull = bNew; }
    inline sal_Bool IsHyphForbud() const
        { return pFly ? bNoMidHyph : bNoEndHyph; }
    inline void SetHyphForbud( const sal_Bool bNew )
        { if ( pFly ) bNoMidHyph = bNew; else bNoEndHyph = bNew; }
    inline void ChkNoHyph( const sal_uInt8 bEnd, const sal_uInt8 bMid )
        { bNoEndHyph = (nMaxHyph && bEnd >= nMaxHyph);
          bNoMidHyph = (nMaxHyph && bMid >= nMaxHyph); }
    inline sal_Bool IsIgnoreFly() const { return bIgnoreFly; }
    inline void SetIgnoreFly( const sal_Bool bNew ) { bIgnoreFly = bNew; }
    inline sal_Bool IsFakeLineStart() const { return bFakeLineStart; }
    inline void SetFakeLineStart( const sal_Bool bNew ) { bFakeLineStart = bNew; }
    inline sal_Bool IsStop() const { return bStop; }
    inline void SetStop( const sal_Bool bNew ) { bStop = bNew; }
    inline SwLinePortion *GetRest() { return pRest; }
    inline void SetRest( SwLinePortion *pNewRest ) { pRest = pNewRest; }
    inline sal_Bool IsNewLine() const { return bNewLine; }
    inline void SetNewLine( const sal_Bool bNew ) { bNewLine = bNew; }
    inline sal_Bool IsShift() const { return bShift; }
    inline void SetShift( const sal_Bool bNew ) { bShift = bNew; }
    inline sal_Bool IsInterHyph() const { return bInterHyph; }
    inline sal_Bool IsAutoHyph() const { return bAutoHyph; }
    inline sal_Bool IsUnderFlow() const { return bUnderFlow; }
    inline void ClrUnderFlow() { bUnderFlow = sal_False; }
    inline sal_Bool IsDropInit() const { return bDropInit; }
    inline void SetDropInit( const sal_Bool bNew ) { bDropInit = bNew; }
    inline sal_Bool IsQuick() const { return bQuick; }
    inline sal_Bool IsTest() const { return bTestFormat; }

    inline xub_StrLen GetLineStart() const { return nLineStart; }
    inline void SetLineStart( const xub_StrLen nNew ) { nLineStart = nNew; }

    // these are used during fly calculation
    inline KSHORT GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const KSHORT nNew ) { nLineHeight = nNew; }
    inline KSHORT GetLineNettoHeight() const { return nLineNettoHeight; }
    inline void SetLineNettoHeight( const KSHORT nNew ) { nLineNettoHeight = nNew; }

    inline const SwLinePortion *GetUnderFlow() const { return pUnderFlow; }
    inline SwLinePortion *GetUnderFlow() { return pUnderFlow; }
    inline void SetUnderFlow( SwLinePortion *pNew )
           { pUnderFlow = pNew; bUnderFlow = sal_True; }
    inline xub_StrLen GetSoftHyphPos() const { return nSoftHyphPos; }
    inline void SetSoftHyphPos( const xub_StrLen nNew ) { nSoftHyphPos = nNew; }

    inline void SetParaFtn();

    // FlyFrms
    inline SwFlyPortion *GetFly() { return pFly; }
    inline void SetFly( SwFlyPortion *pNew ) { pFly = pNew; }

    inline const SwAttrSet& GetCharAttr() const;

    // Tabs
    inline SwTabPortion *GetLastTab() { return pLastTab; }
    inline void SetLastTab( SwTabPortion *pNew ) { pLastTab = pNew; }
    inline xub_Unicode GetTabDecimal() const { return cTabDecimal; }
    inline void SetTabDecimal( const xub_Unicode cNew ) { cTabDecimal = cNew;}

    // Last*
    inline SwFldPortion *GetLastFld() { return pLastFld; }
    inline void SetLastFld( SwFldPortion *pNew ) { pLastFld = pNew; }

    inline void ClearHookChar() { cHookChar = 0; }
    inline void SetHookChar( const xub_Unicode cNew ) { cHookChar = cNew; }
    inline xub_Unicode GetHookChar() const { return cHookChar; }

    // Done-Flags
    inline sal_Bool IsFtnDone() const { return bFtnDone; }
    inline void SetFtnDone( const sal_Bool bNew ) { bFtnDone = bNew; }
    inline sal_Bool IsErgoDone() const { return bErgoDone; }
    inline void SetErgoDone( const sal_Bool bNew ) { bErgoDone = bNew; }
    inline sal_Bool IsNumDone() const { return bNumDone; }
    inline void SetNumDone( const sal_Bool bNew ) { bNumDone = bNew; }
    inline sal_Bool IsArrowDone() const { return bArrowDone; }
    inline void SetArrowDone( const sal_Bool bNew ) { bArrowDone = bNew; }

    // Fuer SwTxtPortion::Hyphenate
    inline sal_Bool IsSoftHyph( const xub_StrLen nPos ) const;
    sal_Bool ChgHyph( const sal_Bool bNew );

    // Soll die Trennhilfe angeschmissen werden?
    sal_Bool IsHyphenate() const;
    inline void SetHyphStart( const xub_StrLen nNew ) { nHyphStart = nNew; }
    inline xub_StrLen GetHyphStart() const { return nHyphStart; }
    inline void SetHyphWrdStart( const xub_StrLen nNew ) { nHyphWrdStart = nNew; }
    inline xub_StrLen GetHyphWrdStart() const { return nHyphWrdStart; }
    inline void SetHyphWrdLen( const xub_StrLen nNew ) { nHyphWrdLen = nNew; }
    inline xub_StrLen GetHyphWrdLen() const { return nHyphWrdLen; }
    inline xub_StrLen GetUnderScorePos() const { return nUnderScorePos; }
    inline void SetUnderScorePos( xub_StrLen nNew ) { nUnderScorePos = nNew; }

    // ruft HyphenateWord() des Hyphenators
    const ::com::sun::star::beans::PropertyValues	&
                GetHyphValues() const;

    sal_Bool CheckFtnPortion( SwLineLayout* pCurr )
        { return IsFtnInside() && _CheckFtnPortion( pCurr ); }

    // Dropcaps vom SwTxtFormatter::CTOR gerufen.
    const SwFmtDrop *GetDropFmt() const;

    // setzt die FormatInfo wieder in den Anfangszustand
    void Reset( const SwTxtFrm *pFrame); // , const sal_Bool bAll );

    // Sets the last SwKernPortion as pLast, if it is followed by empty portions
    BOOL LastKernPortion();

    // Sucht ab nIdx bis nEnd nach Tabs, TabDec, TXTATR und BRK.
    // Return: gefundene Position, setzt ggf. cHookChar
    xub_StrLen ScanPortionEnd( const xub_StrLen nStart, const xub_StrLen nEnd );

//	friend ostream &operator<<( ostream &rOS, const SwTxtFormatInfo &rInf );
    friend SvStream &operator<<( SvStream &rOS, const SwTxtFormatInfo &rInf );
};

/*************************************************************************
 *						class SwTxtSlot
 *************************************************************************/

// Fuer die Textersetzung und Restaurierung der SwTxtSizeInfo.
// Die Art und Weise ist etwas kriminell, rInf ist const und wird
// trotzdem veraendert. Da rInf im DTOR wieder restauriert wird,
// ist dies zulaessig, es handelt sich um ein "logisches const".
// Die beiden Klassen SwTxtSlot und SwTxtSlotLen sind Zwillinge, sie
// unterscheiden sich nur im Ctor in der Zuweisung der Textlaenge
// an pInf. Aenderungen muessen in beiden gepflegt werden!

class SwTxtSlot
{
    const XubString *pOldTxt;
    XubString aTxt;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    sal_Bool bOn;
protected:
    SwTxtSizeInfo *pInf;
public:
    SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor );
    ~SwTxtSlot();
    inline sal_Bool IsOn() const { return bOn; }
};

class SwTxtSlotLen
{
    const XubString *pOldTxt;
    XubString aTxt;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    sal_Bool bOn;
protected:
    SwTxtSizeInfo *pInf;
public:
    // Der Ersetzungstring kommt wahlweise aus der Portion via GetExpText()
    // oder aus dem char Pointer pCh, wenn dieser ungleich NULL ist.
    SwTxtSlotLen( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor,
                  const sal_Char *pCh = NULL );
    ~SwTxtSlotLen();
    inline sal_Bool IsOn() const { return bOn; }
};

/*************************************************************************
 *						class SwFontSave
 *************************************************************************/

class SwFontSave
{
    SwTxtSizeInfo *pInf;
    SwFont		  *pFnt;
    SwAttrIter	  *pIter;
public:
    SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pFnt,
                SwAttrIter* pItr = NULL );
   ~SwFontSave();
};

/*************************************************************************
 *						class SwDefFontSave
 *************************************************************************/


/*************************************************************************
 *						class SwFtnSave
 *************************************************************************/

class SwFtnSave
{
    SwTxtSizeInfo *pInf;
    SwFont		 *pFnt;
    SwFont		 *pOld;
public:
    SwFtnSave( const SwTxtSizeInfo &rInf, const SwTxtFtn *pTxtFtn );
   ~SwFtnSave();
};

/*************************************************************************
 *                       Inline-Implementierungen SwTxtSizeInfo
 *************************************************************************/

inline KSHORT SwTxtSizeInfo::GetAscent() const
{
    return ((SwFont*)GetFont())->GetAscent( pVsh, GetOut() );
}

inline KSHORT SwTxtSizeInfo::GetTxtHeight() const
{
    return ((SwFont*)GetFont())->GetHeight( pVsh, GetOut() );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const XubString &rTxt ) const
{
    return GetTxtSize( pOut, 0, rTxt, 0, rTxt.Len(), 0 );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI,
                                            const xub_StrLen nNewIdx,
                                            const xub_StrLen nNewLen,
                                            const USHORT nCompress ) const
{
    return GetTxtSize( pOut, pSI, *pTxt, nNewIdx, nNewLen, nCompress );
}

/*************************************************************************
 *                       Inline-Implementierungen SwTxtPaintInfo
 *************************************************************************/

inline SwTwips SwTxtPaintInfo::GetPaintOfst() const
{
    return GetParaPortion()->GetRepaint()->GetOfst();
}

inline void SwTxtPaintInfo::SetPaintOfst( const SwTwips nNew )
{
    GetParaPortion()->GetRepaint()->SetOfst( nNew );
}


inline void SwTxtPaintInfo::DrawText( const XubString &rText,
                            const SwLinePortion &rPor,
                            const xub_StrLen nStart, const xub_StrLen nLen,
                            const sal_Bool bKern ) const
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	((SwTxtPaintInfo*)this)->_DrawText( rText, rPor, nStart, nLen, bKern );
}

inline void SwTxtPaintInfo::DrawText( const SwLinePortion &rPor,
        const xub_StrLen nLen, const sal_Bool bKern ) const
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001((SwTxtPaintInfo*)this)->_DrawText( *pTxt, rPor, nIdx, nLen, bKern );
}

inline void SwTxtPaintInfo::DrawWrongText( const SwLinePortion &rPor,
                                const xub_StrLen nLen, const sal_Bool bKern ) const
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	((SwTxtPaintInfo*)this)->_DrawText( *pTxt, rPor, nIdx, nLen, bKern, sal_True );
}

/*************************************************************************
 *                       Inline-Implementierungen SwTxtFormatInfo
 *************************************************************************/

inline xub_StrLen SwTxtFormatInfo::GetReformatStart() const
{
    return GetParaPortion()->GetReformat()->Start();
}

inline const SwAttrSet& SwTxtFormatInfo::GetCharAttr() const
{
    return GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
}

inline void SwTxtFormatInfo::SetParaFtn()
{
    GetTxtFrm()->SetFtn( sal_True );
}

inline sal_Bool SwTxtFormatInfo::IsSoftHyph( const xub_StrLen nPos ) const
{
    return CHAR_SOFTHYPHEN == GetTxtFrm()->GetTxtNode()->GetTxt().GetChar(nPos);
}



} //namespace binfilter
#endif

