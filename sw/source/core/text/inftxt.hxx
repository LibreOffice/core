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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_INFTXT_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_INFTXT_HXX
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
class SwViewShell;
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
#define OPTDBG( rInf )   false
#endif

// Respects the attribute LineSpace when calculating the Height/Ascent
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
    inline bool HasSpecialAlign( bool bVert ) const
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

    friend SvStream & WriteSwLineInfo( SvStream &rOS, const SwLineInfo &rInf );
};

class SwTxtInfo
{
    // Implementation in txthyph.cxx
    friend void SetParaPortion( SwTxtInfo *pInf, SwParaPortion *pRoot );
    SwParaPortion *pPara;
    sal_Int32 nTxtStart;                 // TxtOfst for Follows

protected:
    SwTxtInfo()
        : pPara(0)
        , nTxtStart(0)
        {}
public:
    void CtorInitTxtInfo( SwTxtFrm *pFrm );
    SwTxtInfo( const SwTxtInfo &rInf );
    SwTxtInfo( SwTxtFrm *pFrm ) { CtorInitTxtInfo( pFrm ); }
    SwParaPortion *GetParaPortion() { return pPara; }
    const SwParaPortion *GetParaPortion() const { return pPara; }
    sal_Int32 GetTxtStart() const { return nTxtStart; }

    friend SvStream & WriteSwTxtInfo( SvStream &rOS, const SwTxtInfo &rInf );
};

typedef ::std::map< sal_uLong, sal_IntPtr > SwTxtPortionMap;

class SwTxtSizeInfo : public SwTxtInfo
{
protected:
    // during formatting, a small database is built, mapping portion pointers
    // to their maximum size (used for kana compression)
    SwTxtPortionMap m_aMaxWidth;
    // for each line, an array of compression values is calculated
    // this array is passed over to the info structure
    std::deque<sal_uInt16>* m_pKanaComp;

    SwViewShell    *m_pVsh;

    // m_pOut is the output device, m_pRef is the device used for formatting
    OutputDevice* m_pOut;
    OutputDevice* m_pRef;

