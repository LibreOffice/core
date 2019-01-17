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
#include <memory>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <map>

#include <swtypes.hxx>
#include <swrect.hxx>
#include <txtfly.hxx>
#include <swfont.hxx>
#include "porlay.hxx"
#include <txtfrm.hxx>
#include <ndtxt.hxx>
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
class SwTextFrame;
class SwTextSizeInfo;
class SwViewOption;
class SwViewShell;
class SwAttrIter;
struct SwMultiCreator;
class SwMultiPortion;
namespace sw { class WrongListIterator; }

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

    std::unique_ptr<SvxTabStopItem> pRuler;
    const SvxLineSpacingItem *pSpace;
    SvxParaVertAlignItem::Align nVertAlign;
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
    const SvxLineSpacingItem *GetLineSpacing() const { return pSpace; }
    sal_uInt16 GetDefTabStop() const { return nDefTabStop; }
    void SetDefTabStop( sal_uInt16 nNew ) const
        { const_cast<SwLineInfo*>(this)->nDefTabStop = nNew; }

    // vertical alignment
    SvxParaVertAlignItem::Align GetVertAlign() const { return nVertAlign; }
    bool HasSpecialAlign( bool bVert ) const
        { return bVert ?
                 ( SvxParaVertAlignItem::Align::Baseline  != nVertAlign ) :
                 ( SvxParaVertAlignItem::Align::Baseline  != nVertAlign &&
                   SvxParaVertAlignItem::Align::Automatic != nVertAlign ); }

    sal_uInt16 NumberOfTabStops() const;

    bool IsListTabStopIncluded() const
    {
        return bListTabStopIncluded;
    }
    long GetListTabStopPosition() const
    {
        return nListTabStopPosition;
    }
};

class SwTextInfo
{
    // Implementation in txthyph.cxx
    friend void SetParaPortion( SwTextInfo *pInf, SwParaPortion *pRoot );
    SwParaPortion *m_pPara;
    TextFrameIndex m_nTextStart; // TextOfst for Follows

protected:
    SwTextInfo()
        : m_pPara(nullptr)
        , m_nTextStart(0)
    {}

public:
    void CtorInitTextInfo( SwTextFrame *pFrame );
    SwTextInfo( const SwTextInfo &rInf );
    explicit SwTextInfo( SwTextFrame *pFrame ) { CtorInitTextInfo( pFrame ); }
    SwParaPortion *GetParaPortion() { return m_pPara; }
    const SwParaPortion *GetParaPortion() const { return m_pPara; }
    TextFrameIndex GetTextStart() const { return m_nTextStart; }
};

class SwTextSizeInfo : public SwTextInfo
{
private:
    typedef std::map< SwLinePortion const *, sal_uInt16 > SwTextPortionMap;

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
    SwTextFrame *m_pFrame;
    const SwViewOption *m_pOpt;
    const OUString *m_pText;
    TextFrameIndex m_nIdx;
    TextFrameIndex m_nLen;
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
    void CtorInitTextSizeInfo( OutputDevice* pRenderContext, SwTextFrame *pFrame,
                   TextFrameIndex nIdx);
    SwTextSizeInfo();
public:
    SwTextSizeInfo( const SwTextSizeInfo &rInf );
    SwTextSizeInfo( const SwTextSizeInfo &rInf, const OUString* pText,
                   TextFrameIndex nIdx = TextFrameIndex(0) );
    SwTextSizeInfo(SwTextFrame *pTextFrame, TextFrameIndex nIndex = TextFrameIndex(0));

    // GetMultiAttr returns the text attribute of the multiportion,
    // if rPos is inside any multi-line part.
    // rPos will set to the end of the multi-line part.
    std::unique_ptr<SwMultiCreator> GetMultiCreator(TextFrameIndex &rPos, SwMultiPortion const* pM) const;

