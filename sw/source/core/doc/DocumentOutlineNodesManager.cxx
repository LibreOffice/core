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
#include <DocumentOutlineNodesManager.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <modeltoviewhelper.hxx>

namespace sw
{

DocumentOutlineNodesManager::DocumentOutlineNodesManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

IDocumentOutlineNodes::tSortedOutlineNodeList::size_type DocumentOutlineNodesManager::getOutlineNodesCount() const
{
    return m_rDoc.GetNodes().GetOutLineNds().size();
}

int DocumentOutlineNodesManager::getOutlineLevel( const tSortedOutlineNodeList::size_type nIdx ) const
{
    return m_rDoc.GetNodes().GetOutLineNds()[ nIdx ]->
                                GetTextNode()->GetAttrOutlineLevel()-1;
}

OUString GetExpandTextMerged(SwRootFrame const*const pLayout,
        SwTextNode const& rNode, bool const bWithNumber,
        bool const bWithSpacesForLevel, ExpandMode const i_mode)
{
    if (pLayout && pLayout->IsHideRedlines())
    {
        SwTextFrame const*const pFrame(static_cast<SwTextFrame*>(rNode.getLayoutFrame(pLayout)));
        if (pFrame)
        {
            sw::MergedPara const*const pMerged = pFrame->GetMergedPara();
            if (pMerged)
            {
                if (&rNode != pMerged->pParaPropsNode)
                {
                    return OUString();
                }
                else
                {
                    ExpandMode const mode(ExpandMode::HideDeletions | i_mode);
                    OUStringBuffer ret(rNode.GetExpandText(pLayout, 0, -1,
                        bWithNumber, bWithNumber, bWithSpacesForLevel, mode));
                    for (sal_uLong i = rNode.GetIndex() + 1;
                         i <= pMerged->pLastNode->GetIndex(); ++i)
                    {
                        SwNode *const pTmp(rNode.GetNodes()[i]);
                        if (pTmp->GetRedlineMergeFlag() == SwNode::Merge::NonFirst)
                        {
                            ret.append(pTmp->GetTextNode()->GetExpandText(
                                pLayout, 0, -1, false, false, false, mode));
                        }
                    }
                    return ret.makeStringAndClear();
                }
            }
        }
    }
    return rNode.GetExpandText(pLayout, 0, -1, bWithNumber,
                    bWithNumber, bWithSpacesForLevel, i_mode);
}

OUString DocumentOutlineNodesManager::getOutlineText(
                              const tSortedOutlineNodeList::size_type nIdx,
                              SwRootFrame const*const pLayout,
                              const bool bWithNumber,
                              const bool bWithSpacesForLevel,
                              const bool bWithFootnote ) const
{
    SwTextNode const*const pNode(m_rDoc.GetNodes().GetOutLineNds()[ nIdx ]->GetTextNode());
    return GetExpandTextMerged(pLayout, *pNode,
            bWithNumber, bWithSpacesForLevel,
            (bWithFootnote ? ExpandMode::ExpandFootnote : ExpandMode(0)));
}

SwTextNode* DocumentOutlineNodesManager::getOutlineNode( const tSortedOutlineNodeList::size_type nIdx ) const
{
    return m_rDoc.GetNodes().GetOutLineNds()[ nIdx ]->GetTextNode();
}

bool DocumentOutlineNodesManager::isOutlineInLayout(
        const tSortedOutlineNodeList::size_type nIdx,
        SwRootFrame const& rLayout) const
{
    auto const pNode(m_rDoc.GetNodes().GetOutLineNds()[ nIdx ]->GetTextNode());
    return sw::IsParaPropsNode(rLayout, *pNode);
}

void DocumentOutlineNodesManager::getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const
{
    orOutlineNodeList.clear();
    orOutlineNodeList.reserve( getOutlineNodesCount() );

    const tSortedOutlineNodeList::size_type nOutlCount = getOutlineNodesCount();
    for ( tSortedOutlineNodeList::size_type i = 0; i < nOutlCount; ++i )
    {
        orOutlineNodeList.push_back(
            m_rDoc.GetNodes().GetOutLineNds()[i]->GetTextNode() );
    }
}

DocumentOutlineNodesManager::~DocumentOutlineNodesManager()
{
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
