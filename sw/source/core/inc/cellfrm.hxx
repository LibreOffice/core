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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_CELLFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_CELLFRM_HXX

#include "layfrm.hxx"

class SwTableBox;
struct SwCursorMoveState;
class SwBorderAttrs;

/// SwCellFrame is one table cell in the document layout.
class SW_DLLPUBLIC SwCellFrame final : public SwLayoutFrame
{
    const SwTableBox* m_pTabBox;

    virtual void DestroyImpl() override;
    virtual ~SwCellFrame() override;

    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
    virtual const SwCellFrame* DynCastCellFrame() const override { return this; }

public:
    SwCellFrame( const SwTableBox &, SwFrame*, bool bInsertContent );

    virtual bool GetModelPositionForViewPoint( SwPosition *, Point&, SwCursorMoveState* = nullptr, bool bTestBackground = false ) const override;
    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&, PaintFrameMode mode = PAINT_ALL,
                        SwPrintData const*const pPrintData = nullptr ) const override;
    virtual void CheckDirection( bool bVert ) override;

    // #i103961#
    virtual void Cut() override;

    const SwTableBox *GetTabBox() const { return m_pTabBox; }

    // used for breaking table rows:
    SwCellFrame* GetFollowCell() const;
    SwCellFrame* GetPreviousCell() const;

    virtual bool IsLeaveUpperAllowed() const override;
    virtual bool IsCoveredCell() const override;

    // used for rowspan stuff:
    const SwCellFrame& FindStartEndOfRowSpanCell( bool bStart ) const;
    tools::Long GetLayoutRowSpan() const;

    /// If this is a vertically merged cell, then looks up its covered cell in rRow.
    const SwCellFrame* GetCoveredCellInRow(const SwRowFrame& rRow) const;

    /// If this is a vertically merged cell, then looks up its covered cells.
    std::vector<const SwCellFrame*> GetCoveredCells() const;

    void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;

    void dumpAsXml(xmlTextWriterPtr writer = nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