    bool OnWin() const { return m_bOnWin; }
    void SetOnWin( const bool bNew ) { m_bOnWin = bNew; }
    bool NotEOL() const { return m_bNotEOL; }
    void SetNotEOL( const bool bNew ) { m_bNotEOL = bNew; }
    bool URLNotify() const { return m_bURLNotify; }
    bool StopUnderflow() const { return m_bStopUnderflow; }
    void SetStopUnderflow( const bool bNew ) { m_bStopUnderflow = bNew; }
    bool IsFootnoteInside() const { return m_bFootnoteInside; }
    void SetFootnoteInside( const bool bNew ) { m_bFootnoteInside = bNew; }
    bool IsOtherThanFootnoteInside() const { return m_bOtherThanFootnoteInside; }
    void SetOtherThanFootnoteInside( const bool bNew ) { m_bOtherThanFootnoteInside = bNew; }
    bool IsMulti() const { return m_bMulti; }
    void SetMulti( const bool bNew ) { m_bMulti = bNew; }
    bool IsFirstMulti() const { return m_bFirstMulti; }
    void SetFirstMulti( const bool bNew ) { m_bFirstMulti = bNew; }
    bool IsRuby() const { return m_bRuby; }
    void SetRuby( const bool bNew ) { m_bRuby = bNew; }
    bool IsHanging() const { return m_bHanging; }
    void SetHanging( const bool bNew ) { m_bHanging = bNew; }
    bool HasScriptSpace() const { return m_bScriptSpace; }
    void SetScriptSpace( const bool bNew ) { m_bScriptSpace = bNew; }
    bool HasForbiddenChars() const { return m_bForbiddenChars; }
    void SetForbiddenChars( const bool bN ) { m_bForbiddenChars = bN; }
    bool SnapToGrid() const { return m_bSnapToGrid; }
    void SetSnapToGrid( const bool bN ) { m_bSnapToGrid = bN; }
    sal_uInt8 GetDirection() const { return m_nDirection; }
    void SetDirection( const sal_uInt8 nNew ) { m_nDirection = nNew; }
    bool IsRotated() const { return ( 1 & m_nDirection ); }

    SwViewShell *GetVsh() { return m_pVsh; }
    const SwViewShell *GetVsh() const { return m_pVsh; }

    vcl::RenderContext *GetOut() { return m_pOut; }
    const vcl::RenderContext *GetOut() const { return m_pOut; }
    void SetOut( OutputDevice* pNewOut ) { m_pOut = pNewOut; }

    vcl::RenderContext *GetRefDev() { return m_pRef; }
    const vcl::RenderContext *GetRefDev() const { return m_pRef; }

    SwFont *GetFont() { return m_pFnt; }
    const SwFont *GetFont() const { return m_pFnt; }
    void SetFont( SwFont *pNew ) { m_pFnt = pNew; }
    void SelectFont();
    void SetUnderFnt( SwUnderlineFont* pNew ) { m_pUnderFnt = pNew; }
    SwUnderlineFont* GetUnderFnt() const { return m_pUnderFnt; }

    const  SwViewOption &GetOpt() const { return *m_pOpt; }
    const OUString &GetText() const { return *m_pText; }
    sal_Unicode GetChar(TextFrameIndex const nPos) const {
        if (m_pText && nPos < TextFrameIndex(m_pText->getLength())) return (*m_pText)[sal_Int32(nPos)];
        return 0;
    }

    sal_uInt16      GetTextHeight() const;

    SwPosSize GetTextSize( OutputDevice* pOut, const SwScriptInfo* pSI,
                          const OUString& rText, TextFrameIndex nIdx,
                          TextFrameIndex nLen ) const;
    SwPosSize GetTextSize() const;
    void GetTextSize( const SwScriptInfo* pSI, TextFrameIndex nIdx,
                      TextFrameIndex nLen, const sal_uInt16 nComp,
                      sal_uInt16& nMinSize, sal_uInt16& nMaxSizeDiff,
                      vcl::TextLayoutCache const* = nullptr) const;
    inline SwPosSize GetTextSize(const SwScriptInfo* pSI, TextFrameIndex nIdx,
                                 TextFrameIndex nLen) const;
    inline SwPosSize GetTextSize( const OUString &rText ) const;

