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
#include <tools/mempool.hxx>
#include "cntfrm.hxx"
#include "ndtxt.hxx"

class SwCharRange;
class SwTextNode;
class SwTextFormatter;
class SwTextFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class SwBodyFrm;
class SwTextFootnote;
class SwInterHyphInfo;      // Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrameFormat;
class OutputDevice;
class SwTestFormat;
struct SwCrsrMoveState;
struct SwFillData;
class SwPortionHandler;
class SwScriptInfo;
class SwViewOption;
class SwWrongList;

#define NON_PRINTING_CHARACTER_COLOR RGB_COLORDATA(0x26, 0x8b, 0xd2)

/// Represents the visualization of a paragraph.
class SwTextFrm: public SwContentFrm
{
    friend class SwTextIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class TextFrmLockGuard; // May Lock()/Unlock()
    friend bool sw_ChangeOffset( SwTextFrm* pFrm, sal_Int32 nNew );

    static SwCache *pTextCache;  // Pointer to the Line Cache
    static long nMinPrtLine;    // This Line must not be underrun when printing
                                // Hack for table cells stretching multiple pages

    sal_uLong  mnAllLines        :24; // Line count for the Paint (including nThisLines)
    sal_uLong  mnThisLines       :8; // Count of Lines of the Frame

    // The x position for flys anchored at this paragraph.
    // These values are calculated in SwTextFrm::CalcBaseOfstForFly()
    SwTwips mnFlyAnchorOfst;
    // The x position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorOfstNoWrap;
    SwTwips mnFootnoteLine;
    // OD 2004-03-17 #i11860# - re-factoring of #i11859#
    // member for height of last line (value needed for proportional line spacing)
    SwTwips mnHeightOfLastLine;
    // member for the additional first line offset, which is caused by the list
    // label alignment for list level position and space mode LABEL_ALIGNMENT.
    // This additional first line offset is used for the text formatting.
    // It is NOT used for the determination of printing area.
    SwTwips mnAdditionalFirstLineOffset;

    sal_Int32 mnOffset; // Is the offset in the Content (character count)

    sal_uInt16 mnCacheIndex; // Index into the cache, USHRT_MAX if there's definitely no fitting object in the cache

    // Separates the Master and creates a Follow or adjusts the data in the Follow
    void _AdjustFollow( SwTextFormatter &rLine, const sal_Int32 nOffset,
                               const sal_Int32 nStrEnd, const sal_uInt8 nMode );

    // Iterates all Lines and sets the line spacing using the attribute
    void CalcLineSpace();

    // Only called in Format
    void AdjustFrm( const SwTwips nChgHeight, bool bHasToFit = false );

    // Evaluates the Preps in Format()
    bool CalcPreps();
    void PrepWidows( const sal_uInt16 nNeed, bool bNotify = true );
    void _InvalidateRange( const SwCharRange &, const long = 0);
    inline void InvalidateRange( const SwCharRange &, const long = 0);

    // WidowsAndOrphans, AdjustFrm, AdjustFollow
    void FormatAdjust( SwTextFormatter &rLine, WidowsAndOrphans &rFrmBreak,
                       const sal_Int32 nStrLen, const bool bDummy );

    bool mbLocked        : 1;        // In the Format?
    bool mbWidow         : 1;        // Are we a Widow?
    bool mbJustWidow     : 1;        // Did we just request to be a Widow?
    bool mbEmpty         : 1;        // Are we an empty paragraph?
    bool mbInFootnoteConnect  : 1;        // Is in Connect at the moment
    bool mbFootnote           : 1;        // Has at least one footnote
    bool mbRepaint       : 1;        // TextFrm: Repaint is ready to be fetched
    bool mbHasBlinkPortions      : 1;        // Contains Blink Portions
    bool mbFieldFollow   : 1;        // Start with Field rest of the Master
    bool mbHasAnimation  : 1;        // Contains animated SwGrfNumPortion
    bool mbIsSwapped     : 1;        // during text formatting we swap the
                                    // width and height for vertical formatting
    // OD 14.03.2003 #i11760# - flag to control, if follow is formatted in
    // method <CalcFollow(..)>.
    // E.g., avoid formatting of follow, if method <SwLayoutFrm::FormatWidthCols(..)>
    // is running.
    bool mbFollowFormatAllowed : 1;

