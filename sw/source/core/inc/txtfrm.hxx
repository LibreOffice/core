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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TXTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TXTFRM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include "cntfrm.hxx"
#include <ndtxt.hxx>
#include "TextFrameIndex.hxx"

namespace com { namespace sun { namespace star { namespace linguistic2 { class XHyphenatedWord; } } } }

namespace sw { namespace mark { class IMark; } }
class SwCharRange;
class SwTextNode;
class SwTextAttrEnd;
class SwTextFormatter;
class SwTextFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class SwBodyFrame;
class SwTextFootnote;
class SwInterHyphInfo;      // Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrameFormat;
class OutputDevice;
class SwTestFormat;
struct SwCursorMoveState;
struct SwFillData;
class SwPortionHandler;
class SwScriptInfo;
enum class ExpandMode;

#define NON_PRINTING_CHARACTER_COLOR Color(0x26, 0x8b, 0xd2)

/// a clone of SwInterHyphInfo, but with TextFrameIndex instead of node index
class SwInterHyphInfoTextFrame
{
private:
    /// output: hyphenated word
    css::uno::Reference<css::linguistic2::XHyphenatedWord> m_xHyphWord;
public:
    /// input: requested range to hyphenate
    TextFrameIndex m_nStart;
    TextFrameIndex m_nEnd;
    /// output: found word
    TextFrameIndex m_nWordStart;
    TextFrameIndex m_nWordLen;

    SwInterHyphInfoTextFrame(SwTextFrame const& rFrame,
            SwTextNode const& rNode, SwInterHyphInfo const& rHyphInfo);
    void UpdateTextNodeHyphInfo(SwTextFrame const& rFrame,
            SwTextNode const& rNode, SwInterHyphInfo & o_rHyphInfo);

    void SetHyphWord(const css::uno::Reference<css::linguistic2::XHyphenatedWord> &xHW)
    {
        m_xHyphWord = xHW;
    }
};

namespace sw {

/**
 * Describes a part of a single text node, which will be part of a text frame,
 * even when redlines are hidden at a layout level.
 */
struct Extent
{
    SwTextNode * /*const logically, but need assignment for std::vector*/ pNode;
    sal_Int32 nStart;
    sal_Int32 nEnd;
    Extent(SwTextNode *const p, sal_Int32 const s, sal_Int32 const e)
        : pNode(p), nStart(s), nEnd(e)
    {
        assert(pNode);
        assert(nStart != nEnd);
    }
};

struct MergedPara;

std::pair<SwTextNode*, sal_Int32> MapViewToModel(MergedPara const&, TextFrameIndex nIndex);
TextFrameIndex MapModelToView(MergedPara const&, SwTextNode const* pNode, sal_Int32 nIndex);

enum class FrameMode { New, Existing };
std::unique_ptr<sw::MergedPara> CheckParaRedlineMerge(SwTextFrame & rFrame, SwTextNode & rTextNode, FrameMode eMode);

bool FrameContainsNode(SwContentFrame const& rFrame, sal_uLong nNodeIndex);
bool IsParaPropsNode(SwRootFrame const& rLayout, SwTextNode const& rNode);
SwTextNode * GetParaPropsNode(SwRootFrame const& rLayout, SwNodeIndex const& rNode);
SwPosition GetParaPropsPos(SwRootFrame const& rLayout, SwPosition const& rPos);
std::pair<SwTextNode *, SwTextNode *>
GetFirstAndLastNode(SwRootFrame const& rLayout, SwNodeIndex const& rPos);

SwTextNode const& GetAttrMerged(SfxItemSet & rFormatSet,
        SwTextNode const& rNode, SwRootFrame const* pLayout);

void GotoPrevLayoutTextFrame(SwNodeIndex & rIndex, SwRootFrame const* pLayout);
void GotoNextLayoutTextFrame(SwNodeIndex & rIndex, SwRootFrame const* pLayout);

TextFrameIndex UpdateMergedParaForDelete(MergedPara & rMerged,
        bool isRealDelete,
        SwTextNode const& rNode, sal_Int32 nIndex, sal_Int32 nLen);

void MoveMergedFlysAndFootnotes(std::vector<SwTextFrame*> const& rFrames,
        SwTextNode const& rFirstNode, SwTextNode & rSecondNode, bool);

void MoveDeletedPrevFrames(const SwTextNode & rDeletedPrev, SwTextNode & rNode);
enum class Recreate { No, ThisNode, Predecessor };
void CheckResetRedlineMergeFlag(SwTextNode & rNode, Recreate eRecreateMerged);

void UpdateFramesForAddDeleteRedline(SwDoc & rDoc, SwPaM const& rPam);
void UpdateFramesForRemoveDeleteRedline(SwDoc & rDoc, SwPaM const& rPam);

void AddRemoveFlysAnchoredToFrameStartingAtNode(
        SwTextFrame & rFrame, SwTextNode & rTextNode,
        std::set<sal_uLong> *pSkipped);

OUString GetExpandTextMerged(SwRootFrame const* pLayout,
        SwTextNode const& rNode, bool bWithNumber,
        bool bWithSpacesForLevel, ExpandMode i_mode);

bool IsMarkHidden(SwRootFrame const& rLayout, ::sw::mark::IMark const& rMark);
bool IsMarkHintHidden(SwRootFrame const& rLayout,
        SwTextNode const& rNode, SwTextAttrEnd const& rHint);

} // namespace sw

