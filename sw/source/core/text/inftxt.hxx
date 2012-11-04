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
#ifndef _INFTXT_HXX
#define _INFTXT_HXX
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <map>

#include "swtypes.hxx"
#include "txttypes.hxx"
#include "swrect.hxx"
#include "txtfly.hxx"
#include "swfont.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "ndtxt.hxx"
#include <editeng/paravertalignitem.hxx>

class Font;
class OutputDevice;
class SvxBrushItem;
class SvxLineSpacingItem;
class SvxTabStop;
class SvxTabStopItem;
class SwAttrSet;
class SwFldPortion;
class SwFlyPortion;
class SwFmtDrop;
class SwLineLayout;
class SwLinePortion;
class SwParaPortion;
class SwTabPortion;
class SwTxtFrm;
class SwTxtSizeInfo;
class SwViewOption;
class ViewShell;
class SwAttrIter;
struct SwMultiCreator;
class SwMultiPortion;
class SwWrongList;

/* Minimum percentage for kerning */
#define MINKERNPERCENT 5
#define ARROW_WIDTH 200
#define DIR_LEFT2RIGHT 0
#define DIR_BOTTOM2TOP 1
#define DIR_RIGHT2LEFT 2
#define DIR_TOP2BOTTOM 3

#ifdef DBG_UTIL
#define OPTDBG( rInf )   (rInf).IsOptDbg()
#else
#define OPTDBG( rInf )   sal_False
#endif

/*************************************************************************
 * class SwLineInfo
 * Respects the attribute LineSpace when calculating the Height/Ascent
 *************************************************************************/

class SwLineInfo
{
    friend class SwTxtIter;

    SvxTabStopItem* pRuler;
    const SvxLineSpacingItem *pSpace;
    sal_uInt16 nVertAlign;
    KSHORT nDefTabStop;
    bool bListTabStopIncluded;
    long nListTabStopPosition;

    void CtorInitLineInfo( const SwAttrSet& rAttrSet,
                           const SwTxtNode& rTxtNode );

    SwLineInfo();
    ~SwLineInfo();
public:
    // #i24363# tab stops relative to indent - returns the tab stop following nSearchPos or NULL
    const SvxTabStop *GetTabStop( const SwTwips nSearchPos,
                                 const SwTwips nRight ) const;
    inline const SvxLineSpacingItem *GetLineSpacing() const { return pSpace; }
    inline KSHORT GetDefTabStop() const { return nDefTabStop; }
    inline void SetDefTabStop( KSHORT nNew ) const
        { ( (SwLineInfo*)this )->nDefTabStop = nNew; }

    // vertical alignment
    inline sal_uInt16 GetVertAlign() const { return nVertAlign; }
    inline sal_Bool HasSpecialAlign( sal_Bool bVert ) const
        { return bVert ?
                 ( SvxParaVertAlignItem::BASELINE  != nVertAlign ) :
                 ( SvxParaVertAlignItem::BASELINE  != nVertAlign &&
                   SvxParaVertAlignItem::AUTOMATIC != nVertAlign ); }

    sal_uInt16 NumberOfTabStops() const;

    inline bool IsListTabStopIncluded() const
    {
        return bListTabStopIncluded;
    }
    inline long GetListTabStopPosition() const
    {
        return nListTabStopPosition;
    }

    friend SvStream &operator<<( SvStream &rOS, const SwLineInfo &rInf );
};

/*************************************************************************
 * class SwTxtInfo
 *************************************************************************/

class SwTxtInfo
{
    // Implementation in txthyph.cxx
    friend void SetParaPortion( SwTxtInfo *pInf, SwParaPortion *pRoot );
    SwParaPortion *pPara;
    xub_StrLen nTxtStart;                 // TxtOfst for Follows

protected:
    inline SwTxtInfo() : pPara(0) {}
public:
    void CtorInitTxtInfo( SwTxtFrm *pFrm );
    SwTxtInfo( const SwTxtInfo &rInf );
    inline SwTxtInfo( SwTxtFrm *pFrm ) { CtorInitTxtInfo( pFrm ); }
    inline SwParaPortion *GetParaPortion() { return pPara; }
    inline const SwParaPortion *GetParaPortion() const { return pPara; }
    inline xub_StrLen GetTxtStart() const { return nTxtStart; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtInfo &rInf );
};