    void ResetPreps();
    inline void Lock() { mbLocked = true; }
    inline void Unlock() { mbLocked = false; }
    inline void SetWidow( const bool bNew ) { mbWidow = bNew; }
    inline void SetJustWidow( const bool bNew ) { mbJustWidow = bNew; }
    inline void SetEmpty( const bool bNew ) { mbEmpty = bNew; }
    inline void SetFieldFollow( const bool bNew ) { mbFieldFollow = bNew; }

    bool IsIdxInside( const sal_Int32 nPos, const sal_Int32 nLen ) const;

    // Changes the Frame or not (cf. FlyCnt)
    bool _GetCrsrOfst(SwPosition *pPos, const Point &rPoint,
                      const bool bChgFrm, SwCrsrMoveState* = nullptr ) const;
    void FillCrsrPos( SwFillData &rFill ) const;

    // Format exactly one Line
    bool FormatLine( SwTextFormatter &rLine, const bool bPrev );

    // In order to safe stack space, we split this method:
    // _Format calls _Format with parameters
    void _Format( vcl::RenderContext* pRenderContext, SwParaPortion *pPara );
    void _Format( SwTextFormatter &rLine, SwTextFormatInfo &rInf,
                  const bool bAdjust = false );
    void FormatOnceMore( SwTextFormatter &rLine, SwTextFormatInfo &rInf );

    // Formats the Follow and ensures disposing on orphans
    bool CalcFollow(  const sal_Int32 nTextOfst );

    // Corrects the position from which we need to format
    static sal_Int32 FindBrk(const OUString &rText, const sal_Int32 nStart,
                                       const sal_Int32 nEnd);

    // inline branch
    SwTwips _GetFootnoteFrmHeight() const;

    // Outsourced to CalcPreps
    bool CalcPrepFootnoteAdjust();

    // For Footnote and WidOrp: Forced validation
    void ValidateFrm();
    void ValidateBodyFrm();

    bool _GetDropRect( SwRect &rRect ) const;

    void SetPara( SwParaPortion *pNew, bool bDelete = true );

    bool _IsFootnoteNumFrm() const;

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
    void _CalcHeightOfLastLine( const bool _bUseFont = false );

    virtual void DestroyImpl() override;
    virtual ~SwTextFrm();

protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:

    css::uno::Sequence< css::style::TabStop >  GetTabStopInfo( SwTwips CurrentPos ) override;

    /**
     * This is public, as it needs to be called by some methods in order to save the Prepare
     * USE WITH CAUTION!
     */
    void Init();

    /// Is called by FormatSpelling()
    SwRect _AutoSpell( const SwContentNode*, sal_Int32 );

    /// Is called by FormatSpelling()
    SwRect SmartTagScan( SwContentNode* , sal_Int32 );

    /// Is called by CollectAutoCmplWords()
    void CollectAutoCmplWrds( SwContentNode* , sal_Int32 );

