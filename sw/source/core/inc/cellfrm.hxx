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

#include <tools/mempool.hxx>
#include "layfrm.hxx"

class SwTableBox;
struct SwCrsrMoveState;
class SwBorderAttrs;

/// SwCellFrm is one table cell in the document layout.
class SwCellFrm: public SwLayoutFrm
{
    const SwTableBox* m_pTabBox;

    virtual void DestroyImpl() override;
    virtual ~SwCellFrm();

protected:
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 ) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    SwCellFrm( const SwTableBox &, SwFrm*, bool bInsertContent = true );

    virtual bool GetCrsrOfst( SwPosition *, Point&, SwCrsrMoveState* = 0, bool bTestBackground = false ) const override;
    virtual void Paint( vcl::RenderContext& rRenderContext, SwRect const&,
                        SwPrintData const*const pPrintData = NULL ) const override;
    virtual void CheckDirection( bool bVert ) override;

    // #i103961#
    virtual void Cut() override;

    const SwTableBox *GetTabBox() const { return m_pTabBox; }

    // used for breaking table rows:
    SwCellFrm* GetFollowCell() const;
    SwCellFrm* GetPreviousCell() const;

    // used for rowspan stuff:
    const SwCellFrm& FindStartEndOfRowSpanCell( bool bStart, bool bCurrentTab ) const;
    long GetLayoutRowSpan() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwCellFrm)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
