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
OnlineAccessibilityCheck::OnlineAccessibilityCheck(SwDoc& rDocument)
    : m_rDocument(rDocument)
    , m_aAccessibilityCheck(&m_rDocument)
    , m_pCurrentTextNode(nullptr)
    , m_aCurrentNodeIndex(-1)
    , m_nAccessibilityIssues(0)
{
}

void OnlineAccessibilityCheck::runCheck(SwTextNode* pTextNode)
{
    m_aAccessibilityCheck.checkNode(pTextNode);

    for (SwFrameFormat* const& pFrameFormat : pTextNode->GetAnchoredFlys())
    {
        SdrObject* pObject = pFrameFormat->FindSdrObject();
        if (pObject)
            m_aAccessibilityCheck.checkObject(pObject);
    }

    auto aCollection = m_aAccessibilityCheck.getIssueCollection();

    pTextNode->getAccessibilityCheckStatus().pCollection
        = std::make_unique<sfx::AccessibilityIssueCollection>(aCollection);

    m_nAccessibilityIssues = 0;
    auto const& pNodes = m_rDocument.GetNodes();
    for (SwNodeOffset n(0); n < pNodes.Count(); ++n)
    {
        SwNode* pNode = pNodes[n];
        if (pNode && pNode->IsTextNode())
        {
            auto* pCurrentTextNode = pNode->GetTextNode();
            auto& rStatus = pCurrentTextNode->getAccessibilityCheckStatus();
            if (rStatus.pCollection)
                m_nAccessibilityIssues += rStatus.pCollection->getIssues().size();
        }
    }

    SfxBindings* pBindings = m_rDocument.GetDocShell() && m_rDocument.GetDocShell()->GetDispatcher()
                                 ? m_rDocument.GetDocShell()->GetDispatcher()->GetBindings()
                                 : nullptr;
    if (pBindings)
        pBindings->Invalidate(FN_STAT_ACCESSIBILITY_CHECK);
}

void OnlineAccessibilityCheck::update(const SwPosition& rNewPos)
{
    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();

    if (!bOnlineCheckStatus)
        return;

    if (!HasBroadcaster())
    {
        m_pCurrentTextNode = nullptr;
        m_aCurrentNodeIndex = SwNodeOffset(-1);
    }

    auto aNodeIndex = rNewPos.GetNodeIndex();

    m_aAccessibilityCheck.getIssueCollection().clear();

    SwTextNode* pTextNode = rNewPos.GetNode().GetTextNode();
    if (!pTextNode)
    {
        m_pCurrentTextNode = nullptr;
        m_aCurrentNodeIndex = SwNodeOffset(-1);
        return;
    }

    if (pTextNode == m_pCurrentTextNode)
    {
        if (m_aCurrentNodeIndex != aNodeIndex && m_aCurrentNodeIndex >= SwNodeOffset(0)
            && m_aCurrentNodeIndex < pTextNode->GetNodes().Count())
        {
            pTextNode = pTextNode->GetNodes()[m_aCurrentNodeIndex]->GetTextNode();

            if (pTextNode)
            {
                runCheck(pTextNode);
            }
        }
    }
    else if (m_pCurrentTextNode)
    {
        runCheck(m_pCurrentTextNode);
    }

    m_aCurrentNodeIndex = aNodeIndex;

    if (pTextNode && m_pCurrentTextNode != pTextNode)
    {
        EndListeningAll();
        StartListening(pTextNode->GetNotifier());
        m_pCurrentTextNode = pTextNode;
    }
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