/// Represents the visualization of a paragraph. Typical upper is an
/// SwBodyFrame. The first text portion of the first line is az SwParaPortion.
class SW_DLLPUBLIC SwTextFrame: public SwContentFrame
{
    friend class SwTextIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class TextFrameLockGuard; // May Lock()/Unlock()
    friend bool sw_ChangeOffset(SwTextFrame* pFrame, TextFrameIndex nNew);

    /// SwLineLayout cache: the lines are not actually owned by the SwTextFrame
    /// but by this SwCache, so they will be deleted in large documents
    /// if there are too many of them, but the "valid" flags of the frame
    /// will still be set; GetFormatted() is the function that forces
    /// recreation of the SwLineLayout by Format() if necessary.
    static SwCache *s_pTextCache;
    static constexpr long nMinPrtLine = 0;    // This Line must not be underrun when printing
                                // Hack for table cells stretching multiple pages

    sal_uLong  mnAllLines        :24; // Line count for the Paint (including nThisLines)
    sal_uLong  mnThisLines       :8; // Count of Lines of the Frame

    // The x position for flys anchored at this paragraph.
    // These values are calculated in SwTextFrame::CalcBaseOfstForFly()
    SwTwips mnFlyAnchorOfst;
    // The x position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorOfstNoWrap;
    /// The y position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorVertOfstNoWrap;
    SwTwips mnFootnoteLine;
    // OD 2004-03-17 #i11860# - re-factoring of #i11859#
    // member for height of last line (value needed for proportional line spacing)
    SwTwips mnHeightOfLastLine;
    // member for the additional first line offset, which is caused by the list
    // label alignment for list level position and space mode LABEL_ALIGNMENT.
    // This additional first line offset is used for the text formatting.
    // It is NOT used for the determination of printing area.
    SwTwips mnAdditionalFirstLineOffset;

    /// redline merge data
    std::unique_ptr<sw::MergedPara> m_pMergedPara;

    TextFrameIndex mnOffset; // Is the offset in the Content (character count)

    sal_uInt16 mnCacheIndex; // Index into the cache, USHRT_MAX if there's definitely no fitting object in the cache

    // Separates the Master and creates a Follow or adjusts the data in the Follow
    void AdjustFollow_( SwTextFormatter &rLine, TextFrameIndex nOffset,
                               TextFrameIndex nStrEnd, const sal_uInt8 nMode );

    // Iterates all Lines and sets the line spacing using the attribute
    void CalcLineSpace();

    // Only called in Format
    void AdjustFrame( const SwTwips nChgHeight, bool bHasToFit = false );

    // Evaluates the Preps in Format()
    bool CalcPreps();
    void PrepWidows( const sal_uInt16 nNeed, bool bNotify );
    void InvalidateRange_( const SwCharRange &, const long = 0);
    inline void InvalidateRange( const SwCharRange &, const long = 0);

    // WidowsAndOrphans, AdjustFrame, AdjustFollow
    void FormatAdjust( SwTextFormatter &rLine, WidowsAndOrphans &rFrameBreak,
                       TextFrameIndex nStrLen, const bool bDummy );
    void ChangeOffset( SwTextFrame* pFrame, TextFrameIndex nNew );

    bool mbLocked        : 1;        // In the Format?
    bool mbWidow         : 1;        // Is our follow a Widow?
    bool mbJustWidow     : 1;        // Did we just request Widow flag on master?
    bool mbEmpty         : 1;        // Are we an empty paragraph?
    bool mbInFootnoteConnect  : 1;        // Is in Connect at the moment
    bool mbFootnote           : 1;        // Has at least one footnote
    bool mbRepaint       : 1;        // TextFrame: Repaint is ready to be fetched
    bool mbHasBlinkPortions      : 1;        // Contains Blink Portions
    bool mbFieldFollow   : 1;        // Start with Field rest of the Master
    bool mbHasAnimation  : 1;        // Contains animated SwGrfNumPortion
    bool mbIsSwapped     : 1;        // during text formatting we swap the
                                    // width and height for vertical formatting
    // OD 14.03.2003 #i11760# - flag to control, if follow is formatted in
    // method <CalcFollow(..)>.
    // E.g., avoid formatting of follow, if method <SwLayoutFrame::FormatWidthCols(..)>
    // is running.
    bool mbFollowFormatAllowed : 1;