    /**
     * Returns the screen position of rPos. The values are relative to the upper
     * left position of the page frame.
     * Additional information can be obtained by passing an SwCrsrMoveState object.
     * Returns false if rPos > number of character is string
     */
    virtual bool GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCrsrMoveState* pCMS = nullptr ) const override;

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
    virtual bool GetCrsrOfst( SwPosition *, Point&,
                                  SwCrsrMoveState* = nullptr, bool bTestBackground = false ) const override;

    /**
     * Makes sure that the Frame is not switched (e.g. switched for a
     * character-bound Frame)
     */
    inline bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, false ); }

    void   PaintExtraData( const SwRect & rRect ) const; /// Page number etc.
    SwRect Paint();
    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual bool GetInfo( SfxPoolItem & ) const override;

    /**
     * Layout oriented cursor travelling:
     * Left border, right border, previous Line, following Line,
     * same horizontal position
     */
    virtual bool LeftMargin(SwPaM *) const override;
    virtual bool RightMargin(SwPaM *, bool bAPI = false) const override;

    virtual bool UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const override;
    virtual bool UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const override;
    bool _UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;
    bool _UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const;

    /**
     * Prepares the cursor position for a visual cursor move (BiDi).
     * The behaviour is different for insert and overwrite cursors
     */
    void PrepareVisualMove( sal_Int32& nPos, sal_uInt8& nCrsrLevel,
                            bool& bRight, bool bInsertCrsr );

    /// Methods to manage the FollowFrame
    SwContentFrm *SplitFrm( const sal_Int32 nTextPos );
    SwContentFrm *JoinFrm();
    inline sal_Int32  GetOfst() const { return mnOffset; }
           void        _SetOfst( const sal_Int32 nNewOfst );
    inline void        SetOfst ( const sal_Int32 nNewOfst );
    inline void        ManipOfst ( const sal_Int32 nNewOfst ){ mnOffset = nNewOfst; }
           SwTextFrm   *GetFrmAtPos ( const SwPosition &rPos);
    inline const SwTextFrm *GetFrmAtPos ( const SwPosition &rPos) const;
    SwTextFrm&   GetFrmAtOfst( const sal_Int32 nOfst );
    /// If there's a Follow and we don't contain text ourselves
    inline bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    /// Returns the text portion we want to edit (for inline see underneath)
    const OUString& GetText() const;
    inline SwTextNode *GetTextNode()
        { return static_cast< SwTextNode* >( SwContentFrm::GetNode()); }
    inline const SwTextNode *GetTextNode() const
        { return static_cast< const SwTextNode* >( SwContentFrm::GetNode()); }

    SwTextFrm(SwTextNode * const, SwFrm* );

    /**
     * SwContentFrm: the shortcut for the Frames
     * If the void* casts wrongly, it's its own fault!
     * The void* must be checked for 0 in any case!
     *
     * return true if the Portion associated with this SwTxtFrm was
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
    bool TestFormat( const SwFrm* pPrv, SwTwips &nMaxHeight, bool &bSplit );

    /**
     * We format a Line for interactive hyphenation
     * @return found
     */
    bool Hyphenate( SwInterHyphInfo &rInf );

    /// Test grow
    inline SwTwips GrowTst( const SwTwips nGrow );

    SwParaPortion *GetPara();
    inline const SwParaPortion *GetPara() const;
    inline bool HasPara() const;
    bool _HasPara() const;

    // If there are any hanging punctuation portions in the margin
    // the offset will be returned.
    SwTwips HangingMargin() const;

    // RTTI
    DECL_FIXEDMEMPOOL_NEWDEL(SwTextFrm)

    // Locking
    inline bool IsLocked()      const { return mbLocked;     }

    inline bool IsWidow()       const { return mbWidow;      }
    inline bool IsJustWidow()   const { return mbJustWidow;  }
    inline bool IsEmpty()       const { return mbEmpty;      }
    inline bool HasFootnote()        const { return mbFootnote;        }
    inline bool IsInFootnoteConnect()const { return mbInFootnoteConnect;}
    inline bool IsFieldFollow() const { return mbFieldFollow;}

    inline void SetRepaint() const;
    inline void ResetRepaint() const;
    inline bool HasRepaint() const { return mbRepaint; }
    inline void SetBlinkPor() const;
    inline void ResetBlinkPor() const;
    inline bool HasBlinkPor() const { return mbHasBlinkPortions; }
    inline void SetAnimation() const
        { const_cast<SwTextFrm*>(this)->mbHasAnimation = true; }
    inline bool HasAnimation() const { return mbHasAnimation; }

    inline bool IsSwapped() const { return mbIsSwapped; }

    /// Does the Frm have a local footnote (in this Frm or Follow)?
#ifdef DBG_UTIL
    void CalcFootnoteFlag( sal_Int32 nStop = COMPLETE_STRING ); //For testing SplitFrm
#else
    void CalcFootnoteFlag();
