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
WeakNodeContainer::WeakNodeContainer(SwNode* pNode)
    : m_pNode(pNode)
{
    if (m_pNode)
    {
        auto* pBroadcast = dynamic_cast<sw::BroadcastingModify*>(m_pNode);
        if (pBroadcast)
        {
            EndListeningAll();
            StartListening(pBroadcast->GetNotifier());
        }
        else
        {
            m_pNode = nullptr;
        }
    }
}

WeakNodeContainer::~WeakNodeContainer() { EndListeningAll(); }

bool WeakNodeContainer::isAlive()
{
    if (!m_pNode)
        return false;
    if (!HasBroadcaster())
        m_pNode = nullptr;
    return m_pNode;
}

SwNode* WeakNodeContainer::getNode()
{
    if (isAlive())
        return m_pNode;
    return nullptr;
}

OnlineAccessibilityCheck::OnlineAccessibilityCheck(SwDoc& rDocument)
    : m_rDocument(rDocument)
    , m_aAccessibilityCheck(&m_rDocument)
    , m_nPreviousNodeIndex(-1)
    , m_nAccessibilityIssues(0)
    , m_bInitialCheck(false)
    , m_bOnlineCheckStatus(
          !comphelper::IsFuzzing()
              ? officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get()
              : false)
{
}

void OnlineAccessibilityCheck::updateNodeStatus(SwNode* pNode, bool bIssueObjectNameChanged)
{
    if (!pNode->IsContentNode() && !pNode->IsTableNode())
        return;

    m_nAccessibilityIssues = 0;

    if (bIssueObjectNameChanged)
        return;

    auto it = m_aNodes.find(pNode);
    if (it == m_aNodes.end())
    {
        m_aNodes.emplace(pNode, std::make_unique<WeakNodeContainer>(pNode));
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
    if (SwDocShell* pShell = m_rDocument.GetDocShell())
    {
        if (SfxDispatcher* pDispatcher = pShell->GetDispatcher())
        {
            if (SfxBindings* pBindings = pDispatcher->GetBindings())
            {
                pBindings->Invalidate(FN_STAT_ACCESSIBILITY_CHECK);
            }
        }
    }
}

void OnlineAccessibilityCheck::runAccessibilityCheck(SwNode* pNode)
{
    m_aAccessibilityCheck.getIssueCollection().clear();

    m_aAccessibilityCheck.checkNode(pNode);

    for (SwFrameFormat* const& pFrameFormat : pNode->GetAnchoredFlys())
    {
        SdrObject* pObject = pFrameFormat->FindSdrObject();
        if (pObject)
            m_aAccessibilityCheck.checkObject(pNode, pObject);
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

void OnlineAccessibilityCheck::updateCheckerActivity()
{
    bool bOnlineCheckStatus
        = !comphelper::IsFuzzing()
          && officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();

    if (bOnlineCheckStatus != m_bOnlineCheckStatus)
    {
        m_pPreviousNode.reset();
        m_nPreviousNodeIndex = SwNodeOffset(-1);
        m_bInitialCheck = false; // force initial check

        if (!bOnlineCheckStatus)
        {
            clearAccessibilityIssuesFromAllNodes(); // cleanup all accessibility check data on nodes
            m_nAccessibilityIssues = -1;
        }
        else
        {
            m_nAccessibilityIssues = 0;
        }

        m_bOnlineCheckStatus = bOnlineCheckStatus;

        updateStatusbar();
    }
}

void OnlineAccessibilityCheck::update(const SwPosition& rNewPos)
{
    updateCheckerActivity();

    if (!m_bOnlineCheckStatus)
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

    auto pCurrentWeak = std::make_unique<WeakNodeContainer>(pCurrentNode);
    if (!pCurrentWeak->isAlive())
        return;

    // Check if previous node was deleted
    if (!m_pPreviousNode || !m_pPreviousNode->isAlive())
    {
        m_pPreviousNode = std::move(pCurrentWeak);
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
        m_pPreviousNode = std::move(pCurrentWeak);
        m_nPreviousNodeIndex = nCurrenNodeIndex;
        return;
    }

    // Run the document level Accessibility Check
    runDocumentLevelAccessibilityCheck();

    // Get the real previous node from index
    SwNode* pNode = pCurrentNode->GetNodes()[m_nPreviousNodeIndex];

    if (pNode && (pNode->IsContentNode() || pNode->IsTableNode()))
    {
        runAccessibilityCheck(pNode);
        updateNodeStatus(pNode);

        // Assign previous node and index
        m_pPreviousNode = std::move(pCurrentWeak);
        m_nPreviousNodeIndex = nCurrenNodeIndex;
    }
    else
    {
        runAccessibilityCheck(pCurrentNode);
        updateNodeStatus(pCurrentNode);

        m_pPreviousNode.reset();
        m_nPreviousNodeIndex = SwNodeOffset(-1);
    }

    updateStatusbar();
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

void OnlineAccessibilityCheck::resetAndQueue(SwNode* pNode, bool bIssueObjectNameChanged)
{
    if (comphelper::IsFuzzing())
        return;

    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();
    if (!bOnlineCheckStatus)
        return;

    pNode->getAccessibilityCheckStatus().reset();
    m_aNodes.erase(pNode);
    if (&pNode->GetNodes() == &m_rDocument.GetNodes()) // don't add undo array
    {
        runAccessibilityCheck(pNode);
        updateNodeStatus(pNode, bIssueObjectNameChanged);
    }
    updateStatusbar();
}

void OnlineAccessibilityCheck::resetAndQueueDocumentLevel()
{
    if (comphelper::IsFuzzing())
        return;

    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();
    if (!bOnlineCheckStatus)
        return;

    runDocumentLevelAccessibilityCheck();
    updateStatusbar();
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
