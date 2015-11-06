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
#include "swrect.hxx"
#include "txtfly.hxx"
#include "swfont.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "ndtxt.hxx"
#include <editeng/paravertalignitem.hxx>
#include <sal/log.hxx>

namespace vcl { class Font; }
class OutputDevice;
class SvxBrushItem;
class SvxLineSpacingItem;
class SvxTabStop;
class SvxTabStopItem;
class SwAttrSet;
class SwFieldPortion;
class SwFlyPortion;
class SwFormatDrop;
class SwLineLayout;
class SwLinePortion;
class SwParaPortion;
class SwTabPortion;
class SwTextFrm;
class SwTextSizeInfo;
class SwViewOption;
class SwViewShell;
class SwAttrIter;
struct SwMultiCreator;
class SwMultiPortion;
class SwWrongList;

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
    friend class SwTextIter;

    SvxTabStopItem* pRuler;
    const SvxLineSpacingItem *pSpace;
    sal_uInt16 nVertAlign;
    sal_uInt16 nDefTabStop;
    bool bListTabStopIncluded;
    long nListTabStopPosition;

    void CtorInitLineInfo( const SwAttrSet& rAttrSet,
                           const SwTextNode& rTextNode );

    SwLineInfo();
    ~SwLineInfo();
public:
    // #i24363# tab stops relative to indent - returns the tab stop following nSearchPos or NULL
    const SvxTabStop *GetTabStop( const SwTwips nSearchPos,
                                 const SwTwips nRight ) const;
    inline const SvxLineSpacingItem *GetLineSpacing() const { return pSpace; }
    inline sal_uInt16 GetDefTabStop() const { return nDefTabStop; }
    inline void SetDefTabStop( sal_uInt16 nNew ) const
        { const_cast<SwLineInfo*>(this)->nDefTabStop = nNew; }

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
};

class SwTextInfo
{
    // Implementation in txthyph.cxx
    friend void SetParaPortion( SwTextInfo *pInf, SwParaPortion *pRoot );
    SwParaPortion *m_pPara;
    sal_Int32 m_nTextStart; // TextOfst for Follows

protected:
    SwTextInfo()
        : m_pPara(0)
        , m_nTextStart(0)
    {}

public:
    void CtorInitTextInfo( SwTextFrm *pFrm );
    SwTextInfo( const SwTextInfo &rInf );
    explicit SwTextInfo( SwTextFrm *pFrm ) { CtorInitTextInfo( pFrm ); }
    SwParaPortion *GetParaPortion() { return m_pPara; }
    const SwParaPortion *GetParaPortion() const { return m_pPara; }
    sal_Int32 GetTextStart() const { return m_nTextStart; }
};

class SwTextSizeInfo : public SwTextInfo
{
private:
    typedef ::std::map< sal_uIntPtr, sal_uInt16 > SwTextPortionMap;

protected:
    // during formatting, a small database is built, mapping portion pointers
    // to their maximum size (used for kana compression)
    SwTextPortionMap m_aMaxWidth;
    // for each line, an array of compression values is calculated
    // this array is passed over to the info structure
    std::deque<sal_uInt16>* m_pKanaComp;

    SwViewShell    *m_pVsh;

    // m_pOut is the output device, m_pRef is the device used for formatting
    VclPtr<OutputDevice> m_pOut;
    VclPtr<OutputDevice> m_pRef;

    // performance hack - this is only used by SwTextFormatInfo but
    // because it's not even possible to dynamic_cast these things
    // currently it has to be stored here
    std::shared_ptr<vcl::TextLayoutCache> m_pCachedVclData;

    SwFont *m_pFnt;
    SwUnderlineFont *m_pUnderFnt; // Font for underlining
    SwTextFrm *m_pFrm;
    const SwViewOption *m_pOpt;
    const OUString *m_pText;
    sal_Int32 m_nIdx, m_nLen;
    sal_uInt16 m_nKanaIdx;
    bool m_bOnWin     : 1;
    bool m_bNotEOL    : 1;
    bool m_bURLNotify : 1;
    bool m_bStopUnderflow : 1; // Underflow was stopped e.g. by a FlyPortion
    bool m_bFootnoteInside : 1;     // the current line contains a footnote
    bool m_bOtherThanFootnoteInside : 1; // the current line contains another portion than a footnote portion.
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
    sal_uInt8 m_nDirection : 2; // writing direction: 0/90/180/270 degree

protected:
    void CtorInitTextSizeInfo( OutputDevice* pRenderContext, SwTextFrm *pFrm, SwFont *pFnt = 0,
                   const sal_Int32 nIdx = 0,
                   const sal_Int32 nLen = COMPLETE_STRING );
    SwTextSizeInfo();
public:
    SwTextSizeInfo( const SwTextSizeInfo &rInf );
    SwTextSizeInfo( const SwTextSizeInfo &rInf, const OUString* pText,
                   const sal_Int32 nIdx = 0,
                   const sal_Int32 nLen = COMPLETE_STRING );

