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

#include "frmtool.hxx"

class SwPageFrm;
class SwRect;
class SwBorderAttrs;
class SwDoc;
class SwNodeIndex;
// #i44049#
class SwObjectFormatterTextFrm;

/** Base class that provides the general functionalities for frames that are
    allowed at page breaks (flow) and shall continue on the next page (can be
    split), e.g. paragraphs (ContentFrm) or tables (TabFrm).

    Some parts of these functionalities are implemented in FlowFrm while the
    specific ones are done in the corresponding Frm classes. The FlowFrm has to
    be seen as a base class. As such it is no Frm by itself and thus no direct
    instances of FlowFrm can exist.

    Actually it is not even a real Frm. The obvious implementation would be a
    FlowFrm that is virtually inherited from SwFrm and that works with its own
    member data. Further classes would need to inherit from FlowFrm and (via
    multiple base classes since the class tree splits exactly at the branch
    from SwFrm to SwContentFrm and SwLayoutFrm) also virtually from SwFrm as
    well. Unfortunately, this leads - besides problems with compilers and
    debugging programs - to high additional costs, that we IMHO are not able to
    afford nowadays.

    Hence, we use another technique: A FlowFrm keeps a reference to a SwFrm
    - which it is actually itself - and they are friends. As a result, the
    FlowFrm can work with the reference to the SwFrm instead of working with
    its own this-pointer.
 */
class SwFlowFrm
{
    // PrepareMake is allowed to lock/unlock (robustness)
    friend inline void PrepareLock  ( SwFlowFrm * );
    friend inline void PrepareUnlock( SwFlowFrm * );
    friend inline void TableSplitRecalcLock( SwFlowFrm * );
    friend inline void TableSplitRecalcUnlock( SwFlowFrm * );
    // #i44049#
    friend class SwObjectFormatterTextFrm;
    friend class FlowFrmJoinLockGuard;

    // TableSel is allowed to reset the follow-bit
    friend inline void UnsetFollow( SwFlowFrm *pFlow );

    friend void MakeFrms( SwDoc *, const SwNodeIndex &, const SwNodeIndex & );

    friend class SwNode2LayImpl;

    SwFrm& m_rThis;

    // helper methods for MoveSubTree()
    static SwLayoutFrm *CutTree( SwFrm* );
    static bool         PasteTree( SwFrm *, SwLayoutFrm *, SwFrm *, SwFrm* );

    /** indicates that a backward move was done over multiple pages

        Needed for the interaction of _GetPrevxxx and MoveBwd so that multiple
        pages can be skipped at the same time. In addition, it is evaluated by
        the MoveBwd() method in TabFrm.
    */
    static bool m_bMoveBwdJump;

    /** helper method to determine previous frame for calculation of the
        upper space

        #i11860#

        @param _pProposedPrevFrm
        optional input parameter - pointer to frame, which should be used
        instead of the direct previous frame.
    */
    const SwFrm* _GetPrevFrmForUpperSpaceCalc( const SwFrm* _pProposedPrevFrm = 0L ) const;

    /** method to detemine the upper space amount, which is considered for
        the previous frame

        #i11860#
    */
    SwTwips _GetUpperSpaceAmountConsideredForPrevFrm() const;

    /** method to detemine the upper space amount, which is considered for
        the page grid

        #i11860#
    */
    SwTwips _GetUpperSpaceAmountConsideredForPageGrid(
                                const SwTwips _nUpperSpaceWithoutGrid ) const;

protected:
    SwFlowFrm *m_pFollow;
    SwFlowFrm *m_pPrecede;

    bool m_bLockJoin  :1; // if true than joins (and thus deletes) are prohibited!
    bool m_bUndersized:1; // I am smaller than needed
    bool m_bFlyLock   :1; // stop positioning of at-character flyframes

    // checks if forward flow makes sense to prevent infinite moves
    inline bool IsFwdMoveAllowed();
    // #i44049# - method <CalcContent(..)> has to check this property.
    friend void CalcContent( SwLayoutFrm *pLay, bool bNoColl, bool bNoCalcFollow );
    bool IsKeepFwdMoveAllowed();    // like above, forward flow for Keep.

    /** method to determine overlapping of an object that requests floating

        0: no overlapping
        1: objects that are anchored at the FlowFrm overlap
        2: objects that are anchored somewhere else overlap
        3: both types of objects overlap
    */
    sal_uInt8 BwdMoveNecessary( const SwPageFrm *pPage, const SwRect &rRect );