    SwFont *m_pFnt;
    SwUnderlineFont *m_pUnderFnt; // Font for underlining
    SwTxtFrm *m_pFrm;
    const SwViewOption *m_pOpt;
    const OUString *m_pTxt;
    sal_Int32 m_nIdx, m_nLen;
    sal_uInt16 m_nKanaIdx;
    bool m_bOnWin     : 1;
    bool m_bNotEOL    : 1;
    bool m_bURLNotify : 1;
    bool m_bStopUnderflow : 1; // Underflow was stopped e.g. by a FlyPortion
    bool m_bFtnInside : 1;     // the current line contains a footnote
    bool m_bOtherThanFtnInside : 1; // the current line contains another portion than a footnote portion.
                                      // needed for checking keep together of footnote portion with previous portion
    bool m_bMulti : 1;        // inside a multiportion
    bool m_bFirstMulti : 1;   // this flag is used for two purposes:
                                // - the multiportion is the first lineportion
                                // - indicates, if we are currently in second
                                //   line of multi portion
    bool m_bRuby : 1;         // during the formatting of a phonetic line
    bool m_bHanging : 1;      // formatting of hanging punctuation allowed
    bool m_bScriptSpace : 1;  // space between different scripts (Asian/Latin)
    bool m_bForbiddenChars : 1; // Forbidden start/endline characters
    bool m_bSnapToGrid : 1;   // paragraph snaps to grid
    sal_uInt8 m_nDirection : 2;       // writing direction: 0/90/180/270 degree

protected:
    void CtorInitTxtSizeInfo( SwTxtFrm *pFrm, SwFont *pFnt = 0,
                   const sal_Int32 nIdx = 0,
                   const sal_Int32 nLen = COMPLETE_STRING );
    SwTxtSizeInfo()
        : m_pKanaComp(0)
        , m_pVsh(0)
        , m_pOut(0)
        , m_pRef(0)
        , m_pFnt(0)
        , m_pUnderFnt(0)
        , m_pFrm(0)
        , m_pOpt(0)
        , m_pTxt(0)
        , m_nIdx(0)
        , m_nLen(0)
        , m_nKanaIdx(0)
        , m_bOnWin    (false)
        , m_bNotEOL   (false)
        , m_bURLNotify(false)
        , m_bStopUnderflow(false)
        , m_bFtnInside(false)
        , m_bOtherThanFtnInside(false)
        , m_bMulti(false)
        , m_bFirstMulti(false)
        , m_bRuby(false)
        , m_bHanging(false)
        , m_bScriptSpace(false)
        , m_bForbiddenChars(false)
        , m_bSnapToGrid(false)
        , m_nDirection(0)
        {}
public:
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf );
    SwTxtSizeInfo( const SwTxtSizeInfo &rInf, const OUString* pTxt,
                   const sal_Int32 nIdx = 0,
                   const sal_Int32 nLen = COMPLETE_STRING );

    SwTxtSizeInfo( SwTxtFrm *pTxtFrm, SwFont *pTxtFnt = 0,
                   const sal_Int32 nIndex = 0,
                   const sal_Int32 nLength = COMPLETE_STRING )
        : m_bOnWin(false)
    {
        CtorInitTxtSizeInfo( pTxtFrm, pTxtFnt, nIndex, nLength );
    }

    // GetMultiAttr returns the text attribute of the multiportion,
    // if rPos is inside any multi-line part.
    // rPos will set to the end of the multi-line part.
    SwMultiCreator* GetMultiCreator( sal_Int32 &rPos, SwMultiPortion* pM ) const;

    inline bool OnWin() const { return m_bOnWin; }
    inline void SetOnWin( const bool bNew ) { m_bOnWin = bNew; }
    inline bool NotEOL() const { return m_bNotEOL; }
    inline void SetNotEOL( const bool bNew ) { m_bNotEOL = bNew; }
    inline bool URLNotify() const { return m_bURLNotify; }
    inline void SetURLNotify( const bool bNew ) { m_bURLNotify = bNew; }
    inline bool StopUnderflow() const { return m_bStopUnderflow; }
    inline void SetStopUnderflow( const bool bNew ) { m_bStopUnderflow = bNew; }
    inline bool IsFtnInside() const { return m_bFtnInside; }
    inline void SetFtnInside( const bool bNew ) { m_bFtnInside = bNew; }
    inline bool IsOtherThanFtnInside() const { return m_bOtherThanFtnInside; }
    inline void SetOtherThanFtnInside( const bool bNew ) { m_bOtherThanFtnInside = bNew; }
    inline bool IsMulti() const { return m_bMulti; }
    inline void SetMulti( const bool bNew ) { m_bMulti = bNew; }
    inline bool IsFirstMulti() const { return m_bFirstMulti; }
    inline void SetFirstMulti( const bool bNew ) { m_bFirstMulti = bNew; }
    inline bool IsRuby() const { return m_bRuby; }
    inline void SetRuby( const bool bNew ) { m_bRuby = bNew; }
    inline bool IsHanging() const { return m_bHanging; }
    inline void SetHanging( const bool bNew ) { m_bHanging = bNew; }
    inline bool HasScriptSpace() const { return m_bScriptSpace; }
    inline void SetScriptSpace( const bool bNew ) { m_bScriptSpace = bNew; }
    inline bool HasForbiddenChars() const { return m_bForbiddenChars; }
    inline void SetForbiddenChars( const bool bN ) { m_bForbiddenChars = bN; }
    inline bool SnapToGrid() const { return m_bSnapToGrid; }
    inline void SetSnapToGrid( const bool bN ) { m_bSnapToGrid = bN; }
    inline sal_uInt8 GetDirection() const { return m_nDirection; }
    inline void SetDirection( const sal_uInt8 nNew ) { m_nDirection = nNew; }
    inline bool IsRotated() const { return ( 1 & m_nDirection ); }

    inline SwViewShell *GetVsh() { return m_pVsh; }
    inline const SwViewShell *GetVsh() const { return m_pVsh; }

    inline OutputDevice *GetOut() { return m_pOut; }
    inline const OutputDevice *GetOut() const { return m_pOut; }
    inline void SetOut( OutputDevice* pNewOut ) { m_pOut = pNewOut; }

    inline OutputDevice *GetRefDev() { return m_pRef; }
    inline const OutputDevice *GetRefDev() const { return m_pRef; }

    inline SwFont *GetFont() { return m_pFnt; }
    inline const SwFont *GetFont() const { return m_pFnt; }
    inline void SetFont( SwFont *pNew ) { m_pFnt = pNew; }
    void SelectFont();
    inline void SetUnderFnt( SwUnderlineFont* pNew ) { m_pUnderFnt = pNew; }
    inline SwUnderlineFont* GetUnderFnt() const { return m_pUnderFnt; }

    inline const  SwViewOption &GetOpt() const { return *m_pOpt; }
    inline const OUString &GetTxt() const { return *m_pTxt; }
    inline sal_Unicode GetChar( const sal_Int32 nPos ) const
    { if (m_pTxt && nPos < m_pTxt->getLength()) return (*m_pTxt)[ nPos ]; return 0; }

    KSHORT      GetTxtHeight() const;

    SwPosSize GetTxtSize( OutputDevice* pOut, const SwScriptInfo* pSI,
                          const OUString& rTxt, const sal_Int32 nIdx,
                          const sal_Int32 nLen, const sal_uInt16 nComp ) const;
    SwPosSize GetTxtSize() const;
    void GetTxtSize( const SwScriptInfo* pSI, const sal_Int32 nIdx,
                      const sal_Int32 nLen, const sal_uInt16 nComp,
                      sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff ) const;
    inline SwPosSize GetTxtSize( const SwScriptInfo* pSI, const sal_Int32 nIdx,
                                 const sal_Int32 nLen, const sal_uInt16 nComp ) const;
    inline SwPosSize GetTxtSize( const OUString &rTxt ) const;

    sal_Int32 GetTxtBreak( const long nLineWidth,
                                           const sal_Int32 nMaxLen,
                                           const sal_uInt16 nComp ) const;
    sal_Int32 GetTxtBreak( const long nLineWidth,
                                           const sal_Int32 nMaxLen,
                                           const sal_uInt16 nComp,
                                           sal_Int32& rExtraCharPos ) const;

    KSHORT GetAscent() const;

    inline sal_Int32 GetIdx() const { return m_nIdx; }
    inline void SetIdx( const sal_Int32 nNew ) { m_nIdx = nNew; }
    inline sal_Int32 GetLen() const { return m_nLen; }
    inline void SetLen( const sal_Int32 nNew ) { m_nLen = nNew; }
    inline void SetTxt( const OUString &rNew ){ m_pTxt = &rNew; }

    friend SvStream & WriteSwTxtSizeInfo( SvStream &rOS, const SwTxtSizeInfo &rInf );

    // No Bullets for the symbol font!
    inline bool IsNoSymbol() const
    { return RTL_TEXTENCODING_SYMBOL != m_pFnt->GetCharSet( m_pFnt->GetActual() ); }

    void NoteAnimation() const;

    // Home is where Your heart is...
    inline SwTxtFrm *GetTxtFrm() { return m_pFrm; }
    inline const SwTxtFrm *GetTxtFrm() const { return m_pFrm; }

    inline bool HasHint( sal_Int32 nPos ) const
        { return _HasHint( m_pFrm->GetTxtNode(), nPos ); }
    static bool _HasHint( const SwTxtNode* pTxtNode, sal_Int32 nPos );

    // If Kana Compression is enabled, a minimum and maximum portion width
    // is calculated. We format lines with minimal size and share remaining
    // space among compressed kanas.
    // During formatting, the maximum values of compressable portions are
    // stored in m_aMaxWidth and discarded after a line has been formatted.
    inline void SetMaxWidthDiff( sal_uLong nKey, sal_uInt16 nVal )
    {
        m_aMaxWidth.insert( ::std::make_pair( nKey, nVal ) );
    };
    inline sal_uInt16 GetMaxWidthDiff( sal_uLong nKey )
    {
        SwTxtPortionMap::iterator it = m_aMaxWidth.find( nKey );

        if( it != m_aMaxWidth.end() )
            return it->second;
        else
            return 0;
    };
    inline void ResetMaxWidthDiff()
    {
        m_aMaxWidth.clear();
    };
    inline bool CompressLine()
    {
        return !m_aMaxWidth.empty();
    };

    // Feature: Kana Compression

    inline MSHORT GetKanaIdx() const { return m_nKanaIdx; }
    inline void ResetKanaIdx(){ m_nKanaIdx = 0; }
    inline void SetKanaIdx( MSHORT nNew ) { m_nKanaIdx = nNew; }
    inline void IncKanaIdx() { ++m_nKanaIdx; }
    inline void SetKanaComp( std::deque<sal_uInt16> *pNew ){ m_pKanaComp = pNew; }
    inline std::deque<sal_uInt16>* GetpKanaComp() const { return m_pKanaComp; }
    inline sal_uInt16 GetKanaComp() const
        { return ( m_pKanaComp && m_nKanaIdx < m_pKanaComp->size() )
                   ? (*m_pKanaComp)[m_nKanaIdx] : 0; }

