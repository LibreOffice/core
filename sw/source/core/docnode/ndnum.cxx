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

#include <node.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <osl/diagnose.h>

bool CompareSwOutlineNodes::operator()( SwNode* const& lhs, SwNode* const& rhs) const
{
    return lhs->GetIndex() < rhs->GetIndex();
}

bool SwOutlineNodes::Seek_Entry(SwNode* rP, size_type* pnPos) const
{
    const_iterator it = lower_bound(rP);
    *pnPos = it - begin();
    return it != end() && rP->GetIndex() == (*it)->GetIndex();
}

void SwNodes::UpdateOutlineNode(SwNode & rNd)
{
    assert(IsDocNodes()); // no point in m_pOutlineNodes for undo nodes

    SwTextNode * pTextNd = rNd.GetTextNode();

    if (!pTextNd || !pTextNd->IsOutlineStateChanged())
        return;

    bool bFound = m_aOutlineNodes.find(pTextNd) != m_aOutlineNodes.end();

    if (pTextNd->IsOutline())
    {
        if (! bFound)
        {
            // assure that text is in the correct nodes array
            if ( &(pTextNd->GetNodes()) == this )
            {
                m_aOutlineNodes.insert(pTextNd);
            }
            else
            {
                OSL_FAIL( "<SwNodes::UpdateOutlineNode(..)> - given text node isn't in the correct nodes array. This is a serious defect" );
            }
        }
    }
    else
    {
        if (bFound)
            m_aOutlineNodes.erase(pTextNd);
    }

    pTextNd->UpdateOutlineState();

    // update the structure fields
    GetDoc().getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Chapter )->UpdateFields();
}

void SwNodes::UpdateOutlineIdx( const SwNode& rNd )
{
    if( m_aOutlineNodes.empty() )     // no OutlineNodes present ?
        return;

    SwNode* const pSrch = const_cast<SwNode*>(&rNd);

    SwOutlineNodes::size_type nPos;
    if (!m_aOutlineNodes.Seek_Entry(pSrch, &nPos))
        return;
    if( nPos == m_aOutlineNodes.size() )      // none present for updating ?
        return;

    if( nPos )
        --nPos;

    if( !GetDoc().IsInDtor() && IsDocNodes() )
        UpdateOutlineNode( *m_aOutlineNodes[ nPos ]);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