#endif

    /// Hidden
    bool IsHiddenNow() const;       // bHidden && pOut == pPrt
    void HideHidden();              // Remove appendage if Hidden
    void HideFootnotes( sal_Int32 nStart, sal_Int32 nEnd );

    /**
     * Hides respectively shows objects, which are anchored at paragraph,
     * at/as a character of the paragraph, corresponding to the paragraph and
     * paragraph portion visibility.
     */
    void HideAndShowObjects();

    /// Footnote
    void RemoveFootnote( const sal_Int32 nStart = 0,
                    const sal_Int32 nLen = COMPLETE_STRING );
    inline SwTwips GetFootnoteFrmHeight() const;
    SwTextFrm *FindFootnoteRef( const SwTextFootnote *pFootnote );
    inline const SwTextFrm *FindFootnoteRef( const SwTextFootnote *pFootnote ) const
    { return const_cast<SwTextFrm *>(this)->FindFootnoteRef( pFootnote ); }
    void ConnectFootnote( SwTextFootnote *pFootnote, const SwTwips nDeadLine );

    /**
     * If we're a Footnote that grows towards its reference ...
     * public, because it's needed by SwContentFrm::MakeAll
     */
    SwTwips GetFootnoteLine( const SwTextFootnote *pFootnote ) const;

    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void CheckDirection( bool bVert ) override;

    /// Returns the sum of line height in pLine
    sal_uInt16 GetParHeight() const;

    inline       SwTextFrm *GetFollow();
    inline const SwTextFrm *GetFollow() const;

    /// Find the page number of ErgoSum and QuoVadis
    SwTextFrm *FindQuoVadisFrm();

    /**
     * Makes up for formatting if the Idle Handler has struck
     *
     * #i29062# GetFormatted() can trigger a full formatting
     * of the paragraph, causing other layout frames to become invalid. This
     * has to be avoided during painting. Therefore we need to pass the
     * information that we are currently in the paint process.
     */
    SwTextFrm* GetFormatted( bool bForceQuickFormat = false );

    /// Will be moved soon
    inline void SetFootnote( const bool bNew ) { mbFootnote = bNew; }

    /// Respect the Follows
    inline bool IsInside( const sal_Int32 nPos ) const;

    const SwBodyFrm   *FindBodyFrm()   const;

    /// DropCaps and selections
    inline bool GetDropRect( SwRect &rRect ) const
    { return HasPara() && _GetDropRect( rRect ); }

    static SwCache *GetTextCache() { return pTextCache; }
    static void     SetTextCache( SwCache *pNew ) { pTextCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }

    inline sal_uInt16 GetCacheIdx() const { return mnCacheIndex; }
    inline void   SetCacheIdx( const sal_uInt16 nNew ) { mnCacheIndex = nNew; }

    /// Removes the Line information from the Cache
    void ClearPara();

    /// Am I a FootnoteFrm, with a number at the start of the paragraph?
    inline bool IsFootnoteNumFrm() const
    { return IsInFootnote() && !GetIndPrev() && _IsFootnoteNumFrm(); }

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

    inline SwTwips GetAdditionalFirstLineOffset() const
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

    /// Rewires FlyInCntFrm, if nEnd > Index >= nStart
    void MoveFlyInCnt( SwTextFrm *pNew, sal_Int32 nStart, sal_Int32 nEnd );

    /// Calculates the position of FlyInCntFrms
    sal_Int32 CalcFlyPos( SwFrameFormat* pSearch );

    /// Determines the start position and step size of the register
    bool FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff );

    /// Determines the line count
    sal_uInt16 GetLineCount( sal_Int32 nPos );

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
    inline void SwitchRTLtoLTR( SwRect& rRect ) const { SwitchLTRtoRTL( rRect ); }

    /**
     * Calculates the coordinates of a point when switching from
     * RTL to LTR layout
     */
    inline void SwitchRTLtoLTR( Point& rPoint ) const { SwitchLTRtoRTL( rPoint ); };

    inline bool FollowFormatAllowed() const
    {
        return mbFollowFormatAllowed;
    }

    inline void AllowFollowFormat()
    {
        mbFollowFormatAllowed = true;
    }

    inline void ForbidFollowFormat()
    {
        mbFollowFormatAllowed = false;
    }

    SwTwips GetBaseOfstForFly( bool bIgnoreFlysAnchoredAtThisFrame ) const
    {
        return ( bIgnoreFlysAnchoredAtThisFrame ?
                 mnFlyAnchorOfst :
                 mnFlyAnchorOfstNoWrap );
    }

    inline SwTwips GetHeightOfLastLine() const
    {
        return mnHeightOfLastLine;
    }

    static void repaintTextFrames( const SwTextNode& rNode );

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;
};

//use this to protect a SwTextFrm for a given scope from getting merged with
//its neighbour and thus deleted
class TextFrmLockGuard
{
private:
    SwTextFrm *m_pTextFrm;
    bool m_bOldLocked;
public:
    //Lock pFrm for the lifetime of the Cut/Paste call, etc. to avoid
    //SwTextFrm::_AdjustFollow removing the pFrm we're trying to Make
    TextFrmLockGuard(SwFrm* pFrm)
    {
        m_pTextFrm = pFrm->IsTextFrm() ? static_cast<SwTextFrm*>(pFrm) : nullptr;
        if (m_pTextFrm)
        {
            m_bOldLocked = m_pTextFrm->IsLocked();
            m_pTextFrm->Lock();
        }
        else
        {
            m_bOldLocked = false;
        }
    }