#ifdef DBG_UTIL
    bool IsOptDbg() const;
#endif
};

class SwTxtPaintInfo : public SwTxtSizeInfo
{
    const SwWrongList *pWrongList;
    const SwWrongList *pGrammarCheckList;
    const SwWrongList *pSmartTags;
    std::vector<long>* pSpaceAdd;
    const SvxBrushItem *pBrushItem; // For the background
    SwRect      aItemRect;          // Also for the background
    SwTxtFly    aTxtFly;    // Calculate the FlyFrm
    Point       aPos;       // Paint position
    SwRect      aPaintRect; // Original paint rect (from Layout paint)

    MSHORT nSpaceIdx;
    void _DrawText( const OUString &rText, const SwLinePortion &rPor,
                   const sal_Int32 nIdx, const sal_Int32 nLen,
                   const bool bKern, const bool bWrong = false,
                   const bool bSmartTag = false,
                   const bool bGrammarCheck = false );

    SwTxtPaintInfo &operator=(const SwTxtPaintInfo&);
    void _NotifyURL( const SwLinePortion &rPor ) const;

protected:
    SwTxtPaintInfo()
        : pWrongList(0)
        , pGrammarCheckList(0)
        , pSmartTags(0)
        , pSpaceAdd(0)
#ifdef DBG_UTIL
        , pBrushItem((SvxBrushItem*)-1)
#else
        , pBrushItem(0)
#endif
        , nSpaceIdx(0)
        {}

public:
    SwTxtPaintInfo( const SwTxtPaintInfo &rInf );
    SwTxtPaintInfo( const SwTxtPaintInfo &rInf, const OUString* pTxt );

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
    inline void DrawText( const OUString &rText, const SwLinePortion &rPor,
                          const sal_Int32 nIdx = 0,
                          const sal_Int32 nLen = COMPLETE_STRING,
                          const bool bKern = false) const;
    inline void DrawText( const SwLinePortion &rPor, const sal_Int32 nLen,
                          const bool bKern = false ) const;
    inline void DrawMarkedText( const SwLinePortion &rPor, const sal_Int32 nLen,
                                const bool bKern,
                                const bool bWrong,
                                const bool bSmartTags,
                                const bool bGrammarCheck ) const;