    TextFrameIndex GetTextBreak( const long nLineWidth,
                            const TextFrameIndex nMaxLen,
                            const sal_uInt16 nComp,
                            vcl::TextLayoutCache const*) const;
    TextFrameIndex GetTextBreak( const long nLineWidth,
                            const TextFrameIndex nMaxLen,
                            const sal_uInt16 nComp,
                            TextFrameIndex& rExtraCharPos,
                            vcl::TextLayoutCache const*) const;

    sal_uInt16 GetAscent() const;

    TextFrameIndex GetIdx() const { return m_nIdx; }
    void SetIdx(const TextFrameIndex nNew) { m_nIdx = nNew; }
    TextFrameIndex GetLen() const { return m_nLen; }
    void SetLen(const TextFrameIndex nNew) { m_nLen = nNew; }
    void SetText( const OUString &rNew ){ m_pText = &rNew; }

    // No Bullets for the symbol font!
    bool IsNoSymbol() const
    { return RTL_TEXTENCODING_SYMBOL != m_pFnt->GetCharSet( m_pFnt->GetActual() ); }

    void NoteAnimation() const;

    // Home is where Your heart is...
    SwTextFrame *GetTextFrame() { return m_pFrame; }
    const SwTextFrame *GetTextFrame() const { return m_pFrame; }

    bool HasHint(TextFrameIndex nPos) const;

    // If Kana Compression is enabled, a minimum and maximum portion width
    // is calculated. We format lines with minimal size and share remaining
    // space among compressed kanas.
    // During formatting, the maximum values of compressable portions are
    // stored in m_aMaxWidth and discarded after a line has been formatted.
    void SetMaxWidthDiff( const SwLinePortion *nKey, sal_uInt16 nVal )
    {
        m_aMaxWidth.insert( std::make_pair( nKey, nVal ) );
    };
    sal_uInt16 GetMaxWidthDiff( const SwLinePortion *nKey )
    {
        SwTextPortionMap::iterator it = m_aMaxWidth.find( nKey );

        if( it != m_aMaxWidth.end() )
            return it->second;
        else
            return 0;
    };
    void ResetMaxWidthDiff()
    {
        m_aMaxWidth.clear();
    };
    bool CompressLine()
    {
        return !m_aMaxWidth.empty();
    };

    // Feature: Kana Compression

    sal_uInt16 GetKanaIdx() const { return m_nKanaIdx; }
    void ResetKanaIdx(){ m_nKanaIdx = 0; }
    void SetKanaIdx( sal_uInt16 nNew ) { m_nKanaIdx = nNew; }
    void IncKanaIdx() { ++m_nKanaIdx; }
    void SetKanaComp( std::deque<sal_uInt16> *pNew ){ m_pKanaComp = pNew; }
    std::deque<sal_uInt16>* GetpKanaComp() const { return m_pKanaComp; }
    sal_uInt16 GetKanaComp() const
        { return ( m_pKanaComp && m_nKanaIdx < m_pKanaComp->size() )
                   ? (*m_pKanaComp)[m_nKanaIdx] : 0; }

    const std::shared_ptr<vcl::TextLayoutCache>& GetCachedVclData() const
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
    sw::WrongListIterator *m_pWrongList;
    sw::WrongListIterator *m_pGrammarCheckList;
    sw::WrongListIterator *m_pSmartTags;
    std::vector<long>* pSpaceAdd;
    const SvxBrushItem *pBrushItem; // For the background
    SwTextFly    aTextFly;    // Calculate the FlyFrame
    Point       aPos;       // Paint position
    SwRect      aPaintRect; // Original paint rect (from Layout paint)