    SwTextSizeInfo( SwTextFrm *pTextFrm, SwFont *pTextFnt = 0,
                   const sal_Int32 nIndex = 0,
                   const sal_Int32 nLength = COMPLETE_STRING );

    // GetMultiAttr returns the text attribute of the multiportion,
    // if rPos is inside any multi-line part.
    // rPos will set to the end of the multi-line part.
    SwMultiCreator* GetMultiCreator( sal_Int32 &rPos, SwMultiPortion* pM ) const;

    inline bool OnWin() const { return m_bOnWin; }
    inline void SetOnWin( const bool bNew ) { m_bOnWin = bNew; }
    inline bool NotEOL() const { return m_bNotEOL; }
    inline void SetNotEOL( const bool bNew ) { m_bNotEOL = bNew; }
    inline bool URLNotify() const { return m_bURLNotify; }
    inline bool StopUnderflow() const { return m_bStopUnderflow; }
    inline void SetStopUnderflow( const bool bNew ) { m_bStopUnderflow = bNew; }
    inline bool IsFootnoteInside() const { return m_bFootnoteInside; }
    inline void SetFootnoteInside( const bool bNew ) { m_bFootnoteInside = bNew; }
    inline bool IsOtherThanFootnoteInside() const { return m_bOtherThanFootnoteInside; }
    inline void SetOtherThanFootnoteInside( const bool bNew ) { m_bOtherThanFootnoteInside = bNew; }
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

    inline vcl::RenderContext *GetOut() { return m_pOut; }
    inline const vcl::RenderContext *GetOut() const { return m_pOut; }
    inline void SetOut( OutputDevice* pNewOut ) { m_pOut = pNewOut; }

    inline vcl::RenderContext *GetRefDev() { return m_pRef; }
    inline const vcl::RenderContext *GetRefDev() const { return m_pRef; }

    inline SwFont *GetFont() { return m_pFnt; }
    inline const SwFont *GetFont() const { return m_pFnt; }
    inline void SetFont( SwFont *pNew ) { m_pFnt = pNew; }
    void SelectFont();
    inline void SetUnderFnt( SwUnderlineFont* pNew ) { m_pUnderFnt = pNew; }
    inline SwUnderlineFont* GetUnderFnt() const { return m_pUnderFnt; }

    inline const  SwViewOption &GetOpt() const { return *m_pOpt; }
    inline const OUString &GetText() const { return *m_pText; }
    inline sal_Unicode GetChar( const sal_Int32 nPos ) const
    { if (m_pText && nPos < m_pText->getLength()) return (*m_pText)[ nPos ]; return 0; }

    sal_uInt16      GetTextHeight() const;

    SwPosSize GetTextSize( OutputDevice* pOut, const SwScriptInfo* pSI,
                          const OUString& rText, const sal_Int32 nIdx,
                          const sal_Int32 nLen, const sal_uInt16 nComp ) const;
    SwPosSize GetTextSize() const;
    void GetTextSize( const SwScriptInfo* pSI, const sal_Int32 nIdx,
                      const sal_Int32 nLen, const sal_uInt16 nComp,
                      sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff,
                      vcl::TextLayoutCache const* = nullptr) const;
    inline SwPosSize GetTextSize( const SwScriptInfo* pSI, const sal_Int32 nIdx,
                                 const sal_Int32 nLen, const sal_uInt16 nComp ) const;
    inline SwPosSize GetTextSize( const OUString &rText ) const;

    sal_Int32 GetTextBreak( const long nLineWidth,
                                           const sal_Int32 nMaxLen,
                                           const sal_uInt16 nComp,
                           vcl::TextLayoutCache const* = nullptr) const;
    sal_Int32 GetTextBreak( const long nLineWidth,
                                           const sal_Int32 nMaxLen,
                                           const sal_uInt16 nComp,
                                           sal_Int32& rExtraCharPos,
                           vcl::TextLayoutCache const* = nullptr) const;

