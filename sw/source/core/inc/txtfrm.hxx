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
class SwTxtNode;
class SwTxtFormatter;
class SwTxtFormatInfo;
class SwParaPortion;
class WidowsAndOrphans;
class SwBodyFrm;
class SwTxtFtn;
class SwInterHyphInfo;      // Hyphenate()
class SwCache;
class SwBorderAttrs;
class SwFrmFmt;
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
class SwTxtFrm: public SwCntntFrm
{
    friend class SwTxtIter;
    friend class SwTestFormat;
    friend class WidowsAndOrphans;
    friend class SwTxtFrmLocker; // May Lock()/Unlock()
    friend bool sw_ChangeOffset( SwTxtFrm* pFrm, sal_Int32 nNew );

    static SwCache *pTxtCache;  // Pointer to the Line Cache
    static long nMinPrtLine;    // This Line must not be underrun when printing
                                // Hack for table cells stretching multiple pages

    sal_uLong  nAllLines        :24; // Line count for the Paint (including nThisLines)
    sal_uLong  nThisLines       :8; // Count of Lines of the Frame

    // The x position for flys anchored at this paragraph.
    // These values are calculated in SwTxtFrm::CalcBaseOfstForFly()
    SwTwips mnFlyAnchorOfst;
    // The x position for wrap-through flys anchored at this paragraph.
    SwTwips mnFlyAnchorOfstNoWrap;
    SwTwips mnFtnLine;
    // OD 2004-03-17 #i11860# - re-factoring of #i11859#
    // member for height of last line (value needed for proportional line spacing)
    SwTwips mnHeightOfLastLine;
    // member for the additional first line offset, which is caused by the list
    // label alignment for list level position and space mode LABEL_ALIGNMENT.
    // This additional first line offset is used for the text formatting.
    // It is NOT used for the determination of printing area.
    SwTwips mnAdditionalFirstLineOffset;

    sal_Int32 nOfst; // Is the offset in the Cntnt (character count)

    sal_uInt16 nCacheIdx; // Index into the cache, USHRT_MAX if there's definitely no fitting object in the cache

    // Separates the Master and creates a Follow or adjusts the data in the Follow
    void _AdjustFollow( SwTxtFormatter &rLine, const sal_Int32 nOffset,
                               const sal_Int32 nStrEnd, const sal_uInt8 nMode );
    inline void AdjustFollow( SwTxtFormatter &rLine, const sal_Int32 nOffset,
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
    void FormatAdjust( SwTxtFormatter &rLine, WidowsAndOrphans &rFrmBreak,
                       const sal_Int32 nStrLen, const bool bDummy );

    bool bLocked        : 1;        // In the Format?
    bool bWidow         : 1;        // Are we a Widow?
    bool bJustWidow     : 1;        // Did we just request to be a Widow?
    bool bEmpty         : 1;        // Are we an empty paragraph?
    bool bInFtnConnect  : 1;        // Is in Connect at the moment
    bool bFtn           : 1;        // Has at least one footnote
    bool bRepaint       : 1;        // TxtFrm: Repaint is ready to be fetched
    bool bBlinkPor      : 1;        // Contains Blink Portions
    bool bFieldFollow   : 1;        // Start with Field rest of the Master
    bool bHasAnimation  : 1;        // Contains animated SwGrfNumPortion
    bool bIsSwapped     : 1;        // during text formatting we swap the
                                    // width and height for vertical formatting
    // OD 14.03.2003 #i11760# - flag to control, if follow is formatted in
    // method <CalcFollow(..)>.
    // E.g., avoid formatting of follow, if method <SwLayoutFrm::FormatWidthCols(..)>
    // is running.
    bool mbFollowFormatAllowed : 1;

    void ResetPreps();
    inline void Lock() { bLocked = true; }
    inline void Unlock() { bLocked = false; }
    inline void SetWidow( const bool bNew ) { bWidow = bNew; }
    inline void SetJustWidow( const bool bNew ) { bJustWidow = bNew; }
    inline void SetEmpty( const bool bNew ) { bEmpty = bNew; }
    inline void SetFieldFollow( const bool bNew ) { bFieldFollow = bNew; }

    bool IsIdxInside( const sal_Int32 nPos, const sal_Int32 nLen ) const;

    // Changes the Frame or not (cf. FlyCnt)
    bool _GetCrsrOfst(SwPosition *pPos, const Point &rPoint,
                      const bool bChgFrm, SwCrsrMoveState* = 0 ) const;
    void FillCrsrPos( SwFillData &rFill ) const;

    // Format exactly one Line
    bool FormatLine( SwTxtFormatter &rLine, const bool bPrev );

    // In order to safe stack space, we split this method:
    // _Format calls _Format with parameters
    void _Format( SwParaPortion *pPara );
    void _Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
                  const bool bAdjust = false );
    void FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf );

    // Formats the Follow and ensures disposing on orphans
    bool CalcFollow(  const sal_Int32 nTxtOfst );

    // Corrects the position from which we need to format
    static sal_Int32 FindBrk(const OUString &rTxt, const sal_Int32 nStart,
                                       const sal_Int32 nEnd);

    // inline branch
    SwTwips _GetFtnFrmHeight() const;

    // Outsourced to CalcPreps
    bool CalcPrepFtnAdjust();

    // For Ftn and WidOrp: Forced validation
    void ValidateFrm();
    void ValidateBodyFrm();

    bool _GetDropRect( SwRect &rRect ) const;

    void SetPara( SwParaPortion *pNew, bool bDelete = true );

    bool _IsFtnNumFrm() const;

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

    // ST2
    SwWrongList* _SmartTagScan ( const OUString& aTxtToScan, SwWrongList *pSmartTagList,
                                 sal_Int32 nBegin,sal_Int32 nEnd,
                                 sal_Int32 nInsertPos, sal_Int32 nActPos,
                                 sal_Int32 &nChgStart, sal_Int32 &nChgEnd,
                                 sal_Int32 &nInvStart, sal_Int32 &nInvEnd);

    virtual void DestroyImpl() SAL_OVERRIDE;
    virtual ~SwTxtFrm();

protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;

public:

    com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >  GetTabStopInfo( SwTwips CurrentPos ) SAL_OVERRIDE;

    /**
     * This is public, as it needs to be called by some methods in order to save the Prepare
     * USE WITH CAUTION!
     */
    void Init();

    /// Is called by FormatSpelling()
    SwRect _AutoSpell( const SwCntntNode*, sal_Int32 );

    /// Is called by FormatSpelling()
    SwRect SmartTagScan( SwCntntNode* , sal_Int32 );

    /// Is called by CollectAutoCmplWords()
    void CollectAutoCmplWrds( SwCntntNode* , sal_Int32 );

    /**
     * Returns the screen position of rPos. The values are relative to the upper
     * left position of the page frame.
     * Additional information can be obtained by passing an SwCrsrMoveState object.
     * Returns false if rPos > number of character is string
     */
    virtual bool GetCharRect( SwRect& rRect, const SwPosition& rPos,
                                SwCrsrMoveState* pCMS = 0 ) const SAL_OVERRIDE;

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

    virtual bool FillSelection( SwSelectionList& rList, const SwRect& rRect ) const SAL_OVERRIDE;

    /**
     * In nOffset returns the offset of the char within the set
     * text buffer, which is closest to the position provided by
     * aPoint within the layout's SSize.
     *
     * @returns false if the SPoint is outside of the SSize else
     *          returns true
     */
    virtual bool GetCrsrOfst( SwPosition *, Point&,
                                  SwCrsrMoveState* = 0, bool bTestBackground = false ) const SAL_OVERRIDE;

    /**
     * Makes sure that the Frame is not switched (e.g. switched for a
     * character-bound Frame)
     */
    inline bool GetKeyCrsrOfst(SwPosition *pPos, const Point &rPoint ) const
            { return _GetCrsrOfst( pPos, rPoint, false ); }

    void   PaintExtraData( const SwRect & rRect ) const; /// Page number etc.
    SwRect Paint();
    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    virtual bool GetInfo( SfxPoolItem & ) const SAL_OVERRIDE;

    /**
     * Layout oriented cursor travelling:
     * Left border, right border, previous Line, following Line,
     * same horizontal position
     */
    virtual bool LeftMargin(SwPaM *) const SAL_OVERRIDE;
    virtual bool RightMargin(SwPaM *, bool bAPI = false) const SAL_OVERRIDE;

    virtual bool UnitUp(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const SAL_OVERRIDE;
    virtual bool UnitDown(SwPaM *, const SwTwips nOffset = 0,
                            bool bSetInReadOnly = false ) const SAL_OVERRIDE;
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
    SwCntntFrm *SplitFrm( const sal_Int32 nTxtPos );
    SwCntntFrm *JoinFrm();
    inline sal_Int32  GetOfst() const { return nOfst; }
           void        _SetOfst( const sal_Int32 nNewOfst );
    inline void        SetOfst ( const sal_Int32 nNewOfst );
    inline void        ManipOfst ( const sal_Int32 nNewOfst ){ nOfst = nNewOfst; }
           SwTxtFrm   *GetFrmAtPos ( const SwPosition &rPos);
    inline const SwTxtFrm *GetFrmAtPos ( const SwPosition &rPos) const;
    SwTxtFrm&   GetFrmAtOfst( const sal_Int32 nOfst );
    /// If there's a Follow and we don't contain text ourselves
    inline bool IsEmptyMaster() const
        { return GetFollow() && !GetFollow()->GetOfst(); }

    /// Returns the text portion we want to edit (for inline see underneath)
    const OUString& GetTxt() const;
    inline SwTxtNode *GetTxtNode()
        { return static_cast< SwTxtNode* >( SwCntntFrm::GetNode()); }
    inline const SwTxtNode *GetTxtNode() const
        { return static_cast< const SwTxtNode* >( SwCntntFrm::GetNode()); }

    SwTxtFrm(SwTxtNode * const, SwFrm* );

    /**
     * SwCntntFrm: the shortcut for the Frames
     * If the void* casts wrongly, it's its own fault!
     * The void* must be checked for 0 in any case!
     */
    virtual void Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, bool bNotify = true ) SAL_OVERRIDE;

    /**
     * nMaxHeight is the required height
     * bSplit indicates, that the paragraph has to be split
     * bTst indicates, that we are currently doing a test formatting
     */
    virtual bool WouldFit( SwTwips &nMaxHeight, bool &bSplit, bool bTst ) SAL_OVERRIDE;

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
    TYPEINFO_OVERRIDE();
    DECL_FIXEDMEMPOOL_NEWDEL(SwTxtFrm)

    // Locking
    inline bool IsLocked()      const { return bLocked;     }

    inline bool IsWidow()       const { return bWidow;      }
    inline bool IsJustWidow()   const { return bJustWidow;  }
    inline bool IsEmpty()       const { return bEmpty;      }
    inline bool HasFtn()        const { return bFtn;        }
    inline bool IsInFtnConnect()const { return bInFtnConnect;}
    inline bool IsFieldFollow() const { return bFieldFollow;}

    inline void SetRepaint() const;
    inline void ResetRepaint() const;
    inline bool HasRepaint() const { return bRepaint; }
    inline void SetBlinkPor() const;
    inline void ResetBlinkPor() const;
    inline bool HasBlinkPor() const { return bBlinkPor; }
    inline void SetAnimation() const
        { const_cast<SwTxtFrm*>(this)->bHasAnimation = true; }
    inline bool HasAnimation() const { return bHasAnimation; }

    inline bool IsSwapped() const { return bIsSwapped; }

    /// Does the Frm have a local footnote (in this Frm or Follow)?