    void DrawRect( const SwRect &rRect, bool bNoGraphic = false,
                   bool bRetouche = true ) const;

    void DrawSpecial(const SwLinePortion &rPor, sal_Unicode cChar, const Color& rColor) const;
    void DrawTab( const SwLinePortion &rPor ) const;
    void DrawLineBreak( const SwLinePortion &rPor ) const;
    void DrawRedArrow( const SwLinePortion &rPor ) const;
    void DrawPostIts( const SwLinePortion &rPor, bool bScript ) const;
    void DrawBackground( const SwLinePortion &rPor ) const;
    void DrawViewOpt( const SwLinePortion &rPor, const MSHORT nWhich ) const;
    void DrawBackBrush( const SwLinePortion &rPor ) const;

    /**
     * Draw character border around a line portion.
     *
     * @param[in]   rPor    line portion around which border have to be drawn.
    **/
    void DrawBorder( const SwLinePortion &rPor ) const;

    void DrawCheckBox(const SwFieldFormCheckboxPortion &rPor, bool bChecked) const;

    inline void NotifyURL( const SwLinePortion &rPor ) const
        { if( URLNotify() ) _NotifyURL( rPor ); }

    /**
     * Calculate the rectangular area where the portion takes place.
     * @param[in]   rPor        portion for which the method specify the painting area
     * @param[out]  pRect       whole area of the portion
     * @param[out]  pIntersect  part of the portion area clipped by OutputDevice's clip region
     * @param[in]   bInsideBox  area of portion's content, padding and border, but shadow
     *                          is excluded (e.g. for background)
    **/
    void CalcRect( const SwLinePortion& rPor, SwRect* pRect,
                   SwRect* pIntersect = 0, const bool bInsideBox = false ) const;

