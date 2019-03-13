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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FLOWFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FLOWFRM_HXX

#include "frame.hxx"
#include "layfrm.hxx"
#include <swtypes.hxx>

class SvxFormatKeepItem;
class SvxFormatBreakItem;
class SwPageFrame;
class SwRect;
class SwBorderAttrs;
class SwDoc;
class SwNodeIndex;

/** Base class that provides the general functionalities for frames that are
    allowed at page breaks (flow) and shall continue on the next page (can be
    split), e.g. paragraphs (ContentFrame) or tables (TabFrame).

    Some parts of these functionalities are implemented in FlowFrame while the
    specific ones are done in the corresponding Frame classes. The FlowFrame has to
    be seen as a base class. As such it is no Frame by itself and thus no direct
    instances of FlowFrame can exist.

    Actually it is not even a real Frame. The obvious implementation would be a
    FlowFrame that is virtually inherited from SwFrame and that works with its own
    member data. Further classes would need to inherit from FlowFrame and (via
    multiple base classes since the class tree splits exactly at the branch
    from SwFrame to SwContentFrame and SwLayoutFrame) also virtually from SwFrame as
    well. Unfortunately, this leads - besides problems with compilers and
    debugging programs - to high additional costs, that we IMHO are not able to
    afford nowadays.

    Hence, we use another technique: A FlowFrame keeps a reference to a SwFrame
    - which it is actually itself - and they are friends. As a result, the
    FlowFrame can work with the reference to the SwFrame instead of working with
    its own this-pointer.
 */
class SwFlowFrame
{
    // PrepareMake is allowed to lock/unlock (robustness)
    friend inline void PrepareLock  ( SwFlowFrame * );
    friend inline void PrepareUnlock( SwFlowFrame * );
    friend inline void TableSplitRecalcLock( SwFlowFrame * );
    friend inline void TableSplitRecalcUnlock( SwFlowFrame * );
    // #i44049#
    friend class SwObjectFormatterTextFrame;
    friend class FlowFrameJoinLockGuard;

    // TableSel is allowed to reset the follow-bit
    friend inline void UnsetFollow( SwFlowFrame *pFlow );

    friend void MakeFrames( SwDoc *, const SwNodeIndex &, const SwNodeIndex & );

    friend class SwNode2LayImpl;

    SwFrame& m_rThis;

    // helper methods for MoveSubTree()
    static SwLayoutFrame *CutTree( SwFrame* );
    static bool         PasteTree( SwFrame *, SwLayoutFrame *, SwFrame *, SwFrame* );

    /** indicates that a backward move was done over multiple pages

        Needed for the interaction of _GetPrevxxx and MoveBwd so that multiple
        pages can be skipped at the same time. In addition, it is evaluated by
        the MoveBwd() method in TabFrame.
    */
    static bool m_bMoveBwdJump;

    /** helper method to determine previous frame for calculation of the
        upper space

        #i11860#

        @param _pProposedPrevFrame
        optional input parameter - pointer to frame, which should be used
        instead of the direct previous frame.
    */
    const SwFrame* GetPrevFrameForUpperSpaceCalc_( const SwFrame* _pProposedPrevFrame = nullptr ) const;

    /** method to determine the upper space amount, which is considered for
        the previous frame

        #i11860#
    */
    SwTwips GetUpperSpaceAmountConsideredForPrevFrame() const;

    /** method to determine the upper space amount, which is considered for
        the page grid

        #i11860#
    */
    SwTwips GetUpperSpaceAmountConsideredForPageGrid_(
                                const SwTwips _nUpperSpaceWithoutGrid ) const;

protected:
    SwFlowFrame *m_pFollow;
    SwFlowFrame *m_pPrecede;

    bool m_bLockJoin  :1; // if true than joins (and thus deletes) are prohibited!
    bool m_bUndersized:1; // I am smaller than needed
    bool m_bFlyLock   :1; // stop positioning of at-character flyframes

    // checks if forward flow makes sense to prevent infinite moves
    inline bool IsFwdMoveAllowed();
    // #i44049# - method <CalcContent(..)> has to check this property.
    friend void CalcContent( SwLayoutFrame *pLay, bool bNoColl );
    bool IsKeepFwdMoveAllowed( bool bIgnoreMyOwnKeepValue = false );    // like above, forward flow for Keep.

    /** method to determine overlapping of an object that requests floating

        0: no overlapping
        1: objects that are anchored at the FlowFrame overlap
        2: objects that are anchored somewhere else overlap
        3: both types of objects overlap
    */
    sal_uInt8 BwdMoveNecessary( const SwPageFrame *pPage, const SwRect &rRect );

    void LockJoin()   { m_bLockJoin = true;  }
    void UnlockJoin() { m_bLockJoin = false; }