    void ResetPreps();
    void Lock() { mbLocked = true; }
    void Unlock() { mbLocked = false; }
    void SetWidow( const bool bNew ) { mbWidow = bNew; }
    void SetJustWidow( const bool bNew ) { mbJustWidow = bNew; }
    void SetEmpty( const bool bNew ) { mbEmpty = bNew; }
    void SetFieldFollow( const bool bNew ) { mbFieldFollow = bNew; }

    bool IsIdxInside(TextFrameIndex nPos, TextFrameIndex nLen) const;

    // Changes the Frame or not (cf. FlyCnt)
    bool GetCursorOfst_(SwPosition *pPos, const Point &rPoint,
                      const bool bChgFrame, SwCursorMoveState* = nullptr ) const;
    void FillCursorPos( SwFillData &rFill ) const;

    // Format exactly one Line
    bool FormatLine( SwTextFormatter &rLine, const bool bPrev );

    // In order to safe stack space, we split this method:
    // Format_ calls Format_ with parameters
    void Format_( vcl::RenderContext* pRenderContext, SwParaPortion *pPara );
    void Format_( SwTextFormatter &rLine, SwTextFormatInfo &rInf,
                  const bool bAdjust = false );
    void FormatOnceMore( SwTextFormatter &rLine, SwTextFormatInfo &rInf );

    // Formats the Follow and ensures disposing on orphans
    bool CalcFollow(TextFrameIndex nTextOfst);

    virtual void MakePos() override;

    // Corrects the position from which we need to format
    static TextFrameIndex FindBrk(const OUString &rText, TextFrameIndex nStart,
                                  TextFrameIndex nEnd);

    // inline branch
    SwTwips GetFootnoteFrameHeight_() const;

    // Outsourced to CalcPreps
    bool CalcPrepFootnoteAdjust();

    // For Footnote and WidOrp: Forced validation
    void ValidateFrame();
    void ValidateBodyFrame();

    bool GetDropRect_( SwRect &rRect ) const;

    void SetPara( SwParaPortion *pNew, bool bDelete = true );

    bool IsFootnoteNumFrame_() const;

    // Refresh formatting information
    bool FormatQuick( bool bForceQuickFormat );

    // Opt: Format empty paragraphs
    bool FormatEmpty();
    SwTwips EmptyHeight() const;

    // Opt: Paint empty paragraphs
    bool PaintEmpty( const SwRect &, bool bCheck ) const;

    void ChgThisLines(); // Must always be called if the Line count could have changed

    // required for 'new' relative anchor position
    void CalcBaseOfstForFly();

    /** method to determine height of last line, needed for proportional line spacing

        OD 2004-03-17 #i11860#
        OD 2005-05-20 #i47162# - introduce new optional parameter <_bUseFont>
        in order to force the usage of the former algorithm to determine the
        height of the last line, which uses the font.

        @param _bUseFont
        optional input parameter - boolean indicating, if the font has to be
        used to determine the height of the last line. default value: false
    */
    void CalcHeightOfLastLine( const bool _bUseFont = false );

    virtual void DestroyImpl() override;
    virtual ~SwTextFrame() override;

protected:
    virtual void SwClientNotify(SwModify const& rModify, SfxHint const& rHint) override;

public:

    virtual const SvxFormatBreakItem& GetBreakItem() const override;
    virtual const SwFormatPageDesc& GetPageDescItem() const override;

    css::uno::Sequence< css::style::TabStop >  GetTabStopInfo( SwTwips CurrentPos ) override;

    /**
     * This is public, as it needs to be called by some methods in order to save the Prepare
     * USE WITH CAUTION!
     */
    void Init();

    /// Is called by DoIdleJob_() and ExecSpellPopup()
    SwRect AutoSpell_(SwTextNode &, sal_Int32);

    /// Is called by DoIdleJob_()
    SwRect SmartTagScan(SwTextNode &);

    /// Is called by DoIdleJob_()
    void CollectAutoCmplWrds(SwTextNode &, sal_Int32);

