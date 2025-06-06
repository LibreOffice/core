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

#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fmtanchr.hxx>
#include <ndindex.hxx>
#include <frameformats.hxx>
#include <svx/swframetypes.hxx>

bool SwPosFlyFrameCmp::operator()(const SwPosFlyFrame& rA, const SwPosFlyFrame& rB) const
{
    if (rA.GetNode() == rB.GetNode())
    {
        // In this case, the order number decides!
        return rA.GetOrdNum() < rB.GetOrdNum();
    }

    return rA.GetNode() < rB.GetNode();
}

SwPosFlyFrame::SwPosFlyFrame(const SwNode& rNd, const SwFrameFormat* pFormat, sal_uInt16 nArrPos)
    : m_pFrameFormat(pFormat)
    , m_pNode(&rNd)
    , m_nOrdNum(SAL_MAX_UINT32)
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if (RndStdIds::FLY_AT_PAGE == rAnchor.GetAnchorId())
        ;
    else if (pFormat->GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell())
        pFormat->CallSwClientNotify(sw::GetZOrderHint(m_nOrdNum));
    if (m_nOrdNum == SAL_MAX_UINT32)
    {
        m_nOrdNum = pFormat->GetDoc().GetSpzFrameFormats()->size();
        m_nOrdNum += nArrPos;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