#ifdef DBG_UTIL
    void CalcFtnFlag( sal_Int32 nStop = COMPLETE_STRING ); //For testing SplitFrm
#else
    void CalcFtnFlag();
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

    /// Ftn
    void RemoveFtn( const sal_Int32 nStart = 0,
                    const sal_Int32 nLen = COMPLETE_STRING );
    inline SwTwips GetFtnFrmHeight() const;
    SwTxtFrm *FindFtnRef( const SwTxtFtn *pFtn );
    inline const SwTxtFrm *FindFtnRef( const SwTxtFtn *pFtn ) const
    { return const_cast<SwTxtFrm *>(this)->FindFtnRef( pFtn ); }
    void ConnectFtn( SwTxtFtn *pFtn, const SwTwips nDeadLine );

    /**
     * If we're a Ftn that grows towards its reference ...
     * public, because it's needed by SwCntntFrm::MakeAll
     */
    SwTwips GetFtnLine( const SwTxtFtn *pFtn ) const;

    /**
     * Returns the left and the right margin document coordinates
     * (taking the paragraph attributes into account)
     */
    inline SwTwips GetLeftMargin() const;
    inline SwTwips GetRightMargin() const;

    virtual void Format( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    virtual void CheckDirection( bool bVert ) SAL_OVERRIDE;

    /// Returns the sum of line height in pLine
    sal_uInt16 GetParHeight() const;

    /// Returns the remaining height
    inline SwTwips GetRstHeight() const;

    inline       SwTxtFrm *GetFollow();
    inline const SwTxtFrm *GetFollow() const;

    /// Find the page number of ErgoSum and QuoVadis
    SwTxtFrm *FindQuoVadisFrm();

    /**
     * Makes up for formatting if the Idle Handler has struck
     *
     * #i29062# GetFormatted() can trigger a full formatting
     * of the paragraph, causing other layout frames to become invalid. This
     * has to be avoided during painting. Therefore we need to pass the
     * information that we are currently in the paint process.
     */
    SwTxtFrm* GetFormatted( bool bForceQuickFormat = false );

    /// Will be moved soon
    inline void SetFtn( const bool bNew ) { bFtn = bNew; }

    /// Respect the Follows
    inline bool IsInside( const sal_Int32 nPos ) const;

    const SwBodyFrm   *FindBodyFrm()   const;

    /// DropCaps and selections
    inline bool GetDropRect( SwRect &rRect ) const
    { return HasPara() && _GetDropRect( rRect ); }

    static SwCache *GetTxtCache() { return pTxtCache; }
    static void     SetTxtCache( SwCache *pNew ) { pTxtCache = pNew; }

    static long GetMinPrtLine() { return nMinPrtLine; }
    static void SetMinPrtLine( long nNew ) { nMinPrtLine = nNew; }

    inline sal_uInt16 GetCacheIdx() const { return nCacheIdx; }
    inline void   SetCacheIdx( const sal_uInt16 nNew ) { nCacheIdx = nNew; }

    /// Removes the Line information from the Cache
    void ClearPara();

    /// Am I a FtnFrm, with a number at the start of the paragraph?
    inline bool IsFtnNumFrm() const
    { return IsInFtn() && !GetIndPrev() && _IsFtnNumFrm(); }

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
    void MoveFlyInCnt( SwTxtFrm *pNew, sal_Int32 nStart, sal_Int32 nEnd );

    /// Calculates the position of FlyInCntFrms
    sal_Int32 CalcFlyPos( SwFrmFmt* pSearch );

    /// Determines the start position and step size of the register
    bool FillRegister( SwTwips& rRegStart, sal_uInt16& rRegDiff );

    /// Determines the line count
    sal_uInt16 GetLineCount( sal_Int32 nPos );

    /// For displaying the line numbers
    sal_uLong GetAllLines()  const { return nAllLines; }
    sal_uLong GetThisLines() const { return nThisLines;}
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

    static void repaintTextFrames( const SwTxtNode& rNode );

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const SAL_OVERRIDE;
};