    bool CheckMoveFwd( bool& rbMakePage, bool bKeep, bool bIgnoreMyOwnKeepValue );
    bool MoveFwd( bool bMakePage, bool bPageBreak, bool bMoveAlways = false );
    bool MoveBwd( bool &rbReformat );
    virtual bool ShouldBwdMoved( SwLayoutFrame *pNewUpper, bool bHead, bool &rReformat )=0;

public:
    SwFlowFrame( SwFrame &rFrame );
    virtual ~SwFlowFrame();

    const SwFrame& GetFrame() const { return m_rThis; }
          SwFrame& GetFrame()       { return m_rThis; }

    static bool IsMoveBwdJump() { return m_bMoveBwdJump; }
    static void SetMoveBwdJump( bool bNew ){ m_bMoveBwdJump = bNew; }

    void SetUndersized( const bool bNew ) { m_bUndersized = bNew; }
    bool IsUndersized()  const { return m_bUndersized; }

    bool IsPrevObjMove() const;

    /** hook tree onto new parent with minimal operations and notifications */
    void MoveSubTree( SwLayoutFrame* pParent, SwFrame* pSibling = nullptr );

    bool HasFollow() const { return m_pFollow != nullptr; }
    bool IsFollow() const { return nullptr != m_pPrecede; }
    bool IsAnFollow( const SwFlowFrame *pFlow ) const;
    const SwFlowFrame *GetFollow() const { return m_pFollow; }
          SwFlowFrame *GetFollow()       { return m_pFollow; }
    void SetFollow(SwFlowFrame *const pFollow);

    const SwFlowFrame *GetPrecede() const { return m_pPrecede; }
          SwFlowFrame *GetPrecede()       { return m_pPrecede; }

    bool IsJoinLocked() const { return m_bLockJoin; }
    bool IsAnyJoinLocked() const { return m_bLockJoin || HasLockedFollow(); }

    bool IsPageBreak( bool bAct ) const;
    bool IsColBreak( bool bAct ) const;

    /** method to determine if a Keep needs to be considered (Breaks!) */
    bool IsKeep(SvxFormatKeepItem const& rKeep,
                SvxFormatBreakItem const& rBreak,
                bool bBreakCheck = false ) const;

    bool HasLockedFollow() const;

    bool HasParaSpaceAtPages( bool bSct ) const;

    /** method to determine the upper space hold by the frame

        #i11860#

        @param _bConsiderGrid
        optional input parameter - consider the page grid while calculating?
    */
    SwTwips CalcUpperSpace( const SwBorderAttrs *pAttrs = nullptr,
                            const SwFrame* pPr = nullptr,
                            const bool _bConsiderGrid = true ) const;

    /** method to determine the upper space amount, which is considered for
        the previous frame and the page grid, if option 'Use former object
        positioning' is OFF

        #i11860#
    */
    SwTwips GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid() const;

    /** calculation of lower space */
    SwTwips CalcLowerSpace( const SwBorderAttrs* _pAttrs = nullptr ) const;

    /** calculation of the additional space to be considered, if flow frame
        is the last inside a table cell

        #i26250

        @param _pAttrs
        optional input parameter - border attributes of the flow frame.
        Used for optimization, if caller has already determined the border
        attributes.

        @return SwTwips
    */
    SwTwips CalcAddLowerSpaceAsLastInTableCell(
                                    const SwBorderAttrs* _pAttrs = nullptr ) const;

    void CheckKeep();

    void SetFlyLock( bool bNew ){ m_bFlyLock = bNew; }
    bool IsFlyLock() const {    return m_bFlyLock; }

    bool ForbiddenForFootnoteCntFwd() const;

    // Casting of a Frame into a FlowFrame (if it is one, otherwise 0)
    // These methods need to be customized in subclasses!
    static       SwFlowFrame *CastFlowFrame( SwFrame *pFrame );
    static const SwFlowFrame *CastFlowFrame( const SwFrame *pFrame );
};

inline bool SwFlowFrame::IsFwdMoveAllowed()
{
    return m_rThis.GetIndPrev() != nullptr;
}

//use this to protect a SwLayoutFrame for a given scope from getting merged with
//its neighbour and thus deleted
class FlowFrameJoinLockGuard
{
private:
    SwFlowFrame *m_pFlow;
    bool m_bOldJoinLocked;
public:
    //JoinLock pParent for the lifetime of the Cut/Paste call, etc. to avoid
    //SwSectionFrame::MergeNext removing the pParent we're trying to reparent
    //into
    FlowFrameJoinLockGuard(SwLayoutFrame* pFrame)
    {
        m_pFlow = SwFlowFrame::CastFlowFrame(pFrame);
        if (m_pFlow)
        {
            m_bOldJoinLocked = m_pFlow->IsJoinLocked();
            m_pFlow->LockJoin();
        }
        else
        {
            m_bOldJoinLocked = false;
        }
    }

    ~FlowFrameJoinLockGuard()
    {
        if (m_pFlow && !m_bOldJoinLocked)
            m_pFlow->UnlockJoin();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
