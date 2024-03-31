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

#include "layoutatomvisitorbase.hxx"

#include <sal/log.hxx>

using namespace ::com::sun::star;

namespace oox::drawingml {

void LayoutAtomVisitorBase::defaultVisit(LayoutAtom const& rAtom)
{
    for (const auto& pAtom : rAtom.getChildren())
        pAtom->accept(*this);
}

void LayoutAtomVisitorBase::visit(ChooseAtom& rAtom)
{
    for (const auto& pChild : rAtom.getChildren())
    {
        const ConditionAtomPtr pCond = std::dynamic_pointer_cast<ConditionAtom>(pChild);
        if (pCond && pCond->getDecision(mpCurrentNode))
        {
            SAL_INFO("oox.drawingml", "Entering if node: " << pCond->getName());
            pCond->accept(*this);
            break;
        }
    }
}

void LayoutAtomVisitorBase::visit(ConditionAtom& rAtom)
{
    defaultVisit(rAtom);
}

void LayoutAtomVisitorBase::visit(ForEachAtom& rAtom)
{
    if (!rAtom.getRef().isEmpty())
    {
        if (LayoutAtomPtr pRefAtom = rAtom.getRefAtom())
            pRefAtom->accept(*this);
        return;
    }

    if (rAtom.iterator().mbHideLastTrans && !rAtom.iterator().maAxis.empty() && rAtom.iterator().maAxis[0] == XML_followSib)
    {
        // If last transition is hidden and the axis is the follow sibling,
        // then the last atom should not be visited.
        if (mnCurrIdx + mnCurrStep >= mnCurrCnt)
            return;
    }

    sal_Int32 nChildren = 1;
    // Approximate the non-assistant type with the node type.
    if (rAtom.iterator().mnPtType == XML_node || rAtom.iterator().mnPtType == XML_nonAsst)
    {
        // count child data nodes - check all child Atoms for "name"
        // attribute that is contained in diagram's
        // getPointsPresNameMap()
        ShallowPresNameVisitor aVisitor(mrDgm, mpCurrentNode);
        for (const auto& pAtom : rAtom.getChildren())
            pAtom->accept(aVisitor);
        nChildren = aVisitor.getCount();
    }

    const sal_Int32 nCnt = std::min(
        nChildren,
        rAtom.iterator().mnCnt==-1 ? nChildren : rAtom.iterator().mnCnt);

    const sal_Int32 nOldIdx = mnCurrIdx;
    const sal_Int32 nOldStep = mnCurrStep;
    const sal_Int32 nOldCnt = mnCurrCnt;
    const sal_Int32 nStep = rAtom.iterator().mnStep;
    mnCurrStep = nStep;
    mnCurrCnt = nCnt;
    for( mnCurrIdx=0; mnCurrIdx<nCnt && nStep>0; mnCurrIdx+=nStep )
    {
        // TODO there is likely some conditions
        for (const auto& pAtom : rAtom.getChildren())
            pAtom->accept(*this);
    }

    // and restore idx
    mnCurrIdx = nOldIdx;
    mnCurrStep = nOldStep;
    mnCurrCnt = nOldCnt;
}

void LayoutAtomVisitorBase::visit(LayoutNode& rAtom)
{
    // TODO: deduplicate code in descendants

    // stop processing if it's not a child of previous LayoutNode

    const DiagramData::PointsNameMap::const_iterator aDataNode
        = mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if (aDataNode == mrDgm.getData()->getPointsPresNameMap().end()
        || mnCurrIdx >= static_cast<sal_Int32>(aDataNode->second.size()))
        return;

    const svx::diagram::Point* pNewNode = aDataNode->second.at(mnCurrIdx);
    if (!mpCurrentNode || !pNewNode)
        return;

    bool bIsChild = false;
    for (const auto& aConnection : mrDgm.getData()->getConnections())
        if (aConnection.msSourceId == mpCurrentNode->msModelId
            && aConnection.msDestId == pNewNode->msModelId)
            bIsChild = true;

    if (!bIsChild)
        return;

    const svx::diagram::Point* pPreviousNode = mpCurrentNode;
    mpCurrentNode = pNewNode;

    defaultVisit(rAtom);

    mpCurrentNode = pPreviousNode;
}

void ShallowPresNameVisitor::visit(ConstraintAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(RuleAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(AlgAtom& /*rAtom*/)
{
    // stop processing
}

void ShallowPresNameVisitor::visit(ForEachAtom& rAtom)
{
    defaultVisit(rAtom);
}

void ShallowPresNameVisitor::visit(LayoutNode& rAtom)
{
    DiagramData::PointsNameMap::const_iterator aDataNode =
        mrDgm.getData()->getPointsPresNameMap().find(rAtom.getName());
    if( aDataNode != mrDgm.getData()->getPointsPresNameMap().end() )
        mnCnt = std::max(mnCnt,
                         aDataNode->second.size());
}

void ShallowPresNameVisitor::visit(ShapeAtom& /*rAtom*/)
{
    // stop processing
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