    void LockJoin()   { m_bLockJoin = true;  }
    void UnlockJoin() { m_bLockJoin = false; }

    bool CheckMoveFwd( bool& rbMakePage, bool bKeep, bool bMovedBwd );
    bool MoveFwd( bool bMakePage, bool bPageBreak, bool bMoveAlways = false );
    bool MoveBwd( bool &rbReformat );
    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool bHead, bool &rReformat )=0;

public:
    SwFlowFrm( SwFrm &rFrm );
    virtual ~SwFlowFrm();

    const SwFrm& GetFrm() const { return m_rThis; }
          SwFrm& GetFrm()       { return m_rThis; }

    static bool IsMoveBwdJump() { return m_bMoveBwdJump; }
    static void SetMoveBwdJump( bool bNew ){ m_bMoveBwdJump = bNew; }

    inline void SetUndersized( const bool bNew ) { m_bUndersized = bNew; }
    inline bool IsUndersized()  const { return m_bUndersized; }

    bool IsPrevObjMove() const;

    /** hook tree onto new parent with minimal operations and notifications */
    void MoveSubTree( SwLayoutFrm* pParent, SwFrm* pSibling = 0 );

    bool HasFollow() const { return m_pFollow != nullptr; }
    bool IsFollow() const { return 0 != m_pPrecede; }
    bool IsAnFollow( const SwFlowFrm *pFlow ) const;
    const SwFlowFrm *GetFollow() const { return m_pFollow; }
          SwFlowFrm *GetFollow()       { return m_pFollow; }
    void SetFollow(SwFlowFrm *const pFollow);

    const SwFlowFrm *GetPrecede() const { return m_pPrecede; }
          SwFlowFrm *GetPrecede()       { return m_pPrecede; }

    bool IsJoinLocked() const { return m_bLockJoin; }
    bool IsAnyJoinLocked() const { return m_bLockJoin || HasLockedFollow(); }

    bool IsPageBreak( bool bAct ) const;
    bool IsColBreak( bool bAct ) const;

    /** method to determine if a Keep needs to be considered (Breaks!) */
    bool IsKeep( const SwAttrSet& rAttrs, bool bBreakCheck = false ) const;

    bool HasLockedFollow() const;

    bool HasParaSpaceAtPages( bool bSct ) const;

    /** method to determine the upper space hold by the frame

        #i11860#

        @param _bConsiderGrid
        optional input parameter - consider the page grid while calculating?
    */
    SwTwips CalcUpperSpace( const SwBorderAttrs *pAttrs = NULL,
                            const SwFrm* pPr = NULL,
                            const bool _bConsiderGrid = true ) const;

    /** method to determine the upper space amount, which is considered for
        the previous frame and the page grid, if option 'Use former object
        positioning' is OFF

        #i11860#
    */
    SwTwips GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() const;

    /** calculation of lower space */
    SwTwips CalcLowerSpace( const SwBorderAttrs* _pAttrs = 0L ) const;

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
                                    const SwBorderAttrs* _pAttrs = 0L ) const;

    void CheckKeep();

    void SetFlyLock( bool bNew ){ m_bFlyLock = bNew; }
    bool IsFlyLock() const {    return m_bFlyLock; }

    // Casting of a Frm into a FlowFrm (if it is one, otherwise 0)
    // These methods need to be customized in subclasses!
    static       SwFlowFrm *CastFlowFrm( SwFrm *pFrm );
    static const SwFlowFrm *CastFlowFrm( const SwFrm *pFrm );
};

inline bool SwFlowFrm::IsFwdMoveAllowed()
{
    return m_rThis.GetIndPrev() != 0;
}

//use this to protect a SwLayoutFrm for a given scope from getting merged with
//its neighbour and thus deleted
class FlowFrmJoinLockGuard
{
private:
    SwFlowFrm *m_pFlow;
    bool m_bOldJoinLocked;
public:
    //JoinLock pParent for the lifetime of the Cut/Paste call, etc. to avoid
    //SwSectionFrm::MergeNext removing the pParent we're trying to reparent
    //into
    FlowFrmJoinLockGuard(SwLayoutFrm* pFrm)
    {
        m_pFlow = SwFlowFrm::CastFlowFrm(pFrm);
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

    ~FlowFrmJoinLockGuard()
    {
        if (m_pFlow && !m_bOldJoinLocked)
            m_pFlow->UnlockJoin();
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
