/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "ndindex.hxx"
#include "ndtxt.hxx"
#include <svl/listener.hxx>
#include <vcl/timer.hxx>
#include <AccessibilityCheck.hxx>
#include <map>

struct SwPosition;
class SwTextNode;

namespace sw
{
/// Contains the node and tracks if the node gets deleted.
/// Note: the node needs to extend sw::BroadcastingModify.
class WeakNodeContainer : public SvtListener
{
private:
    SwNode* m_pNode;

public:
    WeakNodeContainer(SwNode* pNode);
    ~WeakNodeContainer();

    /// Is the node still alive or it was deleted?
    bool isAlive();

    /// Returns the pointer of the node or nullptr if the node
    /// got deleted.
    SwNode* getNode();
};

class OnlineAccessibilityCheck : public SvtListener
{
private:
    std::map<SwNode*, std::unique_ptr<WeakNodeContainer>> m_aNodes;

    SwDoc& m_rDocument;
    sw::AccessibilityCheck m_aAccessibilityCheck;
    SwNode* m_pPreviousNode;
    SwNodeOffset m_nPreviousNodeIndex;
    sal_Int32 m_nAccessibilityIssues;
    bool m_bInitialCheck;
    bool m_bOnlineCheckStatus;
    std::unique_ptr<sfx::AccessibilityIssueCollection> m_pDocumentAccessibilityIssues;

    void runAccessibilityCheck(SwNode* pNode);
    void updateStatusbar();
    void updateNodeStatus(SwNode* pContentNode);
    void initialCheck();
    void lookForPreviousNodeAndUpdate(SwPosition const& rNewPos);
    void clearAccessibilityIssuesFromAllNodes();
    void runDocumentLevelAccessibilityCheck();

public:
    OnlineAccessibilityCheck(SwDoc& rDocument);
    void update(SwPosition const& rNewPos);
    void resetAndQueue(SwNode* pNode);
    void updateCheckerActivity();
    sal_Int32 getNumberOfAccessibilityIssues() { return m_nAccessibilityIssues; }
};

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