/*************************************************************************
 * class SwTxtSizeInfo
 *************************************************************************/

typedef ::std::map< sal_uLong, sal_IntPtr > SwTxtPortionMap;

class SwTxtSizeInfo : public SwTxtInfo
{
protected:
    // during formatting, a small database is built, mapping portion pointers
    // to their maximum size (used for kana compression)
    SwTxtPortionMap aMaxWidth;
    // for each line, an array of compression values is calculated
    // this array is passed over to the info structure
    std::deque<sal_uInt16>* pKanaComp;

    ViewShell    *pVsh;

    // pOut is the output device, pRef is the device used for formatting
    OutputDevice* pOut;
    OutputDevice* pRef;

    SwFont *pFnt;
    SwUnderlineFont *pUnderFnt; // Font for underlining
    SwTxtFrm *pFrm;
    const SwViewOption *pOpt;
    const XubString *pTxt;
    xub_StrLen nIdx, nLen;
    sal_uInt16 nKanaIdx;
    sal_Bool bOnWin     : 1;
    sal_Bool bNotEOL    : 1;
    sal_Bool bURLNotify : 1;
    sal_Bool bStopUnderFlow : 1; // Underflow was stopped e.g. by a FlyPortion
    sal_Bool bFtnInside : 1;     // the current line contains a footnote
    sal_Bool bOtherThanFtnInside : 1; // the current line contains another portion than a footnote portion.
                                      // needed for checking keep together of footnote portion with previous portion
    sal_Bool bMulti : 1;        // inside a multiportion
    sal_Bool bFirstMulti : 1;   // this flag is used for two purposes:
                                // - the multiportion is the first lineportion
                                // - indicates, if we are currently in second
                                //   line of multi portion
    sal_Bool bRuby : 1;         // during the formatting of a phonetic line
    sal_Bool bHanging : 1;      // formatting of hanging punctuation allowed
    sal_Bool bScriptSpace : 1;  // space between different scripts (Asian/Latin)
    sal_Bool bForbiddenChars : 1; // Forbidden start/endline characters
    sal_Bool bSnapToGrid : 1;   // paragraph snaps to grid
    sal_uInt8 nDirection : 2;       // writing direction: 0/90/180/270 degree

protected:
    void CtorInitTxtSizeInfo( SwTxtFrm *pFrm, SwFont *pFnt = 0,
                   const xub_StrLen nIdx = 0,
                   const xub_StrLen nLen = STRING_LEN );
    SwTxtSizeInfo() : pKanaComp(0), pVsh(0), pOut(0), pRef(0), pFnt(0), pUnderFnt(0), pFrm(0), pOpt(0), pTxt(0) {}
public:
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf );
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf, const XubString &rTxt,
                   const xub_StrLen nIdx = 0,
                   const xub_StrLen nLen = STRING_LEN );

    inline SwTxtSizeInfo( SwTxtFrm *pTxtFrm, SwFont *pTxtFnt = 0,
                   const xub_StrLen nIndex = 0,
                   const xub_StrLen nLength = STRING_LEN )
           { CtorInitTxtSizeInfo( pTxtFrm, pTxtFnt, nIndex, nLength ); }

    // GetMultiAttr returns the text attribute of the multiportion,
    // if rPos is inside any multi-line part.
    // rPos will set to the end of the multi-line part.
    SwMultiCreator* GetMultiCreator( xub_StrLen &rPos, SwMultiPortion* pM ) const;

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
    inline sal_Bool IsOtherThanFtnInside() const { return bOtherThanFtnInside; }
    inline void SetOtherThanFtnInside( const sal_Bool bNew ) { bOtherThanFtnInside = bNew; }
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
    inline sal_Bool SnapToGrid() const { return bSnapToGrid; }
    inline void SetSnapToGrid( const sal_Bool bN ) { bSnapToGrid = bN; }
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
    inline sal_Unicode GetChar( const xub_StrLen nPos ) const
        { return pTxt->GetChar( nPos ); }

    inline KSHORT      GetTxtHeight() const;

    //
    // GetTxtSize
    //
    SwPosSize GetTxtSize( OutputDevice* pOut, const SwScriptInfo* pSI,
                          const XubString& rTxt, const xub_StrLen nIdx,
                          const xub_StrLen nLen, const sal_uInt16 nComp ) const;
    SwPosSize GetTxtSize() const;
    void GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
                      const xub_StrLen nLen, const sal_uInt16 nComp,
                      sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff ) const;
    inline SwPosSize GetTxtSize( const SwScriptInfo* pSI, const xub_StrLen nIdx,
                                 const xub_StrLen nLen, const sal_uInt16 nComp ) const;
    inline SwPosSize GetTxtSize( const XubString &rTxt ) const;

    //
    // GetTxtBreak
    //
    xub_StrLen GetTxtBreak( const long nLineWidth,
                                           const xub_StrLen nMaxLen,
                                           const sal_uInt16 nComp ) const;
    xub_StrLen GetTxtBreak( const long nLineWidth,
                                           const xub_StrLen nMaxLen,
                                           const sal_uInt16 nComp,
                                           xub_StrLen& rExtraCharPos ) const;

    inline KSHORT GetAscent() const;

    inline xub_StrLen GetIdx() const { return nIdx; }
    inline void SetIdx( const xub_StrLen nNew ) { nIdx = nNew; }
    inline xub_StrLen GetLen() const { return nLen; }
    inline void SetLen( const xub_StrLen nNew ) { nLen = nNew; }
    inline void SetTxt( const XubString &rNew ){ pTxt = &rNew; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtSizeInfo &rInf );

    // No Bullets for the symbol font!
    inline sal_Bool IsNoSymbol() const
    { return RTL_TEXTENCODING_SYMBOL != pFnt->GetCharSet( pFnt->GetActual() ); }

    void NoteAnimation() const;

    // Home is where Your heart is...
    inline SwTxtFrm *GetTxtFrm() { return pFrm; }
    inline const SwTxtFrm *GetTxtFrm() const { return pFrm; }

    inline sal_Bool HasHint( xub_StrLen nPos ) const
        { return _HasHint( pFrm->GetTxtNode(), nPos ); }
    static sal_Bool _HasHint( const SwTxtNode* pTxtNode, xub_StrLen nPos );

    // If Kana Compression is enabled, a minimum and maximum portion width
    // is calculated. We format lines with minimal size and share remaining
    // space among compressed kanas.
    // During formatting, the maximum values of compressable portions are
    // stored in aMaxWidth and discarded after a line has been formatted.
    inline void SetMaxWidthDiff( sal_uLong nKey, sal_uInt16 nVal )
    {
        aMaxWidth.insert( ::std::make_pair( nKey, nVal ) );
    };
    inline sal_uInt16 GetMaxWidthDiff( sal_uLong nKey )
    {
        SwTxtPortionMap::iterator it = aMaxWidth.find( nKey );

        if( it != aMaxWidth.end() )
            return it->second;
        else
            return 0;
    };
    inline void ResetMaxWidthDiff()
    {
        aMaxWidth.clear();
    };
    inline sal_Bool CompressLine()
    {
        return (sal_Bool)!aMaxWidth.empty();
    };

    //
    // Feature: Kana Compression
    //
    inline MSHORT GetKanaIdx() const { return nKanaIdx; }
    inline void ResetKanaIdx(){ nKanaIdx = 0; }
    inline void SetKanaIdx( MSHORT nNew ) { nKanaIdx = nNew; }
    inline void IncKanaIdx() { ++nKanaIdx; }
    inline void SetKanaComp( std::deque<sal_uInt16> *pNew ){ pKanaComp = pNew; }
    inline std::deque<sal_uInt16>* GetpKanaComp() const { return pKanaComp; }
    inline sal_uInt16 GetKanaComp() const
        { return ( pKanaComp && nKanaIdx < pKanaComp->size() )
                   ? (*pKanaComp)[nKanaIdx] : 0; }