class SwTxtFrmLocker
{
private:
    SwTxtFrm * const pFrm;
public:
    inline SwTxtFrmLocker( SwTxtFrm *pTxtFrm )
        : pFrm( pTxtFrm->IsLocked() ? 0 : pTxtFrm )
    { if( pFrm ) pFrm->Lock(); }
    inline ~SwTxtFrmLocker() { if( pFrm ) pFrm->Unlock(); }
};

inline const SwParaPortion *SwTxtFrm::GetPara() const
{
    return const_cast<SwTxtFrm*>(this)->GetPara();
}

inline bool SwTxtFrm::HasPara() const
{
    return nCacheIdx!=USHRT_MAX && _HasPara();
}

/// 9104: Frm().Height() - Prt().Height(), see widorp.cxx and 7455, 6114, 7908
inline SwTwips SwTxtFrm::GetRstHeight() const
{
    return !GetUpper() ? 0 : static_cast<const SwFrm*>(GetUpper())->Frm().Top()
                           + static_cast<const SwFrm*>(GetUpper())->Prt().Top()
                           + static_cast<const SwFrm*>(GetUpper())->Prt().Height()
                           - Frm().Top() - (Frm().Height() - Prt().Height());
}

inline SwTwips SwTxtFrm::GetLeftMargin() const
{
    return Frm().Left() + Prt().Left();
}
inline SwTwips SwTxtFrm::GetRightMargin() const
{
    return Frm().Left() + Prt().Left() + Prt().Width();
}
inline SwTwips SwTxtFrm::GrowTst( const SwTwips nGrow )
{
    return Grow( nGrow, true );
}