    inline SwTwips GetPaintOfst() const;
    inline void SetPaintOfst( const SwTwips nNew );
    inline const Point &GetPos() const { return aPos; }
    inline void SetPos( const Point &rNew ) { aPos = rNew; }

    inline const SwRect &GetPaintRect() const { return aPaintRect; }
    inline void SetPaintRect( const SwRect &rNew ) { aPaintRect = rNew; }

    friend SvStream & WriteSwTxtPaintInfo( SvStream &rOS, const SwTxtPaintInfo &rInf );

    // STUFF FOR JUSTIFIED ALIGNMENT

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

    inline void SetSmartTags( const SwWrongList *pNew ){ pSmartTags = pNew; }
    inline const SwWrongList* GetSmartTags() const { return pSmartTags; }
};

class SwTxtFormatInfo : public SwTxtPaintInfo
{
    // temporary arguments for hyphenation
    com::sun::star::beans::PropertyValues   aHyphVals;

    SwLineLayout    *pRoot;       // The Root of the current line (pCurr)
    SwLinePortion   *pLast;       // The last Portion
    SwFlyPortion    *pFly;        // The following FlyPortion
    SwFldPortion    *pLastFld;    // Wrapped Field
    SwLinePortion   *pUnderflow;  // Underflow: Last Portion
    SwLinePortion   *pRest;       // The Rest is the start of the next Line

    SwTabPortion    *pLastTab;     // The _last_ TabPortion

    sal_Int32 nSoftHyphPos;    // SoftHyphPos forr Hyphenation
    sal_Int32 nLineStart;      // Current line start in rTxt
    sal_Int32 nUnderScorePos;  // enlarge repaint if underscore has been found
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