#ifdef DBG_UTIL
    sal_Bool IsOptDbg() const;
#endif
};

/*************************************************************************
 * class SwTxtPaintInfo
 *************************************************************************/

class SwTxtPaintInfo : public SwTxtSizeInfo
{
    const SwWrongList *pWrongList;
    const SwWrongList *pGrammarCheckList;
    const SwWrongList *pSmartTags;    // SMARTTAGS
    std::vector<long>* pSpaceAdd;
    const SvxBrushItem *pBrushItem; // For the background
    SwRect      aItemRect;          // Also for the background
    SwTxtFly    aTxtFly;    // Calculate the FlyFrm
    Point       aPos;       // Paint position
    SwRect      aPaintRect; // Original paint rect (from Layout paint)

    MSHORT nSpaceIdx;
    void _DrawText( const XubString &rText, const SwLinePortion &rPor,
                   const xub_StrLen nIdx, const xub_StrLen nLen,
                   const sal_Bool bKern, const sal_Bool bWrong = sal_False,
                   const sal_Bool bSmartTag = sal_False,
                   const sal_Bool bGrammarCheck = sal_False );  // SMARTTAGS

    SwTxtPaintInfo &operator=(const SwTxtPaintInfo&);
    void _NotifyURL( const SwLinePortion &rPor ) const;
    void _DrawBackBrush( const SwLinePortion &rPor ) const;

protected:
#ifdef DBG_UTIL
    SwTxtPaintInfo() { pFrm = 0; pWrongList = 0; pGrammarCheckList = 0; pSmartTags = 0; pSpaceAdd = 0;
                       pBrushItem = ((SvxBrushItem*)-1);}
#else
    SwTxtPaintInfo() { pFrm = 0; pWrongList = 0; pGrammarCheckList = 0; pSmartTags = 0; pSpaceAdd = 0;
                       pBrushItem = 0;}
#endif
public:
    SwTxtPaintInfo( const SwTxtPaintInfo &rInf );
    SwTxtPaintInfo( const SwTxtPaintInfo &rInf, const XubString &rTxt );