    sal_uInt16 GetAscent() const;

    inline sal_Int32 GetIdx() const { return m_nIdx; }
    inline void SetIdx( const sal_Int32 nNew ) { m_nIdx = nNew; }
    inline sal_Int32 GetLen() const { return m_nLen; }
    inline void SetLen( const sal_Int32 nNew ) { m_nLen = nNew; }
    inline void SetText( const OUString &rNew ){ m_pText = &rNew; }

    // No Bullets for the symbol font!
    inline bool IsNoSymbol() const
    { return RTL_TEXTENCODING_SYMBOL != m_pFnt->GetCharSet( m_pFnt->GetActual() ); }

    void NoteAnimation() const;

    // Home is where Your heart is...
    inline SwTextFrm *GetTextFrm() { return m_pFrm; }
    inline const SwTextFrm *GetTextFrm() const { return m_pFrm; }

    inline bool HasHint( sal_Int32 nPos ) const
        { return _HasHint( m_pFrm->GetTextNode(), nPos ); }
    static bool _HasHint( const SwTextNode* pTextNode, sal_Int32 nPos );

    // If Kana Compression is enabled, a minimum and maximum portion width
    // is calculated. We format lines with minimal size and share remaining
    // space among compressed kanas.
    // During formatting, the maximum values of compressable portions are
    // stored in m_aMaxWidth and discarded after a line has been formatted.
    inline void SetMaxWidthDiff( const void *nKey, sal_uInt16 nVal )
    {
        m_aMaxWidth.insert( ::std::make_pair( reinterpret_cast<sal_uIntPtr>(nKey), nVal ) );
    };
    inline sal_uInt16 GetMaxWidthDiff( const void *nKey )
    {
        SwTextPortionMap::iterator it = m_aMaxWidth.find( reinterpret_cast<sal_uIntPtr>(nKey) );

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

    inline sal_uInt16 GetKanaIdx() const { return m_nKanaIdx; }
    inline void ResetKanaIdx(){ m_nKanaIdx = 0; }
    inline void SetKanaIdx( sal_uInt16 nNew ) { m_nKanaIdx = nNew; }
    inline void IncKanaIdx() { ++m_nKanaIdx; }
    inline void SetKanaComp( std::deque<sal_uInt16> *pNew ){ m_pKanaComp = pNew; }
    inline std::deque<sal_uInt16>* GetpKanaComp() const { return m_pKanaComp; }
    inline sal_uInt16 GetKanaComp() const
        { return ( m_pKanaComp && m_nKanaIdx < m_pKanaComp->size() )
                   ? (*m_pKanaComp)[m_nKanaIdx] : 0; }

    std::shared_ptr<vcl::TextLayoutCache> GetCachedVclData() const
    {
        return m_pCachedVclData;
    }
    void SetCachedVclData(std::shared_ptr<vcl::TextLayoutCache> const& pCachedVclData)
    {
        m_pCachedVclData = pCachedVclData;
    }
};

class SwTextPaintInfo : public SwTextSizeInfo
{
    const SwWrongList *pWrongList;
    const SwWrongList *pGrammarCheckList;
    const SwWrongList *pSmartTags;
    std::vector<long>* pSpaceAdd;
    const SvxBrushItem *pBrushItem; // For the background
    SwRect      aItemRect;          // Also for the background
    SwTextFly    aTextFly;    // Calculate the FlyFrm
    Point       aPos;       // Paint position
    SwRect      aPaintRect; // Original paint rect (from Layout paint)

    sal_uInt16 nSpaceIdx;
    void _DrawText( const OUString &rText, const SwLinePortion &rPor,
                   const sal_Int32 nIdx, const sal_Int32 nLen,
                   const bool bKern, const bool bWrong = false,
                   const bool bSmartTag = false,
                   const bool bGrammarCheck = false );

    SwTextPaintInfo &operator=(const SwTextPaintInfo&) = delete;
    void _NotifyURL( const SwLinePortion &rPor ) const;

protected:
    SwTextPaintInfo()
        : pWrongList(0)
        , pGrammarCheckList(0)
        , pSmartTags(0)
        , pSpaceAdd(0)
#ifdef DBG_UTIL
        , pBrushItem(reinterpret_cast<SvxBrushItem*>(-1))
#else
        , pBrushItem(0)
#endif
        , nSpaceIdx(0)
        {}

public:
    SwTextPaintInfo( const SwTextPaintInfo &rInf );
    SwTextPaintInfo( const SwTextPaintInfo &rInf, const OUString* pText );