inline bool SwTxtFrm::IsInside( const sal_Int32 nPos ) const
{
    bool bRet = true;
    if( nPos < GetOfst() )
        bRet = false;
    else
    {
        const SwTxtFrm *pFoll = GetFollow();
        if( pFoll && nPos >= pFoll->GetOfst() )
            bRet = false;
    }
    return bRet;
}

inline SwTwips SwTxtFrm::GetFtnFrmHeight() const
{
    if(  !IsFollow() && IsInFtn() && HasPara() )
        return _GetFtnFrmHeight();
    else
        return 0;
}

inline const SwTxtFrm *SwTxtFrm::GetFollow() const
{
    return static_cast<const SwTxtFrm*>(SwCntntFrm::GetFollow());
}
inline SwTxtFrm *SwTxtFrm::GetFollow()
{
    return static_cast<SwTxtFrm*>(SwCntntFrm::GetFollow());
}

inline const SwTxtFrm *SwTxtFrm::GetFrmAtPos( const SwPosition &rPos) const
{
    return const_cast<SwTxtFrm*>(this)->GetFrmAtPos( rPos );
}

inline void SwTxtFrm::AdjustFollow( SwTxtFormatter &rLine,
    const sal_Int32 nOffset, const sal_Int32 nStrEnd, const sal_uInt8 nMode )
{
    if ( HasFollow() )
        _AdjustFollow( rLine, nOffset, nStrEnd, nMode );
}

inline void SwTxtFrm::SetOfst( const sal_Int32 nNewOfst )
{
    if ( nOfst != nNewOfst )
        _SetOfst( nNewOfst );
}

inline void SwTxtFrm::SetRepaint() const
{
    const_cast<SwTxtFrm*>(this)->bRepaint = true;
}
inline void SwTxtFrm::ResetRepaint() const
{
    const_cast<SwTxtFrm*>(this)->bRepaint = false;
}

inline void SwTxtFrm::SetBlinkPor() const
{
    const_cast<SwTxtFrm*>(this)->bBlinkPor = true;
}
inline void SwTxtFrm::ResetBlinkPor() const
{
    const_cast<SwTxtFrm*>(this)->bBlinkPor = false;
}

#define SWAP_IF_SWAPPED( pFrm )\
    bool bUndoSwap = false;   \
    if ( pFrm->IsVertical() && pFrm->IsSwapped() )\
    {                                 \
        bUndoSwap = true;         \
        const_cast<SwTxtFrm*>(pFrm)->SwapWidthAndHeight();         \
    }

#define SWAP_IF_NOT_SWAPPED( pFrm )\
    bool bUndoSwap = false;     \
    if ( pFrm->IsVertical() && ! pFrm->IsSwapped() )\
    {                                   \
        bUndoSwap = true;           \
        const_cast<SwTxtFrm*>(pFrm)->SwapWidthAndHeight();         \
    }

#define UNDO_SWAP( pFrm )\
    if ( bUndoSwap )\
        const_cast<SwTxtFrm*>(pFrm)->SwapWidthAndHeight();

/**
 * Helper class which can be used instead of the macros if a function
 * has too many returns
 */
class SwFrmSwapper
{
    const SwTxtFrm* pFrm;
    bool bUndo;
public:
    SwFrmSwapper( const SwTxtFrm* pFrm, bool bSwapIfNotSwapped );
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