    void CtorInitTxtPaintInfo( SwTxtFrm *pFrame, const SwRect &rPaint );

    void SetBack( const SvxBrushItem *pItem,
                  const SwRect &rRect ) { pBrushItem = pItem; aItemRect = rRect;}
    const SvxBrushItem *GetBrushItem() const { return pBrushItem; }
    const SwRect       &GetBrushRect() const { return aItemRect;  }

    inline SwTxtPaintInfo( SwTxtFrm *pFrame, const SwRect &rPaint )
           { CtorInitTxtPaintInfo( pFrame, rPaint ); }

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
    inline void DrawMarkedText( const SwLinePortion &rPor, const xub_StrLen nLen,
                                const sal_Bool bKern,
                                const sal_Bool bWrong,
                                const sal_Bool bSmartTags,
                                const sal_Bool bGrammarCheck ) const;

    void DrawRect( const SwRect &rRect, sal_Bool bNoGraphic = sal_False,
                   sal_Bool bRetouche = sal_True ) const;
    void DrawTab( const SwLinePortion &rPor ) const;
    void DrawLineBreak( const SwLinePortion &rPor ) const;
    void DrawRedArrow( const SwLinePortion &rPor ) const;
    void DrawPostIts( const SwLinePortion &rPor, sal_Bool bScript ) const;
    void DrawBackground( const SwLinePortion &rPor ) const;
    void DrawViewOpt( const SwLinePortion &rPor, const MSHORT nWhich ) const;
    inline void DrawBackBrush( const SwLinePortion &rPor ) const
    { /* if( pFnt->GetBackColor() ) */ _DrawBackBrush( rPor ); }

