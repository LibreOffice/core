/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <unordered_set>
#include <vector>
#include "epptooxml.hxx"
#include "pptx-animations-cond.hxx"

namespace oox::core
{
class NodeContext;

typedef std::unique_ptr<NodeContext> NodeContextPtr;

class NodeContext
{
    const css::uno::Reference<css::animations::XAnimationNode> mxNode;

    std::vector<NodeContextPtr> maChildNodes;
    std::vector<Cond> maBeginCondList;
    std::vector<Cond> maEndCondList;
    // if the node has valid target or contains at least one valid target.
    bool mbValid;
    // Required to check if the associated shape is present in export or not
    const std::unordered_set<sal_Int32>& mrSlideShapeIDs;
    PowerPointExport& mrPowerPointExport;

    // if the node should be on SubTnLst or ChildTnLst
    bool mbOnSubTnLst;

    // Attributes initialized from mxNode->getUserData().
    sal_Int16 mnEffectNodeType;
    sal_Int16 mnEffectPresetClass;
    OUString msEffectPresetId;
    OUString msEffectPresetSubType;

    bool isValidTarget(const css::uno::Any& rTarget);

    /// constructor helper for initializing user data.
    void initUserData();

    /// constructor helper to initialize maChildNodes.
    /// return true if at least one childnode is valid.
    bool initChildNodes();

    /// constructor helper to initialize mbValid
    void initValid(bool bHasValidChild, bool bIsIterateChild);

public:
    NodeContext(const css::uno::Reference<css::animations::XAnimationNode>& xNode,
                const std::unordered_set<sal_Int32>& rSlideShapeIDs, PowerPointExport& rExport,
                bool bMainSeqChild, bool bIsIterateChild);
    const css::uno::Reference<css::animations::XAnimationNode>& getNode() const { return mxNode; }
    sal_Int16 getEffectNodeType() const { return mnEffectNodeType; }
    sal_Int16 getEffectPresetClass() const { return mnEffectPresetClass; }
    const OUString& getEffectPresetId() const { return msEffectPresetId; }
    const OUString& getEffectPresetSubType() const { return msEffectPresetSubType; }
    bool isValid() const { return mbValid; }
    bool isOnSubTnLst() const { return mbOnSubTnLst; }
    const std::vector<NodeContextPtr>& getChildNodes() const { return maChildNodes; };
    const css::uno::Reference<css::animations::XAnimationNode>& getNodeForCondition() const;
    const std::vector<Cond>& getBeginCondList() const { return maBeginCondList; }
    const std::vector<Cond>& getEndCondList() const { return maEndCondList; }
};
}
