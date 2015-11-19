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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_ROWFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ROWFRM_HXX

#include <tools/mempool.hxx>

#include "layfrm.hxx"

class SwTableLine;
class SwBorderAttrs;

/// SwRowFrame is one table row in the document layout.
class SwRowFrame: public SwLayoutFrame
{
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    /// Only change the Frame size, not the PrtArea SSize
    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) override;

    const SwTableLine * m_pTabLine;
    SwRowFrame * m_pFollowRow;
    // #i29550#
    sal_uInt16 mnTopMarginForLowers;
    sal_uInt16 mnBottomMarginForLowers;
    sal_uInt16 mnBottomLineSize;
    // <-- collapsing
    bool m_bIsFollowFlowRow;
    bool m_bIsRepeatedHeadline;
    bool m_bIsRowSpanLine;

    virtual void DestroyImpl() override;
    virtual ~SwRowFrame();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    SwRowFrame( const SwTableLine &, SwFrame*, bool bInsertContent = true );

    virtual void Cut() override;

    /**
     * Register Flys after a line was created _AND_ inserted
     * Must be called by the creator; the Fly is inserted _after_ it has
     * been created; the same holds true for the Page at which the Flys
     * are to be registered at.
     */
    void RegistFlys( SwPageFrame *pPage = nullptr );

    const SwTableLine *GetTabLine() const { return m_pTabLine; }

    /**
     * Adapts the Cells to the current height; invalidates the Cells if
     * the Direction does not match the height
     */
    void AdjustCells( const SwTwips nHeight, const bool bHeight );

    SwRowFrame* GetFollowRow() const { return m_pFollowRow; }
    void SetFollowRow( SwRowFrame* pNew ) { m_pFollowRow = pNew; }

    // #i29550#
    sal_uInt16 GetTopMarginForLowers() const { return mnTopMarginForLowers; }
    void   SetTopMarginForLowers( sal_uInt16 nNew ) { mnTopMarginForLowers = nNew; }
    sal_uInt16 GetBottomMarginForLowers() const { return mnBottomMarginForLowers; }
    void   SetBottomMarginForLowers( sal_uInt16 nNew ) { mnBottomMarginForLowers = nNew; }
    sal_uInt16 GetBottomLineSize() const { return mnBottomLineSize; }
    void   SetBottomLineSize( sal_uInt16 nNew ) { mnBottomLineSize = nNew; }
    // <-- collapsing

    bool IsRepeatedHeadline() const { return m_bIsRepeatedHeadline; }
    void SetRepeatedHeadline( bool bNew ) { m_bIsRepeatedHeadline = bNew; }

    // --> split table rows
    bool IsRowSplitAllowed() const;
    bool IsFollowFlowRow() const { return m_bIsFollowFlowRow; }
    void SetFollowFlowRow( bool bNew ) { m_bIsFollowFlowRow = bNew; }
    // <-- split table rows

    // #131283# Table row keep feature
    bool ShouldRowKeepWithNext() const;

    // #i4032# NEW TABLES
    bool IsRowSpanLine() const { return m_bIsRowSpanLine; }
    void SetRowSpanLine( bool bNew ) { m_bIsRowSpanLine = bNew; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwRowFrame)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