    void DrawCheckBox( const SwFieldFormPortion &rPor, bool checked) const;

    inline void NotifyURL( const SwLinePortion &rPor ) const
        { if( URLNotify() ) _NotifyURL( rPor ); }

    void CalcRect( const SwLinePortion& rPor, SwRect* pRect, SwRect* pIntersect = 0 ) const;

    inline SwTwips GetPaintOfst() const;
    inline void SetPaintOfst( const SwTwips nNew );
    inline const Point &GetPos() const { return aPos; }
    inline void SetPos( const Point &rNew ) { aPos = rNew; }

    inline const SwRect &GetPaintRect() const { return aPaintRect; }
    inline void SetPaintRect( const SwRect &rNew ) { aPaintRect = rNew; }

    friend SvStream &operator<<( SvStream &rOS, const SwTxtPaintInfo &rInf );

    //
    // STUFF FOR JUSTIFIED ALIGNMENT
    //
    inline MSHORT GetSpaceIdx() const { return nSpaceIdx; }
    inline void ResetSpaceIdx(){nSpaceIdx = 0; }
    inline void SetSpaceIdx( MSHORT nNew ) { nSpaceIdx = nNew; }
    inline void IncSpaceIdx() { ++nSpaceIdx; }
    inline void RemoveFirstSpaceAdd() { pSpaceAdd->erase( pSpaceAdd->begin() ); }
    inline long GetSpaceAdd() const
        { return ( pSpaceAdd && nSpaceIdx < pSpaceAdd->size() )
                   ? (*pSpaceAdd)[nSpaceIdx] : 0; }

    inline void SetpSpaceAdd( std::vector<long>* pNew ){ pSpaceAdd = pNew; }
    inline std::vector<long>* GetpSpaceAdd() const { return pSpaceAdd; }


    inline void SetWrongList( const SwWrongList *pNew ){ pWrongList = pNew; }
    inline const SwWrongList* GetpWrongList() const { return pWrongList; }

    inline void SetGrammarCheckList( const SwWrongList *pNew ){ pGrammarCheckList = pNew; }
    inline const SwWrongList* GetGrammarCheckList() const { return pGrammarCheckList; }

    // SMARTTAGS
    inline void SetSmartTags( const SwWrongList *pNew ){ pSmartTags = pNew; }
    inline const SwWrongList* GetSmartTags() const { return pSmartTags; }
};

/*************************************************************************
 * class SwTxtFormatInfo
 *************************************************************************/

class SwTxtFormatInfo : public SwTxtPaintInfo
{
    // temporary arguments for hyphenation
    com::sun::star::beans::PropertyValues   aHyphVals;

    SwLineLayout    *pRoot;       // The Root of the current line (pCurr)
    SwLinePortion   *pLast;       // The last Portion
    SwFlyPortion    *pFly;        // The following FlyPortion
    SwFldPortion    *pLastFld;    // Wrapped Field
    SwLinePortion   *pUnderFlow;  // Underflow: Last Portion
    SwLinePortion   *pRest;       // The Rest is the start of the next Line

    SwTabPortion    *pLastTab;     // The _last_ TabPortion