    ~TextFrmLockGuard()
    {
        if (m_pTextFrm && !m_bOldLocked)
            m_pTextFrm->Unlock();
    }
};

inline const SwParaPortion *SwTextFrm::GetPara() const
{
    return const_cast<SwTextFrm*>(this)->GetPara();
}

inline bool SwTextFrm::HasPara() const
{
    return mnCacheIndex!=USHRT_MAX && _HasPara();
}

inline SwTwips SwTextFrm::GrowTst( const SwTwips nGrow )
{
    return Grow( nGrow, true );
}

inline bool SwTextFrm::IsInside( const sal_Int32 nPos ) const
{
    bool bRet = true;
    if( nPos < GetOfst() )
        bRet = false;
    else
    {
        const SwTextFrm *pFoll = GetFollow();
        if( pFoll && nPos >= pFoll->GetOfst() )
            bRet = false;
    }
    return bRet;
}

inline SwTwips SwTextFrm::GetFootnoteFrmHeight() const
{
    if(  !IsFollow() && IsInFootnote() && HasPara() )
        return _GetFootnoteFrmHeight();
    else
        return 0;
}

inline const SwTextFrm *SwTextFrm::GetFollow() const
{
    return static_cast<const SwTextFrm*>(SwContentFrm::GetFollow());
}
inline SwTextFrm *SwTextFrm::GetFollow()
{
    return static_cast<SwTextFrm*>(SwContentFrm::GetFollow());
}

inline const SwTextFrm *SwTextFrm::GetFrmAtPos( const SwPosition &rPos) const
{
    return const_cast<SwTextFrm*>(this)->GetFrmAtPos( rPos );
}

inline void SwTextFrm::SetOfst( const sal_Int32 nNewOfst )
{
    if ( mnOffset != nNewOfst )
        _SetOfst( nNewOfst );
}

inline void SwTextFrm::SetRepaint() const
{
    const_cast<SwTextFrm*>(this)->mbRepaint = true;
}
inline void SwTextFrm::ResetRepaint() const
{
    const_cast<SwTextFrm*>(this)->mbRepaint = false;
}

inline void SwTextFrm::SetBlinkPor() const
{
    const_cast<SwTextFrm*>(this)->mbHasBlinkPortions = true;
}
inline void SwTextFrm::ResetBlinkPor() const
{
    const_cast<SwTextFrm*>(this)->mbHasBlinkPortions = false;
}

class TemporarySwap {
protected:
    explicit TemporarySwap(SwTextFrm * frame, bool swap):
        frame_(frame), undo_(false)
    {
        if (frame_->IsVertical() && swap) {
            undo_ = true;
            frame_->SwapWidthAndHeight();
        }
    }

    ~TemporarySwap() {
        if (undo_) {
            frame_->SwapWidthAndHeight();
        }
    }

private:
    TemporarySwap(TemporarySwap &) = delete;
    void operator =(TemporarySwap &) = delete;

    SwTextFrm * frame_;
    bool undo_;
};

class SwSwapIfSwapped: private TemporarySwap {
public:
    explicit SwSwapIfSwapped(SwTextFrm* frame):
        TemporarySwap(frame, frame->IsSwapped()) {}
};

class SwSwapIfNotSwapped: private TemporarySwap {
public:
    explicit SwSwapIfNotSwapped(SwTextFrm* frame):
        TemporarySwap(frame, !frame->IsSwapped()) {}
};

/**
 * Helper class which can be used instead of the macros if a function
 * has too many returns
 */
class SwFrmSwapper
{
    const SwTextFrm* pFrm;
    bool bUndo;
public:
    SwFrmSwapper( const SwTextFrm* pFrm, bool bSwapIfNotSwapped );
    ~SwFrmSwapper();
};

class SwLayoutModeModifier
{
    const OutputDevice& rOut;
    ComplexTextLayoutMode nOldLayoutMode;
public:
    SwLayoutModeModifier( const OutputDevice& rOutp );
    ~SwLayoutModeModifier();
    void Modify( bool bChgToRTL );
    void SetAuto();
};

class SwDigitModeModifier
{
    const OutputDevice& rOut;
    LanguageType nOldLanguageType;
public:
    SwDigitModeModifier( const OutputDevice& rOutp, LanguageType eCurLang );
    ~SwDigitModeModifier();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