    /**
     * Returns the screen position of rPos. The values are relative to the upper
     * left position of the page frame.
     * Additional information can be obtained by passing an SwCursorMoveState object.
     * Returns false if rPos > number of character is string
     */
    virtual bool GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCursorMoveState* pCMS = nullptr, bool bAllowFarAway = true ) const override;

    /// A slimmer version of GetCharRect for autopositioning Frames
    bool GetAutoPos( SwRect &, const SwPosition& ) const;

    /**
     * Determine top of line for given position in the text frame
     *
     * OD 11.11.2003 #i22341#
     * Assumption: given position exists in the text frame or in a follow of it
     * OD 2004-02-02 - adjustment
     * Top of first paragraph line is the top of the paragraph.
     * OD 2004-03-18 #i11860# - Consider upper space amount considered for
     * previous frame and the page grid.
     *
     * @param _onTopOfLine
     * output parameter - top of line, if the given position is found in the
     * text frame.
     *
     * @param _rPos
     * input parameter - reference to the position in the text frame
     *
     * @return boolean indicating, if the top of line for the given position
     * has been determined or not.
     */
    bool GetTopOfLine( SwTwips& _onTopOfLine,
                       const SwPosition& _rPos ) const;

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const override;

    /**
     * In nOffset returns the offset of the char within the set
     * text buffer, which is closest to the position provided by
     * aPoint within the layout's SSize.
     *
     * @returns false if the SPoint is outside of the SSize else
     *          returns true
     */
    virtual bool GetCursorOfst( SwPosition *, Point&,
                                  SwCursorMoveState* = nullptr, bool bTestBackground = false ) const override;

    /**
     * Makes sure that the Frame is not switched (e.g. switched for a
     * character-bound Frame)
     */
    bool GetKeyCursorOfst(SwPosition *pPos, const Point &rPoint ) const
            { return GetCursorOfst_( pPos, rPoint, false ); }

    void   PaintExtraData( const SwRect & rRect ) const; /// Page number etc.
    SwRect GetPaintSwRect();
    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual bool GetInfo( SfxPoolItem & ) const override;

    /**
     * Layout oriented cursor travelling:
     * Left border, right border, previous Line, following Line,
     * same horizontal position
     */
    virtual bool LeftMargin(SwPaM *) const override;
    virtual bool RightMargin(SwPaM *, bool bAPI = false) const override;

    virtual bool UnitUp(SwPaM *, const SwTwips nOffset,
                            bool bSetInReadOnly ) const override;
    virtual bool UnitDown(SwPaM *, const SwTwips nOffset,
                            bool bSetInReadOnly ) const override;
    bool UnitUp_(SwPaM *, const SwTwips nOffset,
                            bool bSetInReadOnly ) const;
    bool UnitDown_(SwPaM *, const SwTwips nOffset,
                            bool bSetInReadOnly ) const;

    /**
     * Prepares the cursor position for a visual cursor move (BiDi).
     * The behaviour is different for insert and overwrite cursors
     */
    void PrepareVisualMove( TextFrameIndex& nPos, sal_uInt8& nCursorLevel,
                            bool& bRight, bool bInsertCursor );

    /// Methods to manage the FollowFrame
    void            SplitFrame(TextFrameIndex nTextPos);
    SwContentFrame *JoinFrame();
    TextFrameIndex GetOfst() const { return mnOffset; }
           void        SetOfst_(TextFrameIndex nNewOfst);
    inline void        SetOfst (TextFrameIndex nNewOfst);
    void ManipOfst(TextFrameIndex const nNewOfst) { mnOffset = nNewOfst; }
           SwTextFrame   *GetFrameAtPos ( const SwPosition &rPos);
    inline const SwTextFrame *GetFrameAtPos ( const SwPosition &rPos) const;
    SwTextFrame&   GetFrameAtOfst(TextFrameIndex nOfst);
    /// If there's a Follow and we don't contain text ourselves
    bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    void SetMergedPara(std::unique_ptr<sw::MergedPara> p);
    sw::MergedPara      * GetMergedPara()       { return m_pMergedPara.get(); }
    sw::MergedPara const* GetMergedPara() const { return m_pMergedPara.get(); }

    /// Returns the text portion we want to edit (for inline see underneath)
    const OUString& GetText() const;
    SwTextNode const* GetTextNodeForParaProps() const;
    SwTextNode const* GetTextNodeForFirstText() const;
    SwTextNode      * GetTextNodeFirst()
        { return const_cast<SwTextNode*>(const_cast<SwTextFrame const*>(this)->GetTextNodeFirst()); };
    SwTextNode const* GetTextNodeFirst() const;
    SwDoc      & GetDoc()
        { return const_cast<SwDoc &>(const_cast<SwTextFrame const*>(this)->GetDoc()); }
    SwDoc const& GetDoc() const;

    SwTextFrame(SwTextNode * const, SwFrame* );

    /**
     * SwContentFrame: the shortcut for the Frames
     * If the void* casts wrongly, it's its own fault!
     * The void* must be checked for 0 in any case!
     *
     * return true if the Portion associated with this SwTextFrame was
     * potentially destroyed and replaced by Prepare
     */
    virtual bool Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = nullptr, bool bNotify = true ) override;

    /**
     * nMaxHeight is the required height
     * bSplit indicates, that the paragraph has to be split
     * bTst indicates, that we are currently doing a test formatting
     */
    virtual bool WouldFit( SwTwips &nMaxHeight, bool &bSplit, bool bTst ) override;

    /**
     * The WouldFit equivalent for temporarily rewired TextFrames
     * nMaxHeight returns the required size here too and bSplit
     * determines whether the paragraph needs to be split.
     * We pass the potential predecessor for the distance calculation
     */
    bool TestFormat( const SwFrame* pPrv, SwTwips &nMaxHeight, bool &bSplit );

    /**
     * We format a Line for interactive hyphenation
     * @return found
     */
    bool Hyphenate(SwInterHyphInfoTextFrame & rInf);

    /// Test grow
    inline SwTwips GrowTst( const SwTwips nGrow );

    SwParaPortion *GetPara();
    inline const SwParaPortion *GetPara() const;
    inline bool HasPara() const;
    bool HasPara_() const;

    /// map position in potentially merged text frame to SwPosition
    std::pair<SwTextNode*, sal_Int32> MapViewToModel(TextFrameIndex nIndex) const;
    SwPosition MapViewToModelPos(TextFrameIndex nIndex) const;
    TextFrameIndex MapModelToView(SwTextNode const* pNode, sal_Int32 nIndex) const;
    TextFrameIndex MapModelToViewPos(SwPosition const& rPos) const;

    // If there are any hanging punctuation portions in the margin
    // the offset will be returned.
    SwTwips HangingMargin() const;

    // Locking
    bool IsLocked()      const { return mbLocked;     }

    bool IsWidow()       const { return mbWidow;      }
    bool IsJustWidow()   const { return mbJustWidow;  }
    bool IsEmpty()       const { return mbEmpty;      }
    bool HasFootnote()        const { return mbFootnote;        }
    bool IsInFootnoteConnect()const { return mbInFootnoteConnect;}
    bool IsFieldFollow() const { return mbFieldFollow;}

    inline void SetRepaint() const;
    inline void ResetRepaint() const;
    bool HasRepaint() const { return mbRepaint; }
    inline void SetBlinkPor() const;
    inline void ResetBlinkPor() const;
    bool HasBlinkPor() const { return mbHasBlinkPortions; }
    void SetAnimation() const
        { const_cast<SwTextFrame*>(this)->mbHasAnimation = true; }
    bool HasAnimation() const { return mbHasAnimation; }

    bool IsSwapped() const { return mbIsSwapped; }

    /// Does the Frame have a local footnote (in this Frame or Follow)?