    xub_StrLen nSoftHyphPos;    // SoftHyphPos forr Hyphenation
    xub_StrLen nHyphStart;      // TxtPos at which the interactive hyphen is at the moment
    xub_StrLen nHyphWrdStart;   // Position of the found word
    xub_StrLen nHyphWrdLen;     // Length of the found word
    xub_StrLen nLineStart;      // Current line start in rTxt
    xub_StrLen nUnderScorePos;  // enlarge repaint if underscore has been found
    // #i34348# Changed type from sal_uInt16 to SwTwips
    SwTwips nLeft;              // Left margin
    SwTwips nRight;             // Right margin
    SwTwips nFirst;             // EZE
    KSHORT nRealWidth;          // "real" line width
    KSHORT nWidth;              // "virtual" line width
    KSHORT nLineHeight;         // Final height after CalcLine
    KSHORT nLineNettoHeight;    // line height without spacing
    KSHORT nForcedLeftMargin;   // Shift of left margin due to frame

    sal_Int16  nMinLeading;     // minimum number of chars before hyphenation point
    sal_Int16  nMinTrailing;    // minimum number of chars after hyphenation point
    sal_Int16  nMinWordLength;  // minimum length of word to be hyphenated

    sal_Bool bFull   : 1;      // Line is full
    sal_Bool bFtnDone  : 1;    // Ftn already formatted
    sal_Bool bErgoDone : 1;    // ErgoDone already formatted
    sal_Bool bNumDone  : 1;    // bNumDone already formatted
    sal_Bool bArrowDone : 1;   // Arrow to the left for scrolling paragraphs
    sal_Bool bStop   : 1;      // Cancel immediately, discarding the line
    sal_Bool bNewLine  : 1;    // Format another line
    sal_Bool bShift  : 1;      // Position change: Repaint until further notice
    sal_Bool bUnderFlow : 1;   // Context: UnderFlow() ?
    sal_Bool bInterHyph: 1;    // Interactive hyphenation?
    sal_Bool bAutoHyph : 1;    // Automatic hyphenation?
    sal_Bool bDropInit : 1;    // Set DropWidth
    sal_Bool bQuick  : 1;      // FormatQuick()
    sal_Bool bNoEndHyph  : 1;  // Switch off hyphenation at the line end (due to MaxHyphens)
    sal_Bool bNoMidHyph  : 1;  // Switch off hyphenation before flys (due to MaxHyphens)
    sal_Bool bIgnoreFly: 1;    // FitToContent ignores flys
    sal_Bool bFakeLineStart: 1; // String has been replaced by field portion
                                // info structure only pretends that we are at
                                // the beginning of a line
    sal_Bool bTabOverflow;      // Tabs are expanding after the end margin

    sal_Unicode   cTabDecimal;  // the current decimal delimiter
    sal_Unicode   cHookChar;    // For tabs in fields etc.
    sal_uInt8   nMaxHyph;       // Max. line count of followup hyphenations
    sal_Bool   bTestFormat;     // Test formatting from WouldFit, no notification etc.

    // Hyphenating ...
    sal_Bool InitHyph( const sal_Bool bAuto = sal_False );
    sal_Bool _CheckFtnPortion( SwLineLayout* pCurr );

public:
    void CtorInitTxtFormatInfo( SwTxtFrm *pFrm, const sal_Bool bInterHyph = sal_False,
        const sal_Bool bQuick = sal_False, const sal_Bool bTst = sal_False );
    inline SwTxtFormatInfo(SwTxtFrm *pFrame,const sal_Bool bInterHyphL=sal_False,
            const sal_Bool bQuickL = sal_False, const sal_Bool bTst = sal_False )
           { CtorInitTxtFormatInfo( pFrame, bInterHyphL, bQuickL, bTst ); }

    // For the formatting inside a double line in a line (multi-line portion)
    // we need a modified text-format-info:
    SwTxtFormatInfo( const SwTxtFormatInfo& rInf, SwLineLayout& rLay,
        SwTwips nActWidth );

    inline KSHORT Width() const { return nWidth; }
    inline void Width( const KSHORT nNew ) { nWidth = nNew; }
           void Init();

    // Returns the first changed position of the paragraph
    inline xub_StrLen GetReformatStart() const;