    bool bFull : 1;             // Line is full
    bool bFtnDone : 1;          // Ftn already formatted
    bool bErgoDone : 1;         // ErgoDone already formatted
    bool bNumDone : 1;          // bNumDone already formatted
    bool bArrowDone : 1;        // Arrow to the left for scrolling paragraphs
    bool bStop : 1;             // Cancel immediately, discarding the line
    bool bNewLine : 1;          // Format another line
    bool bShift : 1;            // Position change: Repaint until further notice
    bool bUnderflow : 1;        // Context: Underflow() ?
    bool bInterHyph : 1;        // Interactive hyphenation?
    bool bAutoHyph : 1;         // Automatic hyphenation?
    bool bDropInit : 1;         // Set DropWidth
    bool bQuick : 1;            // FormatQuick()
    bool bNoEndHyph : 1;        // Switch off hyphenation at the line end (due to MaxHyphens)
    bool bNoMidHyph : 1;        // Switch off hyphenation before flys (due to MaxHyphens)
    bool bIgnoreFly : 1;        // FitToContent ignores flys
    bool bFakeLineStart : 1;    // String has been replaced by field portion
                                // info structure only pretends that we are at
                                // the beginning of a line
    bool bTabOverflow : 1;      // Tabs are expanding after the end margin
    bool bTestFormat : 1;       // Test formatting from WouldFit, no notification etc.

    sal_Unicode   cTabDecimal;  // the current decimal delimiter
    sal_Unicode   cHookChar;    // For tabs in fields etc.
    sal_uInt8   nMaxHyph;       // Max. line count of followup hyphenations

    // Hyphenating ...
    bool InitHyph( const bool bAuto = false );
    bool _CheckFtnPortion( SwLineLayout* pCurr );

public:
    void CtorInitTxtFormatInfo( SwTxtFrm *pFrm, const bool bInterHyph = false,
        const bool bQuick = false, const bool bTst = false );
    inline SwTxtFormatInfo(SwTxtFrm *pFrame, const bool bInterHyphL = false,
            const bool bQuickL = false, const bool bTst = false)
           { CtorInitTxtFormatInfo( pFrame, bInterHyphL, bQuickL, bTst ); }

    // For the formatting inside a double line in a line (multi-line portion)
    // we need a modified text-format-info:
    SwTxtFormatInfo( const SwTxtFormatInfo& rInf, SwLineLayout& rLay,
        SwTwips nActWidth );

    inline KSHORT Width() const { return nWidth; }
    inline void Width( const KSHORT nNew ) { nWidth = nNew; }
           void Init();

    // Returns the first changed position of the paragraph
    inline sal_Int32 GetReformatStart() const;

    // Margins
    inline SwTwips Left() const { return nLeft; }
    inline void Left( const SwTwips nNew ) { nLeft = nNew; }
    inline SwTwips Right() const { return nRight; }
    inline void Right( const SwTwips nNew ) { nRight = nNew; }
    inline SwTwips First() const { return nFirst; }
    inline void First( const SwTwips nNew ) { nFirst = nNew; }
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
    inline bool IsFull() const { return bFull; }
    inline void SetFull( const bool bNew ) { bFull = bNew; }
    inline bool IsHyphForbud() const
        { return pFly ? bNoMidHyph : bNoEndHyph; }
    inline void SetHyphForbud( const bool bNew )
        { if ( pFly ) bNoMidHyph = bNew; else bNoEndHyph = bNew; }
    inline void ChkNoHyph( const sal_uInt8 bEnd, const sal_uInt8 bMid )
        { bNoEndHyph = (nMaxHyph && bEnd >= nMaxHyph);
          bNoMidHyph = (nMaxHyph && bMid >= nMaxHyph); }
    inline bool IsIgnoreFly() const { return bIgnoreFly; }
    inline void SetIgnoreFly( const bool bNew ) { bIgnoreFly = bNew; }
    inline bool IsFakeLineStart() const { return bFakeLineStart; }
    inline void SetFakeLineStart( const bool bNew ) { bFakeLineStart = bNew; }
    inline bool IsStop() const { return bStop; }
    inline void SetStop( const bool bNew ) { bStop = bNew; }
    inline SwLinePortion *GetRest() { return pRest; }
    inline void SetRest( SwLinePortion *pNewRest ) { pRest = pNewRest; }
    inline bool IsNewLine() const { return bNewLine; }
    inline void SetNewLine( const bool bNew ) { bNewLine = bNew; }
    inline bool IsShift() const { return bShift; }
    inline void SetShift( const bool bNew ) { bShift = bNew; }
    inline bool IsInterHyph() const { return bInterHyph; }
    inline bool IsAutoHyph() const { return bAutoHyph; }
    inline bool IsUnderflow() const { return bUnderflow; }
    inline void ClrUnderflow() { bUnderflow = false; }
    inline bool IsDropInit() const { return bDropInit; }
    inline void SetDropInit( const bool bNew ) { bDropInit = bNew; }
    inline bool IsQuick() const { return bQuick; }
    inline bool IsTest() const { return bTestFormat; }

