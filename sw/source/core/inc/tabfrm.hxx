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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TABFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TABFRM_HXX

#include <tools/mempool.hxx>
#include "layfrm.hxx"
#include "flowfrm.hxx"

class SwTable;
class SwBorderAttrs;
class SwAttrSetChg;

/// SwTabFrm is one table in the document layout, containing rows (which contain cells).
class SwTabFrm: public SwLayoutFrm, public SwFlowFrm
{
    friend void CalcContent( SwLayoutFrm *pLay, bool bNoColl, bool bNoCalcFollow );

    // does the special treatment for _Get[Next|Prev]Leaf()
    using SwFrm::GetLeaf;

    SwTable * m_pTable;

    bool m_bComplete        :1; /// Set entries for Repaint without needing to
                                /// set the base class' CompletePaint
                                /// With that we would want to avoid unnecessary
                                /// table repaints
    bool m_bCalcLowers      :1; /// For stability of the content in MakeAll
    bool m_bLowersFormatted :1; /// Communication between MakeAll and Layact
    bool m_bLockBackMove    :1; /// The Master took care of the BackMove test
    bool m_bResizeHTMLTable :1; /// Call the Resize of the HTMLTableLayout in the MakeAll
                                /// This is an optimization, so that we don't have to call
                                /// it in ContentFrm::Grow; there it might be called for
                                /// _every_ Cell

    bool m_bONECalcLowers   :1; /// Primarily for the StarONE SS
                                /// The Contents are formatted via Calc() on MakeAll in any
                                /// case. There are no further invalidations and that path can
                                /// hardly give any guarantees

    bool m_bHasFollowFlowLine :1; /// Means that the first line in the follow
                                /// is indented to contain content from a broken
                                /// cell
    bool m_bIsRebuildLastLine :1; /// Means that currently the last line of the
                                /// TabFrame is rebuilt. In this case we do not
                                //  want any notification to the master table

    bool m_bRestrictTableGrowth :1;     // Usually, the table may grow infinitely,
                                        // as the table can be split in SwTabFrm::MakeAll
                                        // In MakeAll, this flag is set to indicate that
                                        // the table may only grow inside its upper. This
                                        // is necessary, in order to let the text flow into
                                        // the FollowFlowLine

    bool m_bRemoveFollowFlowLinePending :1;

    // #i26945#
    bool m_bConsiderObjsForMinCellHeight :1; // Usually, the floating screen objects
                                           // are considered during the calculation
                                           // for the minimal cell height.
                                           // For the splitting table rows algorithm
                                           // we need not to consider floating
                                           // screen object for the preparation
                                           // of the re-calculation of the
                                           // last table row.
    // #i26945#
    bool m_bObjsDoesFit :1; // For splitting table rows algorithm, this boolean
                          // indicates, if the floating screen objects fits

    bool m_bInRecalcLowerRow : 1;

    /**
     * Split() splits the Frm at the specified position: a Follow is
     * created and constructed and inserted directly after this.
     * Join() gets the Follow's content and destroys it.
     */
    bool Split( const SwTwips nCutPos, bool bTryToSplit, bool bTableRowKeep );
    bool Join();

    void _UpdateAttr(
        const SfxPoolItem*,
        const SfxPoolItem*, sal_uInt8 &,
        SwAttrSetChg *pa = 0,
        SwAttrSetChg *pb = 0 );

    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool bHead, bool &rReformat ) override;

    virtual void DestroyImpl() override;
    virtual ~SwTabFrm();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 ) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    // only changes the Framesize, not the PrtArea size
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) override;