    void CtorInitTextPaintInfo( OutputDevice* pRenderContext, SwTextFrm *pFrame, const SwRect &rPaint );

    const SvxBrushItem *GetBrushItem() const { return pBrushItem; }
    const SwRect       &GetBrushRect() const { return aItemRect;  }

    SwTextPaintInfo( SwTextFrm *pFrame, const SwRect &rPaint );

    inline SwTwips X() const { return aPos.X(); }
    inline void X( const long nNew ) { aPos.X() = nNew; }
    inline SwTwips Y() const { return aPos.Y(); }
    inline void Y( const SwTwips nNew ) { aPos.Y() = nNew; }

    inline SwTextFly& GetTextFly() { return aTextFly; }
    inline const SwTextFly& GetTextFly() const { return aTextFly; }
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

    void DrawTab( const SwLinePortion &rPor ) const;
    void DrawLineBreak( const SwLinePortion &rPor ) const;
    void DrawRedArrow( const SwLinePortion &rPor ) const;
    void DrawPostIts( const SwLinePortion &rPor, bool bScript ) const;
    void DrawBackground( const SwLinePortion &rPor ) const;
    void DrawViewOpt( const SwLinePortion &rPor, const sal_uInt16 nWhich ) const;
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

    // STUFF FOR JUSTIFIED ALIGNMENT

    inline sal_uInt16 GetSpaceIdx() const { return nSpaceIdx; }
    inline void ResetSpaceIdx(){nSpaceIdx = 0; }
    inline void SetSpaceIdx( sal_uInt16 nNew ) { nSpaceIdx = nNew; }
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

class SwTextFormatInfo : public SwTextPaintInfo
{
    // temporary arguments for hyphenation
    css::beans::PropertyValues   aHyphVals;

    SwLineLayout    *pRoot;       // The Root of the current line (pCurr)
    SwLinePortion   *pLast;       // The last Portion
    SwFlyPortion    *pFly;        // The following FlyPortion
    SwFieldPortion    *pLastField;    // Wrapped Field
    SwLinePortion   *pUnderflow;  // Underflow: Last Portion
    SwLinePortion   *pRest;       // The Rest is the start of the next Line

    SwTabPortion    *pLastTab;     // The _last_ TabPortion

    sal_Int32 nSoftHyphPos;    // SoftHyphPos for Hyphenation
    sal_Int32 nLineStart;      // Current line start in rText
    sal_Int32 nUnderScorePos;  // enlarge repaint if underscore has been found
    // #i34348# Changed type from sal_uInt16 to SwTwips
    SwTwips nLeft;              // Left margin
    SwTwips nRight;             // Right margin
    SwTwips nFirst;             // EZE
    sal_uInt16 nRealWidth;      // "real" line width
    sal_uInt16 nWidth;          // "virtual" line width
    sal_uInt16 nLineHeight;     // Final height after CalcLine
    sal_uInt16 nLineNetHeight; // line height without spacing
    sal_uInt16 nForcedLeftMargin; // Shift of left margin due to frame

    sal_Int16  nMinLeading;     // minimum number of chars before hyphenation point
    sal_Int16  nMinTrailing;    // minimum number of chars after hyphenation point
    sal_Int16  nMinWordLength;  // minimum length of word to be hyphenated

    bool bFull : 1;             // Line is full
    bool bFootnoteDone : 1;          // Footnote already formatted
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
    bool _CheckFootnotePortion( SwLineLayout* pCurr );

public:
    void CtorInitTextFormatInfo( OutputDevice* pRenderContext, SwTextFrm *pFrm, const bool bInterHyph = false,
        const bool bQuick = false, const bool bTst = false );
    SwTextFormatInfo(OutputDevice* pRenderContext, SwTextFrm *pFrame, const bool bInterHyphL = false,
            const bool bQuickL = false, const bool bTst = false);

    // For the formatting inside a double line in a line (multi-line portion)
    // we need a modified text-format-info:
    SwTextFormatInfo( const SwTextFormatInfo& rInf, SwLineLayout& rLay,
        SwTwips nActWidth );