    sal_uInt16 nSpaceIdx;
    void DrawText_(const OUString &rText, const SwLinePortion &rPor,
                   const TextFrameIndex nIdx, const TextFrameIndex nLen,
                   const bool bKern, const bool bWrong = false,
                   const bool bSmartTag = false,
                   const bool bGrammarCheck = false );

    SwTextPaintInfo &operator=(const SwTextPaintInfo&) = delete;

protected:
    SwTextPaintInfo()
        : m_pWrongList(nullptr)
        , m_pGrammarCheckList(nullptr)
        , m_pSmartTags(nullptr)
        , pSpaceAdd(nullptr)
        , pBrushItem(nullptr)
        , nSpaceIdx(0)
        {}

public:
    SwTextPaintInfo( const SwTextPaintInfo &rInf );
    SwTextPaintInfo( const SwTextPaintInfo &rInf, const OUString* pText );

    void CtorInitTextPaintInfo( OutputDevice* pRenderContext, SwTextFrame *pFrame, const SwRect &rPaint );

    const SvxBrushItem *GetBrushItem() const { return pBrushItem; }

    SwTextPaintInfo( SwTextFrame *pFrame, const SwRect &rPaint );

    SwTwips X() const { return aPos.X(); }
    void X( const long nNew ) { aPos.setX(nNew); }
    SwTwips Y() const { return aPos.Y(); }
    void Y( const SwTwips nNew ) { aPos.setY(nNew); }

    SwTextFly& GetTextFly() { return aTextFly; }
    const SwTextFly& GetTextFly() const { return aTextFly; }
    inline void DrawText( const OUString &rText, const SwLinePortion &rPor,
                          TextFrameIndex nIdx = TextFrameIndex(0),
                          TextFrameIndex nLen = TextFrameIndex(COMPLETE_STRING),
                          const bool bKern = false) const;
    inline void DrawText( const SwLinePortion &rPor, TextFrameIndex nLen,
                          const bool bKern = false ) const;
    inline void DrawMarkedText( const SwLinePortion &rPor, TextFrameIndex nLen,
                                const bool bWrong,
                                const bool bSmartTags,
                                const bool bGrammarCheck ) const;

    void DrawRect( const SwRect &rRect, bool bRetouche ) const;

    void DrawTab( const SwLinePortion &rPor ) const;
    void DrawLineBreak( const SwLinePortion &rPor ) const;
    void DrawRedArrow( const SwLinePortion &rPor ) const;
    void DrawPostIts( bool bScript ) const;
    void DrawBackground( const SwLinePortion &rPor ) const;
    void DrawViewOpt( const SwLinePortion &rPor, PortionType nWhich ) const;
    void DrawBackBrush( const SwLinePortion &rPor ) const;

    /**
     * Draw character border around a line portion.
     *
     * @param[in]   rPor    line portion around which border have to be drawn.
    **/
    void DrawBorder( const SwLinePortion &rPor ) const;

    void DrawCheckBox(const SwFieldFormCheckboxPortion &rPor, bool bChecked) const;

    /**
     * Calculate the rectangular area where the portion takes place.
     * @param[in]   rPor        portion for which the method specify the painting area
     * @param[out]  pRect       whole area of the portion
     * @param[out]  pIntersect  part of the portion area clipped by OutputDevice's clip region
     * @param[in]   bInsideBox  area of portion's content, padding and border, but shadow
     *                          is excluded (e.g. for background)
    **/
    void CalcRect( const SwLinePortion& rPor, SwRect* pRect,
                   SwRect* pIntersect = nullptr, const bool bInsideBox = false ) const;

    inline SwTwips GetPaintOfst() const;
    inline void SetPaintOfst( const SwTwips nNew );
    const Point &GetPos() const { return aPos; }
    void SetPos( const Point &rNew ) { aPos = rNew; }

    const SwRect &GetPaintRect() const { return aPaintRect; }

    // STUFF FOR JUSTIFIED ALIGNMENT