    inline sal_Int32 GetLineStart() const { return nLineStart; }
    inline void SetLineStart( const sal_Int32 nNew ) { nLineStart = nNew; }

    // these are used during fly calculation
    inline KSHORT GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const KSHORT nNew ) { nLineHeight = nNew; }
    inline KSHORT GetLineNettoHeight() const { return nLineNettoHeight; }
    inline void SetLineNettoHeight( const KSHORT nNew ) { nLineNettoHeight = nNew; }

    inline const SwLinePortion *GetUnderflow() const { return pUnderflow; }
    inline SwLinePortion *GetUnderflow() { return pUnderflow; }
    inline void SetUnderflow( SwLinePortion *pNew )
           { pUnderflow = pNew; bUnderflow = true; }
    inline sal_Int32 GetSoftHyphPos() const { return nSoftHyphPos; }
    inline void SetSoftHyphPos( const sal_Int32 nNew ) { nSoftHyphPos = nNew; }

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
    inline bool IsFtnDone() const { return bFtnDone; }
    inline void SetFtnDone( const bool bNew ) { bFtnDone = bNew; }
    inline bool IsErgoDone() const { return bErgoDone; }
    inline void SetErgoDone( const bool bNew ) { bErgoDone = bNew; }
    inline bool IsNumDone() const { return bNumDone; }
    inline void SetNumDone( const bool bNew ) { bNumDone = bNew; }
    inline bool IsArrowDone() const { return bArrowDone; }
    inline void SetArrowDone( const bool bNew ) { bArrowDone = bNew; }

    // For SwTxtPortion::Hyphenate
    inline bool IsSoftHyph( const sal_Int32 nPos ) const;
    bool ChgHyph( const bool bNew );

    // Should the hyphenate helper be discarded?
    bool IsHyphenate() const;
    inline sal_Int32 GetUnderScorePos() const { return nUnderScorePos; }
    inline void SetUnderScorePos( sal_Int32 nNew ) { nUnderScorePos = nNew; }

    // Calls HyphenateWord() of Hyphenator
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >
                HyphWord( const OUString &rTxt, const sal_uInt16 nMinTrail );
    const com::sun::star::beans::PropertyValues &
                GetHyphValues() const;

    bool CheckFtnPortion( SwLineLayout* pCurr )
        { return IsFtnInside() && _CheckFtnPortion( pCurr ); }

    // Dropcaps called by SwTxtFormatter::CTOR
    const SwFmtDrop *GetDropFmt() const;

    // Sets the last SwKernPortion as pLast, if it is followed by empty portions
    bool LastKernPortion();

    // Looks for tabs, TabDec, TXTATR and BRK from nIdx until nEnd.
    // Return: Position; sets cHookChar if necessary
    sal_Int32 ScanPortionEnd( const sal_Int32 nStart, const sal_Int32 nEnd );

    friend SvStream & WriteSwTxtFormatInfo( SvStream &rOS, const SwTxtFormatInfo &rInf );

    inline void SetTabOverflow( bool bOverflow ) { bTabOverflow = bOverflow; }
    inline bool IsTabOverflow() { return bTabOverflow; }
};