    // Margins
    inline SwTwips Left() const { return nLeft; }
    inline void Left( const SwTwips nNew ) { nLeft = nNew; }
    inline SwTwips Right() const { return nRight; }
    inline void Right( const SwTwips nNew ) { nRight = nNew; }
    inline SwTwips First() const { return nFirst; }
    inline void First( const SwTwips nNew ) { nFirst = nNew; }
    inline SwTwips CurrLeft() const { return (nLineStart ? nLeft : nFirst); }
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
    inline sal_Unicode GetTabDecimal() const { return cTabDecimal; }
    inline void SetTabDecimal( const sal_Unicode cNew ) { cTabDecimal = cNew;}

    // Last*
    inline SwFldPortion *GetLastFld() { return pLastFld; }
    inline void SetLastFld( SwFldPortion *pNew ) { pLastFld = pNew; }

    inline void ClearHookChar() { cHookChar = 0; }
    inline void SetHookChar( const sal_Unicode cNew ) { cHookChar = cNew; }
    inline sal_Unicode GetHookChar() const { return cHookChar; }

    // Done-Flags
    inline sal_Bool IsFtnDone() const { return bFtnDone; }
    inline void SetFtnDone( const sal_Bool bNew ) { bFtnDone = bNew; }
    inline sal_Bool IsErgoDone() const { return bErgoDone; }
    inline void SetErgoDone( const sal_Bool bNew ) { bErgoDone = bNew; }
    inline sal_Bool IsNumDone() const { return bNumDone; }
    inline void SetNumDone( const sal_Bool bNew ) { bNumDone = bNew; }
    inline sal_Bool IsArrowDone() const { return bArrowDone; }
    inline void SetArrowDone( const sal_Bool bNew ) { bArrowDone = bNew; }

    // For SwTxtPortion::Hyphenate
    inline sal_Bool IsSoftHyph( const xub_StrLen nPos ) const;
    sal_Bool ChgHyph( const sal_Bool bNew );

    // Should the hyphenate helper be discarded?
    sal_Bool IsHyphenate() const;
    inline void SetHyphStart( const xub_StrLen nNew ) { nHyphStart = nNew; }
    inline xub_StrLen GetHyphStart() const { return nHyphStart; }
    inline void SetHyphWrdStart( const xub_StrLen nNew ) { nHyphWrdStart = nNew; }
    inline xub_StrLen GetHyphWrdStart() const { return nHyphWrdStart; }
    inline void SetHyphWrdLen( const xub_StrLen nNew ) { nHyphWrdLen = nNew; }
    inline xub_StrLen GetHyphWrdLen() const { return nHyphWrdLen; }
    inline xub_StrLen GetUnderScorePos() const { return nUnderScorePos; }
    inline void SetUnderScorePos( xub_StrLen nNew ) { nUnderScorePos = nNew; }

    // Calls HyphenateWord() of Hyphenator
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >
                HyphWord( const String &rTxt, const sal_uInt16 nMinTrail );
    const com::sun::star::beans::PropertyValues &
                GetHyphValues() const;

    sal_Bool CheckFtnPortion( SwLineLayout* pCurr )
        { return IsFtnInside() && _CheckFtnPortion( pCurr ); }

    // Dropcaps called by SwTxtFormatter::CTOR
    const SwFmtDrop *GetDropFmt() const;

    // Resets the FormatInfo back to its original state
    void Reset( const SwTxtFrm *pFrame); // , const sal_Bool bAll );

    // Sets the last SwKernPortion as pLast, if it is followed by empty portions
    sal_Bool LastKernPortion();

    // Looks for tabs, TabDec, TXTATR and BRK from nIdx until nEnd.
    // Return: Position; sets cHookChar if necessary
    xub_StrLen ScanPortionEnd( const xub_StrLen nStart, const xub_StrLen nEnd );

    friend SvStream &operator<<( SvStream &rOS, const SwTxtFormatInfo &rInf );