    sal_uInt16 GetSpaceIdx() const { return nSpaceIdx; }
    void ResetSpaceIdx(){nSpaceIdx = 0; }
    void SetSpaceIdx( sal_uInt16 nNew ) { nSpaceIdx = nNew; }
    void IncSpaceIdx() { ++nSpaceIdx; }
    void RemoveFirstSpaceAdd() { pSpaceAdd->erase( pSpaceAdd->begin() ); }
    long GetSpaceAdd() const
        { return ( pSpaceAdd && nSpaceIdx < pSpaceAdd->size() )
                   ? (*pSpaceAdd)[nSpaceIdx] : 0; }

    void SetpSpaceAdd( std::vector<long>* pNew ){ pSpaceAdd = pNew; }
    std::vector<long>* GetpSpaceAdd() const { return pSpaceAdd; }

    void SetWrongList(sw::WrongListIterator *const pNew) { m_pWrongList = pNew; }
    sw::WrongListIterator* GetpWrongList() const { return m_pWrongList; }

    void SetGrammarCheckList(sw::WrongListIterator *const pNew) { m_pGrammarCheckList = pNew; }
    sw::WrongListIterator* GetGrammarCheckList() const { return m_pGrammarCheckList; }

    void SetSmartTags(sw::WrongListIterator *const pNew) { m_pSmartTags = pNew; }
    sw::WrongListIterator* GetSmartTags() const { return m_pSmartTags; }
};

class SwTextFormatInfo : public SwTextPaintInfo
{
    // temporary arguments for hyphenation
    css::beans::PropertyValues   m_aHyphVals;

    SwLineLayout    *m_pRoot;       // The Root of the current line (pCurr)
    SwLinePortion   *m_pLast;       // The last Portion
    SwFlyPortion    *m_pFly;        // The following FlyPortion
    SwLinePortion   *m_pUnderflow;  // Underflow: Last Portion
    SwLinePortion   *m_pRest;       // The Rest is the start of the next Line

    SwTabPortion    *m_pLastTab;     // The _last_ TabPortion

    TextFrameIndex m_nSoftHyphPos;   ///< SoftHyphPos for Hyphenation
    TextFrameIndex m_nLineStart;     ///< Current line start in rText
    TextFrameIndex m_nUnderScorePos; ///< enlarge repaint if underscore has been found
    // #i34348# Changed type from sal_uInt16 to SwTwips
    SwTwips m_nLeft;              // Left margin
    SwTwips m_nRight;             // Right margin
    SwTwips m_nFirst;             // EZE
    /// First or left margin, depending on context.
    SwTwips m_nLeftMargin = 0;
    sal_uInt16 m_nRealWidth;      // "real" line width
    sal_uInt16 m_nWidth;          // "virtual" line width
    sal_uInt16 m_nLineHeight;     // Final height after CalcLine
    sal_uInt16 m_nLineNetHeight; // line height without spacing
    sal_uInt16 m_nForcedLeftMargin; // Shift of left margin due to frame

    bool m_bFull : 1;             // Line is full
    bool m_bFootnoteDone : 1;          // Footnote already formatted
    bool m_bErgoDone : 1;         // ErgoDone already formatted
    bool m_bNumDone : 1;          // bNumDone already formatted
    bool m_bArrowDone : 1;        // Arrow to the left for scrolling paragraphs
    bool m_bStop : 1;             // Cancel immediately, discarding the line
    bool m_bNewLine : 1;          // Format another line
    bool m_bShift : 1;            // Position change: Repaint until further notice
    bool m_bUnderflow : 1;        // Context: Underflow() ?
    bool m_bInterHyph : 1;        // Interactive hyphenation?
    bool m_bAutoHyph : 1;         // Automatic hyphenation?
    bool m_bDropInit : 1;         // Set DropWidth
    bool m_bQuick : 1;            // FormatQuick()
    bool m_bNoEndHyph : 1;        // Switch off hyphenation at the line end (due to MaxHyphens)
    bool m_bNoMidHyph : 1;        // Switch off hyphenation before flys (due to MaxHyphens)
    bool m_bIgnoreFly : 1;        // FitToContent ignores flys
    bool m_bFakeLineStart : 1;    // String has been replaced by field portion
                                // info structure only pretends that we are at
                                // the beginning of a line
    bool m_bTabOverflow : 1;      // Tabs are expanding after the end margin
    bool m_bTestFormat : 1;       // Test formatting from WouldFit, no notification etc.