#ifdef DBG_UTIL
    void CalcFootnoteFlag(TextFrameIndex nStop = TextFrameIndex(COMPLETE_STRING)); //For testing SplitFrame
#else
    void CalcFootnoteFlag();
#endif

    /// Hidden
    bool IsHiddenNow() const;       // bHidden && pOut == pPrt
    void HideHidden();              // Remove appendage if Hidden
    void HideFootnotes(TextFrameIndex nStart, TextFrameIndex nEnd);

    /**
     * Hides respectively shows objects, which are anchored at paragraph,
     * at/as a character of the paragraph, corresponding to the paragraph and
     * paragraph portion visibility.
     */
    void HideAndShowObjects();

    /// Footnote
    void RemoveFootnote(TextFrameIndex nStart,
                        TextFrameIndex nLen = TextFrameIndex(COMPLETE_STRING));
    inline SwTwips GetFootnoteFrameHeight() const;
    SwTextFrame *FindFootnoteRef( const SwTextFootnote *pFootnote );
    const SwTextFrame *FindFootnoteRef( const SwTextFootnote *pFootnote ) const
    { return const_cast<SwTextFrame *>(this)->FindFootnoteRef( pFootnote ); }
    void ConnectFootnote( SwTextFootnote *pFootnote, const SwTwips nDeadLine );

    /**
     * If we're a Footnote that grows towards its reference ...
     * public, because it's needed by SwContentFrame::MakeAll
     */
    SwTwips GetFootnoteLine( const SwTextFootnote *pFootnote ) const;

    TextFrameIndex GetDropLen(TextFrameIndex nWishLen) const;

    LanguageType GetLangOfChar(TextFrameIndex nIndex, sal_uInt16 nScript,
            bool bNoChar = false) const;

    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void CheckDirection( bool bVert ) override;

    /// Returns the sum of line height in pLine
    sal_uInt16 GetParHeight() const;

    inline       SwTextFrame *GetFollow();
    inline const SwTextFrame *GetFollow() const;

    /// Find the page number of ErgoSum and QuoVadis
    SwTextFrame *FindQuoVadisFrame();

    /**
     * In case the SwLineLayout was cleared out of the s_pTextCache, recreate it
     *
     * #i29062# GetFormatted() can trigger a full formatting
     * of the paragraph, causing other layout frames to become invalid. This
     * has to be avoided during painting. Therefore we need to pass the
     * information that we are currently in the paint process.
     */
    SwTextFrame* GetFormatted( bool bForceQuickFormat = false );

    /// Will be moved soon
    void SetFootnote( const bool bNew ) { mbFootnote = bNew; }

    /// Respect the Follows
    inline bool IsInside(TextFrameIndex nPos) const;

    /// DropCaps and selections
    bool GetDropRect( SwRect &rRect ) const
    { return HasPara() && GetDropRect_( rRect ); }

    static SwCache *GetTextCache() { return s_pTextCache; }
    static void     SetTextCache( SwCache *pNew ) { s_pTextCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }

    sal_uInt16 GetCacheIdx() const { return mnCacheIndex; }
    void   SetCacheIdx( const sal_uInt16 nNew ) { mnCacheIndex = nNew; }

    /// Removes the Line information from the Cache
    void ClearPara();

    /// Am I a FootnoteFrame, with a number at the start of the paragraph?
    bool IsFootnoteNumFrame() const
    { return IsInFootnote() && !GetIndPrev() && IsFootnoteNumFrame_(); }

    /**
     * Simulates a formatting as if there were not right margin or Flys or other
     * obstacles and returns the width
     */
    SwTwips CalcFitToContent();

    /**
     * Simulate format for a list item paragraph, whose list level attributes
     * are in LABEL_ALIGNMENT mode, in order to determine additional first
     * line offset for the real text formatting due to the value of label
     * adjustment attribute of the list level.
     */
    void CalcAdditionalFirstLineOffset();

    SwTwips GetAdditionalFirstLineOffset() const
    {
        return mnAdditionalFirstLineOffset;
    }

    /**
     * Returns the additional line spacing for the next paragraph
     * @param _bNoPropLineSpacing: control, whether the value of a
     *        proportional line spacing is returned or not
     */
    long GetLineSpace( const bool _bNoPropLineSpacing = false ) const;

    /// Returns the first line height
    sal_uInt16 FirstLineHeight() const;

    /// Rewires FlyInContentFrame, if nEnd > Index >= nStart
    void MoveFlyInCnt(SwTextFrame *pNew, TextFrameIndex nStart, TextFrameIndex nEnd);

    /// Calculates the position of FlyInContentFrames
    TextFrameIndex CalcFlyPos( SwFrameFormat const * pSearch );

    /// Determines the start position and step size of the register
    bool FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff );

    /// Determines the line count
    sal_uInt16 GetLineCount(TextFrameIndex nPos);

    /// For displaying the line numbers
    sal_uLong GetAllLines()  const { return mnAllLines; }
    sal_uLong GetThisLines() const { return mnThisLines;}
    void RecalcAllLines();

    /// Stops the animations within numberings
    void StopAnimation( OutputDevice *pOut );

    /// Visit all portions for Accessibility
    void VisitPortions( SwPortionHandler& rPH ) const;

    /// Returns the script info stored at the paraportion
    const SwScriptInfo* GetScriptInfo() const;

    /// Swaps width and height of the text frame
    void SwapWidthAndHeight();

    /**
     * Calculates the coordinates of a rectangle when switching from
     * horizontal to vertical layout
     */
    void SwitchHorizontalToVertical( SwRect& rRect ) const;

    /**
     * Calculates the coordinates of a point when switching from
     * horizontal to vertical layout
     */
    void SwitchHorizontalToVertical( Point& rPoint ) const;

    /**
     * Calculates the a limit value when switching from
     * horizontal to vertical layout
     */
    long SwitchHorizontalToVertical( long nLimit ) const;

    /**
     * Calculates the coordinates of a rectangle when switching from
     * vertical to horizontal layout
     */
    void SwitchVerticalToHorizontal( SwRect& rRect ) const;

    /**
     * Calculates the coordinates of a point when switching from
     * vertical to horizontal layout
     */
    void SwitchVerticalToHorizontal( Point& rPoint ) const;

    /**
     * Calculates the a limit value when switching from
     * vertical to horizontal layout
     */
    long SwitchVerticalToHorizontal( long nLimit ) const;

    /**
     * Calculates the coordinates of a rectangle when switching from
     * LTR to RTL layout
     */
    void SwitchLTRtoRTL( SwRect& rRect ) const;

    /**
     * Calculates the coordinates of a point when switching from
     * LTR to RTL layout
     */
    void SwitchLTRtoRTL( Point& rPoint ) const;

    /**
     * Calculates the coordinates of a rectangle when switching from
     * RTL to LTR layout
     */
    void SwitchRTLtoLTR( SwRect& rRect ) const { SwitchLTRtoRTL( rRect ); }

    /**
     * Calculates the coordinates of a point when switching from
     * RTL to LTR layout
     */
    void SwitchRTLtoLTR( Point& rPoint ) const { SwitchLTRtoRTL( rPoint ); };

    bool FollowFormatAllowed() const
    {
        return mbFollowFormatAllowed;
    }

    void AllowFollowFormat()
    {
        mbFollowFormatAllowed = true;
    }

    void ForbidFollowFormat()
    {
        mbFollowFormatAllowed = false;
    }

    SwTwips GetBaseOfstForFly( bool bIgnoreFlysAnchoredAtThisFrame ) const
    {
        return ( bIgnoreFlysAnchoredAtThisFrame ?
                 mnFlyAnchorOfst :
                 mnFlyAnchorOfstNoWrap );
    }

    SwTwips GetBaseVertOffsetForFly(bool bIgnoreFlysAnchoredAtThisFrame) const;

    SwTwips GetHeightOfLastLine() const
    {
        return mnHeightOfLastLine;
    }

    static void repaintTextFrames( const SwTextNode& rNode );

    void RegisterToNode(SwTextNode &, bool isForceNodeAsFirst = false);

    bool IsSymbolAt(TextFrameIndex) const;
    OUString GetCurWord(SwPosition const&) const;
    sal_uInt16 GetScalingOfSelectedText(TextFrameIndex nStt, TextFrameIndex nEnd);

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;
};