    inline sal_uInt16 Width() const { return nWidth; }
    inline void Width( const sal_uInt16 nNew ) { nWidth = nNew; }
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
    inline sal_uInt16 RealWidth() const { return nRealWidth; }
    inline void RealWidth( const sal_uInt16 nNew ) { nRealWidth = nNew; }
    inline sal_uInt16 ForcedLeftMargin() const { return nForcedLeftMargin; }
    inline void ForcedLeftMargin( const sal_uInt16 nN ) { nForcedLeftMargin = nN; }

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
    inline bool IsUnderflow() const { return bUnderflow; }
    inline void ClrUnderflow() { bUnderflow = false; }
    inline bool IsDropInit() const { return bDropInit; }
    inline void SetDropInit( const bool bNew ) { bDropInit = bNew; }
    inline bool IsQuick() const { return bQuick; }
    inline bool IsTest() const { return bTestFormat; }

    inline sal_Int32 GetLineStart() const { return nLineStart; }
    inline void SetLineStart( const sal_Int32 nNew ) { nLineStart = nNew; }

    // these are used during fly calculation
    inline sal_uInt16 GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const sal_uInt16 nNew ) { nLineHeight = nNew; }
    inline sal_uInt16 GetLineNetHeight() const { return nLineNetHeight; }
    inline void SetLineNetHeight( const sal_uInt16 nNew ) { nLineNetHeight = nNew; }

    inline const SwLinePortion *GetUnderflow() const { return pUnderflow; }
    inline SwLinePortion *GetUnderflow() { return pUnderflow; }
    inline void SetUnderflow( SwLinePortion *pNew )
           { pUnderflow = pNew; bUnderflow = true; }
    inline sal_Int32 GetSoftHyphPos() const { return nSoftHyphPos; }
    inline void SetSoftHyphPos( const sal_Int32 nNew ) { nSoftHyphPos = nNew; }

    inline void SetParaFootnote();

    // FlyFrms
    inline SwFlyPortion *GetFly() { return pFly; }
    inline void SetFly( SwFlyPortion *pNew ) { pFly = pNew; }

    inline const SwAttrSet& GetCharAttr() const;

    // Tabs
    inline SwTabPortion *GetLastTab() { return pLastTab; }
    inline void SetLastTab( SwTabPortion *pNew ) { pLastTab = pNew; }
    inline sal_Unicode GetTabDecimal() const { return cTabDecimal; }
    inline void SetTabDecimal( const sal_Unicode cNew ) { cTabDecimal = cNew;}

    inline void ClearHookChar() { cHookChar = 0; }
    inline void SetHookChar( const sal_Unicode cNew ) { cHookChar = cNew; }
    inline sal_Unicode GetHookChar() const { return cHookChar; }

    // Done-Flags
    inline bool IsFootnoteDone() const { return bFootnoteDone; }
    inline void SetFootnoteDone( const bool bNew ) { bFootnoteDone = bNew; }
    inline bool IsErgoDone() const { return bErgoDone; }
    inline void SetErgoDone( const bool bNew ) { bErgoDone = bNew; }
    inline bool IsNumDone() const { return bNumDone; }
    inline void SetNumDone( const bool bNew ) { bNumDone = bNew; }
    inline bool IsArrowDone() const { return bArrowDone; }
    inline void SetArrowDone( const bool bNew ) { bArrowDone = bNew; }

    // For SwTextPortion::Hyphenate
    bool ChgHyph( const bool bNew );

    // Should the hyphenate helper be discarded?
    bool IsHyphenate() const;
    inline sal_Int32 GetUnderScorePos() const { return nUnderScorePos; }
    inline void SetUnderScorePos( sal_Int32 nNew ) { nUnderScorePos = nNew; }

    // Calls HyphenateWord() of Hyphenator
    css::uno::Reference< css::linguistic2::XHyphenatedWord >
                HyphWord( const OUString &rText, const sal_Int32 nMinTrail );
    const css::beans::PropertyValues &
                GetHyphValues() const;

    bool CheckFootnotePortion( SwLineLayout* pCurr )
        { return IsFootnoteInside() && _CheckFootnotePortion( pCurr ); }

    // Dropcaps called by SwTextFormatter::CTOR
    const SwFormatDrop *GetDropFormat() const;

    // Sets the last SwKernPortion as pLast, if it is followed by empty portions
    bool LastKernPortion();

    // Looks for tabs, TabDec, TXTATR and BRK from nIdx until nEnd.
    // Return: Position; sets cHookChar if necessary
    sal_Int32 ScanPortionEnd( const sal_Int32 nStart, const sal_Int32 nEnd );