    sal_Unicode   m_cTabDecimal;  // the current decimal delimiter
    sal_Unicode   m_cHookChar;    // For tabs in fields etc.
    sal_uInt8   m_nMaxHyph;       // Max. line count of followup hyphenations

    // Hyphenating ...
    bool InitHyph( const bool bAuto = false );
    bool CheckFootnotePortion_( SwLineLayout const * pCurr );

public:
    void CtorInitTextFormatInfo( OutputDevice* pRenderContext, SwTextFrame *pFrame, const bool bInterHyph = false,
        const bool bQuick = false, const bool bTst = false );
    SwTextFormatInfo(OutputDevice* pRenderContext, SwTextFrame *pFrame, const bool bInterHyphL = false,
            const bool bQuickL = false, const bool bTst = false);

    // For the formatting inside a double line in a line (multi-line portion)
    // we need a modified text-format-info:
    SwTextFormatInfo( const SwTextFormatInfo& rInf, SwLineLayout& rLay,
        SwTwips nActWidth );

    sal_uInt16 Width() const { return m_nWidth; }
    void Width( const sal_uInt16 nNew ) { m_nWidth = nNew; }
           void Init();

    /**
     * Returns the distance between the current horizontal position and the end
     * of the line.
     */
    SwTwips GetLineWidth();

    // Returns the first changed position of the paragraph
    inline TextFrameIndex GetReformatStart() const;

    // Margins
    SwTwips Left() const { return m_nLeft; }
    void Left( const SwTwips nNew ) { m_nLeft = nNew; }
    SwTwips Right() const { return m_nRight; }
    void Right( const SwTwips nNew ) { m_nRight = nNew; }
    SwTwips First() const { return m_nFirst; }
    void First( const SwTwips nNew ) { m_nFirst = nNew; }
    void LeftMargin( const SwTwips nNew) { m_nLeftMargin = nNew; }
    sal_uInt16 RealWidth() const { return m_nRealWidth; }
    void RealWidth( const sal_uInt16 nNew ) { m_nRealWidth = nNew; }
    sal_uInt16 ForcedLeftMargin() const { return m_nForcedLeftMargin; }
    void ForcedLeftMargin( const sal_uInt16 nN ) { m_nForcedLeftMargin = nN; }

    sal_uInt8 &MaxHyph() { return m_nMaxHyph; }
    const sal_uInt8 &MaxHyph() const { return m_nMaxHyph; }

    SwLineLayout *GetRoot() { return m_pRoot; }
    const SwLineLayout *GetRoot() const { return m_pRoot; }