// For the text replacement and restoration of SwTxtSizeInfo.
// The way this is done is a bit of a hack: Although rInf is const we change it
// anyway.
// Because rInf is restorated again in the DTOR, we can do this.
// You could call it a "logical const", if you wish.
class SwTxtSlot
{
    OUString aTxt;
    const OUString *pOldTxt;
    const SwWrongList* pOldSmartTagList;
    const SwWrongList* pOldGrammarCheckList;
    SwWrongList* pTempList;
    sal_Int32 nIdx;
    sal_Int32 nLen;
    bool bOn;
protected:
    SwTxtSizeInfo *pInf;
public:
    // The replacement string originates either from the portion via GetExpText()
    // or from the rCh, if it is not empty.
    SwTxtSlot( const SwTxtSizeInfo *pNew, const SwLinePortion *pPor, bool bTxtLen,
               bool bExgLists, OUString const & rCh = OUString() );
    ~SwTxtSlot();
    bool IsOn() const { return bOn; }
};

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

inline KSHORT SwTxtSizeInfo::GetAscent() const
{
    SAL_WARN_IF( !GetOut(), "sw.core", "SwTxtSizeInfo::GetAscent() without m_pOut" );

    return ((SwFont*)GetFont())->GetAscent( m_pVsh, *GetOut() );
}

inline KSHORT SwTxtSizeInfo::GetTxtHeight() const
{
    SAL_WARN_IF( !GetOut(), "sw.core", "SwTxtSizeInfo::GetTxtHeight() without m_pOut" );

    return ((SwFont*)GetFont())->GetHeight( m_pVsh, *GetOut() );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const OUString &rTxt ) const
{
    return GetTxtSize( m_pOut, 0, rTxt, 0, rTxt.getLength(), 0 );
}

inline SwPosSize SwTxtSizeInfo::GetTxtSize( const SwScriptInfo* pSI,
                                            const sal_Int32 nNewIdx,
                                            const sal_Int32 nNewLen,
                                            const sal_uInt16 nCompress ) const
{
    return GetTxtSize( m_pOut, pSI, *m_pTxt, nNewIdx, nNewLen, nCompress );
}

inline SwTwips SwTxtPaintInfo::GetPaintOfst() const
{
    return GetParaPortion()->GetRepaint()->GetOfst();
}

inline void SwTxtPaintInfo::SetPaintOfst( const SwTwips nNew )
{
    GetParaPortion()->GetRepaint()->SetOfst( nNew );
}

inline void SwTxtPaintInfo::DrawText( const OUString &rText,
                            const SwLinePortion &rPor,
                            const sal_Int32 nStart, const sal_Int32 nLength,
                            const bool bKern ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( rText, rPor, nStart, nLength, bKern );
}

inline void SwTxtPaintInfo::DrawText( const SwLinePortion &rPor,
        const sal_Int32 nLength, const bool bKern ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( *m_pTxt, rPor, m_nIdx, nLength, bKern );
}

inline void SwTxtPaintInfo::DrawMarkedText( const SwLinePortion &rPor,
                                            const sal_Int32 nLength,
                                            const bool bKern,
                                            const bool bWrong,
                                            const bool bSmartTags,
                                            const bool bGrammarCheck ) const
{
    ((SwTxtPaintInfo*)this)->_DrawText( *m_pTxt, rPor, m_nIdx, nLength, bKern, bWrong, bSmartTags, bGrammarCheck );
}

inline sal_Int32 SwTxtFormatInfo::GetReformatStart() const
{
    return GetParaPortion()->GetReformat()->Start();
}

inline const SwAttrSet& SwTxtFormatInfo::GetCharAttr() const
{
    return GetTxtFrm()->GetTxtNode()->GetSwAttrSet();
}

inline void SwTxtFormatInfo::SetParaFtn()
{
    GetTxtFrm()->SetFtn( true );
}

inline bool SwTxtFormatInfo::IsSoftHyph( const sal_Int32 nPos ) const
{
    return CHAR_SOFTHYPHEN == GetTxtFrm()->GetTxtNode()->GetTxt()[nPos];
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