//use this to protect a SwTextFrame for a given scope from getting merged with
//its neighbour and thus deleted
class TextFrameLockGuard
{
private:
    SwTextFrame *m_pTextFrame;
    bool m_bOldLocked;
public:
    //Lock pFrame for the lifetime of the Cut/Paste call, etc. to avoid
    //SwTextFrame::AdjustFollow_ removing the pFrame we're trying to Make
    TextFrameLockGuard(SwFrame* pFrame)
    {
        m_pTextFrame = pFrame->IsTextFrame() ? static_cast<SwTextFrame*>(pFrame) : nullptr;
        if (m_pTextFrame)
        {
            m_bOldLocked = m_pTextFrame->IsLocked();
            m_pTextFrame->Lock();
        }
        else
        {
            m_bOldLocked = false;
        }
    }

    ~TextFrameLockGuard()
    {
        if (m_pTextFrame && !m_bOldLocked)
            m_pTextFrame->Unlock();
    }
};

inline const SwParaPortion *SwTextFrame::GetPara() const
{
    return const_cast<SwTextFrame*>(this)->GetPara();
}

inline bool SwTextFrame::HasPara() const
{
    return mnCacheIndex!=USHRT_MAX && HasPara_();
}

inline SwTwips SwTextFrame::GrowTst( const SwTwips nGrow )
{
    return Grow( nGrow, true );
}