    void SetRoot( SwLineLayout *pNew ) { m_pRoot = pNew; }
    SwLinePortion *GetLast() { return m_pLast; }
    void SetLast( SwLinePortion *pNewLast ) { m_pLast = pNewLast; }
    bool IsFull() const { return m_bFull; }
    void SetFull( const bool bNew ) { m_bFull = bNew; }
    bool IsHyphForbud() const
        { return m_pFly ? m_bNoMidHyph : m_bNoEndHyph; }
    void ChkNoHyph( const sal_uInt8 bEnd, const sal_uInt8 bMid )
        { m_bNoEndHyph = (m_nMaxHyph && bEnd >= m_nMaxHyph);
          m_bNoMidHyph = (m_nMaxHyph && bMid >= m_nMaxHyph); }
    bool IsIgnoreFly() const { return m_bIgnoreFly; }
    void SetIgnoreFly( const bool bNew ) { m_bIgnoreFly = bNew; }
    bool IsFakeLineStart() const { return m_bFakeLineStart; }
    void SetFakeLineStart( const bool bNew ) { m_bFakeLineStart = bNew; }
    bool IsStop() const { return m_bStop; }
    void SetStop( const bool bNew ) { m_bStop = bNew; }
    SwLinePortion *GetRest() { return m_pRest; }
    void SetRest( SwLinePortion *pNewRest ) { m_pRest = pNewRest; }
    bool IsNewLine() const { return m_bNewLine; }
    void SetNewLine( const bool bNew ) { m_bNewLine = bNew; }
    bool IsShift() const { return m_bShift; }
    void SetShift( const bool bNew ) { m_bShift = bNew; }
    bool IsInterHyph() const { return m_bInterHyph; }
    bool IsUnderflow() const { return m_bUnderflow; }
    void ClrUnderflow() { m_bUnderflow = false; }
    bool IsDropInit() const { return m_bDropInit; }
    void SetDropInit( const bool bNew ) { m_bDropInit = bNew; }
    bool IsQuick() const { return m_bQuick; }
    bool IsTest() const { return m_bTestFormat; }

    TextFrameIndex GetLineStart() const { return m_nLineStart; }
    void SetLineStart(TextFrameIndex const nNew) { m_nLineStart = nNew; }

    // these are used during fly calculation
    sal_uInt16 GetLineHeight() const { return m_nLineHeight; }
    void SetLineHeight( const sal_uInt16 nNew ) { m_nLineHeight = nNew; }
    sal_uInt16 GetLineNetHeight() const { return m_nLineNetHeight; }
    void SetLineNetHeight( const sal_uInt16 nNew ) { m_nLineNetHeight = nNew; }

    const SwLinePortion *GetUnderflow() const { return m_pUnderflow; }
    SwLinePortion *GetUnderflow() { return m_pUnderflow; }
    void SetUnderflow( SwLinePortion *pNew )
           { m_pUnderflow = pNew; m_bUnderflow = true; }
    TextFrameIndex GetSoftHyphPos() const { return m_nSoftHyphPos; }
    void SetSoftHyphPos(TextFrameIndex const nNew) { m_nSoftHyphPos = nNew; }

    inline void SetParaFootnote();

    // FlyFrames
    SwFlyPortion *GetFly() { return m_pFly; }
    void SetFly( SwFlyPortion *pNew ) { m_pFly = pNew; }

    inline const SwAttrSet& GetCharAttr() const;

    // Tabs
    SwTabPortion *GetLastTab() { return m_pLastTab; }
    void SetLastTab( SwTabPortion *pNew ) { m_pLastTab = pNew; }
    sal_Unicode GetTabDecimal() const { return m_cTabDecimal; }
    void SetTabDecimal( const sal_Unicode cNew ) { m_cTabDecimal = cNew;}

    void ClearHookChar() { m_cHookChar = 0; }
    void SetHookChar( const sal_Unicode cNew ) { m_cHookChar = cNew; }
    sal_Unicode GetHookChar() const { return m_cHookChar; }

    // Done-Flags
    bool IsFootnoteDone() const { return m_bFootnoteDone; }
    void SetFootnoteDone( const bool bNew ) { m_bFootnoteDone = bNew; }
    bool IsErgoDone() const { return m_bErgoDone; }
    void SetErgoDone( const bool bNew ) { m_bErgoDone = bNew; }
    bool IsNumDone() const { return m_bNumDone; }
    void SetNumDone( const bool bNew ) { m_bNumDone = bNew; }
    bool IsArrowDone() const { return m_bArrowDone; }
    void SetArrowDone( const bool bNew ) { m_bArrowDone = bNew; }

    // For SwTextPortion::Hyphenate
    bool ChgHyph( const bool bNew );

    // Should the hyphenate helper be discarded?
    bool IsHyphenate() const;
    TextFrameIndex GetUnderScorePos() const { return m_nUnderScorePos; }
    void SetUnderScorePos(TextFrameIndex const nNew) { m_nUnderScorePos = nNew; }