public:
    SwTabFrm( SwTable &, SwFrm* );  // calling Regist Flys always after creation _and_pasting!
    SwTabFrm( SwTabFrm & ); // _only_ for the creation of follows

    void JoinAndDelFollows();   // for DelFrms of the TableNodes!

    // calls thr RegistFlys of the rows
    void RegistFlys();

    inline const SwTabFrm *GetFollow() const;
    inline       SwTabFrm *GetFollow();
    SwTabFrm* FindMaster( bool bFirstMaster = false ) const;

    virtual bool GetInfo( SfxPoolItem &rHint ) const override;
    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const override;
    virtual void CheckDirection( bool bVert ) override;

    virtual void Cut() override;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) override;

    virtual bool Prepare( const PrepareHint ePrep = PREP_CLEAR,
                          const void *pVoid = 0, bool bNotify = true ) override;

                 SwContentFrm *FindLastContent();
    inline const SwContentFrm *FindLastContent() const;

    const SwTable *GetTable() const { return m_pTable; }
          SwTable *GetTable()       { return m_pTable; }

    bool IsComplete()  { return m_bComplete; }
    void SetComplete() { m_bComplete = true; }
    void ResetComplete() { m_bComplete = false; }

    bool IsLowersFormatted() const      { return m_bLowersFormatted; }
    void SetLowersFormatted(bool b)   { m_bLowersFormatted = b;    }

    void SetCalcLowers()        { m_bCalcLowers = true;      } // use rarely
    void SetResizeHTMLTable()   { m_bResizeHTMLTable = true; } // same
    void SetONECalcLowers()     { m_bONECalcLowers = true;   }

    // Start: New stuff for breaking table rows

    bool HasFollowFlowLine() const { return m_bHasFollowFlowLine; }
    void SetFollowFlowLine(bool bNew) { m_bHasFollowFlowLine = bNew; }
    //return the SwTabFrm (if any) that this SwTabFrm is a follow flow line for
    SwTabFrm* GetFollowFlowLineFor();

    bool IsRebuildLastLine() const { return m_bIsRebuildLastLine; }
    void SetRebuildLastLine(bool bNew) { m_bIsRebuildLastLine = bNew; }

    bool IsRestrictTableGrowth() const { return m_bRestrictTableGrowth; }
    void SetRestrictTableGrowth( bool bNew ) { m_bRestrictTableGrowth = bNew; }

    bool IsRemoveFollowFlowLinePending() const { return m_bRemoveFollowFlowLinePending; }
    void SetRemoveFollowFlowLinePending(bool bNew) { m_bRemoveFollowFlowLinePending = bNew; }

    bool IsInRecalcLowerRow() const
    {
        return m_bInRecalcLowerRow;
    }
    void SetInRecalcLowerRow( bool bNew )
    {
        m_bInRecalcLowerRow = bNew;
    }

    // #i26945#
    bool IsConsiderObjsForMinCellHeight() const
    {
        return m_bConsiderObjsForMinCellHeight;
    }
    void SetConsiderObjsForMinCellHeight(bool const bConsiderObjsForMinCellHeight)
    {
        m_bConsiderObjsForMinCellHeight = bConsiderObjsForMinCellHeight;
    }

    // #i26945#
    bool DoesObjsFit() const
    {
        return m_bObjsDoesFit;
    }
    void SetDoesObjsFit(bool const bObjsDoesFit)
    {
        m_bObjsDoesFit = bObjsDoesFit;
    }

    bool RemoveFollowFlowLine();

    // End: New stuff for breaking table rows

    bool CalcFlyOffsets(
        SwTwips& rUpper,
        long& rLeftOffset,
        long& rRightOffset ) const;

    SwTwips CalcHeightOfFirstContentLine() const;

    bool IsInHeadline( const SwFrm& rFrm ) const;
    SwRowFrm* GetFirstNonHeadlineRow() const;

    bool IsLayoutSplitAllowed() const;

    // #i29550#
    bool IsCollapsingBorders() const;

    sal_uInt16 GetBottomLineSize() const;

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;

    DECL_FIXEDMEMPOOL_NEWDEL(SwTabFrm)
};

inline const SwContentFrm *SwTabFrm::FindLastContent() const
{
    return const_cast<SwTabFrm*>(this)->FindLastContent();
}

inline const SwTabFrm *SwTabFrm::GetFollow() const
{
    return static_cast<const SwTabFrm*>(SwFlowFrm::GetFollow());
}
inline SwTabFrm *SwTabFrm::GetFollow()
{
    return static_cast<SwTabFrm*>(SwFlowFrm::GetFollow());
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_TABFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