inline bool SwTextFrame::IsInside(TextFrameIndex const nPos) const
{
    bool bRet = true;
    if( nPos < GetOfst() )
        bRet = false;
    else
    {
        const SwTextFrame *pFoll = GetFollow();
        if( pFoll && nPos >= pFoll->GetOfst() )
            bRet = false;
    }
    return bRet;
}

inline SwTwips SwTextFrame::GetFootnoteFrameHeight() const
{
    if(  !IsFollow() && IsInFootnote() && HasPara() )
        return GetFootnoteFrameHeight_();
    else
        return 0;
}

inline const SwTextFrame *SwTextFrame::GetFollow() const
{
    return static_cast<const SwTextFrame*>(SwContentFrame::GetFollow());
}
inline SwTextFrame *SwTextFrame::GetFollow()
{
    return static_cast<SwTextFrame*>(SwContentFrame::GetFollow());
}

inline const SwTextFrame *SwTextFrame::GetFrameAtPos( const SwPosition &rPos) const
{
    return const_cast<SwTextFrame*>(this)->GetFrameAtPos( rPos );
}

inline void SwTextFrame::SetOfst(TextFrameIndex const nNewOfst)
{
    if ( mnOffset != nNewOfst )
        SetOfst_( nNewOfst );
}

inline void SwTextFrame::SetRepaint() const
{
    const_cast<SwTextFrame*>(this)->mbRepaint = true;
}
inline void SwTextFrame::ResetRepaint() const
{
    const_cast<SwTextFrame*>(this)->mbRepaint = false;
}