    inline void SetTabOverflow( bool bOverflow ) { bTabOverflow = bOverflow; }
    inline bool IsTabOverflow() { return bTabOverflow; }

};

/**
 * For the text replacement and restoration of SwTextSizeInfo.
 * The way this is done is a bit of a hack: Although rInf is const we change it
 * anyway.
 * Because rInf is restorated again in the DTOR, we can do this.
 * You could call it a "logical const", if you wish.
 */
class SwTextSlot
{
    OUString aText;
    std::shared_ptr<vcl::TextLayoutCache> m_pOldCachedVclData;
    const OUString *pOldText;
    const SwWrongList* pOldSmartTagList;
    const SwWrongList* pOldGrammarCheckList;
    SwWrongList* pTempList;
    sal_Int32 nIdx;
    sal_Int32 nLen;
    bool bOn;
protected:
    SwTextSizeInfo *pInf;
public:
    // The replacement string originates either from the portion via GetExpText()
    // or from the rCh, if it is not empty.
    SwTextSlot( const SwTextSizeInfo *pNew, const SwLinePortion *pPor, bool bTextLen,
               bool bExgLists, OUString const & rCh = OUString() );
    ~SwTextSlot();
};

class SwFontSave
{
    SwTextSizeInfo *pInf;
    SwFont        *pFnt;
    SwAttrIter    *pIter;
public:
    SwFontSave( const SwTextSizeInfo &rInf, SwFont *pFnt,
                SwAttrIter* pItr = NULL );
   ~SwFontSave();
};

inline sal_uInt16 SwTextSizeInfo::GetAscent() const
{
    SAL_WARN_IF( !GetOut(), "sw.core", "SwTextSizeInfo::GetAscent() without m_pOut" );

    return const_cast<SwFont*>(GetFont())->GetAscent( m_pVsh, *GetOut() );
}

inline sal_uInt16 SwTextSizeInfo::GetTextHeight() const
{
    SAL_WARN_IF( !GetOut(), "sw.core", "SwTextSizeInfo::GetTextHeight() without m_pOut" );

    return const_cast<SwFont*>(GetFont())->GetHeight( m_pVsh, *GetOut() );
}

inline SwPosSize SwTextSizeInfo::GetTextSize( const OUString &rText ) const
{
    return GetTextSize( m_pOut, 0, rText, 0, rText.getLength(), 0 );
}

inline SwPosSize SwTextSizeInfo::GetTextSize( const SwScriptInfo* pSI,
                                            const sal_Int32 nNewIdx,
                                            const sal_Int32 nNewLen,
                                            const sal_uInt16 nCompress ) const
{
    return GetTextSize( m_pOut, pSI, *m_pText, nNewIdx, nNewLen, nCompress );
}

inline SwTwips SwTextPaintInfo::GetPaintOfst() const
{
    return GetParaPortion()->GetRepaint().GetOfst();
}

inline void SwTextPaintInfo::SetPaintOfst( const SwTwips nNew )
{
    GetParaPortion()->GetRepaint().SetOfst( nNew );
}

inline void SwTextPaintInfo::DrawText( const OUString &rText,
                            const SwLinePortion &rPor,
                            const sal_Int32 nStart, const sal_Int32 nLength,
                            const bool bKern ) const
{
    const_cast<SwTextPaintInfo*>(this)->_DrawText( rText, rPor, nStart, nLength, bKern );
}

inline void SwTextPaintInfo::DrawText( const SwLinePortion &rPor,
        const sal_Int32 nLength, const bool bKern ) const
{
    const_cast<SwTextPaintInfo*>(this)->_DrawText( *m_pText, rPor, m_nIdx, nLength, bKern );
}

inline void SwTextPaintInfo::DrawMarkedText( const SwLinePortion &rPor,
                                            const sal_Int32 nLength,
                                            const bool bKern,
                                            const bool bWrong,
                                            const bool bSmartTags,
                                            const bool bGrammarCheck ) const
{
    const_cast<SwTextPaintInfo*>(this)->_DrawText( *m_pText, rPor, m_nIdx, nLength, bKern, bWrong, bSmartTags, bGrammarCheck );
}

inline sal_Int32 SwTextFormatInfo::GetReformatStart() const
{
    return GetParaPortion()->GetReformat().Start();
}

inline const SwAttrSet& SwTextFormatInfo::GetCharAttr() const
{
    return GetTextFrm()->GetTextNode()->GetSwAttrSet();
}

inline void SwTextFormatInfo::SetParaFootnote()
{
    GetTextFrm()->SetFootnote( true );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
