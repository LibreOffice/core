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
#include <unotools/configmgr.hxx>

namespace sw
{
WeakContentNodeContainer::WeakContentNodeContainer(SwNode* pNode)
    : m_pNode(pNode)
{
    if (m_pNode)
    {
        auto* pBroadcast = dynamic_cast<sw::BroadcastingModify*>(pNode);
        if (pBroadcast)
        {
            EndListeningAll();
            StartListening(pBroadcast->GetNotifier());
        }
    }
}

WeakContentNodeContainer::~WeakContentNodeContainer() { EndListeningAll(); }

bool WeakContentNodeContainer::isAlive()
{
    if (!HasBroadcaster())
        m_pNode = nullptr;
    return m_pNode;
}

SwNode* WeakContentNodeContainer::getNode()
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
    , m_bInitialCheck(false)
    , m_bOnlineCheckStatus(
          !utl::ConfigManager::IsFuzzing()
              ? officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get()
              : false)
{
}

void OnlineAccessibilityCheck::updateNodeStatus(SwNode* pNode)
{
    if (!pNode->IsContentNode() && !pNode->IsTableNode())
        return;

    m_nAccessibilityIssues = 0;

    auto it = m_aNodes.find(pNode);
    if (it == m_aNodes.end())
    {
        m_aNodes.emplace(pNode, std::make_unique<WeakContentNodeContainer>(pNode));
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

void OnlineAccessibilityCheck::runAccessibilityCheck(SwNode* pNode)
{
    m_aAccessibilityCheck.getIssueCollection().clear();

    m_aAccessibilityCheck.checkNode(pNode);

    for (SwFrameFormat* const& pFrameFormat : pNode->GetAnchoredFlys())
    {
        SdrObject* pObject = pFrameFormat->FindSdrObject();
        if (pObject)
            m_aAccessibilityCheck.checkObject(pObject);
    }

    auto aCollection = m_aAccessibilityCheck.getIssueCollection();

    pNode->getAccessibilityCheckStatus().pCollection
        = std::make_unique<sfx::AccessibilityIssueCollection>(aCollection);
}

void OnlineAccessibilityCheck::runDocumentLevelAccessibilityCheck()
{
    m_aAccessibilityCheck.getIssueCollection().clear();
    m_aAccessibilityCheck.checkDocumentProperties();
    auto aCollection = m_aAccessibilityCheck.getIssueCollection();
    m_pDocumentAccessibilityIssues
        = std::make_unique<sfx::AccessibilityIssueCollection>(aCollection);
}

void OnlineAccessibilityCheck::initialCheck()
{
    if (m_bInitialCheck)
        return;

    runDocumentLevelAccessibilityCheck();

    auto const& pNodes = m_rDocument.GetNodes();
    for (SwNodeOffset n(0); n < pNodes.Count(); ++n)
    {
        SwNode* pNode = pNodes[n];
        if (pNode)
        {
            runAccessibilityCheck(pNode);
            updateNodeStatus(pNode);
        }
    }

    updateStatusbar();

    m_bInitialCheck = true;
}

void OnlineAccessibilityCheck::update(const SwPosition& rNewPos)
{
    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();

    if (bOnlineCheckStatus != m_bOnlineCheckStatus)
    {
        EndListeningAll();
        m_pPreviousNode = nullptr;
        m_nPreviousNodeIndex = SwNodeOffset(-1);
        m_bInitialCheck = false; // force initial check

        if (!bOnlineCheckStatus)
        {
            clearAccessibilityIssuesFromAllNodes(); // cleanup all accessibility check data on nodes
            updateStatusbar();
        }

        m_bOnlineCheckStatus = bOnlineCheckStatus;
    }

    if (!bOnlineCheckStatus)
        return;

    initialCheck();

    lookForPreviousNodeAndUpdate(rNewPos);
}

void OnlineAccessibilityCheck::lookForPreviousNodeAndUpdate(const SwPosition& rNewPos)
{
    auto nCurrenNodeIndex = rNewPos.GetNodeIndex();
    auto* pCurrentNode = &rNewPos.GetNode();

    if (!pCurrentNode->IsContentNode() && !pCurrentNode->IsTableNode())
        return;

    auto* pCurrentBroadcast = dynamic_cast<sw::BroadcastingModify*>(pCurrentNode);
    if (!pCurrentBroadcast)
        return;

    // Check if previous node was deleted
    if (!HasBroadcaster())
    {
        EndListeningAll();
        StartListening(pCurrentBroadcast->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
        return;
    }

    // Check if node index changed
    if (nCurrenNodeIndex == m_nPreviousNodeIndex)
        return;

    // Check if previous node is valid
    if (m_nPreviousNodeIndex < SwNodeOffset(0)
        || m_nPreviousNodeIndex >= pCurrentNode->GetNodes().Count())
    {
        EndListeningAll();
        StartListening(pCurrentBroadcast->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
        return;
    }

    // Get the real previous node from index
    SwNode* pNode = pCurrentNode->GetNodes()[m_nPreviousNodeIndex];

    if (pNode && (pNode->IsContentNode() || pNode->IsTableNode()))
    {
        runAccessibilityCheck(pNode);
        updateNodeStatus(pNode);
        updateStatusbar();

        // Assign previous node and index
        EndListeningAll();
        StartListening(pCurrentBroadcast->GetNotifier());
        m_pPreviousNode = pCurrentNode;
        m_nPreviousNodeIndex = nCurrenNodeIndex;
    }
    else
    {
        m_pPreviousNode = nullptr;
        m_nPreviousNodeIndex = SwNodeOffset(-1);
    }
}

void OnlineAccessibilityCheck::clearAccessibilityIssuesFromAllNodes()
{
    auto const& pNodes = m_rDocument.GetNodes();
    for (SwNodeOffset n(0); n < pNodes.Count(); ++n)
    {
        SwNode* pNode = pNodes[n];
        if (pNode)
        {
            pNode->getAccessibilityCheckStatus().reset();
        }
    }

    m_aNodes.clear();
    updateStatusbar();
}

void OnlineAccessibilityCheck::resetAndQueue(SwNode* pNode)
{
    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();
    if (!bOnlineCheckStatus)
        return;

    pNode->getAccessibilityCheckStatus().reset();
    m_aNodes.erase(pNode);
    runAccessibilityCheck(pNode);
    updateNodeStatus(pNode);
    updateStatusbar();
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
