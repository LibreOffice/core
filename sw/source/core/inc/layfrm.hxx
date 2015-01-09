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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX

#include "frame.hxx"

class SwAnchoredObject;
class SwCntntFrm;
class SwFlowFrm;
class SwFmtCol;
struct SwCrsrMoveState;
class SwFrmFmt;
class SwBorderAttrs;
class SwFmtFrmSize;
class SwCellFrm;

class SwLayoutFrm: public SwFrm
{
    // The SwFrm in disguise
    friend class SwFlowFrm;
    friend class SwFrm;

    // Releases the Lower while restructuring columns
    friend SwFrm* SaveCntnt( SwLayoutFrm *, SwFrm * );
    friend void   RestoreCntnt( SwFrm *, SwLayoutFrm *, SwFrm *pSibling, bool bGrow );

#ifdef DBG_UTIL
    //removes empty SwSectionFrms from a chain
    friend SwFrm* SwClearDummies( SwFrm* pFrm );
#endif

    void CopySubtree( const SwLayoutFrm *pDest );
protected:
    void Destroy(); // for ~SwRootFrm

    virtual void Format( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    virtual void MakeAll() SAL_OVERRIDE;

    SwFrm * m_pLower;
    std::vector<SwAnchoredObject*> m_VertPosOrientFrmsFor;

    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) SAL_OVERRIDE;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) SAL_OVERRIDE;

    long CalcRel( const SwFmtFrmSize &rSz, bool bWidth ) const;

public:
    // --> #i28701#
    TYPEINFO_OVERRIDE();

    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const;
    void RefreshLaySubsidiary( const SwPageFrm*, const SwRect& ) const;
    void RefreshExtraData( const SwRect & ) const;

    /// Change size of lowers proportionally
    void ChgLowersProp( const Size& rOldSize );

    void AdjustColumns( const SwFmtCol *pCol, bool bAdjustAttributes );

    void ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
        const bool bChgFtn = false );

    /// Paints the column separation line for the inner columns
    void PaintColLines( const SwRect &, const SwFmtCol &,
                        const SwPageFrm * ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const SAL_OVERRIDE;

    virtual bool GetCrsrOfst( SwPosition *, Point&,
                               SwCrsrMoveState* = 0, bool bTestBackground = false ) const SAL_OVERRIDE;

    virtual void Cut() SAL_OVERRIDE;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;

    /**
     * Finds the closest Cntnt for the SPoint
     * Is used for Pages, Flys and Cells if GetCrsrOfst failed
     */
    const SwCntntFrm* GetCntntPos( Point &rPoint, const bool bDontLeave,
                                   const bool bBodyOnly = false,
                                   const bool bCalc = false,
                                   const SwCrsrMoveState *pCMS = 0,
                                   const bool bDefaultExpand = true ) const;

    SwLayoutFrm( SwFrmFmt*, SwFrm* );
    virtual ~SwLayoutFrm();

    virtual void Paint( SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const SAL_OVERRIDE;
    const SwFrm *Lower() const { return m_pLower; }
          SwFrm *Lower()       { return m_pLower; }
    const SwCntntFrm *ContainsCntnt() const;
    inline SwCntntFrm *ContainsCntnt();
    const SwCellFrm *FirstCell() const;
    inline SwCellFrm *FirstCell();

    /**
     * Method <ContainsAny()> doesn't investigate content of footnotes by default.
     * But under certain circumstances this investigation is intended.
     * Thus, introduce new optional parameter <_bInvestigateFtnForSections>.
     * It's default is <false>, still indicating that content of footnotes isn't
     * investigated for sections.
     */
    const SwFrm *ContainsAny( const bool _bInvestigateFtnForSections = false ) const;
    inline SwFrm *ContainsAny( const bool _bInvestigateFtnForSections = false );
    bool IsAnLower( const SwFrm * ) const;

    virtual const SwFrmFmt *GetFmt() const;
    virtual       SwFrmFmt *GetFmt();
    void        SetFrmFmt( SwFrmFmt* );

    /**
     * Moving the Ftns of all Lowers - starting from StartCntnt
     *
     * @returns true if at least one Ftn was moved
     * Calls the page number update if bFtnNums is set
     */
    bool MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                        SwFtnBossFrm *pNewBoss, const bool bFtnNums );

    // --> #i28701# - change purpose of method and its name
    // --> #i44016# - add parameter <_bUnlockPosOfObjs> to
    /// force an unlockposition call for the lower objects.
    void NotifyLowerObjs( const bool _bUnlockPosOfObjs = false );

    /**
     * Invalidates the inner Frames, whose width and/or height are
     * calculated using percentages.
     * Frames that are anchored to this or inner Frames, are also invalidated.
     */
    void InvaPercentLowers( SwTwips nDiff = 0 );

    /// Called by Format for Frames and Areas with columns
    void FormatWidthCols( const SwBorderAttrs &, const SwTwips nBorder,
                          const SwTwips nMinHeight );

    /**
     * InnerHeight returns the height of the content and may be bigger or
     * less than the PrtArea-Height of the layoutframe himself
     */
    SwTwips InnerHeight() const;

    /** method to check relative position of layout frame to
        a given layout frame.

        refactoring of pseudo-local method <lcl_Apres(..)> in
        <txtftn.cxx> for #104840#.

        @param _aCheckRefLayFrm
        constant reference of an instance of class <SwLayoutFrm> which
        is used as the reference for the relative position check.

        @return true, if <this> is positioned before the layout frame <p>
    */
    bool IsBefore( const SwLayoutFrm* _pCheckRefLayFrm ) const;

    const SwFrm* GetLastLower() const;
    inline SwFrm* GetLastLower();

    virtual void PaintBreak() const;

    void SetVertPosOrientFrmFor(SwAnchoredObject *pObj)
    {
        m_VertPosOrientFrmsFor.push_back(pObj);
    }

    void ClearVertPosOrientFrmFor(SwAnchoredObject *pObj)
    {
        m_VertPosOrientFrmsFor.erase(
            std::remove(m_VertPosOrientFrmsFor.begin(),
                m_VertPosOrientFrmsFor.end(), pObj),
            m_VertPosOrientFrmsFor.end());
    }
};

/**
 * In order to save us from duplicating implementations, we cast here
 * a little.
 */
inline SwCntntFrm* SwLayoutFrm::ContainsCntnt()
{
    return (SwCntntFrm*)(static_cast<const SwLayoutFrm*>(this)->ContainsCntnt());
}

inline SwCellFrm* SwLayoutFrm::FirstCell()
{
    return (SwCellFrm*)(static_cast<const SwLayoutFrm*>(this)->FirstCell());
}

inline SwFrm* SwLayoutFrm::ContainsAny( const bool _bInvestigateFtnForSections )
{
    return (SwFrm*)(static_cast<const SwLayoutFrm*>(this)->ContainsAny( _bInvestigateFtnForSections ));
}

/**
 * These SwFrm inlines are here, so that frame.hxx does not need to include layfrm.hxx
 */
inline bool SwFrm::IsColBodyFrm() const
{
    return mnFrmType == FRM_BODY && GetUpper()->IsColumnFrm();
}

inline bool SwFrm::IsPageBodyFrm() const
{
    return mnFrmType == FRM_BODY && GetUpper()->IsPageFrm();
}

inline SwFrm* SwLayoutFrm::GetLastLower()
{
    return const_cast<SwFrm*>(static_cast<const SwLayoutFrm*>(this)->GetLastLower());
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