    inline void SetTabOverflow( sal_Bool bOverflow ) { bTabOverflow = bOverflow; }
    inline sal_Bool IsTabOverflow( ) { return bTabOverflow; }
};

/*************************************************************************
 * class SwTxtSlot
 * For the text replacement and restoration of SwTxtSizeInfo.
 * The way this is done is a bit of a hack: Although rInf is const we change it
 * anyway.
 * Because rInf is restorated again in the DTOR, we can do this.
 * You could call it a "logical const", if you wish.
*************************************************************************/
class SwTxtSlot
{
    XubString aTxt;
    const XubString *pOldTxt;
    const SwWrongList* pOldSmartTagList;
    const SwWrongList* pOldGrammarCheckList;
    SwWrongList* pTempList;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    sal_Bool bOn;
protected:
    SwTxtSizeInfo *pInf;
public:
    // The replacement string originates either from the portion via GetExpText()
    // or from the char * pCh, if it is not NULL
    SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor, bool bTxtLen,
               bool bExgLists, const sal_Char *pCh = NULL );
    ~SwTxtSlot();
    inline sal_Bool IsOn() const { return bOn; }
};

/*************************************************************************
 * class SwFontSave
 *************************************************************************/

class SwFontSave
{
    SwTxtSizeInfo *pInf;
    SwFont        *pFnt;
    SwAttrIter    *pIter;
public:
    SwFontSave( const SwTxtSizeInfo &rInf, SwFont *pFnt,
                SwAttrIter* pItr = NULL );
   ~SwFontSave();
};

/*************************************************************************
 * class SwDefFontSave
 *************************************************************************/

class SwDefFontSave
{
    SwTxtSizeInfo *pInf;
    SwFont        *pFnt;
    SwFont        *pNewFnt;
public:
    SwDefFontSave( const SwTxtSizeInfo &rInf );
   ~SwDefFontSave();
};

/*************************************************************************
 * Inline implementations of SwTxtSizeInfo
 *************************************************************************/

inline KSHORT SwTxtSizeInfo::GetAscent() const
{
    OSL_ENSURE( GetOut(), "SwTxtSizeInfo::GetAscent() without pOut" );
    return ((SwFont*)GetFont())->GetAscent( pVsh, *GetOut() );
}

inline KSHORT SwTxtSizeInfo::GetTxtHeight() const
{
    OSL_ENSURE( GetOut(), "SwTxtSizeInfo::GetTxtHeight() without pOut" );
    return ((SwFont*)GetFont())->GetHeight( pVsh, *GetOut() );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const XubString &rTxt ) const
{
    return GetTxtSize( pOut, 0, rTxt, 0, rTxt.Len(), 0 );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI,
                                            const xub_StrLen nNewIdx,
                                            const xub_StrLen nNewLen,
                                            const sal_uInt16 nCompress ) const
{
    return GetTxtSize( pOut, pSI, *pTxt, nNewIdx, nNewLen, nCompress );
}

/*************************************************************************
 * Inline implementations of SwTxtPaintInfo
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
                            const xub_StrLen nStart, const xub_StrLen nLength,
                            const sal_Bool bKern ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( rText, rPor, nStart, nLength, bKern );
}

inline void SwTxtPaintInfo::DrawText( const SwLinePortion &rPor,
        const xub_StrLen nLength, const sal_Bool bKern ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( *pTxt, rPor, nIdx, nLength, bKern );
}

inline void SwTxtPaintInfo::DrawMarkedText( const SwLinePortion &rPor,
                                            const xub_StrLen nLength,
                                            const sal_Bool bKern,
                                            const sal_Bool bWrong,
                                            const sal_Bool bSmartTags,
                                            const sal_Bool bGrammarCheck ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( *pTxt, rPor, nIdx, nLength, bKern, bWrong, bSmartTags, bGrammarCheck );
}

/*************************************************************************
 * Inline implementations of SwTxtFormatInfo
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



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
