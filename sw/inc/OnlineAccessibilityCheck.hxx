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
/// Contains the content node and tracks if the node
/// gets deleted.
class WeakContentNodeContainer : public SvtListener
{
private:
    SwContentNode* m_pNode;

public:
    WeakContentNodeContainer(SwContentNode* pNode);
    ~WeakContentNodeContainer();

    /// Is the node still alive or it was deleted?
    bool isAlive();

    /// Returns the pointer of the content node or nullptr if the node
    /// got deleted.
    SwContentNode* getNode();
};

class OnlineAccessibilityCheck : public SvtListener
{
private:
    std::map<SwContentNode*, std::unique_ptr<WeakContentNodeContainer>> m_aNodes;

    SwDoc& m_rDocument;
    sw::AccessibilityCheck m_aAccessibilityCheck;
    SwContentNode* m_pPreviousNode;
    SwNodeOffset m_nPreviousNodeIndex;
    sal_Int32 m_nAccessibilityIssues;

    void runCheck(SwContentNode* pNode);
    void updateStatusbar();
    void updateNodeStatus(SwContentNode* pContentNode);

public:
    OnlineAccessibilityCheck(SwDoc& rDocument);
    void update(const SwPosition& rNewPos);
    sal_Int32 getNumberOfAccessibilityIssues() { return m_nAccessibilityIssues; }
};

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
