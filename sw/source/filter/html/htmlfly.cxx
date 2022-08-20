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

#include "htmlfly.hxx"

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <flypos.hxx>

#include <frmfmt.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <osl/diagnose.h>

using namespace css;

SwHTMLPosFlyFrame::SwHTMLPosFlyFrame( const SwPosFlyFrame& rPosFly,
                                  const SdrObject *pSdrObj,
                                  AllHtmlFlags nFlags ) :
    m_pFrameFormat( &rPosFly.GetFormat() ),
    m_pSdrObject( pSdrObj ),
    m_aNodeIndex( rPosFly.GetNdIndex() ),
    m_nOrdNum( rPosFly.GetOrdNum() ),
    m_nContentIndex( 0 ),
    m_nAllFlags( nFlags )
{
    const SwFormatAnchor& rAnchor = rPosFly.GetFormat().GetAnchor();
    if ((RndStdIds::FLY_AT_CHAR != rAnchor.GetAnchorId()) ||
        HtmlPosition::Inside != GetOutPos())
        return;

    // Output of auto-bound frames will be a character farther back,
    // because then the position aligns with Netscape.
    OSL_ENSURE( rAnchor.GetContentAnchor(), "No anchor position?" );
    if( !rAnchor.GetContentAnchor() )
        return;

    m_nContentIndex = rAnchor.GetContentAnchor()->GetContentIndex();
    sal_Int16 eHoriRel = rPosFly.GetFormat().GetHoriOrient().
                                        GetRelationOrient();
    if( text::RelOrientation::FRAME == eHoriRel || text::RelOrientation::PRINT_AREA == eHoriRel )
    {
        const SwContentNode *pCNd = m_aNodeIndex.GetNode().GetContentNode();
        OSL_ENSURE( pCNd, "No Content-Node at PaM position" );
        if( pCNd && m_nContentIndex < pCNd->Len() )
            m_nContentIndex++;
    }
}

bool SwHTMLPosFlyFrame::operator<( const SwHTMLPosFlyFrame& rFrame ) const
{
    if( m_aNodeIndex.GetNode() == rFrame.m_aNodeIndex.GetNode() )
    {
        if( m_nContentIndex == rFrame.m_nContentIndex )
        {
            if( GetOutPos() == rFrame.GetOutPos() )
                return m_nOrdNum < rFrame.m_nOrdNum;
            else
                return GetOutPos() < rFrame.GetOutPos();
        }
        else
            return m_nContentIndex < rFrame.m_nContentIndex;
    }
    else
        return m_aNodeIndex.GetIndex() < rFrame.m_aNodeIndex.GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
