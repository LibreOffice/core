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

#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "dflyobj.hxx"
#include <calbck.hxx>

bool SwPosFlyFrameCmp::operator()(const SwPosFlyFramePtr& rA, const SwPosFlyFramePtr& rB) const
{
    if(rA->GetNdIndex() == rB->GetNdIndex())
    {
        // In this case, the order number decides!
        return rA->GetOrdNum() < rB->GetOrdNum();
    }

    return rA->GetNdIndex() < rB->GetNdIndex();
}

SwPosFlyFrame::SwPosFlyFrame( const SwNodeIndex& rIdx, const SwFrameFormat* pFormat,
                            sal_uInt16 nArrPos )
    : m_pFrameFormat( pFormat ), m_pNodeIndex( const_cast<SwNodeIndex*>(&rIdx) )
{
    bool bFnd = false;
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if (FLY_AT_PAGE == rAnchor.GetAnchorId())
    {
        m_pNodeIndex = new SwNodeIndex( rIdx );
    }
    else if( pFormat->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
        if( RES_FLYFRMFMT == pFormat->Which() )
        {
            // Let's see if we have an SdrObject for this
            SwFlyFrame* pFly = SwIterator<SwFlyFrame,SwFormat>(*pFormat).First();
            if( pFly )
            {
                m_nOrdNum = pFly->GetVirtDrawObj()->GetOrdNum();
                bFnd = true;
            }
        }
        else if( RES_DRAWFRMFMT == pFormat->Which() )
        {
            // Let's see if we have an SdrObject for this
            SwDrawContact* pContact = SwIterator<SwDrawContact,SwFormat>(*pFormat).First();
            if( pContact )
            {
                m_nOrdNum = pContact->GetMaster()->GetOrdNum();
                bFnd = true;
            }
        }
    }

    if( !bFnd )
    {
        m_nOrdNum = pFormat->GetDoc()->GetSpzFrameFormats()->size();
        m_nOrdNum += nArrPos;
    }
}

SwPosFlyFrame::~SwPosFlyFrame()
{
    const SwFormatAnchor& rAnchor = m_pFrameFormat->GetAnchor();
    if (FLY_AT_PAGE == rAnchor.GetAnchorId())
    {
        delete m_pNodeIndex;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