    // Calls HyphenateWord() of Hyphenator
    css::uno::Reference< css::linguistic2::XHyphenatedWord >
                HyphWord( const OUString &rText, const sal_Int32 nMinTrail );
    const css::beans::PropertyValues & GetHyphValues() const;

    bool CheckFootnotePortion( SwLineLayout const * pCurr )
        { return IsFootnoteInside() && CheckFootnotePortion_( pCurr ); }

    // Dropcaps called by SwTextFormatter::CTOR
    const SwFormatDrop *GetDropFormat() const;

    // Sets the last SwKernPortion as pLast, if it is followed by empty portions
    bool LastKernPortion();

    // Looks for tabs, TabDec, TXTATR and BRK from nIdx until nEnd.
    // Return: Position; sets cHookChar if necessary
    TextFrameIndex ScanPortionEnd(TextFrameIndex nStart, TextFrameIndex nEnd);

    void SetTabOverflow( bool bOverflow ) { m_bTabOverflow = bOverflow; }
    bool IsTabOverflow() { return m_bTabOverflow; }

};

/**
 * For the text replacement and restoration of SwTextSizeInfo.
 * The way this is done is a bit of a hack: Although rInf is const we change it
 * anyway.
 * Because rInf is restored again in the DTOR, we can do this.
 * You could call it a "logical const", if you wish.
 */
class SwTextSlot final
{
    OUString aText;
    std::shared_ptr<vcl::TextLayoutCache> m_pOldCachedVclData;
    const OUString *pOldText;
    sw::WrongListIterator * m_pOldSmartTagList;
    sw::WrongListIterator * m_pOldGrammarCheckList;
    std::unique_ptr<SwWrongList> m_pTempList;
    std::unique_ptr<sw::WrongListIterator> m_pTempIter;
    TextFrameIndex nIdx;
    TextFrameIndex nLen;
    bool bOn;
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
                SwAttrIter* pItr = nullptr );
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
    return GetTextSize(m_pOut, nullptr, rText, TextFrameIndex(0), TextFrameIndex(rText.getLength()));
}

inline SwPosSize SwTextSizeInfo::GetTextSize( const SwScriptInfo* pSI,
                                            TextFrameIndex const nNewIdx,
                                            TextFrameIndex const nNewLen) const
{
    return GetTextSize( m_pOut, pSI, *m_pText, nNewIdx, nNewLen );
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
                            const TextFrameIndex nStart, const TextFrameIndex nLength,
                            const bool bKern ) const
{
    const_cast<SwTextPaintInfo*>(this)->DrawText_( rText, rPor, nStart, nLength, bKern );
}

inline void SwTextPaintInfo::DrawText( const SwLinePortion &rPor,
        const TextFrameIndex nLength, const bool bKern ) const
{
    const_cast<SwTextPaintInfo*>(this)->DrawText_( *m_pText, rPor, m_nIdx, nLength, bKern );
}

inline void SwTextPaintInfo::DrawMarkedText( const SwLinePortion &rPor,
                                            const TextFrameIndex nLength,
                                            const bool bWrong,
                                            const bool bSmartTags,
                                            const bool bGrammarCheck ) const
{
    const_cast<SwTextPaintInfo*>(this)->DrawText_( *m_pText, rPor, m_nIdx, nLength, false/*bKern*/, bWrong, bSmartTags, bGrammarCheck );
}

inline TextFrameIndex SwTextFormatInfo::GetReformatStart() const
{
    return GetParaPortion()->GetReformat().Start();
}

inline const SwAttrSet& SwTextFormatInfo::GetCharAttr() const
{
    // sw_redlinehide: this is used for numbering/footnote number portions, so:
    return GetTextFrame()->GetTextNodeForParaProps()->GetSwAttrSet();
}

inline void SwTextFormatInfo::SetParaFootnote()
{
    GetTextFrame()->SetFootnote( true );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
