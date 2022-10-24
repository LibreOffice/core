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

#include <OnlineAccessibilityCheck.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <txtfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <docsh.hxx>
#include <cmdid.h>
#include <officecfg/Office/Common.hxx>

namespace sw
{
WeakContentNodeContainer::WeakContentNodeContainer(SwContentNode* pNode)
    : m_pNode(pNode)
{
    if (m_pNode)
    {
        EndListeningAll();
        StartListening(m_pNode->GetNotifier());
    }
}

WeakContentNodeContainer::~WeakContentNodeContainer() { EndListeningAll(); }

bool WeakContentNodeContainer::isAlive()
{
    if (!HasBroadcaster())
        m_pNode = nullptr;
    return m_pNode;
}

SwContentNode* WeakContentNodeContainer::getNode()
{
    if (isAlive())
        return m_pNode;
    return nullptr;
}

OnlineAccessibilityCheck::OnlineAccessibilityCheck(SwDoc& rDocument)
    : m_rDocument(rDocument)
    , m_aAccessibilityCheck(&m_rDocument)
    , m_pPreviousNode(nullptr)
    , m_nPreviousNodeIndex(-1)
    , m_nAccessibilityIssues(0)
{
}

void OnlineAccessibilityCheck::updateNodeStatus(SwContentNode* pContentNode)
{
    m_nAccessibilityIssues = 0;

    auto it = m_aNodes.find(pContentNode);
    if (it == m_aNodes.end())
    {
        m_aNodes.emplace(pContentNode, std::make_unique<WeakContentNodeContainer>(pContentNode));
    }

    for (auto iterator = m_aNodes.begin(); iterator != m_aNodes.end();)
    {
        auto& pWeakContentNode = iterator->second;
        if (pWeakContentNode->isAlive())
        {
            auto& rStatus = pWeakContentNode->getNode()->getAccessibilityCheckStatus();
            if (rStatus.pCollection)
            {
                m_nAccessibilityIssues += rStatus.pCollection->getIssues().size();
                ++iterator;
            }
            else
            {
                iterator = m_aNodes.erase(iterator);
            }
        }
        else
        {
            iterator = m_aNodes.erase(iterator);
        }
    }
}

void OnlineAccessibilityCheck::updateStatusbar()
{
    SfxBindings* pBindings = m_rDocument.GetDocShell() && m_rDocument.GetDocShell()->GetDispatcher()
                                 ? m_rDocument.GetDocShell()->GetDispatcher()->GetBindings()
                                 : nullptr;
    if (pBindings)
        pBindings->Invalidate(FN_STAT_ACCESSIBILITY_CHECK);
}

void OnlineAccessibilityCheck::runCheck(SwContentNode* pContentNode)
{
    m_aAccessibilityCheck.getIssueCollection().clear();

    m_aAccessibilityCheck.checkNode(pContentNode);

    for (SwFrameFormat* const& pFrameFormat : pContentNode->GetAnchoredFlys())
    {
        SdrObject* pObject = pFrameFormat->FindSdrObject();
        if (pObject)
            m_aAccessibilityCheck.checkObject(pObject);
    }

    auto aCollection = m_aAccessibilityCheck.getIssueCollection();

    pContentNode->getAccessibilityCheckStatus().pCollection
        = std::make_unique<sfx::AccessibilityIssueCollection>(aCollection);

    updateNodeStatus(pContentNode);
    updateStatusbar();
}

void OnlineAccessibilityCheck::update(const SwPosition& rNewPos)
{
    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();

    if (!bOnlineCheckStatus)
        return;

    auto nCurrenNodeIndex = rNewPos.GetNodeIndex();
    if (!rNewPos.GetNode().IsContentNode())
        return;

    auto* pCurrentNode = rNewPos.GetNode().GetContentNode();

    // Check if previous node was deleted
    if (!HasBroadcaster())
    {
        EndListeningAll();
        StartListening(pCurrentNode->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
        return;
    }

    // Check if node index changed
    if (nCurrenNodeIndex == m_nPreviousNodeIndex)
        return;

    // Check previous node is valid
    if (m_nPreviousNodeIndex < SwNodeOffset(0)
        || m_nPreviousNodeIndex >= rNewPos.GetNode().GetNodes().Count())
    {
        EndListeningAll();
        StartListening(pCurrentNode->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
        return;
    }

    // Get the real previous node from index
    SwNode* pNode = rNewPos.GetNode().GetNodes()[m_nPreviousNodeIndex];

    if (pNode && pNode->IsContentNode())
    {
        auto* pContentNode = pNode->GetContentNode();

        runCheck(pContentNode);

        // Assign previous node and index
        EndListeningAll();
        StartListening(pCurrentNode->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
    }
    else
    {
        m_pPreviousNode = nullptr;
        m_nPreviousNodeIndex = SwNodeOffset(-1);
    }
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