inline void SwTextFrame::SetBlinkPor() const
{
    const_cast<SwTextFrame*>(this)->mbHasBlinkPortions = true;
}
inline void SwTextFrame::ResetBlinkPor() const
{
    const_cast<SwTextFrame*>(this)->mbHasBlinkPortions = false;
}

class TemporarySwap {
protected:
    explicit TemporarySwap(SwTextFrame * frame, bool swap):
        m_frame(frame), m_undo(false)
    {
        if (m_frame->IsVertical() && swap) {
            m_undo = true;
            m_frame->SwapWidthAndHeight();
        }
    }

    ~TemporarySwap() {
        if (m_undo) {
            m_frame->SwapWidthAndHeight();
        }
    }

private:
    TemporarySwap(TemporarySwap const &) = delete;
    void operator =(TemporarySwap const &) = delete;

    SwTextFrame * m_frame;
    bool m_undo;
};

class SwSwapIfSwapped: private TemporarySwap {
public:
    explicit SwSwapIfSwapped(SwTextFrame* frame):
        TemporarySwap(frame, frame->IsSwapped()) {}
};

class SwSwapIfNotSwapped: private TemporarySwap {
public:
    explicit SwSwapIfNotSwapped(SwTextFrame* frame):
        TemporarySwap(frame, !frame->IsSwapped()) {}
};

/**
 * Helper class which can be used instead of the macros if a function
 * has too many returns
 */
class SwFrameSwapper
{
    const SwTextFrame* pFrame;
    bool bUndo;
public:
    SwFrameSwapper( const SwTextFrame* pFrame, bool bSwapIfNotSwapped );
    ~SwFrameSwapper();
};

class SwLayoutModeModifier
{
    const OutputDevice& m_rOut;
    ComplexTextLayoutFlags const m_nOldLayoutMode;
public:
    SwLayoutModeModifier( const OutputDevice& rOutp );
    ~SwLayoutModeModifier();
    void Modify( bool bChgToRTL );
    void SetAuto();
};

class SwDigitModeModifier
{
    const OutputDevice& rOut;
    LanguageType const nOldLanguageType;
public:
    SwDigitModeModifier( const OutputDevice& rOutp, LanguageType eCurLang );
    ~SwDigitModeModifier();
};

namespace sw {

/**
 * Describes parts of multiple text nodes, which will form a text frame, even
 * when redlines are hidden at a layout level.
 */
struct MergedPara
{
    sw::WriterMultiListener listener;
    std::vector<Extent> extents;
    /// note: cannot be const currently to avoid UB because SwTextGuess::Guess
    /// const_casts it and modifies it (also, Update will modify it)
    OUString mergedText;
    /// most paragraph properties are taken from the first non-empty node
    SwTextNode * pParaPropsNode;
    /// except break attributes, those are taken from the first node
    SwTextNode *const pFirstNode;
    /// mainly for sanity checks
    SwTextNode const* pLastNode;
    MergedPara(SwTextFrame & rFrame, std::vector<Extent>&& rExtents,
            OUString const& rText,
            SwTextNode *const pProps, SwTextNode *const pFirst,
            SwTextNode const*const pLast)
        : listener(rFrame), extents(std::move(rExtents)), mergedText(rText)
        , pParaPropsNode(pProps), pFirstNode(pFirst), pLastNode(pLast)
    {
        assert(pParaPropsNode);
        assert(pFirstNode);
        assert(pLastNode);
    }
};

/// iterate SwTextAttr in potentially merged text frame
class MergedAttrIterBase
{
protected:
    sw::MergedPara const*const m_pMerged;
    SwTextNode const*const m_pNode;
    size_t m_CurrentExtent;
    size_t m_CurrentHint;
    MergedAttrIterBase(SwTextFrame const& rFrame);
};

class MergedAttrIter
    : public MergedAttrIterBase
{
public:
    MergedAttrIter(SwTextFrame const& rFrame) : MergedAttrIterBase(rFrame) {}
    SwTextAttr const* NextAttr(SwTextNode const** ppNode = nullptr);
};

class MergedAttrIterByEnd
{
private:
    std::vector<std::pair<SwTextNode const*, SwTextAttr const*>> m_Hints;
    SwTextNode const*const m_pNode;
    size_t m_CurrentHint;
public:
    MergedAttrIterByEnd(SwTextFrame const& rFrame);
    SwTextAttr const* NextAttr(SwTextNode const*& rpNode);
    void PrevAttr();
};

class MergedAttrIterReverse
    : public MergedAttrIterBase
{
public:
    MergedAttrIterReverse(SwTextFrame const& rFrame);
    SwTextAttr const* PrevAttr(SwTextNode const** ppNode = nullptr);
};


} // namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
